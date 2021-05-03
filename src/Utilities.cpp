/**
 * @file Utilities.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-06
 */

#include "Utilities.h"
#include <filesystem>
#include <iostream>

namespace rose {

    std::filesystem::path Environment::getenv_path(XDGFilePaths::XDG_Name name, const std::string &appName, bool create) {
        auto [found,path] = mFilePaths.findFilePath(name, appName);
        if (!found && create) {
            std::filesystem::create_directories(path);
        }
        return path;
    }

    Environment::Environment() {
        mHomeDirectory = std::string{getenv("HOME")};
        std::filesystem::path procExec{"/proc"};
        procExec.append("self").append("exe");

        if (std::filesystem::is_symlink(procExec)) {
            mAppName = std::filesystem::read_symlink(procExec).filename().string();

            mDataHome = getenv_path(XDGFilePaths::XDG_DATA_HOME, mAppName, true);
            mConfigHome = getenv_path(XDGFilePaths::XDG_CONFIG_HOME, mAppName, true);
            mCacheHome = getenv_path(XDGFilePaths::XDG_CACHE_HOME, mAppName, true);
            mAppResources = getenv_path(XDGFilePaths::XDG_DATA_DIRS, mAppName + "/resources", false);
            mLibResources = getenv_path(XDGFilePaths::XDG_DATA_DIRS, "Rose/resources", false);
        } else {
            std::cerr << StringCompositor('"', procExec, '"', " is not a symbolic link to application.\n");
            return;
        }
    }
}
