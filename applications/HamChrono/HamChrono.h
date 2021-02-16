/**
 * @file HamChrono.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-11
 */

#pragma once

#include "Cache.h"
#include "MapProjection.h"
#include "Menu.h"
#include "Rose.h"
#include "SettingsNames.h"
#include "Signals.h"
#include "SystemMonitor.h"
#include "TextField.h"
#include "Timer.h"

enum UserSignalTokenValues : rose::SignalToken {
    ConfigButton = rose::FirstUserSignalToken,         ///< Exit the application.
    ExitButton,
};

static constexpr std::array<rose::TextField::FieldSettings,3> ConfigTextFieldSettings = {
        rose::TextField::FieldSettings{rose::DataType::String, set::CALLSIGN, "Call", "", 10, 'M'},
        rose::TextField::FieldSettings{rose::DataType::Real, set::QTH_Lat, "Lat", "Deg", 8, '0'},
        rose::TextField::FieldSettings{rose::DataType::Real, set::QTH_Lon, "Lon", "Deg", 9, '0'},
};

static constexpr std::array<rose::MenuButtonData,2> ConfigMenu{
        rose::MenuButtonData{ "Config", ConfigButton },
        rose::MenuButtonData{ "Exit", ExitButton},
};

/**
 * @class HamChrono
 * @brief A clock for Amateur Radio.
 */
class HamChrono : public rose::Rose {
protected:
    int mMapWidth{};
    int mMapHeight{};
    int mAboveMap{};
    int mLeftMap{};

    rose::SecondTickPtr mSecondTick{};
    rose::SystemData mSystemData{};

    std::shared_ptr<rose::MapProjection> mMapProjection{};

    std::unique_ptr<rose::WebFileCache> solarImageCache;
    std::unique_ptr<rose::WebFileCache> celesTrackEphemeris;
    std::unique_ptr<rose::WebFileCache> clearSkyEphemeris;
    std::shared_ptr<rose::WebFileCache> clearSkyMaps;

    std::shared_ptr<rose::Slot<uint32_t>> mSolarImageCacheSlot;

    rose::GeoPosition mQthLocation{};

    void callsignBlock(std::shared_ptr<rose::Row> &topRow, std::shared_ptr<rose::Column> &sideColumn);

    std::shared_ptr<rose::Slot<rose::Button::SignalType>> mConfigButtonRx{};

public:
    HamChrono() = delete;
    ~HamChrono() override = default;

    HamChrono(int argc, char **argv, const char *name) : rose::Rose(argc, argv, name) {}

    void build();
};

