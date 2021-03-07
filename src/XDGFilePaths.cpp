/**
 * @file XDGFilePaths.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-18
 */

#include "XDGFilePaths.h"
#include <cstdlib>
#include <sstream>

namespace rose {

    XDGFilePaths::XDGFilePaths() {
        mHome = getenv("HOME");
        for (const auto &path : mEnvVars) {
            bool inEnvironment{false};
            std::string envValue;
            auto env = getenv(std::string{path.varName}.c_str());
            if (env) {
                envValue = std::string{env};
                inEnvironment = true;
            } else {
                envValue = path.defaultPath;
            }

            XDG_Path_Set pathSet{};
            std::stringstream strm{envValue};
            std::string value{};
            while (std::getline(strm, value, ':')) {
                if (!inEnvironment && path.homeRelative) {
                    std::filesystem::path pathValue{mHome};
                    pathValue.append(value);
                    pathSet.push_back(pathValue);
                } else {
                    std::filesystem::path pathValue{value};
                    pathSet.push_back(pathValue);
                }
            }
            mPaths[path.name] = pathSet;
        }
    }
}
