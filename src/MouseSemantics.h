/**
 * @file MouseSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#pragma once

#include <SDL.h>
#include <optional>

namespace rose {

/**
 * @class MouseSemantics
 * @brief
 */
    class MouseSemantics {
    protected:
        std::optional<SDL_Event> mEventQue{};

    public:
        MouseSemantics() = default;
        ~MouseSemantics() = default;
        MouseSemantics(MouseSemantics &&) = delete;
        MouseSemantics(const MouseSemantics &) = delete;
        MouseSemantics& operator=(MouseSemantics &&) = delete;
        MouseSemantics& operator=(const MouseSemantics &) = delete;

        void onEvent(SDL_Event &event);

        void processEvent(SDL_Event &event);

        void flushFifo();

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
        void mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y, uint32_t direction);

        void mouseMotion(SDL_Event &event, int32_t x, int32_t y, int32_t relX, int32_t relY);

        void mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y);

        void fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                          float dx, float dy, float pressure);

        void fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                        float dx, float dy, float pressure);

        void fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                      float dx, float dy, float pressure);

        void multiGesture(SDL_Event &event, float dTheta, float dDist, float x, float y, uint16_t nFingers);

    };
}

