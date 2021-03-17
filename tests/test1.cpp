//
// Created by richard on 2021-03-15.
//
#include "GraphicsModel.h"
#include "Application.h"
#include "Font.h"
#include "Frame.h"
#include "Image.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Text.h"
#include "Types.h"

using namespace rose;

struct Test1 : public Application {
    Test1() = delete;

    ~Test1() = default;

    Test1(int argc, char **argv) : Application(argc, argv) {}

    bool keyboardEventCallback(const SDL_KeyboardEvent &keyboardEvent) override {
        static Size Size0{800, 480};
        static Size Size1{1600, 960};
        static Size Size2{2400, 1440};
        static Size Size3{3200, 1920};

        if (keyboardEvent.keysym.mod & (uint) KMOD_CTRL) {
            auto displayIndex = SDL_GetWindowDisplayIndex(getSdlWindow().get());
            Size size{};
            if (keyboardEvent.keysym.sym == SDLK_F8 && mGraphicsModel.displayBounds(displayIndex).size() >= Size3)
                size = Size3;
            else if (keyboardEvent.keysym.sym == SDLK_F7 && mGraphicsModel.displayBounds(displayIndex).size() >= Size2)
                size = Size2;
            else if (keyboardEvent.keysym.sym == SDLK_F6 && mGraphicsModel.displayBounds(displayIndex).size() >= Size1)
                size = Size1;
            else if (keyboardEvent.keysym.sym == SDLK_F5 && mGraphicsModel.displayBounds(displayIndex).size() >= Size0)
                size = Size0;
            else
                return false;

            SDL_SetWindowSize(getSdlWindow().get(), size.w, size.h);
            windowSizeChange(EventSemantics::WindowEventType::SizeChanged, size);
        }

        return Application::keyboardEventCallback(keyboardEvent);
    }
};

class ChronoLayout : public LayoutManager {
public:

    enum Layout {
        TopLeft, TopRight, BottomLeft, BottomRight,
    };

protected:
    Layout mLayout{TopLeft};

public:

    ChronoLayout() = default;

    ~ChronoLayout() override = default;

    /// Layout the contents of the associated manager.
    Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                            LayoutManager::Itr last) override {

        std::cout << "\n\n" <<  __PRETTY_FUNCTION__ << screenRect << '\n';

        int width, height, side, bottom;

        if (screenRect.size() < Size(800, 480)) {
            width = 660;
            height = 330;
            side = 140;
            bottom = 150;
        } else if (screenRect.size() < Size{1600, 960}) {
            side = 140;
            bottom = 150;
            width = screenRect.w - side;
            height = screenRect.h - bottom;
        } else {
            side = 280;
            bottom = 300;
            width = screenRect.w - side;
            height = screenRect.h - bottom;
        }

        if ((float)width/(float)height > 2.0) {
            width = height * 2;
        } else {
            height = width / 2;
        }

        std::cout << "    width: " << width << ", height: " << height << ", Ratio: " << (float)width/(float)height << '\n';

        Rectangle mapRectangle, sideRect, botRect;

        switch (mLayout) {
            case TopLeft:
                mapRectangle = Rectangle{Position::Zero, Size{width,height}};
                sideRect = Rectangle{Position{mapRectangle.w, 0}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position{0, mapRectangle.h}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case TopRight:
                mapRectangle = Rectangle{Position{screenRect.w - width, 0}, Size{width,height}};
                sideRect = Rectangle{Position{0, 0}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position{0, mapRectangle.h}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case BottomLeft:
                mapRectangle = Rectangle{Position{0, screenRect.h - height}, Size{width,height}};
                sideRect = Rectangle{Position{mapRectangle.w, mapRectangle.y}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position{0, 0}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
            case BottomRight:
                mapRectangle = Rectangle{Position{screenRect.w - width, screenRect.h - height}, Size{width,height}};
                sideRect = Rectangle{Position{0, mapRectangle.y}, Size{screenRect.w - mapRectangle.w, mapRectangle.h}};
                botRect = Rectangle{Position{0, 0}, Size{screenRect.w, screenRect.h - mapRectangle.h}};
                break;
        }

        if (first != last) {
            std::dynamic_pointer_cast<Visual>(*first)->setScreenRectangle(mapRectangle);
            for (size_t i = 1; first + i != last; i++) {
                if (i == 1) {
                    std::dynamic_pointer_cast<Visual>(*(first+i))->setScreenRectangle(sideRect);
                } else {
                    std::dynamic_pointer_cast<Visual>(*(first+i))->setScreenRectangle(botRect);
                }
            }
        }

        std::cout << "    " << mapRectangle << sideRect << botRect << '\n';
        return screenRect;
    }

};

class TestWidget : public Widget {
protected:
    color::RGBA mColor;
public:
    TestWidget() = default;

    ~TestWidget() override = default;

    TestWidget(const TestWidget&) = delete;
    TestWidget(TestWidget &&) = delete;
    TestWidget& operator=(const TestWidget&) = delete;
    TestWidget& operator=(TestWidget &&) = delete;

    explicit TestWidget(color::RGBA c) : TestWidget() {
        mColor = c;
    }

    TestWidget(Size size, color::RGBA c) : TestWidget() {
        mPreferredSize = size;
        mColor = c;
    }

    /// Draw the visual.
    void draw(gm::Context &context, const Position &containerPosition) override {
        Rectangle dst{containerPosition + mPos, mSize};
        context.fillRect(dst, mColor);
    }

    /// Layout the visual.
    Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
        return Rectangle{mPreferredPos, mPreferredSize};
    }
};

class TestMap : public TestWidget {
public:
    ~TestMap() override = default;

    TestMap(const TestMap&) = delete;
    TestMap(TestMap &&) = delete;
    TestMap& operator=(const TestMap&) = delete;
    TestMap& operator=(TestMap &&) = delete;

    explicit TestMap() : TestWidget(color::DarkGreenHSVA.toRGBA()) {
    }

    /// Layout the visual.
    Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
        return screenRect;
    }
};

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Test1 application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};

    application.screen() << wdg<Window>()
            << wdg<Manager>() << layout<ChronoLayout>()
                    << wdg<TestMap>() << endw
                    << wdg<TestWidget>(color::DarkYellowHSVA.toRGBA()) << endw
                    << wdg<TestWidget>(color::DarkRedHSVA.toRGBA()) << endw;

    application.run();
}
