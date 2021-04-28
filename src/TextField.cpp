/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-28
 */

#include "TextField.h"
#include "Settings.h"

namespace rose {

    TextField::TextField(int maxLength, Padding padding, PointSize pointSize, const std::string &fontName) {
        mMaxLength = maxLength;
        mPadding = padding;
        mPointSize = pointSize;
        mFontName.fontName = fontName;
        Theme& theme{Theme::getTheme()};
        if (mPointSize.pointSize == 0)
            mPointSize.pointSize = theme.TextPointSize;

        if (mFontName.fontName.empty())
            mFontName.fontName = theme.TextFont;
    }

    TextField::TextField(int maxLength, const std::string &text, const std::string &suffix, const std::string &prefix,
                         Padding padding, PointSize pointSize, const std::string &fontName)
            : TextField(maxLength, padding, pointSize, fontName) {
        mSuffixLabel = std::make_shared<TextLabel>(suffix) << mPointSize << mFontName;
        mPrefixLabel = std::make_shared<TextLabel>(prefix) << mPointSize << mFontName;
        mTextLabel = std::make_shared<TextLabel>(text) << mPointSize << mFontName;
    }

    TextField::TextField(Id id, int maxLength, const std::string &prefix, const std::string &suffix, Padding padding,
                         PointSize pointSize, const std::string &fontName)
            : TextField(maxLength, padding, pointSize, fontName) {
        mId = id;
        mSuffixLabel = std::make_shared<TextLabel>(suffix) << mPointSize << mFontName;
        mPrefixLabel = std::make_shared<TextLabel>(prefix) << mPointSize << mFontName;
    }

    void TextField::addedToContainer() {
        Settings& settings{Settings::getSettings()};

        if (mId) {
            if (auto text = settings.getValue<std::string>(mId.idString); text)
                mText = text.value();
        }

        if (mMaxLength == 0)
            mMaxLength = mText.length();

        if (!mTextLabel) {
            mTextLabel = std::make_shared<TextLabel>(mText) << mPointSize << mFontName;
        }

        getNode<TextField>()
                << mPrefixLabel << endw
                << mTextLabel << endw
                << mSuffixLabel;
    }
}
