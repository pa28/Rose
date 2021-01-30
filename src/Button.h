/** @file Button.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-27
  * @brief The Button Class.
  */



#pragma once

#include "Constants.h"
#include "Frame.h"
#include "Label.h"
#include "Entypo.h"

namespace rose {

    /**
     * @class Button
     * @brief A configurable button class
     * @details A member of the ButtonType enumeration is used to specify the action type of the button. The
     * execution of the button function is handled by a set of signals and slots.
     *
     * Not all signals and slots are active for each type of button.
     *
     *   * rose::NormalButton
     *     * A button which is depressed when a left button pressed event or finger touch event occurs and released
     *     when the corresponding button or finger release event occurs.
     *     * On release a signal is transmitted from Button::txPushed, Frame::mInvert is inverted and the new
     *     value is transmitted on the Button::txState signal.
     *     * On receipt of a signal on Button::rxPushed a signal is transmitted on Button::txPushed.
     */
    class Button : public Frame {
    protected:

        SignalSerialNumber mSignalSerialNumber{};   ///< The button serial number

        ButtonType mButtonType{};                   ///< The button type

        std::string mLabelText{};                   ///< The text of the label

        int mLabelFontSize{};                       ///< The font size of the label
        RoseImageId mBadge{RoseImageInvalid};       ///< The button badge

        bool mSelectProgress{};                     ///< True when button selection is in progress.
        ButtonSetState mButtonSelectState{};        ///< The current selected state of the button.

        /**
         * @brief Handle mouse button events. See Widget::mouseButtonEvent
         */
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

        /**
         * @brief Handle mouse enter events. See Widget::mouseEnterEvent
         */
        bool mouseEnterEvent(const Position &p, bool enter) override;

    public:
        Button();
        ~Button() override = default;
        Button(Button &&) = delete;
        Button(const Button &) = delete;
        Button& operator=(Button &&) = delete;
        Button& operator=(const Button &) = delete;

        /**
         * @brief Constructor
         * @param labelString the text label of the button
         */
        explicit Button(const std::string &labelString);

        /**
         * @brief Constructor
         * @param id Widget id string.
         */
        explicit Button(const Id& id);

        /**
         * @brief Constructor
         * @param labelString the text label of the button
         */
        explicit Button(std::string_view &labelString) : Button(std::string{labelString}) {}

        /**
         * @brief Constructor
         * @param labelString The text label of the button
         */
        explicit Button(const char *labelString) : Button(std::string{labelString}) {}

        /**
         * @brief Construct a Button
         * @param labelString the button label
         * @param type the button type
         * @param fontSize the font size to use with the label
         */
        Button(const std::string &labelString, ButtonType type, int fontSize = 0);

        /**
         * Constructor
         * @param id Widget id string.
         * @param type the button type
         * @param fontSize the font size to use with the label
         */
        Button(const Id& id, ButtonType type, int fontSize = 0);

        /**
         * @brief Constructor
         * @param labelString the button label
         * @param type the button type
         * @param fontSize the font size to use with the label
         */
        Button(const char *labelString, ButtonType type, int fontSize = 0)
                : Button(std::string{labelString}, type, fontSize) {}

        /**
         * @brief See Widget::initializeComposite
         */
        void initializeComposite() override;

        using SignalType = ButtonSignalType ;   ///< The content type of Button signals and slots

        /**
         * @brief A Slot to receive Pushed signals on.
         * @details The Pushed signal is immediately transmitted on txPushed
         */
        std::shared_ptr<Slot<SignalType>> rxPushed;

        /**
         * @brief A Signal to transmit Pusehd signals on.
         * @details A Pushed signal is sent when the button is pushed by the user, or when the Button receives
         * a Pushed event.
         */
        Signal<SignalType> txPushed;

        /**
         * @brief A Slot to receive State signals on.
         * @details The State signal is a boolean and true when the button is selected, toggled on, etc.
         */
        std::shared_ptr<Slot<SignalType>> rxState;

        /**
         * @brief A Signal to transmit
         */
        Signal<SignalType> txState;

        /**
         * @brief Set the button select state
         * @param state The new select state.
         */
        void setSelectState(ButtonSetState state) {
            mButtonSelectState = state;
            switch (mButtonType) {
                case NormalButton:
                case CancelButton:
                case MenuCascade:
                case OkButton:
                    setInvert(false);
                    break;
                case ToggleButton:
                case RadioButton:
                case TabButton:
                    setInvert(!state);
                    break;
            }
        }

        /**
         * @brief Access the button select state
         * @return ButonSetState value.
         */
        ButtonSetState getSelectState() const { return mButtonSelectState; }

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        void setHorizontalAlignment(HorizontalAlignment alignment) override {
            if (!mChildren.empty())
                mChildren.front()->setHorizontalAlignment(alignment);
        }

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        void setVerticalAlignment(VerticalAlignment alignment) override {
            if (!mChildren.empty())
                mChildren.front()->setVerticalAlignment(alignment);
        }

        /**
         * @brief Set the text of the label
         * @param text The text
         */
        void setText(const std::string &text) override;

        /**
         * @brief Get the text of the label
         * @return the text.
         */
        const std::string &getText() const {
            return mChildren.front()->as<Label>()->getText();
        }

        /**
         * @brief Set the font name.
         * @param fontName The font name.
         */
        void setFontName(std::string &fontName);

        /**
         * @brief Set the font size.
         * @param fontSize The point size of the font in pixels.
         */
        void setFontSize(int fontSize);

        /**
         * @brief Set the ImageId for the Button (Label).
         * @param imageId an ImageId.
         */
        void setImageId(ImageId imageId) override;
    };
}



