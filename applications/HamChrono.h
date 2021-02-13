/**
 * @file HamChrono.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-11
 */

#pragma once

#include "Cache.h"
#include "Signals.h"
#include "SystemMonitor.h"
#include "Timer.h"
#include "Rose.h"

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

    std::unique_ptr<rose::WebFileCache> solarImageCache;
    std::unique_ptr<rose::WebFileCache> celesTrackEphemeris;
    std::unique_ptr<rose::WebFileCache> clearSkyEphemeris;
    std::unique_ptr<rose::WebFileCache> clearSkyMaps;

    std::map<std::string,uint32_t> mMapNameToId{};

    std::shared_ptr<rose::Slot<uint32_t>> mSolarImageCacheSlot;
    std::shared_ptr<rose::Slot<uint32_t>> mMapsCacheSlot;

    void callsignBlock(std::shared_ptr<rose::Row> &parent);

public:
    HamChrono() = delete;
    ~HamChrono() override = default;

    HamChrono(int argc, char **argv, const char *name) : rose::Rose(argc, argv, name) {}

    void build();
};

