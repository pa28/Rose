/**
 * @file HamChrono.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-11
 */

#include "sstream"
#include <curlpp/cURLpp.hpp>
#include <SDL2/SDL_image.h>
#include "ConfigDialog.h"
#include "Container.h"
#include "Frame.h"
#include "HamChrono.h"
#include "ImageView.h"
#include "LinearScale.h"
#include "Manipulators.h"
#include "MapProjection.h"
#include "Menu.h"
#include "SettingsNames.h"
#include "TimeBox.h"
#include "Ephemeris.h"
#include "Plan13.h"

using namespace rose;

static constexpr std::array<Rose::IconFileItem,2> fileIcons = {
        Rose::IconFileItem{ static_cast<ImageId>(set::AppImageId::Sun), Size{0,0}, "35px-Sun.png"},
        Rose::IconFileItem{ static_cast<ImageId>(set::AppImageId::Moon), Size{0,0}, "full_moon.png"},
};

void HamChrono::build() {
#ifdef Debug
    std::cout << __PRETTY_FUNCTION__ << " Debug\n";
#endif
    createFileIcons(fileIcons, mSharedImages);

    TextField::Settings(mSettings, ConfigTextFieldSettings);

    std::array<color::RGBA,10> mIconColor = {
            mTheme.mTextColour, mTheme.mBaseColor, mTheme.mBlack,
            color::RGBA{255u, 0u, 0u, 255u}, mTheme.mRed,
            color::RGBA{255u, 255u, 0u, 255u}, mTheme.mYellow,
            color::RGBA{0u, 255u, 0u, 255u}, mTheme.mGreen, mTheme.mWhite
    };

    mConfigButtonRx = std::make_shared<Slot<Button::SignalType>>();
    mConfigButtonRx->setCallback([&](uint32_t, Button::SignalType button){
        switch (button.second) {
            case ExitButton:
                createPopup<ExitDialog>() << mConfigButtonRx;
                needsLayout();
                break;
            case ConfigButton:
                createPopup<ConfigDialog>();
                needsLayout();
                break;
            case DialogOk:
                break;
            case ExitDialogOk:
                mRunEventLoop = false;
                break;
        }
    });

    if (mCmdLineParser.cmdOptionExists(set::CALLSIGN_OPT)) {
        auto callsign = mCmdLineParser.getCmdOption(set::CALLSIGN_OPT);
        mSettings->setValue(set::CALLSIGN, callsign);
    }

    if (mCmdLineParser.cmdOptionExists(set::QTH_Lat_Opt) && mCmdLineParser.cmdOptionExists(set::QTH_Lon_Opt)) {
        auto lat = strtod(mCmdLineParser.getCmdOption(set::QTH_Lat_Opt).c_str(), nullptr);
        auto lon = strtod(mCmdLineParser.getCmdOption(set::QTH_Lon_Opt).c_str(), nullptr);
        GeoPosition qth{lat,lon};
        mSettings->setValue(set::QTH, qth);
    } else {
        mQthLocation = mSettings->getValue(set::QTH, GeoPosition{0.,0.});
    }

    solarImageCache = std::make_unique<WebFileCache>("https://sdo.gsfc.nasa.gov/assets/img/latest/",
                                                     mCacheHome, "NASASolarImages",
                                                     std::chrono::minutes{15});

    celesTrackEphemeris = std::make_unique<WebFileCache>("https://www.celestrak.com/NORAD/elements/",
                                                         mCacheHome, "CelesTrack",
                                                         std::chrono::hours{24});

    clearSkyEphemeris = std::make_unique<WebFileCache>("http://clearskyinstitute.com/ham/HamClock/",
                                                       mCacheHome, "ClearSky",
                                                       std::chrono::hours{24});

    clearSkyMaps = std::make_shared<WebFileCache>("https://www.clearskyinstitute.com/ham/HamClock/maps/",
                                                  mCacheHome, "ClearSkyMaps",
                                                  std::chrono::hours{24 * 30});

    mSecondTick = std::make_shared<SecondTick>();

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

    static constexpr std::array<MapDataType, 4> mapType = {MapDataType::TerrainDay,
                                                           MapDataType::TerrainNight,
                                                           MapDataType::CountriesDay,
                                                           MapDataType::CountriesNight};

    static constexpr std::array<char, 2> dayNight = {'D', 'N'};

    for (auto &type : mapType) {
        bool validType = true;
        char dn;
        std::string typeName;
        switch (type) {
            case rose::MapDataType::TerrainDay:
                dn = 'D';
                typeName = "Terrain";
                break;
            case rose::MapDataType::TerrainNight:
                dn = 'N';
                typeName = "Terrain";
                break;
            case rose::MapDataType::CountriesDay:
                dn = 'D';
                typeName = "Countries";
                break;
            case rose::MapDataType::CountriesNight:
                dn = 'N';
                typeName = "Countries";
                break;
            case rose::MapDataType::MapCount:
                validType = false;
                break;
        }

        if (validType) {
            ss.str("");
            ss << "map-" << dn << '-' << mapSize << '-' << typeName << ".bmp";
            auto srcName = ss.str();
            ss.str("");
            ss << dn << '_' << typeName;
            auto userName = ss.str();
            clearSkyMaps->emplace(std::pair{static_cast<uint32_t>(type), CacheObject{srcName, userName}});
        }
    }

    mSecondTick->txSecond.connect(mSystemData.rxTrigger);

    auto mainWindow = createWindow() << BackgroundColor(mTheme.mBaseColor);

    auto topRow = mainWindow << wdg<Container>() << Size{mWidth, mAboveMap} << Position::Zero
                             << wdg<Row>();

    auto sideColumn = mainWindow << wdg<Container>() << Size{mLeftMap, mHeight - mAboveMap} << Position{0, mAboveMap}
                                 << wdg<Column>();

    callsignBlock(topRow, sideColumn);

    for (auto &solar : *solarImageCache) {
        topRow << wdg<Frame>() << BorderStyle::BevelIn << wdg<ImageView>(solar.first);
    }

    mainWindow << wdg<Container>()
               << Position{mLeftMap, mAboveMap}
               << wdg<MapProjection>(clearSkyMaps, Size{mMapWidth, mMapHeight})
               >> mMapProjection;

    solarImageCache->connect(mSecondTick->txSecond, mSecondTick->txMinute);
    celesTrackEphemeris->connect(mSecondTick->txSecond, mSecondTick->txHour);
    clearSkyEphemeris->connect(mSecondTick->txSecond, mSecondTick->txHour);
    clearSkyMaps->connect(mSecondTick->txSecond, mSecondTick->txHour);

    mSecondTick->txSecond.connect(mMapProjection->secondRx);
    mSecondTick->txMinute.connect(mMapProjection->minuteRx);

    solarImageCache->fetchAll();
    celesTrackEphemeris->fetchAll();
    clearSkyEphemeris->fetchAll();
    clearSkyMaps->fetchAll();


    std::filesystem::path ephemerisFile{"/home/richard/.cache/HamChrono/ClearSky/esats.pl?tlename=Moon"};
    Ephemeris ephemeris{ephemerisFile};

    mMapProjection->setMoonEphemeris(ephemeris["Moon"]);
}

