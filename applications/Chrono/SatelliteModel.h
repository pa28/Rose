/**
 * @file SatelliteModel.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-28
 */

#pragma once

#include "Utilities.h"
#include "WebCache.h"
#include "Plan13.h"
#include <memory>
#include <algorithm>

namespace rose {

    class ClearSkyEphemeris : public WebCache {
    public:
        ClearSkyEphemeris() = delete;

        ~ClearSkyEphemeris() override = default;

        /**
         * @brief Constructor
         * @param rootUri The base URI locating all the items manage by the cache.
         * @param xdgDir The XDG compliant specification for the user caching directory. See rose::Environment.
         * @param storeRoot A relative path from xdgDir that this cache will use.
         * @param duration The period of time cache items should be considered valid before being reloaded.
         */
        ClearSkyEphemeris(const std::string &rootUri, const path &xdgDir, const std::string &storeRoot,
                          std::chrono::system_clock::duration duration) :
                WebCache(rootUri, xdgDir, storeRoot, duration) {}

        template<typename It>
        ClearSkyEphemeris(const std::string &rootUri, const path &xdgDir, const std::string &storeRoot,
                          std::chrono::system_clock::duration duration, It first, It last)
                : WebCache(rootUri, xdgDir, storeRoot, duration) {
            setCacheItem(first, last);
        }

        std::string constructUrl(const local_id_t &localId) override {
            if (localId == "Amateur")
                return mRootURI + "esats.pl?getall=";
            else if (localId == "Moon")
                return mRootURI + "esats.pl?tlename=Moon";
            return mRootURI;
        }
    };

    static constexpr std::array<WebCacheItem,2> CS_Ephem{
            WebCacheItem{0, "Moon"},
            WebCacheItem{1, "Amateur"},
    };

    /**
     * @class Ephemeris
     * @brief
     */
    class Ephemeris : public std::map<std::string_view, std::array<std::string_view,3>> {
    protected:
        std::string mEphemerisSet{};

    public:
        using iterator = std::map<std::string_view, std::array<std::string_view,3>>::iterator;

        Ephemeris() = default;
        ~Ephemeris() = default;

        explicit Ephemeris(const std::filesystem::path& filePath);

        void readFile(const std::filesystem::path &filePath);

    };

    /**
     * @class SatelliteModel
     * @brief
     */
    class SatelliteModel {
    protected:
        std::unique_ptr<ClearSkyEphemeris> mEphemerisCache{};

        WebCacheProtocol::slot_type mCacheLoaded{};

        Ephemeris mEphemeris{};

        SatelliteModel();

    public:
        static SatelliteModel& getModel() {
            static SatelliteModel instance{};
            return instance;
        }

        ~SatelliteModel() = default;

        SatelliteModel(const SatelliteModel&) = delete;

        SatelliteModel(SatelliteModel &&) = delete;

        SatelliteModel& operator=(const SatelliteModel&) = delete;

        SatelliteModel& operator=(SatelliteModel&&) = delete;

        auto begin() {
            return mEphemeris.begin();
        }

        auto end() {
            return mEphemeris.end();
        }

        [[nodiscard]] auto begin() const {
            return mEphemeris.cbegin();
        }

        [[nodiscard]] auto end() const {
            return mEphemeris.cend();
        }
    };

    class SatelliteObservation {
    protected:
        Observer mObserver{};

        std::vector<Satellite> mConstellation{};

    public:
        explicit SatelliteObservation(const Observer &observer);

        void predict(const DateTime &dateTime);

        void passPrediction();
    };

    static constexpr long COARSE_DT = 90L;
    static constexpr long FINE_DT = (-2L);
    static constexpr double SAT_MIN_EL = 1.;

    struct SatellitePassData {
        Satellite satellite{};
        bool riseOk{false}, setOk{false}, everUp{false}, everDown{false};
        long deltaTime{COARSE_DT};
        double altitude{}, azimuth{}, range{}, rangeRate{}, latRad{}, lonRad{}, periodDays{},
            maxAltitude{}, prevAltitude{0}, setAz{0}, riseAz{0};

        DateTime srchTime{}, riseTime{}, setTime{};

        SatellitePassData() = default;
        ~SatellitePassData() = default;

        /// Return true if pass not found and search time not exceeded.
        bool search(DateTime& now) const noexcept {
            return (!setOk || !riseOk) && srchTime < now + 2.0F && (srchTime > now || altitude > -1.);
        }

        [[nodiscard]] bool goodPass(double minAltitude) const noexcept {
            return riseOk && setOk && maxAltitude >= minAltitude;
        }

        void setTopo(const Observer& observer) {
            auto topo = satellite.topo(observer);
            altitude = std::get<0>(topo);
            azimuth = std::get<1>(topo);
            range = std::get<2>(topo);
            rangeRate = std::get<3>(topo);
        }

        void setGeo() {
            auto geo = satellite.geo();
            latRad = std::get<0>(geo);
            lonRad = std::get<1>(geo);
        }
    };
}

