/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#include <numeric>
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

            if (mHasFocus) {
                int caretX = dst.x();
                std::for_each(mText.begin(), mCaretLoc, [&](char c) {
                    auto[hminx, hmaxx, hminy, hmaxy, hadvance] = getGlyphMetrics(mFont, c);
                    caretX += hadvance;
                });

                Rectangle caret{caretX, dst.y(), 2, dst.height()};
                renderer.fillRect(caret, mTextColor);
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
        mCaretLoc = mText.end();
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
                case '\t':
                case '\r':
                    mCaretLoc = mText.end();
                    break;
                case '\b':
                    if (!mText.empty()) {
                        mCaretLoc--;
                        mText.erase(mCaretLoc);
                    }
                    break;
                default:
                    if (mText.size() < mMaxLength && c > SDLK_ESCAPE && c < SDLK_DELETE) {
                        if (mCaretLoc == mText.end()) {
                            mText.push_back(c);
                            mCaretLoc = mText.end();
                        } else {
                            mText.insert(mCaretLoc, c);
                            mCaretLoc++;
                        }
                    } else
                        return false;
            }
        }

        mTextTexture.reset();
        setNeedsDrawing();
        return true;
    }

    bool TextField::keyboardEvent(uint state, uint repeat, SDL_Keysym keysym) {
        auto keyName = SDL_GetScancodeName(keysym.scancode);
        switch (keysym.sym) {
            case SDLK_HOME:
                mCaretLoc = mText.begin();
                break;
            case SDLK_END:
                mCaretLoc = mText.end();
                break;
            case SDLK_LEFT:
                if (state && mCaretLoc > mText.begin())
                    mCaretLoc--;
                else
                    return true;
                break;
            case SDLK_RIGHT:
                if (state && mCaretLoc < mText.end())
                    mCaretLoc++;
                else
                    return true;
                break;
            default:
                return false;
        }

        mTextTexture.reset();
        setNeedsDrawing();
        return true;
    }

    bool TextField::keyboardFocusEvent(bool focus) {
        mHasFocus = focus;
        setNeedsDrawing();
        return true;
    }
}
