/**
 * @file Ephemeris.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-17
 */

#include "Ephemeris.h"

Ephemeris::Ephemeris(const std::filesystem::path& filePath) {
    readFile(filePath);
}

void Ephemeris::readFile(const std::filesystem::path &filePath) {
    mEphemerisMap.clear();
    mEphemerisSet.clear();

    std::ifstream ephemFile(filePath.string());
    std::string str((std::istreambuf_iterator<char>(ephemFile)),std::istreambuf_iterator<char>());
    mEphemerisSet = str;

    auto first = mEphemerisSet.c_str();
    auto last = mEphemerisSet.c_str() + mEphemerisSet.length();

    auto ptr = first;
    while (ptr < last) {
        auto data = ptr;
        while (*ptr != '\n' && ptr < last) ++ptr;
        std::string_view name{data, static_cast<std::string_view::size_type>(ptr - data)};
        ++ptr;
        data = ptr;
        while (*ptr != '\n' && ptr < last) ++ptr;
        ++ptr;
        while (*ptr != '\n' && ptr < last) ++ptr;
        mEphemerisMap.emplace(name, std::string_view{data, static_cast<std::string_view::size_type>(ptr - data)});
        ++ptr;
    }
}
