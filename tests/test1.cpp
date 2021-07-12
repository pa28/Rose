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

        std::cout << __PRETTY_FUNCTION__ << '\n';

        auto DC = [](int r, int y) {
            auto dr = static_cast<double>(r);
            auto dy = static_cast<double>(y);
            double x = std::sqrt(dr*dr - dy*dy);
            return std::ceil(x) - x;
        };

        Position<int> c{200, 200};
        auto baseColor = color::RGBA::OpaqueBlack;
        int thickness = 0;
        int radius = 100;
        int y = 0;
        int x = radius;
        float lastAlpha = 0.;
        Position<int> p{x,y};
        gm::DrawColorGuard drawColorGuard(context, baseColor);
        context.setDrawBlendMode(SDL_BLENDMODE_BLEND);
        context.drawPoint(c + p, baseColor);
        context.drawPoint(c + p.mirrorX(), baseColor);
        p.swap();
        context.drawPoint(c + p, baseColor);
        context.drawPoint(c + p.mirrorY(), baseColor);
        while (x > y) {
            ++y;
            auto alpha = static_cast<float>(DC(radius, y));
            if (alpha < lastAlpha)
                --x;
            Position<int> p0{x,y};
            Position<int> p1{x-thickness-1,y};

            auto ralphaColor = baseColor.withAlpha(1.0f - alpha);
            auto alphaColor = baseColor.withAlpha(alpha);

            // Segments 0 and 3 are horizontally opposed.
            // Draw segment 0 - This is the way to stuff Wu anti-aliasing.
            context.drawPoint(c + p0, ralphaColor);
            context.drawPoint(c + p1, alphaColor);
            if (thickness > 0) {
                Position<int> t1{p0.x - thickness, p0.y};
                if (thickness > 1) {
                    Position<int> t0{p0.x - 1, p0.y};
                    context.drawLine(c + t0, c + t1);
                }
                context.drawPoint(c+t1, baseColor);
            }
            // Draw segment 3
            context.drawPoint(c + p0.mirrorX(), ralphaColor);
            if (thickness > 1) {
                for (Position<int> t{-p0.x+1, p0.y}; t.x < -p1.x; ++t.x)
                    context.drawPoint(c+t, baseColor);
            }
            context.drawPoint(c + p1.mirrorX(), alphaColor);

            // Segments 4 and 7 are horizontally opposed.
            // Draw Segment 7
            context.drawPoint(c + p0.mirrorY(), ralphaColor);
            if (thickness > 1) {
                for (Position<int> t{p0.x-1, -p0.y}; t.x > p1.x; --t.x)
                    context.drawPoint(c+t, baseColor);
            }
            context.drawPoint(c + p1.mirrorY(), alphaColor);
            // Draw segment 4
            context.drawPoint(c - p0, ralphaColor);
            if (thickness > 1) {
                for (Position<int> t{-p0.x+1, -p0.y}; t.x < -p1.x; ++t.x)
                    context.drawPoint(c+t, baseColor);
            }
            context.drawPoint(c - p1, alphaColor);
            p0.swap();
            p1.swap();

            // Segments 1 and 2 are horizontally opposed.
            // Draw segment 1
            context.drawPoint(c + p0, ralphaColor);
            context.drawPoint(c + p1, alphaColor);
            // Draw segment 2
            context.drawPoint(c + p0.mirrorX(), ralphaColor);
            context.drawPoint(c + p1.mirrorX(), alphaColor);

            // Segments 5 and 6 are horizontally opposed.
            // Draw segment 6
            context.drawPoint(c + p0.mirrorY(), ralphaColor);
            context.drawPoint(c + p1.mirrorY(), alphaColor);
            // Draw segment 5
            context.drawPoint(c - p0, ralphaColor);
            context.drawPoint(c - p1, alphaColor);
            lastAlpha = alpha;
        }
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
