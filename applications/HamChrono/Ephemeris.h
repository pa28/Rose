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
class Ephemeris : public std::map<std::string_view, std::array<std::string_view,3>> {
protected:
    std::string mEphemerisSet{};

public:
    Ephemeris() = default;
    ~Ephemeris() = default;

    explicit Ephemeris(const std::filesystem::path& filePath);

    void readFile(const std::filesystem::path &filePath);

};

