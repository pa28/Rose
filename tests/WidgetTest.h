
#pragma once

#include "LinearScale.h"
#include "Rose.h"
#include "Signals.h"
#include "Slider.h"
#include "SystemMonitor.h"
#include "Timer.h"

class Test : public rose::Rose {
protected:
    float mHue{}, mSat{}, mVal{};

    std::shared_ptr<rose::Window> mMainWindow{};
    std::shared_ptr<rose::SecondTick> mSecondTick{};
    std::shared_ptr<rose::SystemData> mSystemData{};

public:

    Test(int argc, char **argv, const std::string_view title) : rose::Rose(rose::Size{800, 480},
                                                                                 argc, argv, title) {

    }

    std::shared_ptr<rose::Slot<rose::Slider::SignalType>> sliderRx;

    void build();

};