/**
 * @file ButtonSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 */

#pragma once

#include "Callbacks.h"

namespace rose {

    class Widget;

    /**
     * @enum ButtonType
     * @brief The button type.
     * @details Determines how buttons respond to user interaction.
     */
    enum class ButtonType {
        Label,              ///< A button without any button semantics but can be inverted and animated.
        PushButton,         ///< When pressed and released signals an action.
        ToggleButton,       ///< When pressed and released changes state from Active to Inactive or back.
    };

    /**
     * @class ButtonSemantics
     * @brief Interpret interaction events to drive button operation semantics.
     */
    class ButtonSemantics {
    protected:

        /**
         * @enum InteractionState
         * @brief Internal state.
         */
        enum InteractionState {
            Inactive,           ///< Button in Inactive state.
            Active,             ///< Button in Active state.
            PressedActive,      ///< Button was pressed in the Active state.
            PressedInactive,    ///< Button was pressed in the Inactive state.
            SetActive,          ///< Transitional state moving to Active.
            SetInactive,        ///< Transitional state moving to Inactive.
        };

        InteractionState mState{Inactive};                  ///< Hold the internal interaction state.
        ButtonType mButtonType{ButtonType::PushButton};     ///< The button type.
        Widget &mWidget;                                    ///< The Widget that implements the button.
        uint mButtonCommandId{0};                           ///< The Widget command associated with the button.


        void setButtonState(bool active);   ///< Called when the Active/Inactive state is resolved.
        void enterLeaveCallback();          ///< Callback when the pointer enters or leaves the Widget.
        void displayState();                ///< Called when the button display state changes.

        ButtonDisplayCallback mButtonDisplayCallback{};         ///< A callback to the Widget to change the display state.
        ButtonStateChangeCallback mButtonStateChangeCallback{}; ///< A callback to the Widget with the new button state.
        ButtonCommandCallback mButtonCommandCallback{};         ///< A callback to the Widget with the button state and command id.

    public:
        ButtonSemantics() = delete;
        virtual ~ButtonSemantics() = default;

        explicit ButtonSemantics(Widget &);

        ButtonSemantics(const ButtonSemantics&) = delete;
        ButtonSemantics(ButtonSemantics&&) = delete;

        ButtonSemantics& operator=(const ButtonSemantics&) = delete;
        ButtonSemantics& operator=(ButtonSemantics&&) = delete;

        void setButtonType(ButtonType buttonType);

        /**
         * @brief Set the ButtonDisplayCallback function.
         * @param buttonDisplayCallback The function.
         */
        void setButtonDisplayCallback(ButtonDisplayCallback buttonDisplayCallback) {
            mButtonDisplayCallback = std::move(buttonDisplayCallback);
        }

        /**
         * @brief Set the ButtonStateChangeCallback function.
         * @param buttonStateChangeCallback The function.
         */
        void setButtonStateChangeCallback(ButtonStateChangeCallback buttonStateChangeCallback) {
            mButtonStateChangeCallback = std::move(buttonStateChangeCallback);
        }

        /**
         * @param Set the ButtonCommandCallback.
         * @param commandId The identifier of the command to the Widget receiving the callback.
         * @param buttonCommandCallback The callback.
         */
        void setButtonCommandCallback(uint commandId, ButtonCommandCallback buttonCommandCallback) {
            mButtonCommandId = commandId;
            mButtonCommandCallback = std::move(buttonCommandCallback);
        }
    };
}

