/**
 * @file ButtonSemantics.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 */

#include <stdexcept>
#include "PointerInteractions.h"
#include "Visual.h"

namespace rose {

    ButtonSemantics::ButtonSemantics(Widget &widget) : mWidget(widget) {

        // Get and process the mouse button events from the Widget.
        mWidget.setButtonEventCallback([&](bool pressed, uint button, uint clicks) {
            auto oldState = mState;
            switch (mButtonType) {
                case ButtonType::PushButton:
                    switch (mState) {
                        case Inactive:
                            if (pressed && (button & 1u))
                                mState = PressedInactive;
                            else if (!pressed && !(button &1u))
                                return true;
                            break;
                        case PressedInactive:
                            if (!pressed && !(button & 1u))
                                mState = SetActive;
                            break;
                        case Active:
                        case PressedActive:
                        case SetInactive:
                        case SetActive:
                            throw std::logic_error("Button transition to terminal in state machine.");
                    }
                    if (mState == SetActive) {
                        setButtonState(true);
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
            if (oldState != mState)
                displayState();
            return true;
        });

        // Get and process the enter events from the Widget.
        mWidget.setEnterLeaveEventCallback([&]() {
            enterLeaveCallback();
            return true;
        });

        // Get and process the leave events from the widget.
        mWidget.setLeaveEventCallback([&]() {
            enterLeaveCallback();
            return true;
        });

        // Get and process the keyboard shortcut events from the widget.
        mWidget.setKeyboardShortcutCallback([&](SDL_Keycode keycode, bool state, uint repeat){

            switch (mButtonType) {
                case ButtonType::PushButton:
                    switch (mState) {
                        case Inactive:
                            if (state && (repeat == 0))
                                mState = PressedInactive;
                            break;
                        case PressedInactive:
                            if (!state || repeat != 0)
                                mState = SetActive;
                            break;
                        case Active:
                        case PressedActive:
                        case SetInactive:
                        case SetActive:
                            throw std::logic_error("Button transition to terminal in state machine.");
                    }
                    if (mState == SetActive) {
                        setButtonState(true);
                    }
                    break;
                case ButtonType::ToggleButton:
                    switch (mState) {
                        case Inactive:
                            if (state && repeat == 0)
                                mState = PressedInactive;
                            break;
                        case Active:
                            if (state && repeat == 0)
                                mState = PressedActive;
                            break;
                        case PressedInactive:
                            if (!state || repeat > 0)
                                mState = SetActive;
                            break;
                        case PressedActive:
                            if (!state || repeat > 0)
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
        });
    }

    void ButtonSemantics::setButtonState(bool active) {
        switch (mButtonType) {
            case ButtonType::PushButton:
                if (active) {
                    mState = Inactive;
                    if (mButtonStateChangeCallback)
                        mButtonStateChangeCallback(ButtonStateChange::Pushed);
                    if (mButtonCommandCallback)
                        mButtonCommandCallback(ButtonStateChange::Pushed, mButtonCommandId);
                }
                break;
            case ButtonType::ToggleButton:
                if (active) {
                    mState = Active;
                    if (mButtonStateChangeCallback)
                        mButtonStateChangeCallback(ButtonStateChange::On);
                    if (mButtonCommandCallback)
                        mButtonCommandCallback(ButtonStateChange::On, mButtonCommandId);
                } else {
                    mState = Inactive;
                    if (mButtonStateChangeCallback)
                        mButtonStateChangeCallback(ButtonStateChange::Off);
                    if (mButtonCommandCallback)
                        mButtonCommandCallback(ButtonStateChange::Off, mButtonCommandId);
                }
                break;
            case ButtonType::Label:
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
        if (mButtonDisplayCallback)
            switch (mState) {
                case Active:
                    mButtonDisplayCallback(ButtonDisplayState::Active);
                    break;
                case Inactive:
                    mButtonDisplayCallback(ButtonDisplayState::Inactive);
                    break;
                case PressedInactive:
                    mButtonDisplayCallback(ButtonDisplayState::PressedInactive);
                    break;
                case PressedActive:
                    mButtonDisplayCallback(ButtonDisplayState::PressedActive);
                    break;
                case SetActive:
                case SetInactive:
                    throw std::logic_error("Button transition to terminal in state machine.");
            }
    }

    void ButtonSemantics::setButtonType(ButtonType buttonType) {
        mButtonType = buttonType;
        if (mState != Inactive) {
            mState = Inactive;
            displayState();
            setButtonState(false);
        }
    }
}
