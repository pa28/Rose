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

//    bool Container::keyboardEvent(int key, int scancode, int action, int modifiers) {
//        return Widget::keyboardEvent(key, scancode, action, modifiers);
//    }

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

        auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

        if (mSingleSize) {
//            auto widgetRect = parentRect.moveOrigin(mLayoutHints.mAssignedRect->getPosition());

            for (auto &child : mChildren) {
                child->draw(renderer, widgetRect);
            }
        }
    }

    Rectangle Grid::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto gridAvailable = clampAvailableArea(available, mPos, mSize);
        Rectangle gridLayout{gridAvailable};
        if (mSingleSize) {
            int count = 0;
            int stack = 0;
            Rectangle childAvailable{Position::Zero, mSingleSize.value()};
            Position childPos{};
            for (auto &child : mChildren) {
                LayoutHints &childHints{child->layoutHints()};
                auto layout = child->widgetLayout(renderer, childAvailable, 0);
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

            Size gridSize{mSingleSize->width() * (stack+1),
                          mSingleSize->height() * (int)(stack ? mStride : mChildren.size()) };
            gridLayout = Rectangle{0, 0, gridSize.width(), gridSize.height()};
        }

        return gridLayout;
    }

    void Box::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (!mVisible)
            return;

        auto widgetRect = parentRect.moveOrigin(mLayoutHints.mAssignedRect->getPosition());

        for (auto &child : mChildren) {
            child->draw(renderer, widgetRect);
        }
    }

    Rectangle Box::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto boxAvailable = clampAvailableArea(available, mPos, mSize);
        auto maxSecondary = mMinOrthogonal;

        bool first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, boxAvailable, 0);
            if (!childHints.mShrinkable) {
                maxSecondary = std::max(maxSecondary, childHints.mAssignedRect->sizeSecondary(mOrientation));
            }
            boxAvailable.sizePrimary(mOrientation) -= childHints.mAssignedRect->sizePrimary(mOrientation);
            if (!first)
                boxAvailable.sizePrimary(mOrientation) -= mContainerHints.internalSpace;
            else
                first = false;
        }

        first = true;
        boxAvailable = clampAvailableArea(available, mPos, mSize);
        boxAvailable.sizeSecondary(mOrientation) = maxSecondary;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, boxAvailable, 1);
            boxAvailable.sizePrimary(mOrientation) -= childHints.mAssignedRect->sizePrimary(mOrientation);
            if (!first)
                boxAvailable.sizePrimary(mOrientation) -= mContainerHints.internalSpace;
            else
                first = false;
        }

        int totalPrimary{0};
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (childHints.mShrinkable) {
                if (mOrientation == Orientation::Vertical)
                    childHints.mAssignedRect.value() = wdigetRatioWidth(childHints.mAssignedRect->getSize(),
                                                                        maxSecondary);
                else
                    childHints.mAssignedRect.value() = widgetRatioHeight(childHints.mAssignedRect->getSize(),
                                                                         maxSecondary);
            } else if (childHints.mElastic.secondary(mOrientation)) {
                childHints.mAssignedRect->sizeSecondary(mOrientation) = maxSecondary;
            }
            totalPrimary += childHints.mAssignedRect->sizePrimary(mOrientation);
        }

        int posPrimary = mContainerHints.startOffset;
        first = true;
        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            if (!first)
                posPrimary += mContainerHints.internalSpace;
            else
                first = false;
            childHints.mAssignedRect->positionPrimary(mOrientation) = posPrimary;
            posPrimary += childHints.mAssignedRect->sizePrimary(mOrientation);
            if (mOrientation == Orientation::Vertical) {
                // Column implementation.
                if (childHints.mElastic.horizontal()) {
                    childHints.mAssignedRect->width() = maxSecondary;
                } else {
                    switch (childHints.mHorAlign) {
                        case HorizontalAlignment::Unset:
                        case HorizontalAlignment::Left:
                            childHints.mAssignedRect->x() = 0;
                            break;
                        case HorizontalAlignment::Center:
                            childHints.mAssignedRect->x() = (maxSecondary - childHints.mAssignedRect->width()) / 2;
                            break;
                        case HorizontalAlignment::Right:
                            childHints.mAssignedRect->x() = maxSecondary - childHints.mAssignedRect->width();
                            break;
                    }
                }
            } else {
                // Row implementation.
                if (childHints.mElastic.vertical()) {
                    childHints.mAssignedRect->height() = maxSecondary;
                } else {
                    switch (childHints.mVerAlign) {
                        case VerticalAlignment::Unset:
                        case VerticalAlignment::Top:
                            childHints.mAssignedRect->y() = 0;
                            break;
                        case VerticalAlignment::Center:
                            childHints.mAssignedRect->y() = (maxSecondary - childHints.mAssignedRect->height()) / 2;
                            break;
                        case VerticalAlignment::Bottom:
                            childHints.mAssignedRect->y() = maxSecondary - childHints.mAssignedRect->height();
                            break;
                    }
                }
            }

        }
        posPrimary += mContainerHints.endOffset;

        auto layout = Rectangle{mPos, mSize};
        layout.sizeSecondary(mOrientation) = maxSecondary;
        layout.sizePrimary(mOrientation) = posPrimary;

        if (mContainerHints.fillToEnd && layoutStage > 0 &&
            layout.sizePrimary(mOrientation) < available.sizePrimary(mOrientation)) {
            auto extraSpace = available.sizePrimary(mOrientation) - layout.sizePrimary(mOrientation);
            size_t elasticChildren = 0;
            for (auto &child : mChildren) {
                if (child->layoutHints().mElastic.primary(mOrientation))
                    ++elasticChildren;
            }

            if (elasticChildren) {
                auto perChildSpace = extraSpace / elasticChildren;
                int positionShift = 0;
                for (auto &child : mChildren) {
                    child->layoutHints().mAssignedRect->positionPrimary(mOrientation) += positionShift;
                    if (child->layoutHints().mElastic.primary(mOrientation)) {
                        if (elasticChildren == 1) {
                            child->layoutHints().mAssignedRect->sizePrimary(mOrientation) += extraSpace;
                            layout.sizePrimary(mOrientation) += extraSpace;
                            positionShift += extraSpace;
                            extraSpace = 0;
                        } else {
                            child->layoutHints().mAssignedRect->sizePrimary(mOrientation) += perChildSpace;
                            layout.sizePrimary(mOrientation) += perChildSpace;
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
}
