//
// Created by richard on 2021-03-06.
//

#include "StructuredTypes.h"
#include "Font.h"
#include "GraphicsModel.h"
#include "Visual.h"

int main(int argc, char **argv) {
    rose::Environment& environment{rose::Environment::getEnvironment()};
    rose::gm::GraphicsModel graphicsModel{};
    std::string title{"Test0"};
    graphicsModel.initialize(title, rose::Size{800,480});
    graphicsModel.eventLoop();
}
