/**
 * @file SatelliteModel.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-28
 */

#include "SatelliteModel.h"
#include "Utilities.h"

namespace rose {

    SatelliteModel::SatelliteModel() {
        mEphemerisCache = std::make_unique<ClearSkyEphemeris>("http://clearskyinstitute.com/ham/HamClock/",
                                                              Environment::getEnvironment().cacheHome(),
                                                              "Ephemeris", std::chrono::hours{24},
                                                              CS_Ephem.begin(), CS_Ephem.end());

        mCacheLoaded = WebCacheProtocol::createSlot();
        mCacheLoaded->receiver = [&](uint32_t id,long status) {
            std::cout << __PRETTY_FUNCTION__ << ' ' << id << ' ' << status << '\n';
        };
        mEphemerisCache->cacheLoaded.connect(mCacheLoaded);

        mEphemerisCache->fetchAll();
    }
}
