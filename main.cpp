#include <iostream>
#include <future>
#include <TimeBox.h>
#include <curlpp/cURLpp.hpp>
#include <SDL2/SDL_image.h>
#include "Manipulators.h"
#include "Border.h"
#include "Button.h"
#include "Cache.h"
#include "Menu.h"
#include "ImageView.h"
#include "LinearScale.h"
#include "Rose.h"
#include "Settings.h"
#include "SystemMonitor.h"
#include "Tab.h"
#include "Timer.h"
#include "Frame.h"

using namespace rose;

enum UserSignalTokenValues : rose::SignalToken {
    ExitButton = FirstUserSignalToken,         ///< Exit the application.
    TitleGadget,
};

static constexpr std::array<MenuButtonData,1> exitMenu{
        MenuButtonData{ "Exit", ExitButton },
};

class App : public Rose {
public:
    App() = delete;

    ~App() override = default;

    SecondTickPtr mSecondTick{};
    SystemData mSystemData{};

    std::unique_ptr<WebFileCache> solarImageCache;
    std::unique_ptr<WebFileCache> celesTrackEphemeris;
    std::unique_ptr<WebFileCache> clearSkyEphemeris;

    std::shared_ptr<Slot<uint32_t>> solarImageCacheSlot;
    std::shared_ptr<Slot<Button::SignalType>> menuButtonRx;

    std::shared_ptr<LinearScale> mSystemStatus;

    std::mutex mMutex;

