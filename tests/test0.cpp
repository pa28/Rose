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

    application.screen() << std::make_shared<Window>()
                         << std::make_shared<TextLabel>("Hello World", "FreeSans", 20);

    application.run();
}
