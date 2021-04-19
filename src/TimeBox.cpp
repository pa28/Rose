/**
 * @file TimeBox.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-19
 */

#include "TimeBox.h"

#include <utility>
#include "Button.h"

namespace rose {

    TimeBox::TimeBox(std::shared_ptr<TimerTick> timerTick) : mTimerTick(std::move(timerTick)) {
        secondSlot = TickProtocol::createSlot();
        secondSlot->receiver = [&](int seconds) {
            std::cout << __PRETTY_FUNCTION__ << '\n';
        };

        setLayoutManager(std::make_unique<LinearLayout>(Orientation::Horizontal));
    }

    void TimeBox::initialize() {
        mTimerTick->secondSignal.connect(secondSlot);

        this->getNode<TimeBox>() << wdg<TextLabel>("01:10") << Padding{0}
                                 << FontName{Theme::getTheme().FixedBoldFont} << endw
                                 << wdg<TextLabel>("00 GMT") << Padding{0}
                                 << FontName{Theme::getTheme().FixedBoldFont}
                                 << PointSize{2*Theme::getTheme().LabelPointSize/3};
    }

    void TimeBox::draw(gm::Context &context, const Position &containerPosition) {
        Manager::draw(context, containerPosition);
    }

    Rectangle TimeBox::layout(gm::Context &context, const Rectangle &screenRect) {
        if (Container::empty())
            initialize();
        return Manager::layout(context, screenRect);
    }
}
