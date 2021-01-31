/**
 * @file EventSemantics.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#include "EventSemantics.h"

#include <iostream>
#include "Rose.h"

namespace rose {

    template<typename Arg, typename ... Args>
    std::ostream &print(std::ostream &strm, Arg &&arg, Args&& ... args) {
        strm << std::forward<Arg>(arg) << ':';
        ((strm << ' ' << std::forward<Args>(args)), ...);
        return strm;
    }

    EventSemantics::EventSemantics(Rose &rose) : mRose(rose) {}

    void EventSemantics::onEvent(SDL_Event &event) {
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
            case SDL_FINGERUP:
            case SDL_FINGERDOWN:
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
            case SDL_MULTIGESTURE:
                if (mEventQue) {
                    if (mEventQue->type != SDL_FINGERDOWN && mEventQue->type != SDL_FINGERUP && mEventQue->type != SDL_FINGERMOTION)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_TEXTINPUT:
                if (mEventQue) {
                    if (mEventQue->type != SDL_KEYDOWN && mEventQue->type != SDL_KEYUP)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            default:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                processEvent(event);
        }
    }

    void EventSemantics::flushFifo() {
        if (mEventQue) {
            processEvent(mEventQue.value());
            mEventQue.reset();
        }
    }

    void EventSemantics::processEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                mouseWheel(event.wheel.timestamp, event.wheel.windowID, event.wheel.which,
                                  event.wheel.x, event.wheel.y, event.wheel.direction == SDL_MOUSEWHEEL_NORMAL);
                break;
            case SDL_MOUSEMOTION:
                mouseMotion(event, event.motion.state, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseButton(event, event.button.button, event.button.state, event.button.clicks, event.button.x, event.button.y);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                keyEvent(event, event.key.state, event.key.repeat, event.key.keysym);
                break;
            case SDL_TEXTINPUT:
                textInputEvent(event, std::string{event.text.text});
                break;
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

    void EventSemantics::mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y,
                                    uint32_t direction) {
        print(std::cout, __FUNCTION__, timestamp, windowId, which, x, y, direction, '\n');
    }

    void
    EventSemantics::mouseMotion(SDL_Event &event, uint32_t state, int32_t x, int32_t y, int32_t relX, int32_t relY) {

        print(std::cout, __FUNCTION__, (event.motion.state ? "Drag" : "Move"), x, y, relX, relY, '\n');
    }

    void
    EventSemantics::mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y) {
        print( std::cout, __FUNCTION__, button, state, clicks, x, y, '\n');
    }

    void EventSemantics::fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                                      float dx, float dy, float pressure) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
    }

    void
    EventSemantics::fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                               float dy, float pressure) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
    }

    void
    EventSemantics::fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                             float dy, float pressure) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
    }

    void
    EventSemantics::multiGesture(SDL_Event &event, float dTheta, float dDist, float x, float y, uint16_t nFingers) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, dTheta, dDist, x, y, nFingers, '\n');
    }

    void EventSemantics::keyEvent(SDL_Event &event, uint state, uint repeat, SDL_Keysym keysym) {
        auto keyName = SDL_GetScancodeName(keysym.scancode);
        print(std::cout, __FUNCTION__, event.key.timestamp, state, repeat, keyName, '\n');
    }

    void EventSemantics::textInputEvent(SDL_Event &event, const string &text) {
        print(std::cout, __FUNCTION__, text, '\n');
    }
}
