//
// Created by richard on 2021-03-06.
//

#include "Application.h"
#include "Button.h"
#include "Font.h"
#include "Frame.h"
#include "ImageStore.h"
#include "Keyboard.h"
#include "Layout.h"
#include "Manager.h"
#include "Popup.h"
#include "Theme.h"
#include "Types.h"
#include "TextField.h"

using namespace rose;

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Application application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};
    Theme& theme{Theme::getTheme()};

    PointSize prefixPointSize{theme.TextPointSize};
    FontName prefixFontName{theme.TextFont};
    std::shared_ptr<Column> column{};
    application.screen() << wdg<Window>()
                         << wdg<Frame>(5) << Position{10, 10} << theme.SemiBevelFrame
                         << wdg<Manager>() << Id{"row"} << makeLayout<LinearLayout>(Orientation::Vertical, 5)
                         << wdg<ImageLabel>(ImageId::HeartEmpty)
                            << curve<ActionCurves::CursorPulse>()
                            << FrameColor{FrameColorType::AnimateColor, theme.rgba(rose::ThemeColor::Red)}
                            << FrameColor{FrameColorType::InactiveColor, theme.hsva(rose::ThemeColor::Red).withValue(0.25).toRGBA()}
                            << AnimationEnable::Enable
                            << endw
                         << wdg<ImageButton>(ImageId::ThreeDots, ButtonType::ToggleButton)
                            << theme.SemiBevelFrame
                            << endw
                         << wdg<TextButton>(Id{"lblHello"}, [&](ButtonStateChange buttonStateChange){
                                 if (buttonStateChange == rose::ButtonStateChange::Pushed) {
                                     std::cout << "Local Button state: Pushed\n";
                                     application.screen() << wdg<Dialog>()
                                             << Position::Zero
                                             << wdg<Column>() >> column
                                                 << wdg<Grid>(2)
                                                     << wdg<TextLabel>("Callsign:") << prefixPointSize << prefixFontName << endw
                                                     << wdg<TextField>(10, "MM0MMM-9" ) << endw
                                                     << wdg<TextLabel>("Latitude:") << prefixPointSize << prefixFontName  << endw
                                                     << wdg<TextField>(8, "0", "\xc2\xb0")
                                                             << RegexPattern{FloatPattern2} << endw
                                                     << wdg<TextLabel>("Longitude:") << prefixPointSize << prefixFontName  << endw
                                                     << wdg<TextField>(9, "0", "\xc2\xb0")
                                                             << RegexPattern{FloatPattern2} << endw;
                                     if (!application.keyboardFound())
                                         column << wdg<Keyboard>();
                                     application.layout();
                                 }
                             })
                            << theme.SemiBevelFrame
                            << endw
                         << wdg<TextLabel>(Id{"lblHello"})
                            << curve<ActionCurves::PulsePerSecond>()
                            << FrameColor{FrameColorType::AnimateColor, theme.rgba(rose::ThemeColor::Green)}
                            << AnimationEnable::Enable
                            << endw;

    application.run();
}
