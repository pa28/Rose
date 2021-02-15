/**
 * @file SettingNames.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-15
 */

#pragma once

#include <string>

namespace set {
    static constexpr std::string_view CALLSIGN = "CALLSIGN";
    static constexpr std::string_view CALLSIGN_OPT = "-callsign";
    static constexpr std::string_view QTH_Location = "QTH_Location";
    static constexpr std::string_view QTH_Lat = "-lat";
    static constexpr std::string_view QTH_Lon = "-lon";
}