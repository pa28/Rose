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

namespace rose {

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
        color::RGBA mTextFgColor{};         ///< The foreground color to use.
        color::RGBA mTextBgColor{};         ///< The background color to use if rendering Shaded.
        RenderStyle mRenderStyle{Blended};  ///< The style of rendering Solid, Shaded, or Blended.
        std::string mFontName{};            ///< The name of the True Type Font to use.
        int mPointSize{0};                  ///< The point (pixel) size of the font.
        gm::Texture mTexture{};             ///< The generated Texture.
        Size mTextSize{};                   ///< The size of the Texture in pixels.
        Status mStatus{OK};                 ///< The Status of the last operation.

    public:
        Text() = default;
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
    };
}

