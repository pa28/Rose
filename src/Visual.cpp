/**
 * @file Visual.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include "Visual.h"
#include "Application.h"
#include "GraphicsModel.h"

namespace rose {

    SemanticGesture SemanticGesture::None{0x0};
    SemanticGesture SemanticGesture::Click{0x1};
    SemanticGesture SemanticGesture::Drag{0x2};
    SemanticGesture SemanticGesture::Scroll{0x4};
    SemanticGesture SemanticGesture::Key{0x8};
    SemanticGesture SemanticGesture::Multi{0x10};

    Screen::Screen(Application &application) : mApplication(application) {

    }

    void rose::Screen::draw(gm::Context &context, const Position<int> &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->draw(context, mScreenRect.position());
            }
        }
    }

    Rectangle Screen::layout(gm::Context &context, const Rectangle &screenRect) {
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->layout(context, screenRect);
            }
        }
        return screenRect;
    }

    void Window::generateBaseTexture(gm::Context &context, const Position<int> &containerPosition) {
        if (baseTextureNeeded(containerPosition)) {
            mBaseTexture = gm::Texture{context, mScreenRect.size()};
        }

        gm::RenderTargetGuard renderTargetGuard(context, mBaseTexture);
        context.setDrawColor(color::DarkBaseColor);
        context.renderClear();
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->draw(context, Position<int>{});
            }
        }
    }

    void Window::drawBaseTexture(gm::Context &context, const Position<int> &containerPosition) {
        if (mBaseTexture) {
            setScreenRectangle(containerPosition);
            context.renderCopy(mBaseTexture, mScreenRect);
        } else {
            draw(context, containerPosition);
        }
    }

    void Window::draw(gm::Context &context, const Position<int> &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->draw(context, mScreenRect.position());
            }
        }
    }

    Rectangle Window::layout(gm::Context &context, const Rectangle &screenRect) {
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                auto rect = manager->layout(context, screenRect);
                manager->setScreenRectangle(rect);
            }
        }
        return screenRect;
    }

    std::shared_ptr<Widget> Window::pointerWidget(const Position<int> &position) {
        auto windowRectangle = getScreenRectangle(Position<int>{});
        for (auto &content : *this) {
            if (auto widget = std::dynamic_pointer_cast<Widget>(content); widget) {
                auto widgetRectangle = widget->getScreenRectangle(windowRectangle.position());
                if (widgetRectangle.contains(position)) {
                    if (auto ptrWdg = widget->pointerWidget(position, windowRectangle.position()); ptrWdg)
                        return ptrWdg;
                }
            }
        }
        return nullptr;
    }

    std::shared_ptr<Widget>
    Widget::pointerWidget(const Position<int> &position, const Position<int> &containerPosition) {
        auto widgetRectangle = getScreenRectangle(containerPosition);
        if (auto manager = getNode<Manager>(); manager) {
            for (auto &content : *manager) {
                if (auto widget = std::dynamic_pointer_cast<Widget>(content); widget) {
                    auto wRect = widget->getScreenRectangle(widgetRectangle.position());
                    if (wRect.contains(position)) {
                        return widget->pointerWidget(position, widgetRectangle.position());
                    }
                }
            }
            return manager;
        }
        return getNode<Widget>();
    }

    Position<int> Widget::computeScreenPosition() {
        std::shared_ptr<Widget> parent = std::dynamic_pointer_cast<Widget>(container());
        if (parent) {
            auto position = parent->computeScreenPosition();
            position = position + mPos;
            return position;
        }

        return Position<int>{};
    }

    bool Widget::contains(const Position<int> &position) {
        Rectangle screenRectangle{computeScreenPosition(), mSize};
        return screenRectangle.contains(position);
    }

    bool Widget::buttonEvent(bool pressed, uint button, uint clicks, bool passed) {
        if (mButtonEventCallback)
            if (mButtonEventCallback(pressed, button, clicks)) {
                if (passed) {
                    getApplication().capturePointerWidget(getNode<Widget>());
                }
                return true;
            }

        if (auto widget = std::dynamic_pointer_cast<Widget>(container()); widget) {
            return widget->buttonEvent(pressed, button, clicks, true);
        }

        return false;
    }

    bool
    Widget::mouseMotionEvent(bool pressed, uint button, const Position<int> &mousePos, const Position<int> &relativePos,
                             bool passed) {
        if (mMouseMotionCallback) {
            if (mMouseMotionCallback(pressed, button, mousePos, relativePos)) {
                if (passed) {
                    getApplication().capturePointerWidget(getNode<Widget>());
                }
                return true;
            }
        }

        if (auto widget = std::dynamic_pointer_cast<Widget>(container()); widget && button != 0) {
            return widget->mouseMotionEvent(pressed, button, mousePos, relativePos, true);
        }

        return false;
    }

    bool Widget::mouseScrollEvent(const Position<int>& deltaPos, bool passed) {
        std::cout << __PRETTY_FUNCTION__ << deltaPos << '\n';
        if (mMouseScrollCallback)
            if (mMouseScrollCallback(deltaPos)) {
                if (passed)
                    getApplication().captureScrollWheelWidget(getNode<Widget>());
                return true;
            }

        if (auto widget = std::dynamic_pointer_cast<Widget>(container()); widget) {
            return widget->mouseScrollEvent(deltaPos, true);
        }

        return false;
    }

    bool Widget::keyboardShortcutEvent(uint32_t shortcutCode, bool state, uint repeat) {
        std::cout << __PRETTY_FUNCTION__ << (state ? " Pressed." : " Released.") << ' ' << repeat << '\n';
        if (mKeyboardShortcutCallback) {
            mKeyboardShortcutCallback(shortcutCode, state, repeat);
            return true;
        }
        return false;
    }

    bool Widget::keyTextInputEvent(const std::string& text) {
        if (mTextInputCallback) {
            mTextInputCallback(text);
            return true;
        }
        return false;
    }

    bool Widget::keyboardFocusEvent(bool hasFocus) {
        if (mKeyboardFocusCallback) {
            mKeyboardFocusCallback(hasFocus);
            return true;
        }
        return false;
    }

    bool Widget::keyboardEvent(const SDL_KeyboardEvent &keyboardEvent) {
        if (mKeyboardEventCallback) {
            mKeyboardEventCallback(keyboardEvent);
            return true;
        }
        return false;
    }

    void Manager::draw(gm::Context &context, const Position<int> &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto visual = std::dynamic_pointer_cast<Visual>(content); visual) {
                visual->draw(context, drawPadding(mScreenRect.position()));
            }
        }
    }

    Rectangle Manager::layout(gm::Context &context, const Rectangle &screenRect) {
        Rectangle managerRect{screenRect};

        if (mPreferredSize != Size::Zero) {
            managerRect = mPreferredSize;
        }

        auto rect = mLayoutManager->layoutContent(context, managerRect, begin(), end());
        rect = layoutPadding(rect);
        return Rectangle{mPreferredPos,rect.size()};
    }

    Manager::Manager() {
        mLayoutManager = std::make_unique<SimpleLayout>();
    }

    Rectangle
    SimpleLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) {
        while (first != last) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(*first); manager) {
                auto contentRect = manager->layout(context, screenRect);
                contentRect = manager->getPosition();
                manager->setScreenRectangle(contentRect);
            } else if (auto widget = std::dynamic_pointer_cast<Widget>(*first); widget) {
                auto contentRect = widget->layout(context, screenRect);
                widget->setScreenRectangle(contentRect + widget->getPosition());
            }
            first++;
        }
        return screenRect;
    }
}
