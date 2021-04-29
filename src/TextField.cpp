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

        Theme &theme{Theme::getTheme()};
        if (pointSize.pointSize == 0)
            mPointSize = theme.TextPointSize;

        if (fontName.empty())
            mFontName = theme.TextFont;

        setTextInputCallback([&](const std::string &text) {
            textInputEvent(text);
        });

        setKeyboardFocusCallback([&](bool hasFocus) {
            keyboardFocusEvent(hasFocus);
        });

        setKeyboardEvent([&](const SDL_KeyboardEvent& keyEvent) {
            keyboardEvent(keyEvent);
        });
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
        Settings &settings{Settings::getSettings()};

        if (mId) {
            if (auto text = settings.getValue<std::string>(mId.idString); text)
                mText = text.value();
        }

        if (mMaxSize == 0)
            mMaxSize = mText.length();
    }

    void TextField::textInputEvent(const std::string &text) {
        std::cout << __PRETTY_FUNCTION__ << ' ' << text << '\n';
    }

    void TextField::keyboardFocusEvent(bool hasFocus) {
        std::cout << __PRETTY_FUNCTION__ << ' ' << (hasFocus ? "Gained Focus.\n" : "Lost Focus.\n");
    }

    void TextField::keyboardEvent(const SDL_KeyboardEvent &keyEvent) {
        std::cout << __PRETTY_FUNCTION__ << ' '
                  << (int)keyEvent.state << ' '
                  << (int)keyEvent.repeat << ' '
                  << (int)keyEvent.keysym.mod << ' '
                  << SDL_GetKeyName(keyEvent.keysym.sym)
                  << '\n';
    }
}
