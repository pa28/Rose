/**
 * @file TimeBox.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-19
 */

#include "TimeBox.h"

#include <utility>
#include <chrono>
#include "Button.h"
#include "Application.h"

namespace rose {
    namespace ch = std::chrono;

    TimeBox::TimeBox(std::shared_ptr<TimerTick> timerTick) : mTimerTick(std::move(timerTick)),
             locale_time_put(std::use_facet<std::time_put<char>>(std::locale())) {
        secondSlot = TickProtocol::createSlot();
        secondSlot->receiver = [&](int seconds) {
            if (mDisplaySeconds || seconds == 0) {
                updateTimeDisplay();
            }
        };

        setLayoutManager(std::make_unique<LinearLayout>(Orientation::Horizontal));
    }

    void TimeBox::initialize() {
        this->getNode<TimeBox>() << wdg<TextLabel>("01:10") << Padding{0}
                                 << FontName{Theme::getTheme().FixedBoldFont} << endw
                                 << wdg<TextLabel>("00 GMT") << Padding{0}
                                 << FontName{Theme::getTheme().FixedBoldFont}
                                 << PointSize{2*Theme::getTheme().LabelPointSize/3};

        updateTimeDisplay();

        mTimerTick->secondSignal.connect(secondSlot);
    }

    void TimeBox::draw(gm::Context &context, const Position &containerPosition) {
        Manager::draw(context, containerPosition);
    }

    Rectangle TimeBox::layout(gm::Context &context, const Rectangle &screenRect) {
        if (Container::empty())
            initialize();
        return Manager::layout(context, screenRect);
    }

    void TimeBox::updateTimeDisplay() {
        auto now = ch::system_clock::now();
        auto tt = ch::system_clock::to_time_t(now);
        auto tm = mLocalTime ? *localtime(&tt) : *gmtime(&tt);

        std::stringstream hm{};
        hm.imbue(std::locale());

        put_locale_time(hm, ' ', &tm, HoursMinutesFmt);

        std::stringstream sec{};
        sec.imbue(std::locale());
        put_locale_time(sec, ' ', &tm, mDisplaySeconds ? LongSecondsFmt : ShortSecondsFmt);

        bool redrawBackground = false;
        if (auto first = begin(); first != end()) {
            if (auto hmLabel = (*first)->getNode<TextLabel>(); hmLabel) {
                redrawBackground |= hmLabel->setText(hm.str());
            }
            first++;
            if (first != end()) {
                if (auto secLabel = (*first)->getNode<TextLabel>(); secLabel) {
                    redrawBackground |= secLabel->setText(sec.str());
                }
            }
        }
        if (redrawBackground)
            getApplication().redrawBackground();
    }
}
