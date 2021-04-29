/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-28
 */

#include "TextField.h"
#include "Settings.h"

namespace rose {

    TextField::TextField(int maxLength, PointSize pointSize, const std::string &fontName) : TextLabel() {
        setPointSize(pointSize.pointSize);
        setFontName(fontName);
        setTextMaxSize(maxLength);

        Theme& theme{Theme::getTheme()};
        if (pointSize.pointSize == 0)
            mPointSize = theme.TextPointSize;

        if (fontName.empty())
            mFontName = theme.TextFont;
    }

    TextField::TextField(int maxLength, const std::string &text, const std::string &suffix,
                         PointSize pointSize,
                         const std::string &fontName)
            : TextField(maxLength, pointSize, fontName) {
        setText(text);
        setSuffix(suffix);
    }

    TextField::TextField(Id id, int maxLength, const std::string &suffix, PointSize pointSize,
                         const std::string &fontName)
            : TextField(maxLength, pointSize, fontName) {
        mId = id;
        setSuffix(suffix);
    }

    void TextField::addedToContainer() {
        Settings& settings{Settings::getSettings()};

        if (mId) {
            if (auto text = settings.getValue<std::string>(mId.idString); text)
                mText = text.value();
        }

        if (mMaxSize == 0)
            mMaxSize = mText.length();
    }
}
