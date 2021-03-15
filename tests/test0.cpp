//
// Created by richard on 2021-03-06.
//

#include "GraphicsModel.h"
#include "Application.h"
#include "Font.h"
#include "Frame.h"
#include "Image.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Text.h"
#include "Types.h"

using namespace rose;

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Application application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};

    application.screen() << wdg<Window>()
                         << wdg<Frame>(5) << Position{10, 10} << FrameColor{FrameColorType::InactiveColor, color::DarkYellowHSVA.toRGBA()}
                         << wdg<Manager>() << Id{"row"} << layout<LinearLayout>(Orientation::Vertical)
                         << wdg<Frame>(0) << curve<ActionCurves::HeartBeat>() << FrameColor{FrameColorType::ActiveColor, color::DarkRedHSVA.toRGBA()}
                         << wdg<ImageLabel>(ImageId::HeartEmpty, 40)
                         << endw
                         << endw
                         << wdg<Frame>(5) << curve<ActionCurves::PositiveSine>()
                                 << FrameColor{FrameColorType::ActiveColor, color::DarkGreenHSVA.toRGBA()}
                                << wdg<TextLabel>( Id{"lblHello"}, "FreeSansBold", 30)
                                << endw
                            << endw
                         << endw;

    application.run();
}
