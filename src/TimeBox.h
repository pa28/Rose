/**
 * @file TimeBox.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-19
 */

#pragma once

#include <utility>

#include "Layout.h"
#include "Signals.h"
#include "TimerTick.h"

namespace rose {

    /**
     * @class TimeBox
     * @brief
     */
    class TimeBox : public Manager {
    protected:
        std::shared_ptr<TimerTick> mTimerTick{};      ///< The source of time signals.
        bool mDisplaySeconds{true}; ///< Display seconds in time.
        bool mLocalTime{false};     ///< If time zone is empty display local time if true.
        std::string mTimeZone{};    ///< The time zone if not empty.

        TickProtocol::slot_type secondSlot{};   ///< Receive time signals.

        void initialize();

    public:
        TimeBox() = delete;

        ~TimeBox() override = default;

        TimeBox(const TimeBox&) = delete;

        TimeBox(TimeBox&&) = delete;

        TimeBox& operator=(const TimeBox&) = delete;

        TimeBox& operator=(TimeBox&&) = delete;

        explicit TimeBox(std::shared_ptr<TimerTick> timerTick);

        TimeBox(std::shared_ptr<TimerTick> timerTick, bool seconds, bool localTime = false) : TimeBox(std::move(timerTick)) {
            mDisplaySeconds = seconds;
            mLocalTime = localTime;
        }

        TimeBox(std::shared_ptr<TimerTick> timerTick, const std::string &timeZone, bool seconds = true) : TimeBox(std::move(timerTick)) {
            mDisplaySeconds = seconds;
            mTimeZone = timeZone;
        }

        /**
         * @brief Draw the TimeBox and contents.
         * @param context The graphics context used to draw the manager and contents.
         * @param containerPosition The Position of the Container that holds the Manager.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;

        /**
         * @brief Layout the TimeBox and contents.
         * @param context The context that will be used to draw the Manager and contents.
         * @param screenRect The screen rectangle available to the Manager.
         * @return The rectangle occupied by the Manager.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;
    };
}

