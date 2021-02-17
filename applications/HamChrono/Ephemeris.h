/**
 * @file Ephemeris.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-17
 */

#pragma once

#include <map>
#include <filesystem>
#include <string>
#include <fstream>
#include <streambuf>

/**
 * @class Ephemeris
 * @brief
 */
class Ephemeris {
protected:

    std::map<std::string_view,std::string_view> mEphemerisMap{};
    std::string mEphemerisSet{};

public:
    Ephemeris() = default;
    ~Ephemeris() = default;

    explicit Ephemeris(const std::filesystem::path& filePath);

    void readFile(const std::filesystem::path &filePath);

};

