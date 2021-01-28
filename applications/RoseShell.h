/**
 * @file RoseShell.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-24
 */

#pragma once

#include <memory>
#include "Rose.h"
#include "Timer.h"
#include "Button.h"
#include "Menu.h"

enum UserSignalTokenValues : rose::SignalToken {
    RoseButton = rose::FirstUserSignalToken,        ///< Launch Rose Demo application.
    LifeButton,                                     ///< Launch Conway's Game of Life.
    HamClockButton,
    ExitButton,
};

static constexpr std::array<rose::MenuButtonData,4> appMenu {
        rose::MenuButtonData{ "Rose Demo", RoseButton },
        rose::MenuButtonData{ "Conway's Life", LifeButton},
        rose::MenuButtonData{ "Ham Clock", HamClockButton},
        rose::MenuButtonData{"Exit", ExitButton},
};


/**
 * @class RoseShell
 * @brief A GUI Shell to select which application to run on the FrameBuffer.
 */
class RoseShell : public rose::Rose {
protected:
    std::shared_ptr<rose::SecondTick> mSecondTick;
    std::shared_ptr<rose::Slot<rose::Button::SignalType>> mMenuButtonRx;

public:
    RoseShell() = delete;
    RoseShell(int argc, char **argv, std::string_view title);

    void build();
};

