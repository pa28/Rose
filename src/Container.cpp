//
// Created by richard on 2020-11-16.
//

#include <algorithm>
#include "Container.h"
#include "Math.h"
#include "Rose.h"

namespace rose {
    using namespace rose::util;

    Container::Container() {
        mClassName = "Container";
    }

    std::shared_ptr<Widget> Container::findWidget(const Position &pos) {
        auto relPos = pos - mLayoutHints.mAssignedRect->getPosition();
        for (auto &child : ReverseContainerView(mChildren)) {
            auto childHints = child->layoutHints();
            if (childHints.mAssignedRect->contains(relPos)) {
                if (auto container = child->as<Container>(); container)
                    return container->findWidget(relPos);
                else
                    return child;
            }
        }

        return getWidget();
    }

    std::shared_ptr<Widget> Container::findWidget(const Id &id) {
        for (auto &child : mChildren) {
            if (child->mId == id)
                return child;
            if (auto container = child->as<Container>(); container)
                if (auto widget = container->findWidget(id); widget)
                    return widget;
        }
        return nullptr;
    }

    bool Container::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        for (const auto &child : ReverseContainerView(mChildren)) {
            auto childHints = child->layoutHints();
            if (child->getVisible() && childHints.mAssignedRect->contains(mousePos))
                return child->mouseButtonEvent(mousePos - childHints.mAssignedRect->getPosition(), button, down, modifiers);
        }
        return false;
    }

    bool Container::mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) {
        for (auto &child : ReverseContainerView(mChildren)) {
            if (child->getVisible()) {
                auto childHints = child->layoutHints();
                auto relCursorPosition = cursorPosition - childHints.mAssignedRect->getPosition();
                bool contained = childHints.mAssignedRect->contains(relCursorPosition);
                bool prevContained = childHints.mAssignedRect->contains(relCursorPosition - rel);
                if (contained != prevContained) {
                    if (auto container = child->as<Container>(); container &&
                                                                 container->mouseMotionEvent(relCursorPosition, rel, button,
                                                                                             modifiers))
                        return true;
                    else
                        child->mouseEnterEvent(cursorPosition, contained);
                }
                if ((contained || prevContained) &&
                    child->mouseMotionEvent(relCursorPosition, rel, button, modifiers))
                    return true;
            }
        }
        return false;
    }

    bool Container::mouseDragEvent(const Position &mousePosition, const Position &rel, int button, int modifiers) {
        auto screenPosition = getScreenPosition();
        auto relMousePos = mousePosition - screenPosition;
        for (auto &child : ReverseContainerView(mChildren)) {
            if (!child->getVisible())
                continue;
            if (child->contains(relMousePos))
                return child->mouseDragEvent(relMousePos, rel, button, modifiers);
        }
        return false;
    }

    bool Container::scrollEvent(const Position &p, double relX, double relY) {
        auto relMousePos = p - mLayoutHints.mAssignedRect->getPosition();
        for (auto &child : ReverseContainerView(mChildren)) {
            if (!child->getVisible())
                continue;
            if (child->contains(relMousePos))
                return child->scrollEvent(relMousePos, relX, relY);
        }
        return false;
    }

    bool Container::keyboardEvent(int key, int scancode, int action, int modifiers) {
        return Widget::keyboardEvent(key, scancode, action, modifiers);
    }

    bool Container::keyboardCharacterEvent(unsigned int codepoint) {
        return Widget::keyboardCharacterEvent(codepoint);
    }

    void Container::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        assertLayout();
        if (mVisible) {
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);
            Widget::draw(renderer, parentRect);

            for (auto &child : mChildren) {
                child->draw(renderer, widgetRect);
            }
        }
    }

    std::shared_ptr<Container> Container::findDragContainer(std::shared_ptr<Container> container) {
        while (container) {
            if (container->supportsDrag())
                return container;
            container = container->parent();
        }
        return nullptr;
    }

    bool Container::mouseEnterEvent(const Position &p, bool enter) {
        for (auto &child : ReverseContainerView(mChildren))
            child->mouseEnterEvent(p, enter);

        return false;
    }

    Size Container::widgetRatioHeight(Size size, int height) {
        float ratio = std::min((float) height / (float) size.width(), (float) height / (float) size.height());
        return rose::Size{roundToInt((float)size.width() * ratio), roundToInt((float)size.height() * ratio)};
    }

    Size Container::wdigetRatioWidth(Size size, int width) {
        float ratio = std::min((float) width / (float) size.width(), (float) width / (float) size.height());
        return rose::Size{roundToInt((float)size.width() * ratio), roundToInt((float)size.height() * ratio)};
    }

    Window::Window(const shared_ptr<Rose>& parent) : Container() {
        mRose = parent;
        mPos = Position{0, 0};
        mSize = Size{parent->width(), parent->height()};
        mClassName = "Window";
    }

    Window::Window(const shared_ptr<Rose>& parent, const Size &size) : Window(parent) {
        mSize = size;
        mPos = Position{std::clamp((parent->width() - size.width()) / 2, 0, parent->width()),
                           std::clamp((parent->height() - size.height()) / 2, 0, parent->height())};
    }

    Window::Window(const shared_ptr<Rose>& parent, const Position &pos, const Size &size) : Window(parent) {
        mSize = size;
        mPos = pos;
    }

    void Column::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        assertLayout();
        if (!mVisible)
            return;

        auto widgetRect = parentRect.moveOrigin(mLayoutHints.mAssignedRect->getPosition());

        for (auto &child : mChildren) {
            child->draw(renderer, widgetRect);
        }
    }

    Rectangle Column::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto columnAvailable = clampAvailableArea(available, mPos, mSize);
        auto maxWidth = mMinWidth;

        bool first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->initialLayout(renderer, columnAvailable);
            if (!childHints.mShrinkable) {
                maxWidth = std::max(maxWidth, childHints.mAssignedRect->width());
            }
            columnAvailable.height() -= childHints.mAssignedRect->height();
            if (!first)
                columnAvailable.height() -= mContainerHints.verticalSpacing;
            else
                first = false;
        }

        first = true;
        columnAvailable = clampAvailableArea(available, mPos, mSize);
        columnAvailable.width() = maxWidth;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->initialLayout(renderer, columnAvailable);
            columnAvailable.height() -= childHints.mAssignedRect->height();
            if (!first)
                columnAvailable.height() -= mContainerHints.verticalSpacing;
            else
                first = false;
        }

        int totalHeight{0};
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (childHints.mShrinkable) {
                childHints.mAssignedRect.value() = wdigetRatioWidth(childHints.mAssignedRect->getSize(), maxWidth);
            } else if (childHints.mElastic) {
                childHints.mAssignedRect->width() = maxWidth;
            }
            totalHeight += childHints.mAssignedRect->height();
        }

        int posY = 0;
        first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (!first)
                posY += mContainerHints.verticalSpacing;
            else
                first = false;
            childHints.mAssignedRect->y() = posY;
            posY += childHints.mAssignedRect->height();
            if (childHints.mElastic) {
                childHints.mAssignedRect->width() = maxWidth;
            } else {
                switch (childHints.mHorAlign) {
                    case HorizontalAlignment::Unset:
                    case HorizontalAlignment::Left:
                        childHints.mAssignedRect->x() = 0;
                        break;
                    case HorizontalAlignment::Center:
                        childHints.mAssignedRect->x() = (maxWidth - childHints.mAssignedRect->width()) / 2;
                        break;
                    case HorizontalAlignment::Right:
                        childHints.mAssignedRect->x() = maxWidth - childHints.mAssignedRect->width();
                        break;
                }
            }
        }

        auto layout = Rectangle{mPos, mSize};
        layout.width() = maxWidth;
        layout.height() = posY;

        return layout;
    }

    void Row::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        assertLayout();
        if (!mVisible)
            return;

        auto widgetRect = parentRect.moveOrigin(mLayoutHints.mAssignedRect->getPosition());

        for (auto &child : mChildren) {
            child->draw(renderer, widgetRect);
        }
    }

    Rectangle Row::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto rowAvailable = clampAvailableArea(available, mPos, mSize);
        int maxHeight{0};

        bool first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->initialLayout(renderer, rowAvailable);
            rowAvailable.width() -= childHints.mAssignedRect->width();
            if (!childHints.mShrinkable) {
                maxHeight = std::max(maxHeight, childHints.mAssignedRect->height());
            }
            rowAvailable.width() -= childHints.mAssignedRect->width();
            if (!first)
                rowAvailable.width() -= mContainerHints.horizontalSpacing;
            else
                first = false;
        }

        first = true;
        rowAvailable = clampAvailableArea(available, mPos, mSize);
        rowAvailable.height() = maxHeight;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->initialLayout(renderer, rowAvailable);
            rowAvailable.width() -= childHints.mAssignedRect->width();
            if (!first)
                rowAvailable.width() -= mContainerHints.horizontalSpacing;
            else
                first = false;
        }

        int totalWidth{0};
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (childHints.mShrinkable) {
                childHints.mAssignedRect.value() = widgetRatioHeight(childHints.mAssignedRect->getSize(), maxHeight);
            } else if (childHints.mElastic) {
                childHints.mAssignedRect->height() = maxHeight;
            }
            totalWidth += childHints.mAssignedRect->width();
        }

        int posX = 0;
        first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (!first)
                posX += mContainerHints.horizontalSpacing;
            else
                first = false;
            childHints.mAssignedRect->x() = posX;
            posX += childHints.mAssignedRect->width();
            if (!childHints.mElastic) {
                switch (childHints.mVerAlign) {
                    case VerticalAlignment::Unset:
                    case VerticalAlignment::Top:
                        childHints.mAssignedRect->y() = 0;
                        break;
                    case VerticalAlignment::Center:
                        childHints.mAssignedRect->y() = (maxHeight - childHints.mAssignedRect->height()) / 2;
                        break;
                    case VerticalAlignment::Bottom:
                        childHints.mAssignedRect->y() = maxHeight - childHints.mAssignedRect->height();
                        break;
                }
            }
        }

        auto layout = Rectangle{mPos, mSize};
        layout.width() = posX;
        layout.height() = maxHeight;

        return layout;
    }

