/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#include <numeric>
#include <utility>
#include "TextField.h"
#include "Utilities.h"

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
        mCaretLoc = mText.end();
    }

    TextField::TextField(Id id, int padding, FontSize fontSize, const string &fontName) : Frame(padding) {
        mId = std::move(id);
        mFontSize = fontSize;
        mFontName = fontName;
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
                if (mValidationPattern) {
                    mValidEntry = std::regex_match(mText, *mValidationPattern);
                } else {
                    mValidEntry = true;
                }
                auto textColor = mModified ? (mValidEntry ? mTextColor : mErrorColor) : mUnmodifiedColor;
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

    static constexpr char SET_PRE[] = "_pre";
    static constexpr char SET_SUF[] = "_suf";
    static constexpr char SET_MAXLEN[] = "_maxLen";
    static constexpr char SET_EM[] = "_Em";
    static constexpr char SET_TYPE[] = "_type";

    void TextField::enterSettings(std::unique_ptr<rose::Settings> &db, const TextField::FieldSettings &setting) {
        if (!setting.id.empty()) {
            Id id{setting.id};
            db->setValue(id + SET_TYPE, static_cast<int>(setting.dataType));
            if (!setting.suffix.empty())
                db->setValue(id + SET_SUF, setting.suffix);
            if (!setting.prefix.empty())
                db->setValue(id + SET_PRE, setting.prefix);
            if (setting.maxLength)
                db->setValue(id + SET_MAXLEN, setting.maxLength);
            if (setting.eM)
                db->setValue(id + SET_EM, static_cast<int>(setting.eM));
        }
    }

    void TextField::setPair(const Id &pairId, shared_ptr<TextField> &pair) {
        if (mPair) {
            mPair->mPair.reset();
            mPair->mPairId.clear();
            mPair->mPairIdx = 0;
            mPair.reset();
        }
        mPair = pair;
        mPairId = pairId;
        mPairIdx = 0;
        mPair->mPair = getWidget<TextField>();
        mPair->mPairId = pairId;
        mPair->mPairIdx = 1;

        if (mDataType == DataType::Int) {
            auto pairValue = rose()->settings()->getValue(mPairId,Position::Zero);
            mText = util::fmtNumber(pairValue.at(mPairIdx), mMaxLength - 1);
            mCaretLoc = mText.end();
            mPair->mText = util::fmtNumber(pairValue.at(mPair->mPairIdx), mPair->mMaxLength - 1);
            mPair->mCaretLoc = mPair->mText.end();
        } else {
            auto pairValue = rose()->settings()->getValue(mPairId,GeoPosition{0.,0.});
            mText = util::fmtNumber(pairValue.at(mPairIdx), mMaxLength - 1);
            mCaretLoc = mText.end();
            mPair->mText = util::fmtNumber(pairValue.at(mPair->mPairIdx), mPair->mMaxLength - 1);
            mPair->mCaretLoc = mPair->mText.end();
        }
    }

    void TextField::initializeComposite() {
        Frame::initializeComposite();

        if (!mId.empty() && rose()->settings()) {
            mDataType = static_cast<DataType>(rose()->settings()->getValue(mId + SET_TYPE,
                                                                           static_cast<int>(DataType::Unset)));
            switch (mDataType) {
                case DataType::Char: {
                    auto value = rose()->settings()->getValue<int>(mId);
                    if (value) {
                        mText = static_cast<char>(value.value());
                    } else {
                        mText.clear();
                    }
                }
                    break;
                case DataType::Int:
                    mText = util::fmtNumber(rose()->settings()->getValue(mId, 0), mMaxLength - 1);
                    break;
                case DataType::Real:
                    mText = util::fmtNumber(rose()->settings()->getValue(mId, 0.), mMaxLength - 1);
                    break;
                case DataType::Unset:
                case DataType::String:
                    mText = rose()->settings()->getValue(mId, std::string{});
                    break;
            }
            mCaretLoc = mText.end();
            mPrefix = rose()->settings()->getValue(mId + SET_PRE, std::string{});
            mSuffix = rose()->settings()->getValue(mId + SET_SUF, std::string{});
            mMaxLength = rose()->settings()->getValue(mId + SET_MAXLEN, (int) 10);
            mEm = static_cast<char>(rose()->settings()->getValue(mId + SET_EM, (int) 'M'));
        }

        mSupportsKeyboard = true;

        if (mFontSize == 0)
            mFontSize = rose()->theme().mFontPointSize;

        if (mFontName.empty())
            mFontName = rose()->theme().mDefaultFontName;

        mTextColor = rose()->theme().mTextColour;
        mErrorColor = rose()->theme().mRed;
        mUnmodifiedColor = rose()->theme().mGreen;
        mCaretLoc = mText.end();
        mModified = false;
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

    void TextField::saveValue() {
        if (!mPairId.empty() && rose()->hasSettings()) {
            if (mValidEntry && mPair->mValidEntry) {
                if (mModified || mPair->mModified) {
                    if (mDataType == DataType::Int) {
                        Position p{};
                        p.at(mPairIdx) = mText.empty() ? 0 : stoi(mText);
                        p.at(mPair->mPairIdx) = mPair->mText.empty() ? 0 : stoi(mPair->mText);
                        rose()->settings()->setValue(mPairId, p);
                    } else if (mDataType == DataType::Real) {
                        GeoPosition p{};
                        p.at(mPairIdx) = mText.empty() ? 0 : stod(mText);
                        p.at(mPair->mPairIdx) = mPair->mText.empty() ? 0 : stod(mPair->mText);
                        rose()->settings()->setValue(mPairId, p);
                    }
                    mModified = false;
                    mPair->mModified = false;
                    mTextTexture.reset();
                    mPair->mTextTexture.reset();
                }
            }
        } else if (!mId.empty() && rose()->hasSettings()) {
            if (mValidationPattern) {
                mValidEntry = std::regex_match(mText, *mValidationPattern);
            } else {
                mValidEntry = true;
            }

            if (mValidEntry) {
                switch(mDataType) {
                    case DataType::Char:
                        if (mText.empty())
                            rose()->settings()->setValue(mId, 0);
                        else
                            rose()->settings()->setValue(mId, static_cast<int>(mText.front()));
                        break;
                    case DataType::Int:
                        if (mText.empty())
                            rose()->settings()->setValue(mId, 0);
                        else
                            rose()->settings()->setValue(mId, stoi(mText));
                        break;
                    case DataType::Real:
                        if (mText.empty())
                            rose()->settings()->setValue(mId, 0);
                        else
                            rose()->settings()->setValue(mId, stod(mText));
                        break;
                    case DataType::Unset:
                    case DataType::String:
                        rose()->settings()->setValue(mId, mText);
                        break;
                }

                mTextTexture.reset();
                mModified = false;
                setNeedsDrawing();
            }
        }
    }

    bool TextField::textInputEvent(const string &text) {
        for (auto c:text) {
            switch (c) {
                case '\t':
                case '\r':
                    mCaretLoc = mText.end();
                    saveValue();
                    break;
                case '\b':
                    if (!mText.empty() && mCaretLoc != mText.begin()) {
                        mCaretLoc--;
                        mText.erase(mCaretLoc);
                        mModified = true;
                    }
                    break;
                default:
                    if (mText.size() < mMaxLength && c > SDLK_ESCAPE && c < SDLK_DELETE) {
                        if (mCaretLoc == mText.end()) {
                            mText.push_back(mToUpper ? toupper(c) : c);
                            mCaretLoc = mText.end();
                        } else {
                            mText.insert(mCaretLoc, mToUpper ? toupper(c) :  c);
                            mCaretLoc++;
                        }
                        mModified = true;
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
