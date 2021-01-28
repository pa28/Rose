/** @file Constants.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-15
  * @brief Constants and Enumerations.
  */

#pragma once

#include "Types.h"

namespace rose {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static constexpr uint32_t rmask = 0xff000000;
    static constexpr uint32_t rshift = 24U;
    static constexpr uint32_t gmask = 0x00ff0000;
    static constexpr uint32_t gshift = 16U;
    static constexpr uint32_t bmask = 0x0000ff00;
    static constexpr uint32_t bshift = 8U;
    static constexpr uint32_t amask = 0x000000ff;
    static constexpr uint32_t ashift = 0U;
    static constexpr uint32_t cmask = rmask | gmask | bmask;
#else
    static constexpr uint32_t rmask = 0x000000ffu;
    static constexpr uint32_t rshift = 0U;
    static constexpr uint32_t gmask = 0x0000ff00u;
    static constexpr uint32_t gshift = 8U;
    static constexpr uint32_t bmask = 0x00ff0000u;
    static constexpr uint32_t bshift = 16U;
    static constexpr uint32_t amask = 0xff000000u;
    static constexpr uint32_t ashift = 24U;
    static constexpr uint32_t cmask = rmask | gmask | bmask;
#endif

    /**
     * @defgroup Enumerations Widget Enumerated Values
     * @{
     */

    /**
     * @enum SignalTokenValues
     * @brief These are the signal tokens used by built-in Widgets that transmit Signals with SignalTokens.
     */
    enum SignalTokenValues : rose::SignalToken {
        ExitDialogOk,                   ///< ExitDialog Ok ActionButton
        ExitDialogCancel,               ///< ExitDialog Cancel ActionButton
        FirstUserSignalToken            ///< The value that can be used for the first SignalToken of a user application.
    };

    /**
     * @enum ActionButtonType
     * @brief Types of Dialog action buttons
     * @details The ActionButtonType implies the Button badge and Label.
     */
    enum ActionButtonType {
        ActionButtonOk,     ///< Acknowledge the message or authorize the action described in the dialog.
        ActionButtonCancel, ///< Cancel the action described in the dialog.
    };

    /**
     * @enum ButtonSetState
     * @brief A type alias for bool to set the button state.
     */
    enum ButtonSetState : bool {
        ButtonOff = false,  ///< Button off
        ButtonOn = true,    ///< Button on
    };

    /**
     * @enum ButtonType
     * @brief The action type of the button
     */
    enum ButtonType {
        NormalButton,       ///< A normal push button
        CancelButton,       ///< A normal button with a cancel badge
        OkButton,           ///< A normal button with a check badge
        ToggleButton,       ///< A toggle button
        RadioButton,        ///< A radio button
        TabButton,          ///< A Tab widget button
        MenuCascade,        ///< A Cascade Button, treated as a NormalButton
    };

    static constexpr std::array<std::string_view,2> ActionButtonLabel {
            "OK",       ///< Label for ActionButtonOk
            "Cancel",   ///< Label for ActionButtonCancel
    };

    /**
     * @enum CacheError
     * @brief Error values return by the cache.
     */
    enum CacheError : uint32_t {
        CacheErrorNone = 0,         ///< No error
        CacheErrorNotFound = 1,     ///< Cache item not found.
        CacheErrorWrite = 2,        ///< Could not write to cache store.
        CacheErrorRead = 3,         ///< Could not read from cache store.
        CacheErrorOffset = 10,      ///< Offset for additional errors.
    };

    /**
     * @brief The type of CascadeButton.
     * @details This determines the behaviour of the CascadeButton and associated menu.
     */
    enum class CascadeButtonType {
        Unset,              ///< Not set to a valid value.
        CascadeDown,        ///< Post the associated menu below
        CascadeRight,       ///< Post the associated menu to the right
    };

    enum class Gradient {
        None,                       ///< No background.
        DarkToLight,                ///< Black at LowerBound, White at UpperBound.
        LightToDark,                ///< White at LowerBound, Black at UpperBound.
        GreenToRed,                 ///< Green at LowerBound, Red at UpperBound.
        RedToGreen,                 ///< Red at LowerBound, Green at UpperBound.
        GreenYellowRed,             ///< Green at LowerBound, through Yellow, to Red at UpperBound.
        RedYellowGreen,             ///< Red at LowerBound, through Yellow, to Green at UpperBound.
    };

    /**
     * @enum Modality
     * @brief The Modality of a transient Window, a Popup or derivative.
     */
    enum Modality {
        NotModal,       ///< Not modal, interactions with elements outside the container are allowed.
        Ephemeral,      ///< Not modal, but interactions outside the container cause it to close.
        Modal,          ///< Modal, no interactions outsie the container are allowed while it is open.
    };

    /**
     * @enum Manip
     * @brief Widget manipulators without arguments.
     */
    enum class Manip {
        Parent,             ///< Return the parent of the Widget.
    };

    /**
     * @enum Orientation
     * @brief Possible values for Widget orientation.
     */
    enum class Orientation {
        Unset,          ///< Not set to a valid value.
        Horizontal,     ///< Horizontal orientation.
        Vertical,       ///< Vertical orientation.
    };

