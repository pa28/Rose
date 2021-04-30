/**
 * @file Text.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#include "Settings.h"
#include "Surface.h"
#include "Text.h"
#include "Theme.h"
#include <SDL.h>
#include <SDL_ttf.h>

namespace rose {

    Text::Text() {
        Theme& theme{Theme::getTheme()};
        auto test = theme.hsva(ThemeColor::Red).withValue(0.75);
        mFontName = theme.BoldFont;
        mPointSize = theme.LabelPointSize;
        mTextFgColor = theme.rgba(ThemeColor::Text);
        mTextBgColor = color::RGBA::TransparentBlack;
        mCaretColor = theme.rgba(ThemeColor::YellowText);
        mRegexFail = theme.rgba(ThemeColor::RedText);
        mDataSaved = theme.rgba(ThemeColor::GreenText);
    }

    Text::Status Text::createTextureBlended(gm::Context &context) {
        if (mText.empty() && mSuffix.empty())
            return TextEmpty;

        if (!mFont) {
            FontCache &fontCache = FontCache::getFontCache();
            mFont = fontCache.getFont(mFontName, mPointSize);
        }

        if (mFont) {
            gm::Surface surface{};
            auto textAndSuffix = mText + mSuffix;
            auto fgColor = mTextFgColor;
            if (mSaveToSettings)
                fgColor = mDataSaved;
            else if (!mTextValidated)
                fgColor = mRegexFail;

            switch (mRenderStyle) {
                case Blended:
                    surface.reset(TTF_RenderUTF8_Blended(mFont.get(), textAndSuffix.c_str(), fgColor.toSdlColor()));
                    break;
                case Shaded:
                    surface.reset(
                            TTF_RenderUTF8_Shaded(mFont.get(), textAndSuffix.c_str(), fgColor.toSdlColor(),
                                                  mTextBgColor.toSdlColor()));
                    break;
                case Solid:
                    surface.reset(TTF_RenderUTF8_Solid(mFont.get(), textAndSuffix.c_str(), fgColor.toSdlColor()));
                    break;
            }
            if (surface) {
                mTextSize.w = surface->w;
                mTextSize.h = surface->h;
                if (mMaxSize) {
                    int em;
                    TTF_GlyphMetrics(mFont.get(), eM, nullptr, &em, nullptr, nullptr, nullptr);
                    mTextSize.w = mMaxSize * em;
                }
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

    void Text::setEditingMode(bool editing, int carret) {
        mEditingActive = editing;
        mCaretLocation = std::max(std::min((std::string::size_type)carret, mText.length()),(std::string::size_type)0);
    }

    bool Text::textUpdated() {
        if (mMaxSize && mText.size() > mMaxSize) {
            mText = mText.substr(0, static_cast<unsigned long>(mMaxSize));
            mCaretLocation = std::max(std::min((std::string::size_type)mCaretLocation, mText.length()),(std::string::size_type)0);
            return false;
        }

        mSaveToSettings = false;
        mTexture.reset();
        if (mValidationPattern)
            mTextValidated = std::regex_match(mText, *mValidationPattern);
        else
            mTextValidated = true;
        return true;
    }
}
