//
// Created by richard on 2021-03-15.
//
#include "PointerInteractions.h"
#include "GraphicsModel.h"
#include "Application.h"
#include "Button.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Manager.h"
#include "Settings.h"
#include "TimeBox.h"
#include "TimerTick.h"
#include "Types.h"
#include "CelestialOverlay.h"
#include "GridOverlay.h"
#include "SatelliteModel.h"

using namespace rose;

class ChronoLayout : public LayoutManager {
public:

    enum Layout {
        TopLeft, TopRight, BottomLeft, BottomRight,
    };

protected:
    Layout mLayout{TopLeft};

    static constexpr std::string_view LayoutScheme{"LayoutScheme"};

public:

    ChronoLayout() {
        Settings& settings{Settings::getSettings()};
        mLayout = static_cast<Layout>(settings.getValue(LayoutScheme, static_cast<int>(mLayout)));
    }

    ~ChronoLayout() override = default;

    /// Layout the contents of the associated manager.
    Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                            LayoutManager::Itr last) override {

        int width, height;

        if (screenRect.size() < Size(800, 480)) {
            width = 660;
            height = 330;
        } else if (screenRect.size() < Size{1600, 960}) {
            width = screenRect.w - 140;
            height = screenRect.h - 150;
        } else {
            width = screenRect.w - 280;
            height = screenRect.h - 300;
        }

        if ((float) width / (float) height > 2.0) {
            width = height * 2;
        } else {
            height = width / 2;
        }

        Rectangle mapRectangle, sideRect, botRect;

        switch (mLayout) {
            case TopLeft:
                mapRectangle = Rectangle{Position<int>{}, Size{width, height}};
                sideRect = Rectangle{Position<int>{mapRectangle.w, 0}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, mapRectangle.h}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case TopRight:
                mapRectangle = Rectangle{Position<int>{screenRect.w - width, 0}, Size{width, height}};
                sideRect = Rectangle{Position<int>{0, 0}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, mapRectangle.h}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case BottomLeft:
                mapRectangle = Rectangle{Position<int>{0, screenRect.h - height}, Size{width, height}};
                sideRect = Rectangle{Position<int>{mapRectangle.w, mapRectangle.y},
                                     Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, 0}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case BottomRight:
                mapRectangle = Rectangle{Position<int>{screenRect.w - width, screenRect.h - height}, Size{width, height}};
                sideRect = Rectangle{Position<int>{0, mapRectangle.y}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, 0}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
        }

        if (first != last) {
            std::dynamic_pointer_cast<Widget>(*first)->layout(context, mapRectangle);
            std::dynamic_pointer_cast<Visual>(*first)->setScreenRectangle(mapRectangle);
            for (size_t i = 1; first + i != last; i++) {
                if (i == 1) {
                    std::dynamic_pointer_cast<Visual>(*(first + i))->setScreenRectangle(sideRect);
                } else {
                    std::dynamic_pointer_cast<Row>(*(first + i))->layout(context, botRect);
                    std::dynamic_pointer_cast<Row>(*(first + i))->setScreenRectangle(botRect);
                }
            }
        }

        return screenRect;
    }

    bool setLayout(Layout layout) {
        if (mLayout != layout) {
            mLayout = layout;
            Settings& settings{Settings::getSettings()};
            settings.setValue(LayoutScheme, static_cast<int>(mLayout));
            return true;
        }
        return false;
    }
};

class TestWidget : public Widget {
protected:
    color::RGBA mColor;
    ButtonSemantics mButtonSemantics;

public:
    TestWidget() : mButtonSemantics(static_cast<Widget&>(*this)) {
        mButtonSemantics.setButtonDisplayCallback([](ButtonDisplayState buttonDisplayState){
            switch (buttonDisplayState) {
                case rose::ButtonDisplayState::Active:
                    std::cout << __PRETTY_FUNCTION__ << " Active\n";
                    break;
                case rose::ButtonDisplayState::Inactive:
                    std::cout << __PRETTY_FUNCTION__ << " Inactive\n";
                    break;
                case rose::ButtonDisplayState::PressedInactive:
                    std::cout << __PRETTY_FUNCTION__ << " Pressed Inactive\n";
                    break;
                case rose::ButtonDisplayState::PressedActive:
                    std::cout << __PRETTY_FUNCTION__ << " Pressed Active\n";
                    break;
            }
        });
    }

    ~TestWidget() override = default;

    TestWidget(const TestWidget &) = delete;

    TestWidget(TestWidget &&) = delete;

    TestWidget &operator=(const TestWidget &) = delete;

    TestWidget &operator=(TestWidget &&) = delete;

    explicit TestWidget(color::RGBA c) : TestWidget() {
        mSemanticGesture = SemanticGesture::Key | SemanticGesture::Click | SemanticGesture::Scroll | SemanticGesture::Drag;
        mColor = c;
    }

    TestWidget(Size size, color::RGBA c) : TestWidget() {
        mSemanticGesture = SemanticGesture::Key | SemanticGesture::Click | SemanticGesture::Scroll | SemanticGesture::Drag;
        mPreferredSize = size;
        mColor = c;
    }

