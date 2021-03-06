/**
 * @file Application.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#include "Application.h"
#include "Settings.h"
#include "Utilities.h"
#include "Math.h"
#include <filesystem>
#include <regex>

using namespace rose::gm;

namespace rose {

    Application::Application(int argc, char **argv) : mEventSemantics(*this), mInputParser(argc, argv) {
        mScreen = std::make_shared<Screen>(*this);
        std::regex kbPathRegEx{std::string{KeyboardPathRegEx}.c_str()};
        for(auto& p: std::filesystem::directory_iterator(UsbDeviceByPath)) {
            if (mKeyboardFound = std::regex_match(p.path().string(), kbPathRegEx); mKeyboardFound)
                break;
        }
    }

    void Application::windowStateChange(EventSemantics::WindowEventType type) {
        Settings &settings{Settings::getSettings()};
        switch (type) {
            case EventSemantics::Shown: {
                auto windowFlags = SDL_GetWindowFlags(mGraphicsModel.getSdlWindow().get());
                if (windowFlags & SDL_WINDOW_FULLSCREEN || windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                    mAppState = EventSemantics::FullScreen;
                else if (windowFlags & SDL_WINDOW_MAXIMIZED)
                    mAppState = EventSemantics::Maximized;
                else if (windowFlags & SDL_WINDOW_MINIMIZED)
                    mAppState = EventSemantics::Minimized;
                else
                    mAppState = EventSemantics::Restored;
                settings.setValue(set::SetAppState, static_cast<int>(mAppState));
            }
                break;
            case EventSemantics::Hidden:
                break;
            case EventSemantics::Exposed:
                layout();
                break;
            case EventSemantics::Maximized:
            case EventSemantics::Minimized:
            case EventSemantics::Restored:
                settings.setValue(set::SetAppState, static_cast<int>(type));
                mAppState = type;
                break;
            case EventSemantics::Enter:
            case EventSemantics::Leave:
            case EventSemantics::Focus:
            case EventSemantics::UnFocus:
            case EventSemantics::Close:
            case EventSemantics::Moved:
            case EventSemantics::Resized:
            case EventSemantics::SizeChanged:
                layout();
                break;
            default:
                break;
        }
    }

    void Application::windowSizeChange(EventSemantics::WindowEventType type, Size size) {
        if (mAppState == EventSemantics::Restored) {
            Settings &settings{Settings::getSettings()};
            settings.setValue(set::SetAppSize, size);
        }
    }

    void Application::windowPositionChange(EventSemantics::WindowEventType type, Position<int>& position) {
        if (mAppState == EventSemantics::Restored) {
            Settings &settings{Settings::getSettings()};
            auto borders = windowBorders();
            position.x = std::max(position.x - borders.l, 0);
            position.y = std::max(position.y - borders.t, 0);
            settings.setValue(set::SetAppPosition, position);
        }
    }

    bool Application::keyboardEventCallback(const SDL_KeyboardEvent &keyboardEvent) {
        string keyName{SDL_GetKeyName(keyboardEvent.keysym.sym)};
        if (keyboardEvent.keysym.mod & (uint) KMOD_CTRL && (keyboardEvent.keysym.sym != SDLK_LCTRL) &&
            keyboardEvent.keysym.sym != SDLK_RCTRL) {
            bool returnValue = false;
            switch (keyboardEvent.keysym.sym) {
                case SDLK_F1:
                    SDL_MinimizeWindow(getSdlWindow().get());
                    mAppState = EventSemantics::Minimized;
                    returnValue = true;
                    break;
                case SDLK_F2:
                    SDL_SetWindowFullscreen(getSdlWindow().get(), 0);
                    SDL_RestoreWindow(getSdlWindow().get());
                    mAppState = EventSemantics::Restored;
                    returnValue = true;
                    break;
                case SDLK_F3:
                    if (SDL_GetWindowFlags(getSdlWindow().get()) & (uint) SDL_WINDOW_RESIZABLE) {
                        SDL_SetWindowFullscreen(getSdlWindow().get(), 0);
                        SDL_MaximizeWindow(getSdlWindow().get());
                        mAppState = EventSemantics::Maximized;
                    } else {
                        SDL_SetWindowFullscreen(getSdlWindow().get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
                        mAppState = EventSemantics::FullScreen;
                    }
                    returnValue = true;
                    break;
                case SDLK_F4:
                    SDL_SetWindowFullscreen(getSdlWindow().get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
                    mAppState = EventSemantics::FullScreen;
                    returnValue = true;
                    break;
                default:
                    break;
            }
            if (returnValue) {
                Settings::getSettings().setValue(set::SetAppState, static_cast<int>(mAppState));
                return returnValue;
            }
        }

        if (keyboardEvent.keysym.mod & (uint) KMOD_ALT) {
            if (auto shortcut = mKeyboardShortcuts.find(keyboardEvent.keysym.sym); shortcut !=
                                                                                   mKeyboardShortcuts.end()) {
                if (auto widget = shortcut->second.second.lock(); widget)
                    return widget->keyboardShortcutEvent(shortcut->second.first, keyboardEvent.state == SDL_PRESSED,
                                                         keyboardEvent.repeat);
                else
                    mKeyboardShortcuts.erase(shortcut->first);
            }
        } else if (mKeyFocusWidget) {
            return mKeyFocusWidget->keyboardEvent(keyboardEvent);
        }
        return false;
    }

    bool Application::mouseMotionEventCallback(const SDL_MouseMotionEvent &mouseMotionEvent) {
        bool result = false;

        mMousePosition.x = mouseMotionEvent.x;
        mMousePosition.y = mouseMotionEvent.y;

        Position<int> relativePos{mouseMotionEvent.xrel, mouseMotionEvent.yrel};

        if (auto widget = pointerWidget(mMousePosition); widget) {
            if (mPointerWidget) {
                if (mPointerWidget != widget) {
                    result |= mPointerWidget->leaveEvent();
                    mPointerWidget = widget;
                    result |= mPointerWidget->enterEvent();
                }
                result |= mPointerWidget->mouseMotionEvent(mMouseButtonPressed, mMouseButtonId, mMousePosition,
                                                           relativePos, false);
            } else {
                mPointerWidget = widget;
                return mPointerWidget->enterEvent();
            }
        } else {
            if (mPointerWidget) {
                result |= mPointerWidget->leaveEvent();
                mPointerWidget.reset();
            }
        }

        return result;
    }

    bool Application::fingerTouchEventCallback(const SDL_TouchFingerEvent &fingerTouchEvent) {
        bool result = false;

        auto screenRect = mGraphicsModel.screenRectangle();

        mMousePosition.x = util::roundToInt(fingerTouchEvent.x * (float)screenRect.w);
        mMousePosition.y = util::roundToInt(fingerTouchEvent.y * (float)screenRect.h);
        Position<int> relativePos{util::roundToInt(fingerTouchEvent.dx * (float)screenRect.w),
                             util::roundToInt(fingerTouchEvent.dy * (float)screenRect.h)};

        std::cout << "    Position: " << mMousePosition << ", dP: " << relativePos << '\n';

        if (auto widget = pointerWidget(mMousePosition); widget) {
            if (mPointerWidget) {
                if (mPointerWidget != widget) {
                    result |= mPointerWidget->leaveEvent();
                    mPointerWidget = widget;
                    if (mPointerWidget->supportsKeyboardFocus()) {
                        if (mKeyFocusWidget)
                            mKeyFocusWidget->keyboardFocusEvent(false);
                        mKeyFocusWidget = mPointerWidget;
                        mKeyFocusWidget->keyboardFocusEvent(true);
                    }
                    result |= mPointerWidget->enterEvent();
                }
            } else {
                mPointerWidget = widget;
                if (mPointerWidget->supportsKeyboardFocus()) {
                    if (mKeyFocusWidget)
                        mKeyFocusWidget->keyboardFocusEvent(false);
                    mKeyFocusWidget = mPointerWidget;
                    mKeyFocusWidget->keyboardFocusEvent(true);
                }
            }

            switch (fingerTouchEvent.type) {
                case SDL_FINGERMOTION:
                    result |= mPointerWidget->mouseMotionEvent(mMouseButtonPressed, mMouseButtonId, mMousePosition,
                                                               relativePos, false);
                    break;
                case SDL_FINGERDOWN:
                    mMouseButtonPressed = true;
                    mMouseButtonId = 1;
                    result |= mPointerWidget->enterEvent();
                    result |= mPointerWidget->buttonEvent(mMouseButtonPressed, mMouseButtonId, 0, false);
                    break;
                case SDL_FINGERUP:
                    mMouseButtonPressed = false;
                    mMouseButtonId = 0;
                    result |= mPointerWidget->buttonEvent(mMouseButtonPressed, mMouseButtonId, 0, false);
                    result |= mPointerWidget->leaveEvent();
                    break;
            }
        }

        return result;
    }

    bool Application::textInputEventCallback(const SDL_TextInputEvent& textInputEvent) {
        if (mKeyFocusWidget)
            return mKeyFocusWidget->keyTextInputEvent(std::string{textInputEvent.text});
        return false;
    }

    bool Application::mouseButtonEventCallback(const SDL_MouseButtonEvent &mouseButtonEvent) {
        mMouseButtonPressed = mouseButtonEvent.state == SDL_PRESSED;
        if (mMouseButtonPressed)
            mMouseButtonId |= 1u << (mouseButtonEvent.button - 1u);
        else
            mMouseButtonId &= ~(1u << (mouseButtonEvent.button - 1u));

        if (mPointerWidget) {
            if (mPointerWidget->supportsKeyboardFocus() && !mMouseButtonPressed) {
                if (mKeyFocusWidget)
                    mKeyFocusWidget->keyboardFocusEvent(false);
                mKeyFocusWidget = mPointerWidget;
                mKeyFocusWidget->keyboardFocusEvent(true);
            }
            return mPointerWidget->buttonEvent(mMouseButtonPressed, mMouseButtonId, 0, false);
        }
        return false;
    }

    bool Application::mouseWheelEventCallback(const SDL_MouseWheelEvent &mouseWheelEvent) {
        if (mouseWheelEvent.which == SDL_TOUCH_MOUSEID)
            return false;

        Position<int> deltaPos{mouseWheelEvent.x, mouseWheelEvent.y};
        if (mouseWheelEvent.direction == SDL_MOUSEWHEEL_FLIPPED) {
            deltaPos.x *= -1;
            deltaPos.y *= -1;
        }

        if (mPointerWidget)
            return mPointerWidget->mouseScrollEvent(deltaPos, false);

        return false;
    }

    void Application::initialize(const std::string &title, Size defaultSize) {
        mEventSemantics.setWindowStateChangeCallback(&Application::windowStateChange);
        mEventSemantics.setWindowSizeChangeCallback(&Application::windowSizeChange);
        mEventSemantics.setWindowPositionChangeCallback(&Application::windowPositionChange);
        mEventSemantics.setKeyboardEventCallback(&Application::keyboardEventCallback);
        mEventSemantics.setMouseMotionEventCallback(&Application::mouseMotionEventCallback);
        mEventSemantics.setMouseButtonEventCallback(&Application::mouseButtonEventCallback);
        mEventSemantics.setMouseWheelEventCallback(&Application::mouseWheelEventCallback);
        mEventSemantics.setFingerTouchEventCallback(&Application::fingerTouchEventCallback);
        mEventSemantics.setTextInputEventCallback(&Application::textInputEventCallback);

        mGraphicsModel.eventCallback = [&](SDL_Event e) {
            mEventSemantics.onEvent(e);
        };

        Settings &settings{Settings::getSettings()};

        auto appSize = settings.getValue(set::SetAppSize, defaultSize);
        auto appPos = settings.getValue(set::SetAppPosition, Position<int>{WINDOWPOS_UNDEFINED});
        mAppState = static_cast<EventSemantics::WindowEventType>(settings.getValue(set::SetAppState,
                                                                                   static_cast<uint>(EventSemantics::WindowEventType::Restored)));

        uint32_t extraFlags = 0;
        switch (mAppState) {
            case EventSemantics::Minimized:
                extraFlags |= SDL_WINDOW_MINIMIZED;
                break;
            case EventSemantics::Maximized:
                extraFlags |= SDL_WINDOW_MAXIMIZED;
                break;
            case EventSemantics::FullScreen:
                extraFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                break;
            case EventSemantics::Restored:
            default:
                break;
        }

        mGraphicsModel.initialize(title, appSize, appPos, extraFlags);
    }

    void Application::run() {
        layout();
        mGraphicsModel.eventLoop(mScreen);
    }

    void Application::layout() {
        for (auto &content : ReverseContainerView(*mScreen)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                auto windowRect = window->layout(mGraphicsModel.context(), mGraphicsModel.screenRectangle());
                window->setScreenRectangle(windowRect);
            }
        }
        mGraphicsModel.redrawBackground();
    }

    std::shared_ptr<Widget> Application::pointerWidget(const Position<int>& position) {
        for (auto &content : ReverseContainerView(*mScreen)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                auto windowRect = window->getScreenRectangle(Position<int>{});
                if (windowRect.contains(position)) {
                    return window->pointerWidget(position);
                } else if (window->isModal()) {
                    return nullptr;
                }
            }
        }

        return nullptr;
    }

    void EventSemantics::onEvent(SDL_Event &e) {
        switch (e.type) {
            case SDL_WINDOWEVENT:
                windowEvent(e.window);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                keyboardEvent(e.key);
                break;
            case SDL_TEXTINPUT:
                textInputEvent(e.text);
                break;
            case SDL_TEXTEDITING:
                std::cout << "TextEditingEvent\n";
                break;
            case SDL_MOUSEMOTION:
                mouseMotionEvent(e.motion);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseButtonEvent(e.button);
                break;
            case SDL_MOUSEWHEEL:
                mouseWheelEvent(e.wheel);
                break;
            case SDL_FINGERMOTION:
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
                fingerTouchEvent(e.tfinger);
                break;
            case SDL_MULTIGESTURE:
            case SDL_KEYMAPCHANGED:
                break;
            default:
                std::cout << "Unhandled event: 0x" << std::hex << e.type << std::dec << '\n';
        }
    }

    void EventSemantics::windowEvent(SDL_WindowEvent &e) {
        switch (e.event) {
            case SDL_WINDOWEVENT_SHOWN:
                windowStateChange(Shown);
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                windowStateChange(Hidden);
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                windowStateChange(Exposed);
                break;
            case SDL_WINDOWEVENT_MOVED:
                windowPositionChange(Moved, Position<int>{e.data1, e.data2});
                break;
            case SDL_WINDOWEVENT_RESIZED:
                windowSizeChange(Resized, Size{e.data1, e.data2});
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                windowStateChange(SizeChanged);
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                windowStateChange(Minimized);
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                windowStateChange(Maximized);
                break;
            case SDL_WINDOWEVENT_RESTORED:
                windowStateChange(Restored);
                break;
            case SDL_WINDOWEVENT_ENTER:
                windowStateChange(Enter);
                break;
            case SDL_WINDOWEVENT_LEAVE:
                windowStateChange(Leave);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                windowStateChange(Focus);
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                windowStateChange(UnFocus);
                break;
            case SDL_WINDOWEVENT_CLOSE:
                windowStateChange(Close);
                break;
            case SDL_WINDOWEVENT_TAKE_FOCUS:
            case SDL_WINDOWEVENT_HIT_TEST:
                break;
        }
    }

    void EventSemantics::keyboardEvent(SDL_KeyboardEvent &e) {
        if (keyboardEventCallback)
            if (keyboardEventCallback(mApplication, e))
                return;
    }

    void EventSemantics::mouseMotionEvent(SDL_MouseMotionEvent &e) {
        if (mouseMotionEventCallback)
            if (mouseMotionEventCallback(mApplication, e))
                return;
    }

    void EventSemantics::mouseButtonEvent(SDL_MouseButtonEvent &e) {
        if (mouseButtonEventCallback)
            if (mouseButtonEventCallback(mApplication, e))
                return;
    }

    void EventSemantics::mouseWheelEvent(SDL_MouseWheelEvent &e) {
        if (mouseWheelEventCallback)
            if (mouseWheelEventCallback(mApplication, e))
                return;
    }

    void EventSemantics::fingerTouchEvent(SDL_TouchFingerEvent &e) {
        if (fingerTouchEventCallback)
            fingerTouchEventCallback(mApplication, e);
    }

    void EventSemantics::textInputEvent(SDL_TextInputEvent &e) {
        if (textInputEventCallback)
            textInputEventCallback(mApplication, e);
    }
}
