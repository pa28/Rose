//
// Created by richard on 2021-01-05.
//

#include <iostream>
#include "Border.h"
#include "Button.h"
#include "Color.h"
#include "Gauge.h"
#include "Keyboard.h"
#include "Manipulators.h"
#include "Menu.h"
#include "ScrollArea.h"
#include "Slider.h"
#include "TextField.h"
#include "Theme.h"
#include "WidgetTest.h"

using namespace rose;

enum UserSignalTokenValues : rose::SignalToken {
    ExitButton = FirstUserSignalToken,         ///< Exit the application.
    Hue, Saturation, Lightness,
};

template<typename T>
constexpr T mod(T x, T y) {
    auto trunc = (float) ((int) (x / y));
    auto r = x - trunc * y;
    return r;
}

void Test::build() {
    std::cout << __PRETTY_FUNCTION__ << '\n';
    mTranslateFingerEvents = false;

    sliderRx = std::make_shared<Slot<Slider::SignalType>>();
    sliderRx->setCallback([=](uint32_t, Slider::SignalType signalType) {
        switch (signalType.second) {
            case UserSignalTokenValues::Hue:
                mHue = signalType.first;
                break;
            case UserSignalTokenValues::Saturation:
                mSat = signalType.first;
                break;
            case UserSignalTokenValues::Lightness:
                mVal = signalType.first;
                break;
            default:
                break;
        }
        color::HSVA hsva{{mHue * 360.f, mSat, mVal, 1.0f}};
        color::RGBA color{hsva};
        mMainWindow << BackgroundColor(color);
        needsDrawing(true);
        auto sdlColor = color.toSdlColor();
    });

    createRoundCorners(mRenderer, 5, 10, 2,
                       Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createSquareCorners(mRenderer, 10, 2,
                        Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createStandardIcons();
    createCenters(mRenderer, 5, 10);

    mMainWindow = createWindow() << BackgroundColor(mTheme.mBaseColor);

    auto keyboard = std::make_shared<QUERTY>();
    std::shared_ptr<Column> column;
    mMainWindow << wdg<Frame>(4) << BorderStyle::Notch << CornerStyle::Round
                << wdg<Column>()
                    << wdg<Column>()
                            << wdg<TextField>(10, "", "", "Call", 6)
                                    << BorderStyle::Notch << CornerStyle::Round << FontSize{20}
                                    << Manip::Parent
                            << wdg<TextField>(7, "", "Deg", "Lat", 6)
                                    << BorderStyle::Notch << CornerStyle::Round << FontSize{20}
                                    << Manip::Parent
                            << wdg<TextField>(7, "", "Deg", "Lon", 6)
                                    << BorderStyle::Notch << CornerStyle::Round << FontSize{20}
                                    << Manip::Parent
                            << wdg<LinearScale>(LinearScaleIndicator::DualChannel)
                                    << Orientation::Horizontal
                                    << Manip::Parent
                            << Manip::Parent
                    << wdg<Keyboard>(keyboard);
}

int main(int argc, char **argv) {

    auto app = RoseFactory<Test>(argc,argv,"Test");

    app->build();
    app->initialLayout(app->getRenderer());

    if (app) {
        app->eventLoop();
    }

    return static_cast<int>(app->getErrorCode());
}
