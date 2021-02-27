/**
 * @file SettingNames.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-15
 */

#pragma once

#include <string>
#include "ImageRepository.h"

namespace set {

    enum class AppImageId : rose::ImageId {
        Sun = rose::ImageRepository::FirstApplicationImageId,
        Moon,
        RingRed,
        RingGreen,
        RingBlue,
        RingYellow,
        RingPurple,
        RingAqua,
        DotRed,
        DotGreen,
        DotBlue,
        DotYellow,
        DotPurple,
        DotAqua,
    };

    enum class ClearSkyEphemeris : size_t {

    };

    static constexpr std::string_view CALLSIGN = "CALLSIGN";
    static constexpr std::string_view CALLSIGN_OPT = "-callsign";
    static constexpr std::string_view QTH = "qth";
    static constexpr std::string_view QTH_Lat = "qth_lat";
    static constexpr std::string_view QTH_Lon = "qth_lon";
    static constexpr std::string_view QTH_Lat_Opt = "-lat";
    static constexpr std::string_view QTH_Lon_Opt = "-lon";

    static constexpr std::string_view SatelliteMode = "SatelliteMode";
    static constexpr std::string_view TerrestrialMode = "TerrestrialMode";
    static constexpr std::string_view AzimuthalMode = "AzimuthalMode";
    static constexpr std::string_view CelestialMode = "CelestialMode";
    static constexpr std::string_view AnnotationMode = "AnnotationMode";
    static constexpr std::string_view SatelliteInterestList = "SatelliteInterestList";
    static constexpr std::string_view SatelliteFavorite = "SatelliteFavorite";
}
