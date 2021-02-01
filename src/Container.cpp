//
// Created by richard on 2020-11-16.
//

#include <algorithm>
#include "Container.h"
#include "Math.h"
#include "Rose.h"
#include "ScreenMetrics.h"

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

    bool Container::scrollEvent(const Position &p, int32_t relX, int32_t relY) {
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

    Rectangle Column::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto columnAvailable = clampAvailableArea(available, mPos, mSize);
        auto maxWidth = mMinWidth;

        bool first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, columnAvailable, 0);
            if (!childHints.mShrinkable) {
                maxWidth = std::max(maxWidth, childHints.mAssignedRect->width());
            }
            columnAvailable.height() -= childHints.mAssignedRect->height();
            if (!first)
                columnAvailable.height() -= mContainerHints.internalSpace;
            else
                first = false;
        }

        first = true;
        columnAvailable = clampAvailableArea(available, mPos, mSize);
        columnAvailable.width() = maxWidth;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, columnAvailable, 1);
            columnAvailable.height() -= childHints.mAssignedRect->height();
            if (!first)
                columnAvailable.height() -= mContainerHints.internalSpace;
            else
                first = false;
        }

        int totalHeight{0};
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (childHints.mShrinkable) {
                childHints.mAssignedRect.value() = wdigetRatioWidth(childHints.mAssignedRect->getSize(), maxWidth);
            } else if (childHints.mElastic.horizontal()) {
                childHints.mAssignedRect->width() = maxWidth;
            }
            totalHeight += childHints.mAssignedRect->height();
        }

        int posY = mContainerHints.startOffset;
        first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (!first)
                posY += mContainerHints.internalSpace;
            else
                first = false;
            childHints.mAssignedRect->y() = posY;
            posY += childHints.mAssignedRect->height();
            if (childHints.mElastic.horizontal()) {
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
        posY += mContainerHints.endOffset;

        auto layout = Rectangle{mPos, mSize};
        layout.width() = maxWidth;
        layout.height() = posY;

        if (mContainerHints.fillToEnd && layoutStage > 0 && layout.height() < available.height()) {
            auto extraSpace = available.height() - layout.height();
            size_t elasticChildren = 0;
            for (auto &child : mChildren) {
                if (child->layoutHints().mElastic.vertical())
                    ++elasticChildren;
            }

            if (elasticChildren) {
                auto perChildSpace = extraSpace / elasticChildren;
                int positionShift = 0;
                for (auto &child : mChildren) {
                    child->layoutHints().mAssignedRect->x() += positionShift;
                    if (child->layoutHints().mElastic.vertical()) {
                        if (elasticChildren == 1) {
                            child->layoutHints().mAssignedRect->height() += extraSpace;
                            layout.height() += extraSpace;
                            positionShift += extraSpace;
                            extraSpace = 0;
                        } else {
                            child->layoutHints().mAssignedRect->height() += perChildSpace;
                            layout.height() += perChildSpace;
                            positionShift += perChildSpace;
                            extraSpace -= perChildSpace;
                        }
                        --elasticChildren;
                    }
                }
            }
        }

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

    Rectangle Row::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto rowAvailable = clampAvailableArea(available, mPos, mSize);
        int maxHeight{0};

        bool first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, rowAvailable, 0);
            rowAvailable.width() -= childHints.mAssignedRect->width();
            if (!childHints.mShrinkable) {
                maxHeight = std::max(maxHeight, childHints.mAssignedRect->height());
            }
            rowAvailable.width() -= childHints.mAssignedRect->width();
            if (!first)
                rowAvailable.width() -= mContainerHints.internalSpace;
            else
                first = false;
        }

        first = true;
        rowAvailable = clampAvailableArea(available, mPos, mSize);
        rowAvailable.height() = maxHeight;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, rowAvailable, 1);
            rowAvailable.width() -= childHints.mAssignedRect->width();
            if (!first)
                rowAvailable.width() -= mContainerHints.internalSpace;
            else
                first = false;
        }

        int totalWidth{0};
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (childHints.mShrinkable) {
                childHints.mAssignedRect.value() = widgetRatioHeight(childHints.mAssignedRect->getSize(), maxHeight);
            } else if (childHints.mElastic.vertical()) {
                childHints.mAssignedRect->height() = maxHeight;
            }
            totalWidth += childHints.mAssignedRect->width();
        }

        int posX = mContainerHints.startOffset;
        first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (!first)
                posX += mContainerHints.internalSpace;
            else
                first = false;
            childHints.mAssignedRect->x() = posX;
            posX += childHints.mAssignedRect->width();
            if (!childHints.mElastic.vertical()) {
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
        posX += mContainerHints.endOffset;

        auto layout = Rectangle{mPos, mSize};
        layout.width() = posX;
        layout.height() = maxHeight;

        if (mContainerHints.fillToEnd && layoutStage > 0 && layout.width() < available.width()) {
            auto extraSpace = available.width() - layout.width();
            size_t elasticChildren = 0;
            for (auto &child : mChildren) {
                if (child->layoutHints().mElastic.horizontal())
                    ++elasticChildren;
            }

            if (elasticChildren) {
                auto perChildSpace = extraSpace / elasticChildren;
                auto positionShift = 0;
                for (auto &child : mChildren) {
                    child->layoutHints().mAssignedRect->x() += positionShift;
                    if (child->layoutHints().mElastic.horizontal()) {
                        if (elasticChildren == 1) {
                            child->layoutHints().mAssignedRect->width() += extraSpace;
                            layout.width() += extraSpace;
                            positionShift += extraSpace;
                            extraSpace = 0;
                        } else {
                            child->layoutHints().mAssignedRect->width() += perChildSpace;
                            layout.width() += perChildSpace;
                            positionShift += perChildSpace;
                            extraSpace -= perChildSpace;
                        }
                        --elasticChildren;
                    }
                }
            }
        }

        return layout;
    }

    Grid::Grid(size_t stride, const Size &size, Orientation orientation) : Container() {
        mStride = stride;
        if (size != Size::Zero)
            mSingleSize = size;
        else
            mSingleSize = std::nullopt;
        mOrientation = orientation;
    }

    void Grid::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        assertLayout();
        if (!mVisible)
            return;

        if (mSingleSize) {
            auto widgetRect = parentRect.moveOrigin(mLayoutHints.mAssignedRect->getPosition());

            for (auto &child : mChildren) {
                child->draw(renderer, widgetRect);
            }
        }
    }

    Rectangle Grid::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto gridAvailable = clampAvailableArea(available, mPos, mSize);
        if (mSingleSize) {
            size_t count = 0;
            size_t stack = 0;
            gridAvailable = mSingleSize.value();
            Position childPos{};
            for (auto &child : mChildren) {
                LayoutHints &childHints{child->layoutHints()};
                auto layout = child->widgetLayout(renderer, gridAvailable, 0);
                layout = childPos;
                layout = mSingleSize.value();
                childHints.mAssignedRect = layout;
                if (++count >= mStride) {
                    childPos.primary(mOrientation) = 0;
                    childPos.secondary(mOrientation) += mSingleSize->secondary(mOrientation);
                    count = 0;
                    ++stack;
                } else {
                    childPos.primary(mOrientation) += mSingleSize->primary(mOrientation);
                }
            }

            Size gridSize{};
            gridSize.primary(mOrientation) = mSingleSize->primary(mOrientation) * static_cast<int>(mStride);
            gridSize.secondary(mOrientation) = mSingleSize->secondary(mOrientation) * static_cast<int>(stack);
            return Rectangle{0, 0, gridSize.width(), gridSize.height()};
        }

        return gridAvailable;
    }
}
