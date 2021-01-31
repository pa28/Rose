/**
 * @file MouseSemantics.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#include "MouseSemantics.h"

#include <iostream>

namespace rose {

    template<typename Arg, typename ... Args>
    std::ostream &print(std::ostream &strm, Arg &&arg, Args&& ... args) {
        strm << std::forward<Arg>(arg) << ':';
        ((strm << ' ' << std::forward<Args>(args)), ...);
        return strm;
    }

    bool MouseSemantics::onEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                return mouseWheel(event.wheel.timestamp, event.wheel.windowID, event.wheel.which,
                                  event.wheel.x, event.wheel.y, event.wheel.direction == SDL_MOUSEWHEEL_NORMAL);
            case SDL_MOUSEMOTION:
                return mouseMotion(event, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                return mouseButton(event, event.button.button, event.button.state, event.button.clicks, event.button.x, event.button.y);
//            case SDL_KEYDOWN:
//            case SDL_KEYUP:
//            case SDL_TEXTINPUT:
            case SDL_FINGERMOTION:
                return fingerMotion(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
            case SDL_FINGERDOWN:
                return fingerDown(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
            case SDL_FINGERUP:
                return fingerUp(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
//            case SDL_MULTIGESTURE:
        }
        return false;
    }

    bool MouseSemantics::mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y,
                                    uint32_t direction) {
        print(std::cout, __FUNCTION__, timestamp, windowId, which, x, y, direction, '\n');
        return false;
    }

    bool MouseSemantics::mouseMotion(SDL_Event &event, int32_t x, int32_t y, int32_t relX, int32_t relY) {

        print(std::cout, __FUNCTION__, (event.motion.state ? "Drag" : "Move"), x, y, relX, relY, '\n');
        return false;
    }

    bool
    MouseSemantics::mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y) {
        print( std::cout, __FUNCTION__, button, state, clicks, x, y, '\n');
        return false;
    }

    bool MouseSemantics::fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                                      float dx, float dy, float pressure) {
        print( std::cout, __FUNCTION__, touchId, fingerId, x, y, dx, dy, pressure, '\n');
        return false;
    }

    bool
    MouseSemantics::fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                               float dy, float pressure) {
        print( std::cout, __FUNCTION__, touchId, fingerId, x, y, dx, dy, pressure, '\n');
        return false;
    }

    bool
    MouseSemantics::fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                             float dy, float pressure) {
        print( std::cout, __FUNCTION__, touchId, fingerId, x, y, dx, dy, pressure, '\n');
        return false;
    }
}
