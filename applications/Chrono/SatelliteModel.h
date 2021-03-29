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
    };
}

