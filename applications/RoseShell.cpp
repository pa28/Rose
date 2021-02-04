/**
 * @file RoseShell.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-24
 */

#include "RoseShell.h"
#include "Border.h"
#include "Container.h"
#include "Manipulators.h"
#include "Menu.h"
#include "TimeBox.h"

using namespace rose;

RoseShell::RoseShell(int argc, char **argv, const std::string_view title)
    : Rose(Size{800, 480}, argc, argv, title) {
}

void RoseShell::build() {
    mTranslateFingerEvents = true;
    mSecondTick = std::make_shared<SecondTick>();

    mMenuButtonRx = std::make_shared<Slot<Button::SignalType>>();

    mMenuButtonRx->setCallback([=](uint32_t, Button::SignalType signalType){
        if (signalType.first) {
            switch (signalType.second) {
                case RoseButton:
                    std::cout << "Rose\n";
                    mRunEventLoop = false;
                    break;
                case LifeButton:
                    std::cout << "Life\n";
                    mRunEventLoop = false;
                    break;
                case HamClockButton:
                    std::cout << "/usr/local/bin/hamchrono\n";
                    mRunEventLoop = false;
                    break;
                case UpgradeButton:
                    std::cout << "upgrade\n";
                    mRunEventLoop = false;
                    break;
                case ExitButton:
                    std::cout << "EXIT\n";
                    mRunEventLoop = false;
                    break;
                default:
                    std::cout << "Unknown menu button: " << signalType.second << '\n';
            }
        }
    });

    createRoundCorners(mRenderer, 5, 10, 2,
                       Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createSquareCorners(mRenderer, 10, 2,
                        Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createStandardIcons();
    createCenters(mRenderer, 5, 10);

    auto mainWindow = createWindow() << BackgroundColor(mTheme.mBaseColor);
    auto rowWidget = mainWindow << wdg<Row>();

    rowWidget << wdg<Frame>(6)
              << DrawBackground::None
              << BorderStyle::Notch
              << CornerStyle::Round
                      << wdg<Column>() << InternalSpace{4}
                          << wdg<CascadeButton>("Select Application")
                            << appMenu
                            << mMenuButtonRx
                            << CornerStyle::Square
                            << HorizontalAlignment::Center
                            << VerticalAlignment::Center;

    std::shared_ptr<TimeBox> timeBox;
    std::shared_ptr<DateBox> dateBox;
    std::shared_ptr<TimeBox> gmtTimeBox;
    std::shared_ptr<DateBox> gmtDateBox;


    auto colWidget = rowWidget << wdg<Column>();
    colWidget  << wdg<Frame>(6)
                << DrawBackground::None
                << BorderStyle::Notch
                << CornerStyle::Round
                        << wdg<Column>()
                            << wdg<TimeBox>(mSecondTick) >> timeBox << Manip::Parent
                            << wdg<DateBox>(mSecondTick) >> dateBox << Manip::Parent;

    colWidget   << wdg<Frame>(6)
                << DrawBackground::None
                << BorderStyle::Notch
                << CornerStyle::Round
                        << wdg<Column>()
                            << wdg<TimeBox>(mSecondTick) >> gmtTimeBox << Manip::Parent
                            << wdg<DateBox>(mSecondTick) >> gmtDateBox << Manip::Parent;

    timeBox->setFontSize(90);
    timeBox->setLocalTime(true);
    dateBox->setFontSize(60);
    dateBox->setLocalTime(true);
    gmtTimeBox->setFontSize(90);
    gmtDateBox->setFontSize(60);
}


int main(int argc, char **argv) {

    int retCode = 0;
    try {
        auto app = rose::RoseFactory<RoseShell>(argc, argv, "Rose Shell");

        app->build();
        app->initialLayout(app->getRenderer());

        if (app) {
            app->eventLoop();
        }

        retCode = static_cast<int>(app->getErrorCode());

    } catch (const exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        retCode = static_cast<int>(RoseErrorCode::ROSE_EXCEPTION);
    }

    return retCode;
}
