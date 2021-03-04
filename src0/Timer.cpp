//
// Created by richard on 2020-10-25.
//

#include "Timer.h"
#include <chrono>

namespace rose {
    namespace ch = std::chrono;

    Timer::Timer(Uint32 interval) {
        sdlTimerId = SDL_AddTimer(interval, Timer::TimerCallbackStub, this);
    }

    void Timer::setInterval(Uint32 interval) {
        SDL_RemoveTimer(sdlTimerId);
        sdlTimerId = SDL_AddTimer(interval, Timer::TimerCallbackStub, this);
    }

    Uint32 Timer::timerEvent(Uint32 interval) {
        return interval;
    }

    SecondTick::SecondTick() : Timer(1000) {
        tickToggle = false;
    }

    Uint32 SecondTick::timerEvent(Uint32 interval) {
        auto now = ch::system_clock::now();

        if (txToggle)
            txToggle.transmit(mSignalSerialNumber.serialNumber(), tickToggle);

        tickToggle = !tickToggle;

        int seconds = (int) ch::duration_cast<ch::seconds>(now.time_since_epoch()).count() % 60;
        if (txSecond)
            txSecond.transmit(mSignalSerialNumber.serialNumber(), seconds);

        if (seconds == 0) {
            int minutes = (int) ch::duration_cast<ch::minutes>(now.time_since_epoch()).count() % 60;
            if (txMinute)
                txMinute.transmit(mSignalSerialNumber.serialNumber(), minutes);

            if (minutes == 0) {
                int hours = (int) ch::duration_cast<ch::hours>(now.time_since_epoch()).count() % 24;
                if (txHour)
                    txHour.transmit(mSignalSerialNumber.serialNumber(), hours);

                if (hours == 0) {
                    int days = (int) ch::duration_cast<ch::hours>(now.time_since_epoch()).count() / 24;
                    if (txDay)
                        txDay.transmit(mSignalSerialNumber.serialNumber(), days);
                }
            }
        }
        return 1005 - ch::duration_cast<ch::milliseconds>(now.time_since_epoch()).count() % 1000;
    }
}
