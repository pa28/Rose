//
// Created by richard on 2021-03-15.
//
#include "PointerInteractions.h"
#include "GraphicsModel.h"
#include "Application.h"
#include "Font.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Settings.h"
#include "Types.h"

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
        auto l = settings.getValue(LayoutScheme, static_cast<int>(mLayout));
        mLayout = static_cast<Layout>(settings.getValue(LayoutScheme, static_cast<int>(mLayout)));
    }

    ~ChronoLayout() override = default;

    /// Layout the contents of the associated manager.
    Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                            LayoutManager::Itr last) override {

        std::cout << "\n\n" << __PRETTY_FUNCTION__ << screenRect << '\n';

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

        std::cout << "    width: " << width << ", height: " << height << ", Ratio: " << (float) width / (float) height
                  << '\n';

        Rectangle mapRectangle, sideRect, botRect;

        switch (mLayout) {
            case TopLeft:
                mapRectangle = Rectangle{Position<int>{}, Size{width, height}};
                sideRect = Rectangle{Position<int>{mapRectangle.w, 0},
                                     Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, mapRectangle.h},
                                    Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case TopRight:
                mapRectangle = Rectangle{Position<int>{screenRect.w - width, 0}, Size{width, height}};
                sideRect = Rectangle{Position<int>{0, 0}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, mapRectangle.h},
                                    Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case BottomLeft:
                mapRectangle = Rectangle{Position<int>{0, screenRect.h - height}, Size{width, height}};
                sideRect = Rectangle{Position<int>{mapRectangle.w, mapRectangle.y},
                                     Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, 0}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case BottomRight:
                mapRectangle = Rectangle{Position<int>{screenRect.w - width, screenRect.h - height},
                                         Size{width, height}};
                sideRect = Rectangle{Position<int>{0, mapRectangle.y},
                                     Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position<int>{0, 0}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
        }

        if (first != last) {
            std::dynamic_pointer_cast<Visual>(*first)->setScreenRectangle(mapRectangle);
            for (size_t i = 1; first + i != last; i++) {
                if (i == 1) {
                    std::dynamic_pointer_cast<Visual>(*(first + i))->setScreenRectangle(sideRect);
                } else {
                    std::dynamic_pointer_cast<Visual>(*(first + i))->setScreenRectangle(botRect);
                }
            }
        }

        std::cout << "    " << mapRectangle << sideRect << botRect << '\n';
        return screenRect;
    }

    bool setLayout(Layout layout) {
        if (mLayout != layout) {
            mLayout = layout;
            Settings &settings{Settings::getSettings()};
            settings.setValue(LayoutScheme, static_cast<int>(mLayout));
            return true;
        }
        return false;
    }
};

