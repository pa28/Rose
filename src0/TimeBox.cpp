//
// Created by richard on 2020-10-25.
//

#include "TimeBox.h"
#include "Container.h"
#include <sstream>
#include <locale>
#include <utility>

namespace rose {

    TimeBox::TimeBox(SecondTickPtr secondTick, bool small, bool localTime)
        : Container(), mIsSmall(small), mIsLocalTime(localTime), mSecondTick(std::move(secondTick)),
        locale_time_put( std::use_facet<std::time_put<char>>( std::locale())) {
    }

    void TimeBox::initializeComposite() {
        mClassName = "Timebox";
        rxSecond = std::make_shared<Slot<int>>();
        rxSecond->setCallback([&](uint32_t sn, int) {
            this->timeCallback();
        });

        mHoursMinutes = add<Label>()
                ->withFontName(Widget::rose()->theme().mTimeBoxTimeFont)
                ->withText("00:00")
                ->withFontSize(30);

        mSeconds = add<Label>()
                ->withFontName(Widget::rose()->theme().mTimeBoxTimeFont)
                ->withText("00 GMT")
                ->withFontSize(20);

        if (mIsSmall)
            mSecondTick->txMinute.connect(rxSecond);
        else
            mSecondTick->txSecond.connect(rxSecond);

        timeCallback();
    }

    void TimeBox::timeCallback() {
        using namespace std::chrono;

        auto now = std::chrono::system_clock::now();
        auto tt = system_clock::to_time_t(now);
        auto tm = mIsLocalTime ? *localtime(&tt) : *gmtime(&tt);

        std::stringstream hm;
        hm.imbue(std::locale());

        put_locale_time(hm, ' ', &tm, std::string{rose()->theme().mTimeBoxHoursMinFmt});
        mHoursMinutes->setText(hm.str());

        hm.str("");
        put_locale_time(hm, ' ', &tm,
                        std::string{mIsSmall ? rose()->theme().mTimeBoxSmallSecFmt : rose()->theme().mTimeBoxSecFmt});
        mSeconds->setText(hm.str());
    }

    void TimeBox::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (!mVisible)
            return;

        auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);;
        mHoursMinutes->draw(renderer, widgetRect);
        mSeconds->draw(renderer, widgetRect);
    }

    Rectangle TimeBox::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        int posX = 0;
        int posY = 0;
        for (auto &child : mChildren) {
            auto size = child->widgetLayout(renderer, available, 0);
            child->layoutHints().mAssignedRect = size;
            posX += size.width();
        }

        if (!mLayoutHints.mAssignedRect)
            mLayoutHints.mAssignedRect = Rectangle{};

        mLayoutHints.mAssignedRect.value() = mHoursMinutes->layoutHints().mAssignedRect->getSize();
        mLayoutHints.mAssignedRect->width() += mSeconds->layoutHints().mAssignedRect->width();


        auto[hminx, hmaxx, hminy, hmaxy, hadvance] = mHoursMinutes->getGlyphMetrics('0');
        auto[sminx, smaxx, sminy, smaxy, sadvance] = mSeconds->getGlyphMetrics('0');

        auto[hAscent, hHeight, hDescent, hLineSkip] = mHoursMinutes->getFontMetrics();
        auto[sAscent, sHeight, sDescent, sLineSkip] = mSeconds->getFontMetrics();

        auto x = hadvance - hmaxx + sminx;
        auto b = (sAscent - smaxx);
        auto y = abs(abs(hHeight - sHeight) - abs(hAscent - sAscent));

        mSeconds->layoutHints().mAssignedRect->x() += x + mHoursMinutes->layoutHints().mAssignedRect->width();
        mSeconds->layoutHints().mAssignedRect->y() += y;
        mLayoutHints.mAssignedRect->width() += x;

        return mLayoutHints.mAssignedRect.value();
    }

    DateBox::DateBox(SecondTickPtr secondTick, bool small, bool localTime)
        : Label(), mIsSmall(small), mIsLocalTime(localTime), mSecondTick(std::move(secondTick)),
        locale_time_put(std::use_facet<std::time_put<char>>(std::locale())) {
    }

    void DateBox::initializeComposite() {
        Label::initializeComposite();
        rxHour = std::make_shared<Slot<int>>();
        rxHour->setCallback([&](uint32_t sn, int) {
            this->timeCallback();
        });

        withFontName(Widget::rose()->theme().mDateBoxFont);
        withFontSize(Widget::rose()->theme().mDateBoxFontSize);

        mSecondTick->txMinute.connect(rxHour);
    }

    void DateBox::timeCallback() {
        using namespace std::chrono;

        auto now = std::chrono::system_clock::now();
        auto tt = system_clock::to_time_t(now);
        auto tm = mIsLocalTime ? *localtime(&tt) : *gmtime(&tt);

        std::stringstream hm;
        hm.imbue(std::locale());

        put_locale_time(hm, ' ', &tm,
                        std::string{mIsSmall ? rose()->theme().mTimeBoxSmallDateFmt : rose()->theme().mTimeBoxDateFmt});
        setText(hm.str());
    }
}