void HamChrono::callsignBlock(std::shared_ptr<rose::Row> &topRow, std::shared_ptr<Column> &sideColumn) {
    std::shared_ptr<LinearScale> scale;
    std::shared_ptr<Column> column;
    column = topRow << wdg<Column>();

    column << wdg<Frame>(6) << BorderStyle::Notch << CornerStyle::Round
                << wdg<Column>() << InternalSpace{4}
                    << wdg<CascadeButton>(Id{"CALLSIGN"}, CascadeButtonType::CascadeDown)
                        << ConfigMenu
                        << mConfigButtonRx
                        << CornerStyle::Square
                        << HorizontalAlignment::Center
                        << VerticalAlignment::Center
                        << Elastic(Orientation::Horizontal)
                        << Manip::Parent

                    << wdg<TimeBox>(mSecondTick) << Manip::Parent
                    << wdg<DateBox>(mSecondTick) << Manip::Parent
                    << wdg<LinearScale>(LinearScaleIndicator::DualChannel) >> scale
                        << Parent<Frame>();

    mSystemData.txTemperature.connect(scale->rxScaledValue0);
    mSystemData.txSystem.connect(scale->rxScaledValue1);

    auto qthBlockColumn = column;
    int framPadding = 6;
    if (mWidth == 800) {
        qthBlockColumn = sideColumn;
        framPadding = 3;
    }

    qthBlockColumn << wdg<Frame>(framPadding) << BorderStyle::Notch << CornerStyle::Round << Elastic(Orientation::Horizontal)
                        << wdg<Column>() << InternalSpace{4}
                            << wdg<TimeBox>(mSecondTick, true, true) << Manip::Parent
                            << wdg<DateBox>(mSecondTick, true, true) << Manip::Parent;
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
