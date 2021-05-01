/**
 * @file Text.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#pragma once

#include "Color.h"
#include "Font.h"
#include "GraphicsModel.h"
#include <string>
#include <utility>
#include <regex>

namespace rose {

    static constexpr std::string_view CallPattern = "[A-Z]+[0-9][A-Z]+";
    static constexpr std::string_view FloatPattern = "([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))?";

    struct PointSize {
        int pointSize{};
        PointSize() : pointSize(0) {}
        explicit PointSize(int size) : pointSize(size) {}
    };

    struct FontName {
        std::string fontName;
        FontName() : fontName() {}
        explicit FontName(std::string  name) : fontName(std::move(name)) {}
        FontName& operator=(const FontName& name) {
            fontName = name.fontName;
            return *this;
        }
    };

    /**
     * @class Text
     * @brief Encapsulation of code for rendering Text.
     */
    class Text {
    public:
        enum RenderStyle{
            Blended,        ///< Render text blended on a transparent background.
            Shaded,         ///< Render text shaded on a solid background.
            Solid           ///< Render text without blending or shading. Fastest but lowest quality.
        };

        enum Status {
            OK,                             ///< Operation succeeded.
            TextEmpty,                      ///< The text to be rendered was empty.
            FontError,                      ///< The Font was not found.
            SurfaceError,                   ///< The API returned a nullptr Surface.
            TextureError,                   ///< The API returned a nullptr Texture.
        };

    protected:
        std::string mText{};                ///< The UTF8 or ASCII text to render.
        std::string mSuffix{};              ///< The UTF8 or ASCII suffix.
        color::RGBA mTextFgColor;           ///< The foreground color to use.
        color::RGBA mTextBgColor;           ///< The background color to use if rendering Shaded.
        color::RGBA mCaretColor{};          ///< The color of the caret when editing.
        color::RGBA mRegexFail{};           ///< The text colour for regex validation failure.
        color::RGBA mDataSaved{};           ///< The text colour when the data has been saved to settings.
        float mCaretAlpha{0.f};             ///< The alpha value of the caret during animation.
        RenderStyle mRenderStyle{Blended};  ///< The style of rendering Solid, Shaded, or Blended.
        std::string mFontName;              ///< The name of the True Type Font to use.
        std::shared_ptr<_TTF_Font> mFont{}; ///< The cached font used.
        int mPointSize;                     ///< The point (pixel) size of the font.
        gm::Texture mTexture{};             ///< The generated Texture.
        Size mTextSize{};                   ///< The size of the Texture in pixels.
        Status mStatus{OK};                 ///< The Status of the last operation.
        int mCaretLocation{0};              ///< The location of the caret.
        bool mEditingActive{false};         ///< True when the text is being edited.

        bool mEditable{false};              ///< The label text is editable.
        bool mTextValidated{true};          ///< Ture when the content validates by mValidationPatter.
        bool mSaveToSettings{false};        ///< Set to true when modifications have been saved.
        int mMaxSize{0};                    ///< The maximum number of characters to be held, 0 indicates variable.
        char eM{'N'};                       ///< The character used to compute the maximum screen rectangle width.
        std::unique_ptr<std::regex> mValidationPattern{};   ///< Regular expression to validate content.

        /// Set the editing mode.
        void setEditingMode(bool editing, int carret);

    public:
        Text();
        virtual ~Text() = default;

        Text(const Text &) = delete;
        Text(Text &&) = delete;
        Text& operator=(const Text &) = delete;
        Text& operator=(Text &&) = delete;

        /**
         * @brief Create a Blended Texture from text.
         * @details Fetches the Font corresponding to mFontName and mPointSize, then renders the text in mText as
         * UTF8 to mTexture. The foreground color is set to mTextFgColor. If mRenderStyle is set to Shaded the
         * background color is set to mTextBgColor. The size of the Texture is placed in mTextSize.<p/>
         * If the requested font is not found, or mText is empty any mTexture is reset and mTextSize is set to Zero.
         * @param context The graphics Context.
         * @return The Status of the operation.
         */
        Status createTextureBlended(gm::Context &context);

        /// Set the font point size.
        void setPointSize(int pointSize) {
            mPointSize = pointSize;
        }

        /// Set the font name.
        void setFontName(const std::string &fontName) {
            mFontName = fontName;
        }

        /**
         * @brief Set the text displayed.
         * @param text The new text.
         * @return True if the text has changed, false otherwise.
         */
        bool setText(const std::string &text) {
            if (mText != text) {
                mText = text;
                textUpdated();
                return true;
            }
            return false;
        }

        bool textUpdated();

        void setSuffix(const std::string &suffix) {
            mSuffix = suffix;
        }

        void setTextMaxSize(int maxSize, char em = '\0') {
            mMaxSize = maxSize;
            if (em)
                eM = em;
        }

        void setTextValidationPattern(const std::string& regex) {
            mValidationPattern = std::make_unique<std::regex>(regex);
        }
    };
}

/**
 * @brief Insertion operator to set PointSize on a Text.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param pointSize The PointSize.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::PointSize pointSize) {
    static_assert(std::is_base_of_v<rose::Text,WidgetClass>, "WidgetClass must be derived from rose::Text." );
    widget->setPointSize(pointSize.pointSize);
    return widget;
}

/**
 * @brief Insertion operator to set Font name on a Text.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param pointSize The FontName.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::FontName fontName) {
    static_assert(std::is_base_of_v<rose::Text,WidgetClass>, "WidgetClass must be derived from rose::Text." );
    widget->setFontName(fontName.fontName);
    return widget;
}
