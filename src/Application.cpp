/**
 * @file Application.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#include "Application.h"
#include "Settings.h"
#include "Utilities.h"

using namespace rose::gm;

namespace rose {

    Application::Application(int argc, char **argv) : mEventSemantics(*this), mInputParser(argc, argv) {
        mScreen = std::make_shared<Screen>();
    }

    void Application::windowStateChange(EventSemantics::WindowEventType type) {
        std::cout << __PRETTY_FUNCTION__ << ' ';
        Settings &settings{Settings::getSettings()};
        switch (type) {
            case EventSemantics::Shown:
                std::cout << "Shown" << mGraphicsModel.windowBorders();
                break;
            case EventSemantics::Hidden:
                std::cout << "Hidden";
                break;
            case EventSemantics::Exposed:
                layout();
                std::cout << "Exposed";
                break;
            case EventSemantics::Maximized:
                std::cout << "Maximized";
                settings.setValue(set::SetAppState, static_cast<int>(type));
                mAppState = type;
                break;
            case EventSemantics::Minimized:
                std::cout << "Minimized";
                settings.setValue(set::SetAppState, static_cast<int>(type));
                mAppState = type;
                break;
            case EventSemantics::Restored:
                std::cout << "Restored";
                settings.setValue(set::SetAppState, static_cast<int>(type));
                mAppState = type;
                break;
            case EventSemantics::Enter:
            case EventSemantics::Leave:
            case EventSemantics::Focus:
            case EventSemantics::UnFocus:
            case EventSemantics::Close:
                break;
            case EventSemantics::Moved:
                std::cout << "Moved";
                break;
            case EventSemantics::Resized:
                std::cout << "Resized";
                break;
            case EventSemantics::SizeChanged:
                std::cout << "SizeChanged";
                layout();
                break;
            default:
                std::cout << "Unknown";
                break;
        }
        std::cout << '\n';
    }

    void Application::windowSizeChange(EventSemantics::WindowEventType type, Size size) {
        if (mAppState == EventSemantics::Restored) {
            Settings &settings{Settings::getSettings()};
            settings.setValue(set::SetAppSize, size);
        }
    }

    void Application::windowPositionChange(EventSemantics::WindowEventType type, Position position) {
        std::cout << __PRETTY_FUNCTION__ << position << " Display: "
            << SDL_GetWindowDisplayIndex(getSdlWindow().get()) << '\n';
        if (mAppState == EventSemantics::Restored) {
            Settings &settings{Settings::getSettings()};
            auto borders = windowBorders();
            position.x = std::max(position.x - borders.l, 0);
            position.y = std::max(position.y - borders.t, 0);
            settings.setValue(set::SetAppPosition, position);
        }
    }

    bool Application::keyboardEventCallback(const SDL_KeyboardEvent &keyboardEvent) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << keyboardEvent.windowID
                  << ", state: " << (uint32_t) keyboardEvent.state << ", repeat: " << (uint32_t) keyboardEvent.repeat
                  << ' ' << SDL_GetKeyName(keyboardEvent.keysym.sym) << '\n';

        string keyName{SDL_GetKeyName(keyboardEvent.keysym.sym)};
        if (keyboardEvent.keysym.mod & (uint)KMOD_CTRL)
            switch (keyboardEvent.keysym.sym) {
                case SDLK_F1:
                    SDL_MinimizeWindow(getSdlWindow().get());
                    return true;
                case SDLK_F2:
                    SDL_SetWindowFullscreen(getSdlWindow().get(), 0);
                    SDL_RestoreWindow(getSdlWindow().get());
                    return true;
                case SDLK_F3:
                    if (SDL_GetWindowFlags(getSdlWindow().get()) & (uint)SDL_WINDOW_RESIZABLE) {
                        SDL_SetWindowFullscreen(getSdlWindow().get(), 0);
                        SDL_MaximizeWindow(getSdlWindow().get());
                    } else {
                        SDL_SetWindowFullscreen(getSdlWindow().get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    return true;
                case SDLK_F4:
                    SDL_SetWindowFullscreen(getSdlWindow().get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
                    return true;
                default:
                    break;
            }
        return false;
    }

    bool Application::mouseMotionEventCallback(const SDL_MouseMotionEvent &mouseMotionEvent) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << mouseMotionEvent.windowID << ", which: "
                  << mouseMotionEvent.which << ", state: " << (uint32_t) mouseMotionEvent.state
                  << ", pos: " << Position{mouseMotionEvent.x, mouseMotionEvent.y} << " rel: "
                  << Position{mouseMotionEvent.xrel, mouseMotionEvent.yrel} << '\n';
        return false;
    }

    bool Application::mouseButtonEventCallback(const SDL_MouseButtonEvent &mouseButtonEvent) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << mouseButtonEvent.windowID << ", which: "
                  << mouseButtonEvent.which << ", state: " << (uint32_t) mouseButtonEvent.state
                  << ", pos: " << Position{mouseButtonEvent.x, mouseButtonEvent.y} << '\n';
        SemanticGesture semanticGesture{SemanticGesture::Click | SemanticGesture::Drag | SemanticGesture::Key};
        if (mouseButtonEvent.state) {
            auto focus = focusWidget(semanticGesture, Position{mouseButtonEvent.x, mouseButtonEvent.y});
            if (focus)
                setFocusWidget(focus, SemanticGesture::Click | SemanticGesture::Drag | SemanticGesture::Key);
            else
                clearFocusWidget(focus, SemanticGesture::Click | SemanticGesture::Drag);
        }
        return false;
    }

    bool Application::mouseWheelEventCallback(const SDL_MouseWheelEvent &mouseWheelEvent) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << mouseWheelEvent.windowID << ", which: "
                  << mouseWheelEvent.which << ", direction: " << mouseWheelEvent.direction << ", pos: "
                  << Position{mouseWheelEvent.x, mouseWheelEvent.y} << '\n';
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

        mGraphicsModel.eventCallback = [&](SDL_Event e) {
            mEventSemantics.onEvent(e);
        };

        Settings &settings{Settings::getSettings()};

        auto appSize = settings.getValue(set::SetAppSize, defaultSize);
        auto appPos = settings.getValue(set::SetAppPosition, Position::Undefined);
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

    std::shared_ptr<Widget> Application::focusWidget(SemanticGesture gesture, const Position &position) {
        for (auto &content : ReverseContainerView(*mScreen)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                if (window->getScreenRectangle(Position::Zero).contains(position)) {
                    return window->focusWidget(gesture, position, Position::Zero);
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
                std::cout << "TextInputEvent\n";
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
//                break;
            case SDL_MULTIGESTURE:
//                break;
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
                windowPositionChange(Moved, Position{e.data1, e.data2});
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
}
