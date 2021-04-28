/**
 * @file TextField.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-28
 */

#pragma once

#include "Manager.h"
#include "Button.h"
#include <regex>

namespace rose {

    /**
     * @class TextField
     * @brief An Widget to edit a small amount of text.
     */
    class TextField : public Row {
        int mMaxLength{0};
        PointSize mPointSize{0};
        FontName mFontName{};
        std::string mText{};
        std::string mPrefix{};
        std::string mSuffix{};
        std::string mCheckRegex{};
        std::unique_ptr<std::regex> mValidationPattern{};   ///< Regular expression to validate content.

        std::shared_ptr<TextLabel> mPrefixLabel{};
        std::shared_ptr<TextLabel> mSuffixLabel{};
        std::shared_ptr<TextLabel> mTextLabel{};

    public:
        TextField() noexcept: Row() {}

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
        explicit TextField(int maxLength, Padding padding = Padding(), PointSize pointSize = PointSize(),
                           const std::string &fontName = "");

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
                  const std::string &prefix = "", Padding padding = Padding(), PointSize pointSize = PointSize(),
                  const std::string &fontName = "");

        /**
         * @brief Constructor
         * @details Create a TextField with parameters taken from the Settings database. Settings must be
         * previously set by a call to TextField::Settings(). This has the added benefit of enabling automatic
         * saving and propagation of values.
         * @param id The Id used to identify the TextFiled and its settings.
         */
        explicit TextField(Id id, int maxLength, const std::string &prefix = "", const std::string &suffix = "",
                           Padding padding = Padding(), PointSize pointSize = PointSize(),
                           const std::string &fontName = "");

        void addedToContainer() override;

        void setRegex(const std::string& regex) {
            mCheckRegex = regex;
            mValidationPattern = std::make_unique<std::regex>(regex);
        }
    };

    struct TextFieldRegex {
        std::string regex;
    };
}

inline std::shared_ptr<rose::TextField> operator<<(std::shared_ptr<rose::TextField> textField, const rose::TextFieldRegex& regex) {
    textField->setRegex(regex.regex);
    return textField;
}

