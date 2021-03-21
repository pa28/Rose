/**
 * @file ButtonSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 */

#pragma once

namespace rose {

    class Widget;

    enum class ButtonType {
        PushButton,
        ToggleButton,
    };

    class ButtonSemantics {
    protected:

        enum InteractionState {
            Inactive,
            Active,
            PressedActive,
            PressedInactive,
            SetActive,
            SetInactive,
        };

        InteractionState mState{Inactive};
        ButtonType mButtonType{ButtonType::PushButton};
        Widget &mWidget;

        void setButtonState(bool active);
        void enterLeaveCallback();
        void displayState();

    public:
        ButtonSemantics() = delete;
        virtual ~ButtonSemantics() = default;

        explicit ButtonSemantics(Widget &);

        ButtonSemantics(const ButtonSemantics&) = delete;
        ButtonSemantics(ButtonSemantics&&) = delete;

        ButtonSemantics& operator=(const ButtonSemantics&) = delete;
        ButtonSemantics& operator=(ButtonSemantics&&) = delete;
    };
}

