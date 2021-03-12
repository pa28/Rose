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
                std::cout << "Shown";
                break;
            case EventSemantics::Hidden:
                std::cout << "Hidden";
                break;
            case EventSemantics::Exposed:
                layout();
                std::cout << "Exposed " << mGraphicsModel.windowBorders() << '\n';
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
        if (mAppState == EventSemantics::Restored) {
            Settings &settings{Settings::getSettings()};
            auto borders = windowBorders();
            position.x = std::max(position.x - borders.l, 0);
            position.y = std::max(position.y - borders.t, 0);
            settings.setValue(set::SetAppPosition, position);
        }
    }

    void Application::initialize(const std::string &title, Size defaultSize) {
        mEventSemantics.setWindowStateChangeCallback(&Application::windowStateChange);
        mEventSemantics.setWindowSizeChangeCallback(&Application::windowSizeChange);
        mEventSemantics.setWindowPositionChangeCallback(&Application::windowPositionChange);

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
                window->layout(mGraphicsModel.context(),
                               mGraphicsModel.displayBounds(mGraphicsModel.currentDisplayIndex()));
            }
        }
        mGraphicsModel.redrawBackground();
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
                break;
            case SDL_MULTIGESTURE:
                break;
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
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID
                  << ", state: " << (uint32_t) e.state << ", repeat: " << (uint32_t) e.repeat << '\n';
    }

    void EventSemantics::mouseMotionEvent(SDL_MouseMotionEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID << ", which: " << e.which
                  << ", state: " << (uint32_t) e.state
                  << ", pos: " << Position{e.x, e.y} << " rel: " << Position{e.xrel, e.yrel} << '\n';
    }

    void EventSemantics::mouseButtonEvent(SDL_MouseButtonEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID << ", which: " << e.which
                  << ", state: " << (uint32_t) e.state
                  << ", pos: " << Position{e.x, e.y} << '\n';
    }

    void EventSemantics::mouseWheelEvent(SDL_MouseWheelEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID << ", which: " << e.which
                  << ", direction: " << e.direction << ", pos: " << Position{e.x, e.y} << '\n';
    }
}
