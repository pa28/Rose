/**
 * @file Callbacks.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 * @brief Definitions to support callbacks.
 */

#pragma once

#include <functional>
#include "Types.h"

namespace rose {

    /**
     * @enum ButtonDisplayState
     * @brief The visible state a button is in.
     */
    enum class ButtonDisplayState {
        Inactive,           ///< Button is inactive (not pushed, toggled off, etc).
        Active,             ///< Button is active (pushed, toggled on, etc).
        PressedActive,      ///< User pressed the button in the Active state.
        PressedInactive,    ///< User pressed the button in the Inactive state.
    };

    /**
     * @brief Basic event callback with no argumetns.
     */
    using EventCallback = std::function<bool()>;

    /**
     * @brief Mouse scroll wheel callback
     * @param deltaPos The change in position indicated by scroll wheel motion.
     */
    using ScrollCallback = std::function<bool(Position deltaPos)>;

    /**
     * @brief Mouse button callback.
     * @param pressed True when a button has been pressed, false when released.
     * @param button A cumulative mask of mouse button state.
     * @param clicks The number of multiple clicks reported by the SDL API.
     */
    using ButtonCallback = std::function<bool(bool pressed, uint button, uint clicks)>;

    /**
     * @brief Mouse motion callback.
     * @param pressed True if a mouse button is pressed.
     * @param state A cumulative mask of mouse button state.
     * @param mousePostion The position of the mouse pointer on the Screen.
     * @param relativePostion The mouse position relative the the last reported position.
     */
    using MouseMotionCallback = std::function<bool(bool pressed, uint state, Position mousePosition,
            Position relativePosition)>;

    /**
     * @brief Button display state callback.
     * @details Called when interaction events change the display state of a button which indicate to the user
     * the evolution of the state of the button.
     * @param buttonDisplayState A ButtonDisplayState value indicating how the button should be displayed.
     */
    using ButtonDisplayCallback = std::function<void(ButtonDisplayState buttonDisplayState)>;

}
