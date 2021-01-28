/**
 * @file RoseExec.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-24
 */

#include <string>
#include <array>
#include <filesystem>
#include "Command.h"

using namespace rose;

int main(int argc, char **argv) {
    static constexpr std::array<const char *,2> PrepCommands = {
            "sudo chgrp video /sys/class/backlight/rpi_backlight/brightness",
            "sudo chmod g+w /sys/class/backlight/rpi_backlight/brightness"
    };

    std::filesystem::path backlight{"/sys/class/backlight/rpi_backlight/brightness"};
    if (std::filesystem::exists(backlight)) {
        for (auto &cmd : PrepCommands) {
            auto command = Command(cmd);
            command.wait();
        }
    }

    bool runLoop = true;
    do {
        auto command = Command("RoseShell");
        command.wait();
        if (command.statusCode() == 0) {
            if (command.result().empty() || command.result() == "EXIT\n") {
                runLoop = false;
            } else {
                auto application = Command(command.result().c_str());
                application.wait();
            }
        } else {
            runLoop = false;
        }
    } while (runLoop);

    exit(0);
}
