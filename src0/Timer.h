/** @file Timer.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-25
  * @brief Objects to provide timing signals for Rose applications.
  */

#pragma once

#include <SDL2/SDL.h>
#include "Signals.h"

namespace rose {
    /**
     * @class Timer
     * @brief A helper class to provide an interval timer callback to a client class.
     * @see SDL_AddTimer
     *
     */
    class Timer {
    protected:
        SignalSerialNumber mSignalSerialNumber{};   ///< The serial number for preventing Signal loops.

    public:
        SDL_TimerID sdlTimerId;     ///< The SDL_TimerID from the SDL API.

        /**
         * The static function passed to SDL_AddTimer as the callback
         * @param interval the current interval
         * @param param the pointer to this Timer
         * @return the next interval or 0 to stop the timer
         */
        static Uint32 TimerCallbackStub(Uint32 interval, void *param) {
            auto timer = static_cast<Timer *>(param);
            timer->timerTick.transmit(timer->mSignalSerialNumber.serialNumber(), interval);
            return timer->timerEvent(interval);
        }

        Timer() = delete;   ///< Deleted default constructor.

        Timer(const Timer &) = delete;  ///< Deleted const copy constructor.

        Timer(Timer &) = delete;    ///< Deleted copy constructor.

        Signal<uint32_t> timerTick{};  ///< The tick Signal. Transmitted when the timer expires.

        /**
         * @brief Set a new timer interval.
         * @param interval The new interval.
         */
        void setInterval(Uint32 interval);

        /**
         * Crate the Timer
         * @param interval the requested interval
         */
        explicit Timer(Uint32 interval);

        /**
         * Remove the SDLTimer when destroyed.
         */
        virtual ~Timer() {
            SDL_RemoveTimer(sdlTimerId);
        }

        /**
         * @brief Called when the timer expires. The return value is the next interval of the timer.
         * @param interval
         * @return the new interval
         */
        virtual Uint32 timerEvent(Uint32 interval);
    };

    /**
     * @class SecondTick
     * @brief A class to provide signals on every Second, Minute, Hour, and Day to trigger and synchronize
     * periodic events.
     */
    class SecondTick : public Timer {
    protected:
        bool tickToggle;    ///< The flag that keeps track of the value sent by txToggle.

    public:
        SecondTick();

        ~SecondTick() override = default;

        /**
         * @brief Called when the timer expires. The return value is comuted to synchronize the next
         * expiration with the one second tick of the system clock.
         * @param interval
         * @return next interval
         */
        Uint32 timerEvent(Uint32 interval) override;

        /// Transmit a boolean that alternates between true and false in one second intervals
        Signal<bool> txToggle;

        /// Transmit a signal, the second of the minute, every second.
        Signal<int> txSecond;

        /// Transmit a signal, the minute of the hour, every minute.
        Signal<int> txMinute;

        /// Transmit a signal, the hour of the day, every hour.
        Signal<int> txHour;

        /// Transmit a signal, the number of days since the epoch, every day.
        Signal<int> txDay;
    };

    using SecondTickPtr = std::shared_ptr<SecondTick>;      ///< Typedef for smart pointer to SecondTick
}



