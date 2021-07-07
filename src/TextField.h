/**
 * @file TextField.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-28
 */

#pragma once

#include "Manager.h"
#include "Button.h"
#include "Animation.h"
#include <regex>
#include <utility>

namespace rose {

    struct RegexPattern {
        std::string regexPattern{};
        explicit RegexPattern(std::string  regex) : regexPattern(std::move(regex)) {}
        explicit RegexPattern(const std::string_view& regex) : RegexPattern(std::string{regex}) {}
    };

    /**
     * @class TextField
     * @brief An Widget to edit a small amount of text.
     */
    class TextField : public TextLabel {
    protected:
        /// Erase the character as if the caret is at the specified location and Backspace is used.
        void eraseChar(int location);

    public:
        TextField() noexcept: TextLabel() {}

        ~TextField() override = default;

        TextField(const TextField &) = delete;

        TextField(TextField &&) = delete;

        TextField &operator=(const TextField &) = delete;

        TextField &operator=(TextField &&) = delete;

        /**
         * @brief Constructor
         * @param maxLength The maximum number of characters in the editable text.
         * @param padding The padding around the text.
         * @param fontSize The font size in pixels, defaults to Theme value,
         * @param fontName The font name, defaults to Theme value.
         */
        explicit TextField(int maxLength, PointSize pointSize = PointSize(), const std::string &fontName = "");

        /**
         * @brief Constructor
         * @param maxLength The maximum number of characters in the editable text.
         * @param prefix A prefix string displayed before the editable text.
         * @param suffix
         * @param padding The padding around the text.
         * @param fontSize The font size in pixels, defaults to Theme value,
         * @param fontName The font name, defaults to Theme value.
         */
        TextField(int maxLength, const std::string &text, const std::string &suffix = "",
                  PointSize pointSize = PointSize(), const std::string &fontName = "");

        /**
         * @brief Constructor
         * @details Create a TextField with parameters taken from the Settings database. Settings must be
         * previously set by a call to TextField::Settings(). This has the added benefit of enabling automatic
         * saving and propagation of values.
         * @param id The Id used to identify the TextFiled and its settings.
         */
        explicit TextField(Id id, int maxLength, const std::string &suffix = "",
                           PointSize pointSize = PointSize(), const std::string &fontName = "");

        void draw(gm::Context &context, const Position<int> &containerPosition) override {
            if (mActionCurve && mAnimationEnableState == AnimationEnable::Enable)
                setAnimation(getWindow(), getNode<Animation>(), containerPosition);
            TextLabel::draw(context,containerPosition);
        }

        void addedToContainer() override;

        void keyboardFocusReceive(bool hasFocus);

        void textInputEvent(const std::string& text);

        void keyboardInput(const SDL_KeyboardEvent &keyEvent);

        bool toUpperCase{false};               ///< If true input is forced to upper case.
    };

    struct TextFieldRegex {
        std::string regex;
    };

    struct ToUpperCase {
        bool toUpperCase;
        explicit ToUpperCase(bool upperCase) : toUpperCase(upperCase) {}
    };
}

inline std::shared_ptr<rose::TextField> operator<<(std::shared_ptr<rose::TextField> textField, const rose::RegexPattern& pattern) {
    textField->setTextValidationPattern(pattern.regexPattern);
    return textField;
}

inline std::shared_ptr<rose::TextField> operator<<(std::shared_ptr<rose::TextField> textField, const rose::ToUpperCase &upperCase) {
    textField->toUpperCase = upperCase.toUpperCase;
    return textField;
}
