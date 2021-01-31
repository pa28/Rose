/**
 * @file MouseSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#pragma once

#include <SDL.h>

namespace rose {

/**
 * @class MouseSemantics
 * @brief
 */
    class MouseSemantics {
    protected:

    public:
        MouseSemantics() = default;
        ~MouseSemantics() = default;
        MouseSemantics(MouseSemantics &&) = delete;
        MouseSemantics(const MouseSemantics &) = delete;
        MouseSemantics& operator=(MouseSemantics &&) = delete;
        MouseSemantics& operator=(const MouseSemantics &) = delete;

        bool onEvent(SDL_Event &event);

        /**
         * @brief Mouse Wheel event
         * @param timestamp timestamp of the event
         * @param windowId the window with mouse focus, if any
         * @param which the mouse instance id, or SDL_TOUCH_MOUSEID; see Remarks for details
         * @param x the amount scrolled horizontally, positive to the right and negative to the left
         * @param y the amount scrolled vertically, positive away from the user and negative towards the user
         * @param direction SDL_MOUSEWHEEL_NORMAL or SDL_MOUSEWHEEL_FLIPPED
         * @return true if event processed.
         */
        bool mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y, uint32_t direction);

        bool mouseMotion(SDL_Event &event, int32_t x, int32_t y, int32_t relX, int32_t relY);

        bool mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y);

        bool fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                          float dx, float dy, float pressure);

        bool fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                          float dx, float dy, float pressure);

        bool fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                          float dx, float dy, float pressure);

        bool multiGesture(SDL_Event &event, float dTheta, float dDist, float x, float y, uint16_t nFingers);
    };
}

