/** @file TimeBox.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-25
  * @brief Widgets for displaying the current Time and Date.
  */


#pragma once

#include "Label.h"
#include "Signals.h"
#include "Timer.h"
#include <sstream>

namespace rose {

    /**
     * @class TimeBox
     * @brief A widget to display the current time.
     */
    class TimeBox : public Container {
    protected:
        std::shared_ptr<Label> mHoursMinutes;   ///< Display hours and minutes
        std::shared_ptr<Label> mSeconds;        ///< Display seconds and time zone
        SecondTickPtr mSecondTick;              ///< SecondTick timing source

        bool mIsLocalTime{};          ///< If true, display local time, otherwise GMT
        bool mIsSmall{};            ///< Display a small time display.

        void timeCallback();        ///< Called when a second tick signal arrives

        const std::time_put<char> &locale_time_put;     ///< A time formatter.

        /**
         * A helper function to convert the system time to a localized time
         * @param strm The stream to output time, that is imbued with the locale
         * @param fill The fill char
         * @param tm The std::tm struct with the desired time.
         * @param fmt A format string (see std::locale_time_put::put()
         * @return an iterator one past the last char generated.
         */
        auto put_locale_time(std::stringstream &strm, char fill, const std::tm *tm, const std::string &fmt) {
            if (fmt.empty()) {
                throw std::runtime_error("Format must not be empty");
            } else {
                const char *fmtbeg = fmt.c_str();
                const char *fmtend = fmtbeg + fmt.size();
                return locale_time_put.put(strm, strm, fill, tm, fmtbeg, fmtend);
            }
        }

    public:
        TimeBox() = delete;
        ~TimeBox() override = default;
        TimeBox(TimeBox &&) = delete;
        TimeBox(const TimeBox &) = delete;
        TimeBox& operator=(TimeBox &&) = delete;
        TimeBox& operator=(const TimeBox &) = delete;

        /**
         * @brief Constructor. See rose::Widget::Widget(Rose &rose, Widget *parent)
         * @param secondTick A pointer to the SecondTick object from which timing will be received.
         * @param small if true display the small version of the time.
         */
        explicit TimeBox(SecondTickPtr secondTick, bool small = false);

        void initializeComposite() override;    ///< See Widget::initializeComposit().

        std::shared_ptr<Slot<int>> rxSecond;    ///< Receive second timing on this Slot.

        /// See Widget::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /// See Widget::widgetLayout()
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /**
         * @brief Set the TimeBox to display local or GMT time
         * @param local if true display local time
         */
        void setLocalTime(bool local) {
            mIsLocalTime = local;
            timeCallback();
        }

        /**
         * @brief Set the TimeBox to display local or GMT time and return a rose::std::shared_ptr<TimeBox>{this}
         * @param local if true display local time
         * @return rose::std::shared_ptr<TimeBox>{this}
         */
        std::shared_ptr<TimeBox> withLocalTime(bool local) {
            setLocalTime(local);
            return getWidget<TimeBox>();
        }

        void setFontSize(int fontSize) {
            mHoursMinutes->setFontSize(fontSize);
            mSeconds->setFontSize(2*fontSize/3);
        }
    };

    /**
     * @class DateBox
     * @brief A widget to display the current date.
     */
    class DateBox : public Label {
    protected:
        bool mIsLocalTime{};        ///< If true, display local time, otherwise GMT
        bool mIsSmall{};            ///< Display a small time display.
        SecondTickPtr mSecondTick;  ///< The SecondTick time source.

        void timeCallback();        ///< Called when a second tick signal arrives

        const std::time_put<char> &locale_time_put; ///< A time formatter.

        /**
         * A helper function to convert the system time to a localized time
         * @param strm The stream to output time, that is imbued with the locale
         * @param fill The fill char
         * @param tm The std::tm struct with the desired time.
         * @param fmt A format string (see std::locale_time_put::put()
         * @return an iterator one past the last char generated.
         */
        auto put_locale_time(std::stringstream &strm, char fill, const std::tm *tm, const std::string &fmt) {
            if (fmt.empty()) {
                throw std::runtime_error("Format must not be empty");
            } else {
                const char *fmtbeg = fmt.c_str();
                const char *fmtend = fmtbeg + fmt.size();
                return locale_time_put.put(strm, strm, fill, tm, fmtbeg, fmtend);
            }
        }

    public:
        DateBox() = delete;
        ~DateBox() override = default;
        DateBox(DateBox &&) = delete;
        DateBox(const DateBox &) = delete;
        DateBox& operator=(DateBox &&) = delete;
        DateBox& operator=(const DateBox &) = delete;

        /**
         * @brief Constructor. See rose::Widget::Widget(Rose &rose, Widget *parent)
         * @param secondTick A pointer to the SecondTick timing object.
         * @param small if true display the small version of the date.
         */
        explicit DateBox(SecondTickPtr secondTick, bool small = false);

        /**
         * @brief See Widget::initializeComposite.
         */
        void initializeComposite() override;

        /**
         * @brief Perform initial layout of the widget
         * @param renderer
         * @return the Widget LayoutPack
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override {
            timeCallback();
            Rectangle widgetRect{available};
            if (mPos)
                widgetRect = widgetRect.moveOrigin(mPos.value());
            return Label::widgetLayout(renderer, widgetRect, 0);
        }

        std::shared_ptr<Slot<int>> rxHour;      ///< Receive the hour timing on this Slot.

        /**
         * @brief Set the DateBox to display local or GMT date.
         * @param local if true display local date.
         */
        void setLocalTime(bool local) {
            mIsLocalTime = local;
            timeCallback();
        }

        /**
         * @brief Set the DateBox to display local or GMT date and return a rose::std::shared_ptr<DateBox>{this}
         * @param local if true display local date
         * @return rose::std::shared_ptr<DateBox>{this}
         */
        std::shared_ptr<DateBox> withLocalTime(bool local) {
            setLocalTime(local);
            return getWidget<DateBox>();
        }
    };
}



