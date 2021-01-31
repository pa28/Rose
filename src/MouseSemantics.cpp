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

    void MouseSemantics::onEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                if (mEventQue)
                    processEvent(mEventQue.value());
                mEventQue.reset();
                processEvent(event);
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state) {
                    // Drag event
                    if (mEventQue) {
                        if (mEventQue->type == SDL_MOUSEBUTTONDOWN || mEventQue->type == SDL_MOUSEBUTTONUP)
                            mEventQue.reset();
                        else
                            processEvent(mEventQue.value());
                    }
                    processEvent(event);
                } else {
                    if (mEventQue) {
                        processEvent(mEventQue.value());
                        mEventQue.reset();
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_FINGERMOTION:
                if (mEventQue) {
                    if (mEventQue->type != SDL_FINGERDOWN && mEventQue->type != SDL_FINGERUP)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_FINGERUP:
            case SDL_FINGERDOWN:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_MULTIGESTURE:
                if (mEventQue) {
                    if (mEventQue->type != SDL_FINGERDOWN && mEventQue->type != SDL_FINGERUP && mEventQue->type != SDL_FINGERMOTION)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
        }
    }

    void MouseSemantics::flushFifo() {
        if (mEventQue) {
            processEvent(mEventQue.value());
            mEventQue.reset();
        }
    }

    void MouseSemantics::processEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                mouseWheel(event.wheel.timestamp, event.wheel.windowID, event.wheel.which,
                                  event.wheel.x, event.wheel.y, event.wheel.direction == SDL_MOUSEWHEEL_NORMAL);
                break;
            case SDL_MOUSEMOTION:
                mouseMotion(event, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseButton(event, event.button.button, event.button.state, event.button.clicks, event.button.x, event.button.y);
                break;
//            case SDL_KEYDOWN:
//            case SDL_KEYUP:
//            case SDL_TEXTINPUT:
            case SDL_FINGERMOTION:
                fingerMotion(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
                break;
            case SDL_FINGERDOWN:
                fingerDown(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
                break;
            case SDL_FINGERUP:
                fingerUp(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
                break;
            case SDL_MULTIGESTURE:
                multiGesture(event, event.mgesture.dTheta, event.mgesture.dDist, event.mgesture.x, event.mgesture.y,
                                    event.mgesture.numFingers);
                break;
        }
    }

    void MouseSemantics::mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y,
                                    uint32_t direction) {
        print(std::cout, __FUNCTION__, timestamp, windowId, which, x, y, direction, '\n');
    }

    void MouseSemantics::mouseMotion(SDL_Event &event, int32_t x, int32_t y, int32_t relX, int32_t relY) {

        print(std::cout, __FUNCTION__, (event.motion.state ? "Drag" : "Move"), x, y, relX, relY, '\n');
    }

    void
    MouseSemantics::mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y) {
        print( std::cout, __FUNCTION__, button, state, clicks, x, y, '\n');
    }

    void MouseSemantics::fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                                      float dx, float dy, float pressure) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
    }

    void
    MouseSemantics::fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                               float dy, float pressure) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
    }

    void
    MouseSemantics::fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                             float dy, float pressure) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
    }

    void
    MouseSemantics::multiGesture(SDL_Event &event, float dTheta, float dDist, float x, float y, uint16_t nFingers) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, dTheta, dDist, x, y, nFingers, '\n');
    }
}
