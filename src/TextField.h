/**
 * @file TextField.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#pragma once

#include <regex>
#include "Frame.h"
#include "Texture.h"

namespace rose {

/**
 * @class TextField
 * @brief
 */
    class TextField : public Frame {
        char mEm{'M'};                                      ///< The largest Glyph that will be in the text.
        bool mValidEntry{};                                 ///< True if the current value passes validation.
        bool mHasFocus{};                                   ///< True if the TextField has keyboard focus.
        int mMaxLength{};                                   ///< The maximum length of the text.
        FontSize mFontSize{};                               ///< The font size in pixels.
        std::string mFontName{};                            ///< The font name.
        FontPointer mFont{};                                ///< FontPointer to font in use.
        FontMetrics mFontMetrics{};                         ///< The metrics of the current font.
        color::RGBA mTextColor{};                           ///< Text foreground color.
        color::RGBA mErrorColor{};                          ///< The color used when validation fails.
        std::string mPrefix{};                              ///< The prefix to the text.
        std::string mSuffix{};                              ///< The suffix to the text.
        std::string mText{};                                ///< The current text value.
        Size mPrefixSize{};                                 ///< The size of the rendered Prefix.
        Size mSuffixSize{};                                 ///< The size of the rendered Suffix.
        Size mTextSize{};                                   ///< The size of the rendered Text.
        int mTextMaxGlyph{};                                ///< The width of mEm.
        int mGlyphAdvance{};                                ///< The advance of mEm.

        sdl::Texture mPrefixTexture{};                      ///< Texture to render the Prefix;
        sdl::Texture mSuffixTexture{};                      ///< Texture to render the Suffix;
        sdl::Texture mTextTexture{};                        ///< Texture to render the Text;

        std::regex mValidationPattern{};                    ///< Regular expression to validate content.

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
    };
}

