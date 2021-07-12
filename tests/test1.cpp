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
#include "SDL2_gfxPrimitives.h"

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

    static void drawCircle(gm::Context &context, const Position<int>& center, int radius, color::RGBA &baseColor, bool thick = true) {
        using Segment = unsigned int;
        static constexpr Segment SegmentMirrorX = 0x1;
        static constexpr Segment SegmentMirrorY = 0x2;
        static constexpr Segment SegmentSwap = 0x4;
        static constexpr Segment SegmentFirst = 0x0;
        static constexpr Segment SegmentLast = 0x8;
        static constexpr Segment Segment0 = 0x0;
        static constexpr Segment Segment3 = SegmentMirrorX;
        static constexpr Segment Segment1 = SegmentSwap;
        static constexpr Segment Segment6 = SegmentSwap | SegmentMirrorY;

        int thickness = thick ? 1 : 0;

        auto drawArcStart = [&context, thickness, &baseColor](const Position<int> &center, Position<int> p0, Segment segment) {
            Position<int> p1{p0.x - thickness, p0.y};
            if (segment & SegmentSwap) {
                p0.swap();
                if (thickness > 0)
                    p1.swap();
            }
            if (segment & SegmentMirrorX) {
                p0 = p0.mirrorX();
                if (thickness > 0)
                    p1 = p1.mirrorX();
            }
            if (segment & SegmentMirrorY) {
                p0 = p0.mirrorY();
                if (thickness > 0)
                    p1 = p1.mirrorY();
            }

            context.drawPoint(center + p0, baseColor);
            if (thickness > 0)
                context.drawPoint(center + p1, baseColor);
        };

        auto drawArc = [&context, thickness, &baseColor](const Position<int> &center, Position<int> p0, color::RGBA &raColor,
                                                         const color::RGBA &aColor, Segment segment) {
            Position<int> p1{p0.x - thickness - 1, p0.y};
            Position<int> p2{p0.x - 1, p0.y};
            if (segment & SegmentSwap) {
                p0.swap();
                p1.swap();
                if (thickness > 0)
                    p2.swap();
            }
            if (segment & SegmentMirrorX) {
                p0 = p0.mirrorX();
                p1 = p1.mirrorX();
                if (thickness > 0)
                    p2 = p2.mirrorX();
            }
            if (segment & SegmentMirrorY) {
                p0 = p0.mirrorY();
                p1 = p1.mirrorY();
                if (thickness > 0)
                    p2 = p2.mirrorY();
            }

            context.drawPoint(center + p0, raColor);
            context.drawPoint(center + p1, aColor);

            if (thickness > 0)
                context.drawPoint(center + p2, baseColor);
        };

        auto DC = [](int r, int y) {
            auto dr = static_cast<double>(r);
            auto dy = static_cast<double>(y);
            double x = std::sqrt(dr * dr - dy * dy);
            return std::ceil(x) - x;
        };

        int y = 0;
        int x = radius;
        float lastAlpha = 0.;
        Position<int> p{x,y};
        gm::DrawColorGuard drawColorGuard(context, baseColor);
        context.setDrawBlendMode(SDL_BLENDMODE_BLEND);

        drawArcStart(center, p, Segment0);
        drawArcStart(center, p, Segment1);
        drawArcStart(center, p, Segment6);
        drawArcStart(center, p, Segment3);

        while (x > y) {
            ++y;
            auto alpha = static_cast<float>(DC(radius, y));
            if (alpha < lastAlpha)
                --x;
            Position<int> p0{x, y};

            auto ralphaColor = baseColor.withAlpha(1.0f - alpha);
            auto alphaColor = baseColor.withAlpha(alpha);

            ralphaColor = baseColor * (1.0f - alpha);
            alphaColor = baseColor * alpha;

            for (Segment segment = SegmentFirst; segment != SegmentLast; ++segment)
                drawArc(center, p0, ralphaColor, alphaColor, segment);
            lastAlpha = alpha;
        }
    }

    /// Draw the visual.
    void draw(gm::Context &context, const Position<int> &containerPosition) override {
        Rectangle dst{containerPosition + mPos, mSize};
        context.fillRect(dst, mColor);

        auto baseColor = color::RGBA::OpaqueBlack;
        drawCircle(context, Position<int>{200,200}, 100, baseColor);
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
