/**
 * @file TimeBox.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-19
 */

#pragma once

#include <utility>
#include <atomic>

#include "Layout.h"
#include "Signals.h"
#include "TimerTick.h"
#include "LocalTime.h"

namespace rose {

    /**
     * @class TimeBox
     * @brief
     */
    class TimeBox : public Manager {
    protected:
        static constexpr std::string_view HoursMinutesFmt = "%R";
        static constexpr std::string_view LongSecondsFmt = "%S %Z";
        static constexpr std::string_view ShortSecondsFmt = "%Z";

        std::unique_ptr<cpp_local_time::LocalTime> mLocalTimeConvert{};
        std::atomic_bool mUpdateTimeDisplay{false};

        std::shared_ptr<TimerTick> mTimerTick{};      ///< The source of time signals.
        bool mDisplaySeconds{true}; ///< Display seconds in time.
        bool mLocalTime{false};     ///< If time zone is empty display local time if true.
        std::string mTimeZone{};    ///< The time zone if not empty.

        TickProtocol::slot_type secondSlot{};   ///< Receive time signals.
        const std::time_put<char> &locale_time_put;

        void initialize();

        /**
         * A helper function to convert the system time to a localized time
         * @param strm The stream to output time, that is imbued with the locale
         * @param fill The fill char
         * @param tm The std::tm struct with the desired time.
         * @param fmt A format string (see std::locale_time_put::put()
         * @return an iterator one past the last char generated.
         */
        auto put_locale_time(std::stringstream &strm, char fill, const std::tm *tm, const std::string_view &format) {
            auto fmt = std::string{format};
            if (fmt.empty()) {
                throw std::runtime_error("Format must not be empty");
            } else {
                const char *fmtbeg = fmt.c_str();
                const char *fmtend = fmtbeg + fmt.size();
                return locale_time_put.put(strm, strm, fill, tm, fmtbeg, fmtend);
            }
        }

        void updateTimeDisplay();

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

        TimeBox(std::shared_ptr<TimerTick> timerTick, const char *timeZone, bool seconds = true) : TimeBox(std::move(timerTick)) {
            mDisplaySeconds = seconds;
            mTimeZone = timeZone;
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

    class DateBox : public Manager {
    protected:
        static constexpr std::string_view LongDateFormat = "%a %b %d, %Y";
        static constexpr std::string_view ShortDateFormat = "%a %b %d";

        bool mDisplayYear{true};
        bool mLocalDate{false};
        std::string mTimeZone{};    ///< The time zone if not empty.
        std::shared_ptr<TimerTick> mTimerTick{};      ///< The source of time signals.

        TickProtocol::slot_type minuteSlot{};   ///< Receive time signals.
        const std::time_put<char> &locale_time_put;

        void initialize();

        /**
         * A helper function to convert the system time to a localized time
         * @param strm The stream to output time, that is imbued with the locale
         * @param fill The fill char
         * @param tm The std::tm struct with the desired time.
         * @param fmt A format string (see std::locale_time_put::put()
         * @return an iterator one past the last char generated.
         */
        auto put_locale_time(std::stringstream &strm, char fill, const std::tm *tm, const std::string_view &format) {
            auto fmt = std::string{format};
            if (fmt.empty()) {
                throw std::runtime_error("Format must not be empty");
            } else {
                const char *fmtbeg = fmt.c_str();
                const char *fmtend = fmtbeg + fmt.size();
                return locale_time_put.put(strm, strm, fill, tm, fmtbeg, fmtend);
            }
        }

        void updateDateDisplay();

    public:
        DateBox() = delete;

        ~DateBox() override = default;

        DateBox(const DateBox&) = delete;

        DateBox(DateBox&&) = delete;

        DateBox& operator=(const DateBox&) = delete;

        DateBox& operator=(DateBox&&) = delete;

        explicit DateBox(std::shared_ptr<TimerTick> minutes);

        DateBox(std::shared_ptr<TimerTick> timerTick, bool year, bool localTime = false) : DateBox(std::move(timerTick)) {
            mDisplayYear = year;
            mLocalDate = localTime;
        }

        DateBox(std::shared_ptr<TimerTick> timerTick, const std::string &timeZone, bool year = true) : DateBox(std::move(timerTick)) {
            mDisplayYear = year;
            mTimeZone = timeZone;
        }

        /**
         * @brief Draw the DateBox and contents.
         * @param context The graphics context used to draw the manager and contents.
         * @param containerPosition The Position of the Container that holds the Manager.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;

        /**
         * @brief Layout the DateBox and contents.
         * @param context The context that will be used to draw the Manager and contents.
         * @param screenRect The screen rectangle available to the Manager.
         * @return The rectangle occupied by the Manager.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;
    };
}

