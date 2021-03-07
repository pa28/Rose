//
// Created by richard on 2021-03-06.
//

#include "StructuredTypes.h"
#include "Font.h"
#include "GraphicsModel.h"
#include "Settings.h"
#include "Types.h"
#include "Visual.h"

using namespace rose;

int main(int argc, char **argv) {
    Environment& environment{Environment::getEnvironment()};
    gm::GraphicsModel graphicsModel{};
    Settings& settings{Settings::getSettings()};

    auto appSize = settings.getValue(set::SetAppSize, Size{800,400});
    auto appPos = settings.getValue(set::SetAppPosition, Position::Undefined);

    graphicsModel.initialize(environment.appName(), appSize, appPos);
    graphicsModel.eventLoop();
}
