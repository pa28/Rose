//
// Created by richard on 2021-03-27.
//

#include <chrono>
#include "Utilities.h"
#include "WebCache.h"

using namespace rose;

static constexpr std::array<WebCacheItem,2> Maps{
        WebCacheItem{0, "map-D-660x330-Terrain.bmp"},
        WebCacheItem{1, "map-N-660x330-Terrain.bmp"},
};

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};

    WebCache webCache{"https://www.clearskyinstitute.com/ham/HamClock/maps/",
                      environment.cacheHome(), "Maps", std::chrono::minutes {1},
                      Maps.begin(), Maps.end()};

    if (webCache.fetchAll())
        while (webCache.processFutures())
            ;
}
