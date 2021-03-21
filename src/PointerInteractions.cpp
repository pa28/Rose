/**
 * @file ButtonSemantics.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 */

#include <exception>
#include "PointerInteractions.h"
#include "Visual.h"

namespace rose {

    ButtonSemantics::ButtonSemantics(Widget &widget) : mWidget(widget) {
        mWidget.setButtonEventCallback([&](bool pressed, uint button, uint clicks) {
            switch (mButtonType) {
                case ButtonType::PushButton:
                    switch (mState) {
                        case Inactive:
                        case Active:
                            if (pressed && (button & 1u))
                                mState = PressedInactive;
                            else if (!pressed && !(button &1u))
                                return true;
                            break;
                        case PressedInactive:
                            if (!pressed && !(button & 1u))
                                mState = SetActive;
                            break;
                        case PressedActive:
                            if (!pressed && !(button & 1u))
                                mState = SetInactive;
                            break;
                        case SetInactive:
                        case SetActive:
                            throw std::logic_error("Button transition to terminal in state machine.");
                    }
                    if (mState == SetActive || mState == SetActive) {
                        setButtonState(mState == SetActive);
                    }
                    break;
                case ButtonType::ToggleButton:
                    switch (mState) {
                        case Inactive:
                            if (pressed && (button & 1u))
                                mState = PressedInactive;
                            break;
                        case Active:
                            if (pressed && (button & 1u))
                                mState = PressedActive;
                            break;
                        case PressedInactive:
                            if (!pressed && !(button & 1u))
                                mState = SetActive;
                            break;
                        case PressedActive:
                            if (!pressed && !(button & 1u))
                                mState = SetInactive;
                            break;
                        case SetInactive:
                        case SetActive:
                            throw std::logic_error("Button transition to terminal in state machine.");
                    }
                    if (mState == SetActive || mState == SetInactive) {
                        setButtonState(mState == SetActive);
                    }
                    break;
            }
            displayState();
            return true;
        });

        mWidget.setEnterLeaveEventCallback([&]() {
            enterLeaveCallback();
            return true;
        });

        mWidget.setLeaveEventCallback([&]() {
            enterLeaveCallback();
            return true;
        });

        mWidget.setMouseMotionEventCallback([&](bool pressed, uint state, Position mousePosition,
                                                Position relativePosition) {
            enterLeaveCallback();
            return true;
        });
    }

    void ButtonSemantics::setButtonState(bool active) {
        std::cout << __PRETTY_FUNCTION__ << ' ';
        switch (mButtonType) {
            case ButtonType::PushButton:
                if (active) {
                    mState = Inactive;
                    std::cout << "Pushed\n";
                }
                break;
            case ButtonType::ToggleButton:
                if (active) {
                    mState = Active;
                    std::cout << "Active\n";
                } else {
                    mState = Inactive;
                    std::cout << "Inactive\n";
                }
                break;
        }
    }

    void ButtonSemantics::enterLeaveCallback() {
        auto oldState = mState;
        switch (mState) {
            case Active:
            case Inactive:
                break;
            case PressedActive:
                mState = Active;
                break;
            case PressedInactive:
                mState = Inactive;
                break;
            case SetInactive:
            case SetActive:
                throw std::logic_error("Button transition to terminal in state machine.");
        }
        if (oldState != mState)
            displayState();
    }

    void ButtonSemantics::displayState() {
        std::cout << __PRETTY_FUNCTION__ << ' ';
        switch (mState) {
            case Active:
                std::cout << "Active";
                break;
            case Inactive:
                std::cout << "Inactive";
                break;
            case PressedInactive:
                std::cout << "PressedInactive";
                break;
            case PressedActive:
                std::cout << "PressedActive";
                break;
            case SetActive:
            case SetInactive:
                std::cout << "Error state.";
                break;
        }
        std::cout << '\n';
    }
}
