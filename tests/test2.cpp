//
// Created by richard on 2021-03-27.
//

#include <chrono>
#include "Utilities.h"
#include "WebCache.h"

using namespace rose;

static constexpr std::array<WebCacheItem,4> Maps{
        WebCacheItem{0, "map-D-660x330-Terrain.bmp"},
        WebCacheItem{1, "map-N-660x330-Terrain.bmp"},
        WebCacheItem{2, "map-D-660x330-Countries.bmp"},
        WebCacheItem{3, "map-N-660x330-Countries.bmp"},
};

static constexpr std::array<WebCacheItem,2> CS_Ephem{
        WebCacheItem{0, "Moon"},
        WebCacheItem{1, "Amateur"},
};

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


int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};

    WebCacheProtocol::slot_type mapSlot = WebCacheProtocol::createSlot();
    mapSlot->receiver = [](uint32_t key, long status){
        std::cout << __PRETTY_FUNCTION__ << " Cache file loaded: " << key << ' ' << status << '\n';
    };

    WebCacheProtocol::slot_type ephemSlot = WebCacheProtocol::createSlot();
    ephemSlot->receiver = [](uint32_t key, long status) {
        std::cout << __PRETTY_FUNCTION__ << " Ephemeris file loaded: " << key << ' ' << status << '\n';
    };

    WebCache webCache{"https://www.clearskyinstitute.com/ham/HamClock/maps/",
                      environment.cacheHome(), "Maps",
                      std::chrono::hours{24 * 30},
                      Maps.begin(), Maps.end()};

    webCache.cacheLoaded.connect(mapSlot);

    ClearSkyEphemeris clearSkyEphemeris{"http://clearskyinstitute.com/ham/HamClock/",
                                        environment.cacheHome(), "Ephemeris",
                                        std::chrono::hours{24},
                                        CS_Ephem.begin(), CS_Ephem.end()};

    clearSkyEphemeris.cacheLoaded.connect(ephemSlot);

    webCache.fetchAll();
    clearSkyEphemeris.fetchAll();

    while (webCache.processFutures());

    while (clearSkyEphemeris.processFutures());
}
