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
#include "Math.h"
#include <memory>
#include <algorithm>
#include <chrono>

namespace rose {

    static constexpr std::chrono::milliseconds LunarMonthMilliseconds{2551442976};
    static constexpr std::time_t LunarNewMoonEpoch{1618194720};

    /**
     * @brief Calculate the current phase of the moon in days between [0..29].
     * @return The moon phase.
     */
    inline int MoonPhase() {
        std::chrono::system_clock::time_point newEpoch = std::chrono::system_clock::from_time_t(LunarNewMoonEpoch);
        std::chrono::system_clock::time_point epoch = std::chrono::system_clock::now();
        auto moonAge = std::chrono::duration_cast<std::chrono::milliseconds>(epoch - newEpoch);
        auto phase = std::chrono::duration_cast<std::chrono::hours>(moonAge % LunarMonthMilliseconds);
        auto c = util::roundToInt(static_cast<float>(phase.count()) / 24.);
        return c;
    }

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
        SatelliteObservation() = default;

        explicit SatelliteObservation(const Observer &observer);

        SatelliteObservation(const Observer &observer, const std::string& object);

        void predict(const DateTime &dateTime);

        void passPrediction(uint maxCount, const std::string &favorite);

        [[nodiscard]] const Observer& observer() const {
            return mObserver;
        }

        [[nodiscard]] auto empty() const noexcept {
            return mConstellation.empty();
        }

        [[nodiscard]] auto size() const noexcept {
            return mConstellation.size();
        }

        [[nodiscard]] auto front() const noexcept {
            return mConstellation.front();
        }
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

        /**
         * @brief Create a std::string that describes the pass.
         * @details If relative == 0 then times that are converted are converted to absolute dates and times in GMT.
         * If the rise time is valid and in the future it is entered into the string first followed by " - " and the
         * set time. If relative is not 0 the set time is converted relative to the rise time. This provides the
         * rise time followed by the pass duration.<p/>
         * If the rise time is not valid only the set time is converted providing the set time if relative is 0 or
         * the duration if relative is not 0.
         * @param relative A time_t to make the strings relative to.
         * @return a std::string with the formatted pass timing data.
         */
        [[nodiscard]] std::string passTimeString(time_t relative = 0) const;

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

