//
// Created by richard on 2020-10-20.
//

#include "Container.h"
#include "Widget.h"
#include "Rose.h"

namespace rose {

    Widget::Widget() {
        mClassName = "Widget";
    }

    std::shared_ptr<Window> Widget::window() {
        if (auto uc = mParent.use_count(); mHasParent) {
            auto parent = mParent.lock();
            if (parent) {
                while (parent->mHasParent)
                    parent = parent->mParent.lock();
                return parent->as<Window>();
            }
        }
        return as<Window>();
    }

    std::shared_ptr<Rose> Widget::rose() {
        if (mRose.expired())
            return nullptr;
        return mRose.lock();
    }

    void Widget::requestFocus() {
        rose()->updateFocus(getWidget());
    }

    bool Widget::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        if (button == SDL_BUTTON_LEFT && down && !mFocused)
            requestFocus();
        return true;
    }

    bool
    Widget::mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button,
                             int modifiers) {
        return false;
    }

    bool
    Widget::mouseDragEvent(const Position &p, const Position &rel, int button, int modifiers) {
        auto container = parent();
        while (container) {
            if (container->supportsDrag()) {
                rose()->setDragWidget(container);
                return container->mouseDragEvent(p, rel, button, modifiers);
            }
            container = container->parent();
        }
        return false;
    }

    bool Widget::mouseEnterEvent(const Position &p, bool enter) {
        return false;
    }

    bool Widget::scrollEvent(const Position &p, const double relX, double relY) {
        return false;
    }

    bool Widget::focusEvent(bool focused) {
        mFocused = focused;
        return false;
    }

    bool Widget::keyboardEvent(int key, int scancode, int action, int modifiers) {
        return false;
    }

    bool Widget::keyboardCharacterEvent(unsigned int codepoint) {
        return false;
    }

    void Widget::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        assertLayout();
        if (mVisible && mFillBackground) {
            auto widgetRect = parentRect.moveOrigin(mLayoutHints.mAssignedRect->getPosition());
            /*
             * If the assigned rectangle is valid use it, otherwise build one from position and size.
             * This is essentially the difference between a Widget managed by a Container, and one
             * that is not.
             */
            renderer.fillRect(widgetRect, mBackgroundColor);
        }
    }

    void Widget::setNeedsDrawing() {
        auto rosePtr = rose();
        if (rosePtr)
            rose()->needsDrawing(true);
    }

    void Widget::assertLayout() {
//        if (!mLayoutHints.mAssignedRect)
//            throw std::runtime_error(StringCompositor("Widget \"", mClassName, "\" layout out improper: ",
//                                                      mLayoutHints.mAssignedRect));
//        if (mLayoutHints.mAssignedRect.getSize().width() > 800 || mLayoutHints.mAssignedRect.height() > 480)
//            std::cout << "Too big!\n";
    }

    ImageId Widget::getImageId() const noexcept {
        return RoseImageId::RoseImageInvalid;
    }

    Position Widget::getScreenPosition() const {
        if (mHasParent && !mParent.expired()) {
            Position pos = mParent.lock()->getScreenPosition();
            return mLayoutHints.mAssignedRect->getPosition() + pos;
        }
        // Windows always return Zero for correct draging of PopupWindows.
        return mLayoutHints.mAssignedRect->getPosition();
    }

    std::shared_ptr<Container> Widget::parent() {
        return parent<Container>();
    }
}