    /// Draw the visual.
    void draw(gm::Context &context, const Position<int>& containerPosition) override {
        Rectangle dst{containerPosition + mPos, mSize};
        context.fillRect(dst, mColor);
    }

    /// Layout the visual.
    Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
        return Rectangle{mPreferredPos, mPreferredSize};
    }
};

struct Chrono : public Application {
    std::shared_ptr<Manager> mManager{};

    Chrono() = delete;

    Chrono(int argc, char **argv) : Application(argc, argv) {}

    std::shared_ptr<MapProjection> mapProjection{};

    std::shared_ptr<TimerTick> timerTick{};

    bool keyboardEventCallback(const SDL_KeyboardEvent &keyboardEvent) override {
        static Size Size0{800, 480};
        static Size Size1{1600, 960};
        static Size Size2{2400, 1440};
        static Size Size3{3200, 1920};

        if (keyboardEvent.keysym.mod & (uint) KMOD_CTRL) {
            auto displayIndex = SDL_GetWindowDisplayIndex(getSdlWindow().get());
            if (keyboardEvent.keysym.sym == SDLK_F5 || keyboardEvent.keysym.sym == SDLK_F6 ||
                keyboardEvent.keysym.sym == SDLK_F7 || keyboardEvent.keysym.sym == SDLK_F8) {
                Size size{};
                switch (keyboardEvent.keysym.sym) {
                    case SDLK_F5:
                        size = Size0;
                        break;
                    case SDLK_F6:
                        size = Size1;
                        break;
                    case SDLK_F7:
                        size = Size2;
                        break;
                    case SDLK_F8:
                        size = Size3;
                        break;
                }

                if (mGraphicsModel.displayBounds(displayIndex).size() >= size) {
                    SDL_SetWindowSize(getSdlWindow().get(), size.w, size.h);
                    windowSizeChange(EventSemantics::WindowEventType::SizeChanged, size);
                }
                return true;
            } else if (keyboardEvent.keysym.sym == SDLK_F9 || keyboardEvent.keysym.sym == SDLK_F10 ||
                       keyboardEvent.keysym.sym == SDLK_F11 || keyboardEvent.keysym.sym == SDLK_F12) {
                ChronoLayout::Layout layout{};
                switch (keyboardEvent.keysym.sym) {
                    case SDLK_F9:
                        layout = ChronoLayout::Layout::TopLeft;
                        break;
                    case SDLK_F10:
                        layout = ChronoLayout::Layout::TopRight;
                        break;
                    case SDLK_F11:
                        layout = ChronoLayout::Layout::BottomLeft;
                        break;
                    case SDLK_F12:
                        layout = ChronoLayout::Layout::BottomRight;
                        break;
                }

                if (dynamic_cast<ChronoLayout *>(mManager->layoutManager().get())->setLayout(layout)) {
                    Application::layout();
                }
                return true;
            }
        }

        return Application::keyboardEventCallback(keyboardEvent);
    }

    void build() {
        timerTick = std::make_shared<TimerTick>();

        Environment &environment{Environment::getEnvironment()};

        const auto& xdgDataDir = environment.appResources();

        screen() << wdg<Window>()
                 << wdg<Manager>() >> mManager << makeLayout<ChronoLayout>()
                 << wdg<MapProjection>(timerTick, xdgDataDir) >> mapProjection
                    << wdg<GridOverlay>( timerTick ) << endw
                    << wdg<CelestialOverlay>(timerTick, xdgDataDir) << endw << endw
                    << wdg<TestWidget>(color::DarkYellowHSVA.toRGBA()) << endw
                    << wdg<Row>()
                        << wdg<Column>()
                            << wdg<TextButton>(Id{"Callsign"}) << endw
                            << wdg<TimeDateBox>(timerTick, ":Canada/Eastern", true, true);

        registerKeyboardShortcut(SDLK_m, mapProjection, MapProjection::ShortCutCode::MercatorProjection);
        registerKeyboardShortcut(SDLK_s, mapProjection, MapProjection::ShortCutCode::StationMercatorProjection);
        registerKeyboardShortcut(SDLK_a, mapProjection, MapProjection::ShortCutCode::AzimuthalProjection);
        registerKeyboardShortcut(SDLK_t, mapProjection, MapProjection::ShortCutCode::TerrainMap);
        registerKeyboardShortcut(SDLK_c, mapProjection, MapProjection::ShortCutCode::CountryMap);

        Observer observer{45., -75., 0.};
        SatelliteObservation obs{observer};
        obs.passPrediction(6, "ISS");
    }
};

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Chrono application{argc, argv};

    auto moonPhase = MoonPhase();

    std::cout << "Moon phase: " << util::rad2deg(moonPhase)
              << " sin: " << std::sin(moonPhase)
              << " cos: " << std::cos(moonPhase) << "\n\n";

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};
    application.build();
    application.run();
}
