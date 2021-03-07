//
// Created by richard on 2021-03-06.
//

#include "StructuredTypes.h"
#include "Font.h"
#include "GraphicsModel.h"
#include "Settings.h"
#include "Visual.h"

int main(int argc, char **argv) {
    rose::Environment& environment{rose::Environment::getEnvironment()};
    rose::gm::GraphicsModel graphicsModel{};
    rose::Settings::getSettings();
    graphicsModel.initialize(environment.appName(), rose::Size{800,480});
    graphicsModel.eventLoop();
}
