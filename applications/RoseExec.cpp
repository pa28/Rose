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
            "/usr/bin/sudo /usr/bin/chgrp video /sys/class/backlight/rpi_backlight/brightness",
            "/usr/bin/sudo /usr/bin/chmod g+w /sys/class/backlight/rpi_backlight/brightness"
    };

    static constexpr std::array<const char *,2> UpgradeCommands = {
        "/usr/bin/sudo apt update",
        "/usr/bin/sudo apt -y upgrade"
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
            } else if (command.result() == "upgrade\n") {
                for (auto &upgrade : UpgradeCommands) {
                    Command cmd{upgrade};
                    cmd.wait();
                    if (cmd.statusCode())
                        break;
                }
            } else {
                auto appStr = command.result() + " 2> /dev/null";
                auto application = Command(appStr.c_str());
                application.wait();
            }
        } else {
            runLoop = false;
        }
    } while (runLoop);

    exit(0);
}
