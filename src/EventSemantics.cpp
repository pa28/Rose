/**
 * @file EventSemantics.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-06
 */

#include <iostream>
#include "EventSemantics.h"
#include "Visual.h"

namespace rose {

#if GRAPHICS_MODEL_SDL2

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
                windowPositionChange(Moved,Position{e.data1,e.data2});
                break;
            case SDL_WINDOWEVENT_RESIZED:
                windowSizeChange(Resized,Size{e.data1,e.data2});
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

#endif

}