#if 0

    Grid::Grid(size_t stride, Orientation orientation) : Container() {
        mStride = stride;
        mOrientation = orientation;
        mClassName = "Grid";
    }

    void Grid::draw(sdl::Renderer &renderer, Size size, Position parentPosition) {
        if (!mVisible)
            return;

        if (size)
            mSize = size;
        else
            mSize = mLayoutHints.size;

        if (parentPosition)
            mPos = parentPosition;
        else
            mPos = mLayoutHints.position;

        if (mOrientation == Orientation::Horizontal) {
            int positionX = 0;
            int positionY = 0;

            for (auto child = mChildren.begin(); child != mChildren.end(); ++child) {
                auto idx = child - mChildren.begin();
                auto row = idx / mStride;
                auto col = idx % mStride;

                if (idx > 0 && col == 0) {
                    positionY += mRowLayout.at(row - 1).size.height();
                    positionX = 0;
                }

                (*child)->layoutHints().size = Size{
                        mColLayout.at(col).size.width(),
                        mRowLayout.at(row).size.height()};
                (*child)->layoutHints().position = Position{positionX, positionY};
                positionX += mColLayout.at(col).size.width();
            }
        } else {
            int positionX = 0;
            int positionY = 0;

            for (auto child = mChildren.begin(); child != mChildren.end(); ++child) {
                auto idx = child - mChildren.begin();
                auto col = idx / mStride;
                auto row = idx % mStride;

                if (idx > 0 && row == 0) {
                    positionX += mColLayout.at(col - 1).size.width();
                    positionY = 0;
                }

                (*child)->layoutHints().size = Size{
                        mColLayout.at(col).size.width(),
                        mRowLayout.at(row).size.height()};
                (*child)->layoutHints().position = Position{positionX, positionY};
                positionY += mRowLayout.at(row).size.height();
            }
        }

        for (auto &child : mChildren) {
            child->draw(renderer, limitSize(child), child->layoutHints().position);
        }
    }

    Size Grid::initialLayout(sdl::Renderer &renderer, Size available) {
        // ToDo: handle shrinkable Widgets.
        if (mOrientation == Orientation::Horizontal) {
            for (auto child = mChildren.begin(); child != mChildren.end(); ++child) {
                auto idx = child - mChildren.begin();
                auto row = idx / mStride;
                auto col = idx % mStride;

                if (row >= mRowLayout.size())
                    mRowLayout.emplace_back(LayoutHints{});

                if (col >= mColLayout.size())
                    mColLayout.emplace_back(LayoutHints{});

                auto size = (*child)->initialLayout(renderer, rose::Size());
                (*child)->layoutHints().size = size;
                mRowLayout.at(row).size.height() = std::max(mRowLayout.at(row).size.height(),
                                                            size.height());
                mColLayout.at(col).size.width() = std::max(mColLayout.at(col).size.width(),
                                                           size.width());
            }
        } else {
            for (auto child = mChildren.begin(); child != mChildren.end(); ++child) {
                auto idx = child - mChildren.begin();
                auto col = idx / mStride;
                auto row = idx % mStride;

                if (row >= mRowLayout.size())
                    mRowLayout.emplace_back(LayoutHints{});

                if (col >= mColLayout.size())
                    mColLayout.emplace_back(LayoutHints{});

                auto size = (*child)->initialLayout(renderer, rose::Size());
                (*child)->layoutHints().size = size;
                mRowLayout.at(row).size.height() = std::max(mRowLayout.at(row).size.height(),
                                                            size.height());
                mColLayout.at(col).size.width() = std::max(mColLayout.at(col).size.width(),
                                                           size.width());
            }
        }

        mLayoutHints.clear();

        for (auto &row : mRowLayout) {
            mLayoutHints.size.height() += row.size.height();
        }

        for (auto &col : mColLayout) {
            mLayoutHints.size.width() += col.size.width();
        }

        return mLayoutHints.size;
    }
#endif
}
