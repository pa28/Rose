//
// Created by richard on 2021-03-06.
//

#include "GraphicsModel.h"
#include "Application.h"
#include "Button.h"
#include "Font.h"
#include "Frame.h"
#include "Image.h"
#include "ImageStore.h"
#include "Layout.h"
#include "Text.h"
#include "Theme.h"
#include "Types.h"

using namespace rose;

struct PopupWindow : public Window {
    PopupWindow() {
        setSize(Size{100,100});
        setPosition(Position{0, 0});
    }

    void draw(gm::Context &context, const Position &containerPosition) override {
        setScreenRectangle(containerPosition);
        std::cout << __PRETTY_FUNCTION__ << containerPosition << mScreenRect << '\n';
        context.fillRect(mScreenRect, Theme::getTheme().rgba(rose::ThemeColor::Green));
    }

    Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
        auto rectangle = Rectangle{mPreferredPos, mPreferredSize};
        std::cout << __PRETTY_FUNCTION__ << rectangle << '\n';
        return rectangle;
    }

    void addedToContainer() override {
        std::cout << __PRETTY_FUNCTION__ << '\n';
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
