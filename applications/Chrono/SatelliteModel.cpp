/**
 * @file SatelliteModel.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-28
 */

#include "SatelliteModel.h"
#include "Utilities.h"

namespace rose {

    Ephemeris::Ephemeris(const std::filesystem::path& filePath) {
        readFile(filePath);
    }

    void Ephemeris::readFile(const std::filesystem::path &filePath) {
        clear();
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
            std::string_view l1{data, static_cast<std::string_view::size_type>(ptr - data)};
            ++ptr;
            data = ptr;
            while (*ptr != '\n' && ptr < last) ++ptr;
            std::string_view l2{data, static_cast<std::string_view::size_type>(ptr - data)};
            emplace(name, std::array<std::string_view,3>{name,l1,l2});
            ++ptr;
        }
    }

    SatelliteModel::SatelliteModel() {
        mEphemerisCache = std::make_unique<ClearSkyEphemeris>("http://clearskyinstitute.com/ham/HamClock/",
                                                              Environment::getEnvironment().cacheHome(),
                                                              "Ephemeris", std::chrono::hours{24},
                                                              CS_Ephem.begin(), CS_Ephem.end());

        mCacheLoaded = WebCacheProtocol::createSlot();
        mCacheLoaded->receiver = [&](uint32_t id,long status) {
            std::cout << __PRETTY_FUNCTION__ << ' ' << id << ' ' << status << '\n';
            if (id == 1) {
                auto path = mEphemerisCache->itemLocalPath(id);
                mEphemeris.readFile(path);
            }
        };
        mEphemerisCache->cacheLoaded.connect(mCacheLoaded);

        mEphemerisCache->fetchAll();
    }
}