    App(int argc, char **argv, const std::string_view title) : rose::Rose(Size{800, 480},
                                                                          argc, argv, title) {
        mSettings = std::make_unique<Settings>(mConfigHome);
        mSettings->initializeDatabase();

        if (mCmdLineParser.cmdOptionExists("-callsign")) {
            mSettings->setValue("CALLSIGN", mCmdLineParser.getCmdOption("-callsign"));
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
    }

    static constexpr std::string_view StdLabel0 = "Hello World!";
    static constexpr std::string_view StdLabel1 = "How's it going?";
    static constexpr std::string_view StdFont = "FreeSans";

#define USE_DEBUG_TUPLE DEBUG_TUPLE(true)

    static SDL_Surface* asyncSurfaceCreate(App *self, uint32_t imageId, std::filesystem::path& filePath) {
        return IMG_Load(filePath.string().c_str());
    }

    void build() {
        mTranslateFingerEvents = true;
        mSecondTick = std::make_shared<SecondTick>();
        solarImageCacheSlot = std::make_shared<Slot<uint32_t>>();

        solarImageCacheSlot->setCallback([=](uint32_t, uint32_t item) {
            auto filePath = solarImageCache->cacheRootPath();
            filePath.append(solarImageCache->find(item)->second.objectSrcName());
            sdl::Surface surface{IMG_Load(filePath.string().c_str())};
            if (surface) {
                mImageRepository.setImageSurface(item, surface);
                needsLayout();
            } else
                std::cout << filePath << " load failed.\n";
        });

        menuButtonRx = std::make_shared<Slot<Button::SignalType>>();

        menuButtonRx->setCallback([=](uint32_t, Button::SignalType signalType){
            if (signalType.first) {
                switch (signalType.second) {
                    case ExitButton:
                        createPopup<ExitDialog>()
                                << menuButtonRx;
                        needsLayout();
                        break;
                    case ExitDialogOk:
                        mRunEventLoop = false;
                        break;
                    case ExitDialogCancel:
                        break;
                    default:
                        std::cout << "Unknown menu button: " << signalType.second << '\n';
                }
            }
        });

        solarImageCache->itemFetched.connect(solarImageCacheSlot);

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

        auto mainWindow = createWindow() << BackgroundColor(mTheme.mBaseColor);
        auto rowWidget = mainWindow << wdg<Row>();
        rowWidget << wdg<Frame>(6)
                    << DrawBackground::None
                    << BorderStyle::Notch
                    << CornerStyle::Round
                    << Position(2, 2)
                        << wdg<Column>() << InternalSpace{4}
                            << wdg<CascadeButton>(Id{"CALLSIGN"})
                                << exitMenu
                                << menuButtonRx
                                << CornerStyle::Square
                                << HorizontalAlignment::Center
                                << VerticalAlignment::Center
                                << Manip::Parent
                            << wdg<TimeBox>(mSecondTick) << Manip::Parent
                            << wdg<DateBox>(mSecondTick) << Manip::Parent
                            << wdg<ImageView>(256, true) << Manip::Parent
                            << wdg<LinearScale>(LinearScaleIndicator::DualChannel) >> mSystemStatus << Manip::Parent
                  << Parent<Row>();

        auto tabWidget = rowWidget
                << wdg<Tab>();

        for (auto & solarImage : *solarImageCache) {
            tabWidget << wdg<TabPage>(solarImage.second.objectUsrName())
                        << wdg<ScrollArea>() << wdg<ImageView>(solarImage.first);
        }

//        auto timeDate = background->add<Widget>()
//                ->withLayout<ColumnLayout>(0, 0)
//                ->withPos(PositionInt{0, 0});
//
//        auto button = timeDate->add<Button>("VE3YSH", 40, ButtonType::ToggleButton);
//        auto timeBox = timeDate->add<TimeBox>();
//        auto dateBox = timeDate->add<DateBox>();
//        auto timeBoxS = timeDate->add<TimeBox>(true)->withLocalTime(true);
//        auto dateBoxS = timeDate->add<DateBox>(true)->withLocalTime(true);
//        auto sysMon = timeDate->add<SystemMonitor>();
//
//        sdl::TextureData mTextureData;
//        createScaledIcon(ENTYPO_ICON_TARGET, "entypo", 50, 8, Color{255, 0, 0, 255}, mTextureData);
//        auto imageId = imageRepository().getImageId();
//        imageRepository().setImage(imageId, std::move(mTextureData));
//
//        auto sprite = background->add<Sprite>(imageId)
//                ->withPos(PositionInt {10,10})
//                ->as<Sprite>();
//        sprite->setVector(SizeFloat{300.f, 150.f}, 300000);
//
//        mSecondTick.txSecond.connect(timeBox->rxSecond);
//        mSecondTick.txHour.connect(dateBox->rxHour);
//        mSecondTick.txMinute.connect(timeBoxS->rxSecond);
//        mSecondTick.txHour.connect(dateBoxS->rxHour);
//        mSecondTick.txSecond.connect(sysMon->rxTrigger);

//        rootWidget()->performInitialLayout(mRenderer);
//        if (rootWidget()->getPrefSize() && rootWidget()->getPrefSize() <= rootWidget()->getScreenSize())
//            rootWidget()->performFinalLayout(mRenderer, rootWidget()->getPrefSize());

        solarImageCache->connect(mSecondTick->txSecond, mSecondTick->txMinute);
        celesTrackEphemeris->connect(mSecondTick->txSecond, mSecondTick->txHour);
        clearSkyEphemeris->connect(mSecondTick->txSecond, mSecondTick->txHour);
        mSecondTick->txSecond.connect(mSystemData.rxTrigger);
        mSystemData.txTemperature.connect(mSystemStatus->rxScaledValue0);
        mSystemData.txProcess.connect(mSystemStatus->rxScaledValue1);

        /**
         *  Fetch calls should occur after all the Signal/Slot connections are made to avoid a race between
         *  the resource being available and the notification path being established.
         */
        solarImageCache->fetchAll();
        celesTrackEphemeris->fetchAll();
        clearSkyEphemeris->fetchAll();
    }
};

int main(int argc, char **argv) {

    // Required if using curlpp or an objects which use it, such as CacheWebSource
    curlpp::Cleanup myCleanup;

    auto app = RoseFactory<App>(argc,argv,"My App");

    app->build();
    app->initialLayout(app->getRenderer());

    if (app) {
        app->eventLoop();
    }

    return static_cast<int>(app->getErrorCode());
}
