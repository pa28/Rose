//
// Created by richard on 2021-01-05.
//

#include <iostream>
#include "Border.h"
#include "Button.h"
#include "Color.h"
#include "Gauge.h"
#include "Manipulators.h"
#include "Menu.h"
#include "ScrollArea.h"
#include "Slider.h"
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
    mTranslateFingerEvents = true;

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

#if 1
    auto menu = mMainWindow << wdg<CascadeButton>("File");

    std::shared_ptr<Slider> slider{};
    std::shared_ptr<LinearScale> scale{};

    mMainWindow << wdg<Frame>() << Position{50, 50}
                                   << BorderStyle::Bevel << CornerStyle::Round
                                   << wdg<Border>(4)
                                           << wdg<ScrollArea>() << Size{300,300}
                         << wdg<Column>() << VerticalSpacing{4}
                                << wdg<Label>("A very wide label.")  << Manip::Parent
                                << wdg<Label>("Label left")
                                    << LabelHorizontalAlignment::Left
                                    << Manip::Parent
                                << wdg<Label>("Label right")
                                    << LabelHorizontalAlignment::Right
                                    << Manip::Parent
                                << wdg<Label>("Label centre")
                                    << LabelHorizontalAlignment::Center
                                    << Manip::Parent
                                << wdg<Slider>() >> slider << Id{"Slider"} << Manip::Parent
                                << wdg<Gauge>() << Manip::Parent
                                << wdg<LinearScale>(BevelOutRoundCorners) >> scale << Manip::Parent;

    mMainWindow << wdg<Frame>() << Position{ 10, 350}
                                << BorderStyle::Bevel << CornerStyle::Round
                                << wdg<Border>(4)
            << wdg<Row>() << HorizontalSpacing{4}
                            << wdg<Label>("A very wide label.") << Manip::Parent
                            << wdg<Label>("Label top")
                                << FontSize{20}
                                << LabelVerticalAlignment::Top
                                << Manip::Parent
                            << wdg<Label>("Label bottom")
                                << FontSize{20}
                                << LabelVerticalAlignment::Bottom
                                << Manip::Parent
                            << wdg<Label>("Label centre")
                                << FontSize{20}
                                << LabelVerticalAlignment::Center
                                << Manip::Parent;

    slider->valueTx.connect(scale->valueRx);

#else
    mHue = mTheme.mBaseColorHSLA.hue() / 360.0f;
    mSat = mTheme.mBaseColorHSLA.saturation();
    mVal = mTheme.mBaseColorHSLA.value();
    auto column = mMainWindow << wdg<Column>() << Position{50, 50};
    column << wdg<Label>("Hue Saturation Lightness");
    auto hue = column << wdg<Slider>(mHue) << UserSignalTokenValues::Hue;
    hue->valueTx.connect(sliderRx);
    auto sat = column << wdg<Slider>(mSat) << UserSignalTokenValues::Saturation;
    sat->valueTx.connect(sliderRx);
    auto lit = column << wdg<Slider>(mVal) << UserSignalTokenValues::Lightness;
    lit->valueTx.connect(sliderRx);
#endif

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
