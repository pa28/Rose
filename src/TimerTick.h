/**
 * @file TimerTick.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-19
 */

#pragma once

#include "Signals.h"
#include <SDL.h>

namespace rose {

    using TickProtocol = Protocol<int>;

    /**
     * @class TimerTick
     * @brief Transmit regular signals synchronized to the system clock.
     */
    class TimerTick {
    protected:
        /// Called when the SDL timer expires.
        uint32_t transmit(uint32_t interval);

        SDL_TimerID sdlTimerId;     ///< The SDL_TimerID from the SDL API.

    public:
        TimerTick();

        ~TimerTick();

        TimerTick(const TimerTick&) = delete;

        TimerTick(TimerTick &&) = delete;

        TimerTick& operator=(const TimerTick&) = delete;

        TimerTick& operator=(TimerTick&&) = delete;

        TickProtocol::signal_type secondSignal{};

        TickProtocol::signal_type minuteSignal{};

        TickProtocol::signal_type hourSignal{};

        TickProtocol::signal_type daySignal{};

        /**
         * The static function passed to SDL_AddTimer as the callback
         * @param interval the current interval
         * @param param the pointer to this Timer
         * @return the next interval or 0 to stop the timer
         */
        static uint32_t TimerCallbackStub(uint32_t interval, void *param) {
            auto timer = static_cast<TimerTick *>(param);
            return timer->transmit(interval);
        }
    };
}

