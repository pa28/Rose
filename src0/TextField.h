/**
 * @file TextField.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#pragma once

#include <6>
#include <utility>
#include "Constants.h"
#include "Frame.h"
#include "Texture.h"

namespace rose {

    /**
     * @class TextField
     * @brief An editable, single line, text field.
     * @details The text may have a validation regular expression.
     */
    class TextField : public Frame {
    public:
        struct FieldSettings {
            DataType dataType;
            std::string_view id, prefix, suffix;
            int maxLength;
            char eM;
        };

    protected:
        static void enterSettings(std::unique_ptr<rose::Settings> &db, const FieldSettings &setting);

        void saveValue();

        DataType mDataType{DataType::Unset};                ///< The type of data expected by the TextField.
        char mEm{'M'};                                      ///< The largest Glyph that will be in the text.
        bool mModified{};                                   ///< True when the value has been modified.
        bool mValidEntry{};                                 ///< True if the current value passes validation.
        bool mHasFocus{};                                   ///< True if the TextField has keyboard focus.
        bool mToUpper{};                                    ///< Convert chars to upper case.
        int mMaxLength{};                                   ///< The maximum length of the text.
        FontSize mFontSize{};                               ///< The font size in pixels.
        std::string mFontName{};                            ///< The font name.
        FontPointer mFont{};                                ///< FontPointer to font in use.
        FontMetrics mFontMetrics{};                         ///< The metrics of the current font.
        color::RGBA mTextColor{};                           ///< Text foreground color.
        color::RGBA mErrorColor{};                          ///< The color used when validation fails.
        color::RGBA mUnmodifiedColor{};                     ///< The color used when data is unmodified.
        std::string mPrefix{};                              ///< The prefix to the text.
        std::string mSuffix{};                              ///< The suffix to the text.
        std::string mText{};                                ///< The current text value.
        std::string::iterator mCaretLoc;                    ///< The caret location in mText (one past).
        Size mPrefixSize{};                                 ///< The size of the rendered Prefix.
        Size mSuffixSize{};                                 ///< The size of the rendered Suffix.
        Size mTextSize{};                                   ///< The size of the rendered Text.
        int mTextMaxGlyph{};                                ///< The width of mEm.
        int mGlyphAdvance{};                                ///< The advance of mEm.

        sdl::Texture mPrefixTexture{};                      ///< Texture to render the Prefix;
        sdl::Texture mSuffixTexture{};                      ///< Texture to render the Suffix;
        sdl::Texture mTextTexture{};                        ///< Texture to render the Text;

        std::shared_ptr<std::regex> mValidationPattern{};   ///< Regular expression to validate content.
        std::shared_ptr<TextField> mPair{};                 ///< The companion in a TextField pair.
        Id mPairId{};                                       ///< The Id of the pair.
        size_t mPairIdx{};                                  ///< The index of this object in the pair (0 or 1).

    public:
        TextField() = delete;
        ~TextField() override = default;
        TextField(TextField &&) = delete;
        TextField(const TextField &) = delete;
        TextField& operator=(TextField &&) = delete;
        TextField& operator=(const TextField &) = delete;

        /**
         * @brief Constructor
         * @param maxLength The maximum number of characters in the editable text.
         * @param padding The padding around the text.
         * @param fontSize The font size in pixels, defaults to Theme value,
         * @param fontName The font name, defaults to Theme value.
         */
        explicit TextField(int maxLength, int padding = 0, FontSize fontSize = 0, const string &fontName = "");

        /**
         * @brief Constructor
         * @param maxLength The maximum number of characters in the editable text.
         * @param prefix A prefix string displayed before the editable text.
         * @param suffix
         * @param padding The padding around the text.
         * @param fontSize The font size in pixels, defaults to Theme value,
         * @param fontName The font name, defaults to Theme value.
         */
        TextField(int maxLenght, const std::string &text, const std::string &suffix = "", const std::string &prefix = "",
                  int padding = 0, FontSize fontSize = 0, const string &fontName = "");

        /**
         * @brief Constructor
         * @details Create a TextField with parameters taken from the Settings database. Settings must be
         * previously set by a call to TextField::Settings(). This has the added benefit of enabling automatic
         * saving and propagation of values.
         * @param id The Id used to identify the TextFiled and its settings.
         */
        explicit TextField(Id id, int padding = 0, FontSize fontSize = 0, const string &fontName = "");

        template<size_t N>
        static void Settings(std::unique_ptr<rose::Settings> &db, const std::array<FieldSettings,N> &settings) {
            for (const auto& setting : settings) {
                enterSettings(db, setting);
            }
        }

        /**
         * @brief Set up a pair of TextField widgets.
         * @details A pair of TextField widgets cooperate in editing a pair of integer or real values. This pair
         * takes index = 0.
         * @param pair The other member of the pair.
         */
        void setPair(const Id &pairId, std::shared_ptr<TextField> &pair);

        /// See Frame::widgetLayout()
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /// See Frame::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /// See Widget::initializeComposite();
        void initializeComposite() override;

        /// Set the font size in pixels.
        void setFontSize(FontSize fontSize) override;

        /// Set the font name.
        void setFontName(const std::string &fontName);

        /// Handle text input events.
        bool textInputEvent(const std::string &text) override;

        /// Handle keyboard focus events.
        bool keyboardFocusEvent(bool focus) override;

        /// Handle KeyboardEvent
        bool keyboardEvent(uint state, uint repeat, SDL_Keysym keysym) override;

        /**
         * @brief Set the validation regex.
         * @param regex The validation pattern.
         */
        void setRegex(std::shared_ptr<std::regex> regex) {
            mValidationPattern = std::move(regex);
        }

        /**
         * @brief Set the ToUpper flag.
         * @param toUpper The ToUpper value.
         */
        void setToUpper(bool toUpper) {
            mToUpper = toUpper;
        }

        bool isModified() const { return mModified; }
    };

    struct ToUpperCase {
        bool mToUpperCase{true};
    };
}

/**
 * @brief A Manipulator to set a regular expression on a text field.
 * @tparam WidgetClass The type of Widget.
 * @param widget The Widget.
 * @param regex A std::shared_ptr<std::regex>.
 * @return The Widget.
 */
template <class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const std::shared_ptr<std::regex>& regex) {
    static_assert(std::is_base_of_v<rose::TextField,WidgetClass>, "Regex can only be set on TextField Widgets." );
    widget->setRegex(regex);
    return widget;
}

/**
 * @brief A Manipulator to set the ToUpperCase flag on a TextField.
 * @tparam WidgetClass The type of Widget.
 * @param widget The Widget.
 * @param toUpper The flag value
 * @return The Widget.
 */
template <class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::ToUpperCase toUpperCase) {
    static_assert(std::is_base_of_v<rose::TextField,WidgetClass>, "ToUpperCase can only be set on TextField Widgets." );
    widget->setToUpper(toUpperCase.mToUpperCase);
    return widget;
}