void drawLine(gm::Context &context, Position<float> p0, Position<float> p1) {
    auto plot = [&context](const Position<int>& p, float alpha) {
        return context.drawPoint(p, color::RGBA::OpaqueWhite.withAlpha(alpha));
    };

    auto ipart = [](float x) {
        return std::floor(x);
    };

    auto fpart = [](float x) {
        return x - std::floor(x);
    };

    auto rfpart = [&](float x) {
        return 1.f - fpart(x);
    };

    auto steep = std::abs(p1.y - p0.y) > std::abs(p1.x - p0.x);

    if (steep) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
    }

    if (p0.x > p1.x) {
        std::swap(p0.x, p1.x);
        std::swap(p0.y, p1.y);
    }

    Position<float> d{p1.x - p0.x, p1.y - p0.y};

    auto gradient = 1.f;
    if (d.x != 0.f)
        gradient = d.y / d.x;

    // handle first endpoint
    auto xend = std::round(p0.x);
    auto yend = p0.y + gradient * (xend - p0.x);
    auto xgap = rfpart(p0.x  + 0.5f);
    auto xpxl1 = xend;
    auto ypxl1 = ipart(yend);

    if (steep) {
        plot(Position<int>{static_cast<int>(ypxl1), static_cast<int>(xpxl1)}, rfpart(yend) * xgap);
        plot(Position<int>{static_cast<int>(ypxl1+1), static_cast<int>(xpxl1)}, fpart(yend) * xgap);
    } else {
        plot(Position<int>{static_cast<int>(xpxl1), static_cast<int>(ypxl1)}, rfpart(yend) * xgap);
        plot(Position<int>{static_cast<int>(xpxl1), static_cast<int>(ypxl1+1)}, fpart(yend) * xgap);
    }

    auto intery = yend + gradient;

    // handle second endpoint
    xend = std::round(p1.x);
    yend = p1.y + gradient * (xend - p1.x);
    xgap = fpart(p1.x + 0.5f);
    auto xpxl2 = xend;
    auto ypxl2 = ipart(yend);

    if (steep) {
        plot(Position<int>{static_cast<int>(ypxl2-1), static_cast<int>(xpxl2)}, rfpart(yend) * xgap);
        plot(Position<int>{static_cast<int>(ypxl2+1), static_cast<int>(xpxl2)}, fpart(yend) * xgap);
    } else {
        plot(Position<int>{static_cast<int>(xpxl2), static_cast<int>(ypxl2-1)}, rfpart(yend) * xgap);
        plot(Position<int>{static_cast<int>(xpxl2), static_cast<int>(ypxl2+1)}, fpart(yend) * xgap);
    }

    // main loop
    if (steep) {
        for (auto x = xpxl1 + 1; x < xpxl2; ++x) {
            plot(Position<int>{ipart(intery)-1, x}, rfpart(intery));
            plot(Position<int>{ipart(intery), x}, 1.f);
            plot(Position<int>{ipart(intery)+1, x}, fpart(intery));
            intery += gradient;
        }
    } else {
        for (auto x = xpxl1 + 1; x < xpxl2; ++x) {
            plot(Position<int>{x, ipart(intery)-1}, rfpart(intery));
            plot(Position<int>{x, ipart(intery)}, 1.f);
            plot(Position<int>{x, ipart(intery)+1}, fpart(intery));
            intery += gradient;
        }
    }
}


class TestWidget : public Widget {
protected:
    color::RGBA mColor;
    ButtonSemantics mButtonSemantics;

public:
    TestWidget() : mButtonSemantics(static_cast<Widget &>(*this)) {
        mButtonSemantics.setButtonDisplayCallback([](ButtonDisplayState buttonDisplayState) {
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
    void draw(gm::Context &context, const Position<int> &containerPosition) override {
        Rectangle dst{containerPosition + mPos, mSize};
        context.fillRect(dst, mColor);

        Position<float> center0{300.f, 300.f};
        Position<float> center1{ 600.f, 300.f};
        float phi = 0.f;
        while (phi < 2. * M_PI) {
            Position<float> p{100.f * sin(phi), 100.f * cos(phi)};
            drawLine(context, center0, p + center0);
            drawLine(context, center1, p + center1);
            phi += M_PI / 18.f;
        }
        std::cout << __PRETTY_FUNCTION__ << '\n';
    }

    /// Layout the visual.
    Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
        return Rectangle{mPreferredPos, mPreferredSize};
    }
};

class TestMap : public TestWidget {
public:
    ~TestMap() override = default;

    TestMap(const TestMap &) = delete;

    TestMap(TestMap &&) = delete;

    TestMap &operator=(const TestMap &) = delete;

    TestMap &operator=(TestMap &&) = delete;

    explicit TestMap() : TestWidget(color::DarkGreenHSVA.toRGBA()) {
    }

    /// Layout the visual.
    Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
        return screenRect;
    }
};

struct Chrono : public Application {
    std::shared_ptr<Manager> mManager{};

    Chrono() = delete;

    ~Chrono() = default;

    Chrono(int argc, char **argv) : Application(argc, argv) {}

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
        std::shared_ptr<Widget> widget{};

        auto m = screen() << wdg<Window>()
                          << wdg<Manager>() >> mManager << makeLayout<ChronoLayout>()
                          << wdg<TestMap>() << endw
                          << wdg<TestWidget>(color::DarkYellowHSVA.toRGBA()) >> widget << endw
                          << wdg<TestWidget>(color::DarkRedHSVA.toRGBA()) << endw;
        registerKeyboardShortcut(SDLK_w, widget, 0);
    }
};

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Chrono application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};
    application.build();
    application.run();
}