    /**
     * @enum RoseImageId
     * @brief Identifiers for Textures created by the Rose object and available from the Rose ImageRepository.
     */
    enum RoseImageId : std::size_t {
        RoseImageInvalid = 0,   ///< Invalid image ID
        IconCancel,             ///< Cancel the action, close a window, answer no, etc.
        IconHelp,               ///< Provide help.
        IconInfo,               ///< Provide information.
        IconCheck,              ///< Approve the action, answer yes, etc.
        IconAlert,              ///< Alert the user to an error, unusual or unexpected occurrence.
        IconDown,               ///< Down facing triangle, CascadeButton
        IconRight,              ///< Right facing triangle, CascadeButton
        BevelOutRoundCorners,   ///< Sheet of round corners beveled out
        BevelInRoundCorners,    ///< Sheet of round corners beveled in
        NotchOutRoundCorners,   ///< Sheet of round corners notched out
        NotchInRoundCorners,    ///< Sheet of round corners notched in
        CenterRoundBaseColor,   ///< Center for the round corners in theme base color
        CenterRoundInvertColor, ///< Center for the round corners in theme invert color
        CenterRoundRedColor,    ///< Center for the round corners in red hue
        CenterRoundGreenColor,  ///< Center for the round corners in green hue
        CenterRoundBlueColor,   ///< Center for the round corners in blue hue
        CenterRoundYellowColor, ///< Center for the round corners in yellow hue
        BevelOutSquareCorners,  ///< Sheet of square corners beveled out
        BevelInSquareCorners,   ///< Sheet of square corners beveled in
        NotchOutSquareCorners,  ///< Sheet of square corners notched out
        NotchInSquareCorners,   ///< Sheet of square corners notched in
        CenterSquareBaseColor,  ///< Center for the square corners in theme base color
        CenterSquareInvertColor, ///< Center for the square corners in theme invert color
    };

    /**
     * @brief Determine if an enumerated value is unset.
     * @details When a value is unset it may be set from the application theme defaults or a widget specific
     * default action may be taken.
     * @tparam EnumClassType The enumeration type
     * @param value The enumerated value.
     * @return True if the value is "unset".
     */
    template<typename EnumClassType>
    inline bool unset(EnumClassType value) { return value == EnumClassType::Unset; }

    /**
     * @enum UseBorder
     * @brief The type of border to draw
     */
    enum UseBorder {
        None,       ///< No border
        BevelOut,   ///< A beveled border that gives the illusion the frame stands up from the display.
        BevelIn,    ///< A beveled border that gives the illusion the frame is sunk into the display.
        NotchOut,   ///< A notched border that looks like a ridge surrounding the frame.
        NotchIn,    ///< A notched border that looks like a trench surrounding the frame.
    };

    /**
     * @enum HorizontalAlignment
     * @brief Horizontal alignment values
     */
    enum class HorizontalAlignment {
        Unset,      ///< Not set to a valid value.
        Center,     ///< Center the aligned content horizontally.
        Left,       ///< Left align the content.
        Right,      ///< Right align the content.
    };

    /**
     * @enum VerticalAlignment
     * @brief Vertical alignment values
     */
    enum class VerticalAlignment {
        Unset,      ///< Not set to a valid value.
        Center,     ///< Center the aligned content vertically.
        Top,        ///< Top align the content.
        Bottom,     ///< Bottom align the content.
    };

    /**
     * @enum LabelHorizontalAlignment
     * @brief Label text horizontal alignment.
     */
    enum class LabelHorizontalAlignment {
        Unset,      ///< Not set to a valid value.
        Center,     ///< Center align the label text horizontally.
        Left,       ///< Left align the label text.
        Right,      ///< Right align the label text.
    };

    enum class LabelVerticalAlignment {
        Unset,      ///< Not set to a valid value.
        Center,     ///< Center align the label text.
        Top,        ///< Top align the the label Text.
        Bottom,     ///< Bottom align the label text.
        Baseline,   ///< Allign the text with a common baseline.
    };

    /**
     * @enum BorderStyle
     * @brief The types of border supported.
     */
    enum class BorderStyle {
        Unset,      ///< Not set to a valid value.
        None,       ///< No border
        Bevel,      ///< A beveled out border
        BevelIn,    ///< A beveled in border
        Notch,      ///< A notch border
        TabTop,     ///< TabButton border, tabs on top
        TabLeft,    ///< TabButton border, tabs on left
        TabPage,    ///< TabPage border.
    };

    /**
     * @enum CornerStyle
     * @brief Types of corners supported.
     */
    enum class CornerStyle {
        Unset,      ///< Not set to a valid value.
        Square,     ///< Square corners
        Round,      ///< Round corners.
    };

    /**
     * @enum DrawBackground
     * @brief Determine if the Widget should draw its own background, if supported.
     */
    enum class DrawBackground {
        Unset,      ///< Not set to a valid value.
        None,       ///< Do not draw the background.
        Widget,     ///< Draw the background.
    };
    /**
     * @enum WidgetSize
     * @brief An enumeration to encode how a widget size should be dealt with by a managing container
     */

    enum WidgetSize {
        VariableSize,   ///< The widget can vary in size.
        FixedSize,      ///< The widget is fixed in size.
    };

    /** @} */
}
