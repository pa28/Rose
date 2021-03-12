//
// Created by richard on 2021-03-06.
//

#include "GraphicsModel.h"
#include "Font.h"
#include "Application.h"
#include "Image.h"
#include "ImageStore.h"
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
                         << wdg<Manager>() >> manager
                         << wdg<ImageLabel>(ImageId::Heart) << Position{5,5} << Parent{}
                         << wdg<TextLabel>( Id{"lblHello"}, "FreeSansBold", 30)
                         << Position{30, 0}
                         << Parent{};

    application.run();
}
