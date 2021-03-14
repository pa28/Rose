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

    std::shared_ptr<Manager> manager{};
    std::shared_ptr<Widget> widget{};
    application.screen() << wdg<Window>()
                            << wdg<Manager>() << Id{"row"} << std::make_unique<LinearLayout>(Orientation::Vertical) << Position{10,10}
                                 << wdg<Frame>(0)
                                    << wdg<ImageLabel>(ImageId::HeartEmpty, 40)
                                    << Parent{} << Parent{}
                                 << wdg<Frame>(5)
                                    << wdg<TextLabel>( Id{"lblHello"}, "FreeSansBold", 30)
                                    << Parent{} << Parent{};

    application.run();
}
