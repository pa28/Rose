//
// Created by richard on 2021-03-06.
//

#include "Font.h"
#include "Application.h"
#include "Text.h"
#include "Types.h"

using namespace rose;

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Application application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    std::shared_ptr<Manager> manager{};
    std::shared_ptr<Widget> widget{};
    application.screen() << wdg<Window>()
                         << wdg<Manager>() >> manager
                         << wdg<TextLabel>("Hello World", "FreeSansBold", 30) >> widget
                         << Parent{};

    application.run();
}
