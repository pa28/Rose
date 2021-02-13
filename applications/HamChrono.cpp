/**
 * @file HamChrono.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-11
 */

#include "sstream"
#include <curlpp/cURLpp.hpp>
#include <SDL2/SDL_image.h>
#include "Container.h"
#include "Frame.h"
#include "HamChrono.h"
#include "ImageView.h"
#include "LinearScale.h"
#include "Manipulators.h"
#include "MapProjection.h"
#include "Menu.h"
#include "TimeBox.h"

using namespace rose;

void HamChrono::build() {
    solarImageCache = std::make_unique<WebFileCache>("https://sdo.gsfc.nasa.gov/assets/img/latest/",
                                                     mCacheHome, "NASASolarImages",
                                                     std::chrono::minutes{15});

    celesTrackEphemeris = std::make_unique<WebFileCache>("https://www.celestrak.com/NORAD/elements/",
                                                         mCacheHome, "CelesTrack",
                                                         std::chrono::hours{24});

    clearSkyEphemeris = std::make_unique<WebFileCache>("http://clearskyinstitute.com/ham/HamClock/",
                                                       mCacheHome, "ClearSky",
                                                       std::chrono::hours{24});

    clearSkyMaps = std::make_unique<WebFileCache>("https://www.clearskyinstitute.com/ham/HamClock/maps/",
                                                  mCacheHome, "ClearSkyMaps",
                                                  std::chrono::hours{24 * 30});

    std::make_shared<SecondTick>();

    mSolarImageCacheSlot = std::make_shared<Slot<uint32_t>>();
    mSolarImageCacheSlot->setCallback([=](uint32_t, uint32_t item) {
        auto filePath = solarImageCache->cacheRootPath();
        filePath.append(solarImageCache->find(item)->second.objectSrcName());
        sdl::Surface surface{IMG_Load(filePath.string().c_str())};
        if (surface) {
            mImageRepository.setImageSurface(item, surface);
            needsLayout();
        } else
            std::cout << filePath << " load failed.\n";
    });

    solarImageCache->itemFetched.connect(mSolarImageCacheSlot);

    mMapsCacheSlot = std::make_shared<Slot<uint32_t>>();
    mMapsCacheSlot->setCallback([=](uint32_t, uint32_t item) {
        auto filePath = clearSkyMaps->cacheRootPath();
        filePath.append(clearSkyMaps->find(item)->second.objectSrcName());
        sdl::Surface surface{IMG_Load(filePath.string().c_str())};
        if (surface) {
            mImageRepository.setImageSurface(item, surface);
            needsLayout();
        } else
            std::cout << filePath << " load failed.\n";
    });

    clearSkyMaps->itemFetched.connect(mMapsCacheSlot);

    createRoundCorners(mRenderer, 5, 10, 2,
                       Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createSquareCorners(mRenderer, 10, 2,
                        Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createStandardIcons();
    createCenters(mRenderer, 5, 10);

    solarImageCache->emplace(std::pair{mImageRepository.getImageId(),
                                       CacheObject{"latest_512_0193.jpg", "AIA 193 Å"}});
    solarImageCache->emplace(std::pair{mImageRepository.getImageId(),
                                       CacheObject{"latest_512_0171.jpg", "AIA 171 Å"}});
    solarImageCache->emplace(std::pair{mImageRepository.getImageId(),
                                       CacheObject{"latest_512_HMIB.jpg", "HMIB"}});
    celesTrackEphemeris->emplace(std::pair{1, CacheObject{"amateur.txt", "Amateur"}});
    celesTrackEphemeris->emplace(std::pair{2, CacheObject{"cubesat.txt", "CubeSat"}});
    celesTrackEphemeris->emplace(std::pair{3, CacheObject{"visual.txt", "Visual"}});

    clearSkyEphemeris->emplace(std::pair{1, CacheObject{"esats.pl?getall=", "Amateur"}});
    clearSkyEphemeris->emplace(std::pair{2, CacheObject{"esats.pl?tlename=Moon", "Moon"}});

    auto mapScale = mWidth / 800;
    mMapWidth = (mWidth / mapScale - 140) * mapScale;
    mMapHeight = (mHeight / mapScale - 150) * mapScale;

    mAboveMap = mHeight - mMapHeight;
    mLeftMap = mWidth - mMapWidth;

    std::stringstream ss;
    ss << mMapWidth << 'x' << mMapHeight;
    auto mapSize = ss.str();

    static constexpr std::array<std::string_view, 2> mapType = {"Terrain", "Countries"};
    static constexpr std::array<char, 2> dayNight = {'D', 'N'};
    for (auto &type : mapType) {
        for (auto &dn : dayNight) {
            ss.str("");
            ss << "map-" << dn << '-' << mapSize << '-' << type << ".bmp";
            auto srcName = ss.str();
            ss.str("");
            ss << dn << '_' << type;
            auto userName = ss.str();
            auto imageId = mImageRepository.getImageId();
            clearSkyMaps->emplace(std::pair{imageId, CacheObject{srcName, userName}});
            mMapNameToId.emplace(userName, imageId);
        }
    }

    mSecondTick = std::make_shared<SecondTick>();
    mSecondTick->txSecond.connect(mSystemData.rxTrigger);

    auto mainWindow = createWindow() << BackgroundColor(mTheme.mBaseColor);

    auto topRow = mainWindow << wdg<Container>() << Size{mWidth, mAboveMap} << Position::Zero
                             << wdg<Row>();

    callsignBlock(topRow);

    for (auto &solar : *solarImageCache) {
        topRow << wdg<ImageView>(solar.first);
    }

    mainWindow << wdg<Container>() << Size{mLeftMap, mHeight - mAboveMap} << Position{0, mAboveMap};

    mainWindow << wdg<Container>()
               << Position{mLeftMap, mAboveMap} //<< wdg<ImageView>(clearSkyMaps->findByUserName("D_Terrain"));
               << wdg<MapProjection>(clearSkyMaps->findByUserName("D_Terrain"),
                                     clearSkyMaps->findByUserName("N_Terrain"),
                                     GeoPosition{45.8167, -75.9833}, Size{mMapWidth, mMapHeight});

    solarImageCache->connect(mSecondTick->txSecond, mSecondTick->txMinute);
    celesTrackEphemeris->connect(mSecondTick->txSecond, mSecondTick->txHour);
    clearSkyEphemeris->connect(mSecondTick->txSecond, mSecondTick->txHour);
    clearSkyMaps->connect(mSecondTick->txSecond, mSecondTick->txHour);

    solarImageCache->fetchAll();
    celesTrackEphemeris->fetchAll();
    clearSkyEphemeris->fetchAll();
    clearSkyMaps->fetchAll();
}

void HamChrono::callsignBlock(std::shared_ptr<Row> &parent) {
    std::shared_ptr<LinearScale> scale;
    parent << wdg<Frame>(6) << BorderStyle::Notch << CornerStyle::Round
               << wdg<Column>() << InternalSpace{4}
                   << wdg<CascadeButton>(Id{"CALLSIGN"}, CascadeButtonType::CascadeDown)
                       << Elastic(Orientation::Horizontal) << Manip::Parent
                   << wdg<TimeBox>(mSecondTick) << Manip::Parent
                   << wdg<DateBox>(mSecondTick) << Manip::Parent
                   << wdg<LinearScale>(LinearScaleIndicator::DualChannel) >> scale
                       << Parent<Frame>();
    mSystemData.txTemperature.connect(scale->rxScaledValue0);
    mSystemData.txSystem.connect(scale->rxScaledValue1);
}

int main(int argc, char **argv) {
    // Required if using curlpp or an objects which use it, such as CacheWebSource
    curlpp::Cleanup myCleanup;

    auto app = rose::RoseFactory<HamChrono>(argc, argv, "HamChrono");

    app->build();
    app->initialLayout(app->getRenderer());

    if (app) {
        app->eventLoop();
    }

    return static_cast<int>(app->getErrorCode());
}
