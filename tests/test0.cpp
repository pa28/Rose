//
// Created by richard on 2021-03-06.
//

#include "StructuredTypes.h"
#include "Font.h"
#include "Application.h"
#include "Settings.h"
#include "Types.h"
#include "Visual.h"

using namespace rose;

int main(int argc, char **argv) {
    Environment& environment{Environment::getEnvironment()};
    Application application{};

    application.initialize(environment.appName(), Size{800, 480});
    application.run();
}
