//
// Created by richard on 2021-03-06.
//

#include "GraphicsModel.h"
#include "Application.h"
#include "Button.h"
#include "Font.h"
#include "Frame.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Theme.h"
#include "Types.h"

using namespace rose;

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
    void draw(gm::Context &context, const Position &containerPosition) override {
        Rectangle dst{containerPosition + mPos, mSize};
        context.fillRect(dst, mColor);
    }

    /// Layout the visual.
    Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
        setScreenRectangle(screenRect);
        return Rectangle{mPreferredPos, mPreferredSize};
    }
};

struct PopupWindow : public Window {
    PopupWindow() {
        setSize(Size{200,200});
        setPosition(Position{300, 100});
        mModalWindow = true;
    }

    void draw(gm::Context &context, const Position &containerPosition) override {
        setScreenRectangle(containerPosition);
        std::cout << __PRETTY_FUNCTION__ << containerPosition << mScreenRect << '\n';
        context.fillRect(mScreenRect, Theme::getTheme().rgba(rose::ThemeColor::Green));
    }

    Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
        std::cout << __PRETTY_FUNCTION__ << screenRect << '\n';
        auto rectangle = Window::layout(context, screenRect);
        std::cout << '\t' << rectangle << '\n';
        return Rectangle{50, 50, 200, 200};
    }

    void addedToContainer() override {
        std::cout << __PRETTY_FUNCTION__ << '\n';
        getNode<PopupWindow>() << wdg<Manager>()
                << wdg<TestWidget>(Size{200,200}, color::DarkYellowHSVA.toRGBA());
    }

    ~PopupWindow() override = default;
};

int main(int argc, char **argv) {
    Environment &environment{Environment::getEnvironment()};
    Application application{argc, argv};

    application.initialize(environment.appName(), Size{800, 480});

    ImageStore &imageStore{ImageStore::getStore(application.context())};
    Theme& theme{Theme::getTheme()};

    application.screen() << wdg<Window>()
                         << wdg<Frame>(5) << Position{10, 10} << theme.SemiBevelFrame
                         << wdg<Manager>() << Id{"row"} << makeLayout<LinearLayout>(Orientation::Vertical, 5)
                         << wdg<ImageLabel>(ImageId::HeartEmpty)
                            << curve<ActionCurves::HeartBeat>()
                            << FrameColor{FrameColorType::AnimateColor, theme.rgba(rose::ThemeColor::Red)}
                            << FrameColor{FrameColorType::InactiveColor, theme.hsva(rose::ThemeColor::Red).withValue(0.25).toRGBA()}
                            << AnimationEnable::Enable
                            << endw
                         << wdg<ImageButton>(ImageId::ThreeDots, ButtonType::ToggleButton)
                            << theme.SemiBevelFrame
                            << endw
                         << wdg<TextButton>(Id{"lblHello"}, [&](ButtonStateChange buttonStateChange){
                                 if (buttonStateChange == rose::ButtonStateChange::Pushed) {
                                     std::cout << "Local Button state: Pushed\n";
                                     application.screen() << wdg<PopupWindow>();
                                     application.layout();
                                 }
                             })
                            << theme.SemiBevelFrame
                            << endw
                         << wdg<TextLabel>(Id{"lblHello"})
                            << curve<ActionCurves::PulsePerSecond>()
                            << FrameColor{FrameColorType::AnimateColor, theme.rgba(rose::ThemeColor::Green)}
                            << AnimationEnable::Enable
                            << endw;

    application.run();
}
