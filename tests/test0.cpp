//
// Created by richard on 2021-03-06.
//

#include "StructuredTypes.h"
#include "Visual.h"
#include "GraphicsModel.h"

int main(int argc, char **argv) {
    rose::gm::GraphicsModel graphicsModel{};
    std::string title{"Test0"};
    graphicsModel.initialize(title, rose::Size{800,480});
    graphicsModel.eventLoop();
}
