//
// Created by richard on 2021-03-06.
//

#include "GraphicsModel.h"
#include "Application.h"
#include "Button.h"
#include "Font.h"
#include "Frame.h"
#include "Image.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Text.h"
#include "Theme.h"
#include "Types.h"

using namespace rose;

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Application application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};
    Theme& theme{Theme::getTheme()};

    application.screen() << wdg<Window>()
                         << wdg<Frame>(5) << Position{10, 10} << theme.SemiBevelFrame
                         << wdg<Manager>() << Id{"row"} << makeLayout<LinearLayout>(Orientation::Vertical, 5)
                         << wdg<Frame>(0) << curve<ActionCurves::HeartBeat>()
                         << FrameColor{FrameColorType::AnimateColor, theme.rgba(rose::ThemeColor::Red)}
                         << FrameColor{FrameColorType::InactiveColor, theme.hsva(rose::ThemeColor::Red).withValue(0.25).toRGBA()}
                             << AnimationEnable{rose::AnimationEnable::Enable}
                             << wdg<ImageLabel>(ImageId::HeartEmpty)
                                << endw << endw
                         << wdg<ImageButton>(ImageId::ThreeDots, ButtonType::ToggleButton)
                                 << theme.CleanFrame
                                 << endw
                         << wdg<Frame>(5) << curve<ActionCurves::PulsePerSecond>() << LayoutHint{LayoutHint::BottomRight}
                                     << FrameColor{FrameColorType::InactiveColor, theme.rgba(rose::ThemeColor::Green)}
                                         << wdg<TextLabel>( Id{"lblHello"}, "FreeSansBold", 30)
                                             << endw
                                         << endw
                                     << endw;

    application.run();
}
