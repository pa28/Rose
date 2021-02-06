/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#include "TextField.h"

namespace rose {

    TextField::TextField(int maxLength, int padding, FontSize fontSize, const string &fontName) : Frame(padding) {
        mMaxLength = maxLength;
        mFontSize = fontSize;
        mFontName = fontName;
    }

    TextField::TextField(int maxLength, const string &text, const string &suffix, const string &prefix, int padding,
                         FontSize fontSize, const string &fontName) : Frame(padding) {
        mMaxLength = maxLength;
        mFontSize = fontSize;
        mFontName = fontName;
        mPrefix = prefix;
        mSuffix = suffix;
        mText = text;
    }

    Rectangle TextField::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto frameAvailable = clampAvailableArea(available, mPos, mSize);
        frameAvailable = mLayoutHints.layoutBegin(frameAvailable);
        Rectangle layout{};

        // Layout space for the text field, prefix and suffix.
        if (!mFont) {
            mFont = rose::fetchFont(rose()->fontCache(), mFontName, mFontSize);
            mFontMetrics = rose::getFontMetrics(mFont);
            auto[hminx, hmaxx, hminy, hmaxy, hadvance] = getGlyphMetrics(mFont, mEm);
            mTextMaxGlyph = hmaxx - hminx;
            mGlyphAdvance = hadvance;
        }

        mPrefixSize = mPrefix.empty() ? Size::Zero : Size{textSizeUTF8(mFont, mPrefix)};
        mSuffixSize = mSuffix.empty() ? Size::Zero : Size{textSizeUTF8(mFont, mSuffix)};

        layout.width() = mPrefixSize.width() + mSuffixSize.width() + (mMaxLength + 1) * mGlyphAdvance;
        layout.height() = mFontMetrics.fontLineSkip;

        layout = mLayoutHints.layoutEnd(layout);

        return layout;
    }

    void TextField::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            auto totalText = mPrefix + ' ' + mText + ' ' + mSuffix;
            auto totalTexture = sdl::renderTextureBlendedUTF8(renderer, mFont, totalText, mTextColor);
            Size totalSize{totalTexture.getSize()};

            if (!mPrefixTexture && !mPrefix.empty()) {
                mPrefixTexture = sdl::renderTextureBlendedUTF8(renderer, mFont, mPrefix, mTextColor);
                mPrefixSize = mPrefixTexture.getSize();
            }

            if (!mSuffixTexture && !mSuffix.empty()) {
                mSuffixTexture = sdl::renderTextureBlendedUTF8(renderer, mFont, mSuffix, mTextColor);
                mSuffixSize = mSuffixTexture.getSize();
            }

            if (!mTextTexture && !mText.empty()) {
                auto textColor = (mValidEntry ? mTextColor : mErrorColor);
                mTextTexture = sdl::renderTextureBlendedUTF8(renderer, mFont, mText, textColor);
                mTextSize = mTextTexture.getSize();
            }

            drawFrameOnly(renderer, widgetRect);

            auto dst = widgetRect;
            dst.y() += mLayoutHints.mFrameWidth + mLayoutHints.mPadding.top();
            dst.x() += mLayoutHints.mFrameWidth + mLayoutHints.mPadding.left();

            if (mPrefixTexture) {
                dst = mPrefixSize;
                renderer.renderCopy(mPrefixTexture, dst);
                dst.x() += mGlyphAdvance/2 + mPrefixSize.width();
            }

            if (mTextTexture) {
                dst = mTextSize;
                renderer.renderCopy(mTextTexture, dst);
            }
            dst.x() += mGlyphAdvance/2 + mGlyphAdvance * (mMaxLength);

            if (mSuffixTexture) {
                dst = mSuffixSize;
                renderer.renderCopy(mSuffixTexture, dst);
            }
        }

    }

    void TextField::initializeComposite() {
        Frame::initializeComposite();
        mSupportsKeyboard = true;

        if (mFontSize == 0)
            mFontSize = rose()->theme().mFontPointSize;

        if (mFontName.empty())
            mFontName = rose()->theme().mDefaultFontName;

        mTextColor = rose()->theme().mTextColour;
        mErrorColor = rose()->theme().mRed;
    }

    void TextField::setFontSize(FontSize fontSize) {
        mFontSize = fontSize;
        mFont.reset();
        rose()->needsLayout();
    }

    void TextField::setFontName(const string &fontName) {
        mFontName = fontName;
        mFont.reset();
        rose()->needsLayout();
    }

    bool TextField::textInputEvent(const string &text) {
        for (auto c:text) {
            switch (c) {
                case '\r':
                    break;
                case '\b':
                    if (!mText.empty())
                        mText.erase(mText.end()-1);
                    break;
                default:
                    mText.push_back(c);
            }
        }
        mTextTexture.reset();
        setNeedsDrawing();
        return true;
    }

    bool TextField::keyboardFocusEvent(bool focus) {
        mHasFocus = focus;
        return true;
    }
}
