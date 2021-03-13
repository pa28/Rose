/**
 * @file Text.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#include "Settings.h"
#include "Surface.h"
#include "Text.h"
#include <SDL.h>
#include <SDL_ttf.h>

namespace rose {

    Text::Status Text::createTextureBlended(gm::Context &context) {
        if (mText.empty())
            return TextEmpty;

        FontCache &fontCache = FontCache::getFontCache();
        if (auto font = fontCache.getFont(mFontName, mPointSize); font) {
            gm::Surface surface{};
            switch (mRenderStyle) {
                case Blended:
                    surface.reset(TTF_RenderUTF8_Blended(font.get(), mText.c_str(), mTextFgColor.toSdlColor()));
                    break;
                case Shaded:
                    surface.reset(
                            TTF_RenderUTF8_Shaded(font.get(), mText.c_str(), mTextFgColor.toSdlColor(),
                                                  mTextBgColor.toSdlColor()));
                    break;
                case Solid:
                    surface.reset(TTF_RenderUTF8_Solid(font.get(), mText.c_str(), mTextFgColor.toSdlColor()));
                    break;
            }
            if (surface) {
                mTextSize.w = surface->w;
                mTextSize.h = surface->h;
                mTexture.reset(SDL_CreateTextureFromSurface(context.get(), surface.get()));
                if (mTexture)
                    return mStatus = OK;
            } else {
                mStatus = SurfaceError;
            }
        } else {
            mStatus = FontError;
        }

        mTexture.reset(nullptr);
        mTextSize = Size::Zero;
        return mStatus;
    }

    TextLabel::TextLabel(const std::string &text, const std::string& fontName, int pointSize) : Widget() {
        mText = text;
        mFontName = fontName;
        mPointSize = pointSize;
        mTextBgColor = color::DarkBaseColor;
        mTextFgColor = color::DarkTextColour;
    }

    TextLabel::TextLabel(const Id &id, const std::string &fontName, int pointSize) {
        mId = id;
        mFontName = fontName;
        mPointSize = pointSize;
        mTextBgColor = color::DarkBaseColor;
        mTextFgColor = color::DarkTextColour;
        Settings &settings{Settings::getSettings()};
        mText = settings.getValue(id.idString, std::string{id.idString});
    }

    void TextLabel::draw(gm::Context &context, const Position &containerPosition) {
        if (!mTexture) {
            createTextureBlended(context);
        }

        if (mTexture) {
            Rectangle dst{containerPosition + mPos, mTextSize};
            context.renderCopy(mTexture, dst);
        }
    }
}
