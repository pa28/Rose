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
     * @class ButtonFrame
     * @brief A base class to give Button semantics to anything which may be parented to a Frame.
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
    class ButtonFrame : public Frame {
        friend class RadioBehavior;

    protected:

        SignalSerialNumber mSignalSerialNumber{};   ///< The button serial number

        ButtonType mButtonType{};                   ///< The button type

        bool mSelectProgress{};                     ///< True when button selection is in progress.
        ButtonSetState mButtonSelectState{};        ///< The current selected state of the button.

        /**
         * @brief Handle mouse button events. See Widget::mouseButtonEvent
         */
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

        /// Handle a click transaction cancel event (default implementation: propagate to children)
        bool clickTransactionCancel(const Position &mousePos, int button, bool down, int modifiers) override;

        /// Update the button state in the Setting database.
        void updateStateSetting(ButtonSetState state);

        std::shared_ptr<Slot<std::string>> mSettingsUpdateRx{};  ///< Slot to receive settings updates on.

    public:
        ButtonFrame() = delete;

        ~ButtonFrame() override = default;

        ButtonFrame(ButtonFrame &&) = delete;

        ButtonFrame(const ButtonFrame &) = delete;

        ButtonFrame &operator=(ButtonFrame &&) = delete;

        ButtonFrame &operator=(const ButtonFrame &) = delete;

        explicit ButtonFrame(int padding = 0) : ButtonFrame(Padding{padding}) {}

        explicit ButtonFrame(Padding padding);

        /**
         * @brief See Widget::initializeComposite
         */
        void initializeComposite() override;

        using SignalType = std::pair<bool, SignalToken>;   ///< The content type of ButtonFrame signals and slots

        /**
         * @brief A Slot to receive Pushed signals on.
         * @details The Pushed signal is immediately transmitted on txPushed
         */
        std::shared_ptr<Slot<SignalType>> rxPushed;

        /**
         * @brief A Signal to transmit Pusehd signals on.
         * @details A Pushed signal is sent when the button is pushed by the user, or when the ButtonFrame receives
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

        /// Set the ButtonFrame StateId.
        void setStateId(const StateId &stateId) override {
            Widget::setStateId(stateId);
            if ((mButtonType == ButtonType::ToggleButton || mButtonType == ButtonType::RadioButton) &&
                                                           !mStateId.empty() && rose()->hasSettings()) {
                setInvert(rose()->settings()->getValue(mStateId.value(), 0) != 0);
                rose()->settings()->dataChangeTx.connect(mSettingsUpdateRx);

            }
        }
    };

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
    class Button : public ButtonFrame {
    protected:
        std::string mLabelText{};                   ///< The text of the label
        ImageId mImageId{RoseImageInvalid};         ///< ImageId of the badge
        int mLabelFontSize{};                       ///< The font size of the label
        RoseImageId mBadge{RoseImageInvalid};       ///< The button badge

    public:
        Button() : ButtonFrame(0) {}

        ~Button() override = default;

        Button(Button &&) = delete;

        Button(const Button &) = delete;

        Button &operator=(Button &&) = delete;

        Button &operator=(const Button &) = delete;

        /**
         * @brief Constructor
         * @param labelString the text label of the button
         */
        explicit Button(const std::string &labelString, ButtonType type = ButtonType::NormalButton, int fontSize = 0);

        /**
         * @brief Constructor
         * @param id Widget id string.
         */
        explicit Button(const Id &id, ButtonType type = ButtonType::NormalButton, int fontSize = 0);

        template<typename String>
        explicit Button(String labelString, ButtonType type = ButtonType::NormalButton)
                : Button(std::string{labelString}, type) {}

        /**
         * @brief Construct an Icon Button
         * @param imageId The icon to use
         * @param type The Button type
         */
        explicit Button(RoseImageId imageId, ButtonType type = ButtonType::NormalButton);

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        void setHorizontalAlignment(HorizontalAlignment alignment) override {
            if (auto label = getSingleChild<Label>(); label)
                label->setHorizontalAlignment(alignment);
        }

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        void setVerticalAlignment(VerticalAlignment alignment) override {
            if (auto label = getSingleChild<Label>(); label)
                label->setVerticalAlignment(alignment);
        }

        /**
         * @brief Set the text of the label
         * @param text The text
         */
        void setText(const std::string &text) override {
            if (auto label = getSingleChild<Label>(); label)
                label->setText(text);
        }

        /**
         * @brief Get the text of the label
         * @return the text.
         */
        std::string getText() const {
            if (auto label = getSingleChild<Label>(); label)
                return label->getText();
            return std::string{""};
        }

        /**
         * @brief Set the font name.
         * @param fontName The font name.
         */
        void setFontName(std::string &fontName) {
            if (auto label = getSingleChild<Label>(); label)
                label->setFontName(fontName);
        }

        /**
         * @brief Set the font size.
         * @param fontSize The point size of the font in pixels.
         */
        void setFontSize(int fontSize) override {
            if (auto label = getSingleChild<Label>(); label)
                label->setFontSize(fontSize);
        }

        /**
         * @brief Set the ImageId for the ButtonFrame (Label).
         * @param imageId an ImageId.
         */
        void setImageId(ImageId imageId) override {
            if (auto label = getSingleChild<Label>(); label)
                label->setImageId(imageId);
        }

        /**
         * @brief Set RenderFlip for the ButtonFrame (Label).
         * @param renderFlip
         */
        void setRenderFlip(sdl::RenderFlip renderFlip) {
            if (auto label = getSingleChild<Label>(); label)
                label->setRenderFlip(renderFlip);
        }

        /// See Widget::setSize()
        void setSize(Size size) override;

    };

    class RadioBehavior {
    public:
        enum class State {
            None,
            SetClear,
            Set,
        };

    protected:
        State mState{None};         ///< The current state.
        int mSelected{0};           ///< The current selected ButtonFrame if any.
        bool mNoneIsValid{};        ///< True if all off is a valid state.

        using ButtonListType = std::pair<SignalToken,shared_ptr<ButtonFrame>>;
        std::vector<ButtonListType> mButtons{};    ///< The list of Radio Buttons

        SignalSerialNumber mSignalSerialNumber{};   ///< The object signal serial number

        std::shared_ptr<Slot<ButtonFrame::SignalType>> buttonStateRx{};     ///< Receive managed button state.

    public:
        using SignalType = std::tuple<State,int,SignalToken>;

        RadioBehavior();

        ~RadioBehavior() = default;

        Signal<SignalType> stateTx{};

        /**
         * @brief Constructor
         * @param noneIsValid Set whether the all buttons off is a valid state.
         */
        explicit RadioBehavior(bool noneIsValid) : RadioBehavior() { mNoneIsValid = noneIsValid; }

        /**
         * @brief Add a ButtonFrame to the set.
         * @param button The ButtonFrame to add.
         */
        void emplace_back(std::shared_ptr<ButtonFrame> &button);

        /**
         * @brief Remove all ButtonFrames from the set.
         * @details The resulting state (cleared to none) is transmitted.
         */
        void clear();

        /**
         * @brief Clear the state.
         * @details A clear state is all buttons off, if that is valid, or the first ButtonFrame on.
         * The resulting state is transmitted.
         */
        void clearState();

        /**
         * @brief Get the current state of the RadioBehavior
         * @return A std::pair with the State and Selected button.
         */
        [[nodiscard]] std::pair<State,int> getState() const {
            return std::make_pair(mState, mSelected);
        }

        /**
         * @brief Set the state, see setState(State,int).
         * @param state A std::pair with the new state.
         */
        void setState(std::pair<State,int> state) {
            setState(state.first, state.second);
        }

        /**
         * @brief Set the state.
         * @details If the requested state is invalid, the state is cleared. The new state is transmitted.
         * @param state The selection State.
         * @param selected The selected ButtonFrame.
         */
        void setState(State state, int selected);
    };
}

/**
 * @addtogroup WidgetManip
 * @{
 */

inline std::shared_ptr<rose::Button>
operator<<(std::shared_ptr<rose::Button> widget, rose::sdl::RenderFlip &renderFlip) {
    widget->setRenderFlip(renderFlip);
    return widget;
}

/** @} */


