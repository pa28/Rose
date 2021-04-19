/**
 * @file TimerTick.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-19
 */

#include "TimerTick.h"
#include <chrono>

namespace rose {

    namespace ch = std::chrono;

    TimerTick::TimerTick() {
        sdlTimerId = SDL_AddTimer(1000, TimerTick::TimerCallbackStub, this);
    }

    TimerTick::~TimerTick() {
        SDL_RemoveTimer(sdlTimerId);
    }

    uint32_t TimerTick::transmit(uint32_t interval) {
        auto now = ch::system_clock::now();

        int seconds = (int) ch::duration_cast<ch::seconds>(now.time_since_epoch()).count() % 60;
        if (secondSignal)
            secondSignal.transmit(seconds);

        if (seconds == 0) {
            int minutes = (int) ch::duration_cast<ch::minutes>(now.time_since_epoch()).count() % 60;
            if (minuteSignal)
                minuteSignal.transmit(minutes);

            if (minutes == 0) {
                int hours = (int) ch::duration_cast<ch::hours>(now.time_since_epoch()).count() % 24;
                if (hourSignal)
                    hourSignal.transmit(hours);

                if (hours == 0) {
                    int days = (int) ch::duration_cast<ch::hours>(now.time_since_epoch()).count() / 24;
                    if (daySignal)
                        daySignal.transmit(days);
                }
            }
        }

        // Shift timing to synchronize with system clock.
        return 1005 - ch::duration_cast<ch::milliseconds>(now.time_since_epoch()).count() % 1000;
    }
}
