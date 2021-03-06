/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-28
 */

#include "TextField.h"
#include "Settings.h"
#include "Application.h"
#include "Animation.h"

namespace rose {

    TextField::TextField(int maxLength, PointSize pointSize, const std::string &fontName) : TextLabel() {
        mAnimationCallback = [&](gm::Context &context, const Position<int> &position, uint32_t frame) {
            if (mActionCurve) {
                auto idx = frame % mActionCurve->size();
                mCaretAlpha = (*mActionCurve)[idx];
                draw(context, position);
            } else {
                removeAnimation(getWindow(), getNode<Animation>());
            }
        };

        mAnimationEnableStateCallback = [&](AnimationEnable animationEnable){
            if (animationEnable == AnimationEnable::Disable && mAnimationEnableState == AnimationEnable::Enable) {
                removeAnimation(getWindow(), getNode<Animation>());
            }
        };

        Theme &theme{Theme::getTheme()};

        mActionCurve = std::make_unique<ActionCurves::CursorPulse>();

        setPointSize(pointSize.pointSize);
        setFontName(fontName);
        setTextMaxSize(maxLength);

        if (pointSize.pointSize == 0)
            mPointSize = theme.TextPointSize;

        if (fontName.empty())
            mFontName = theme.TextFont;

        setTextInputCallback([&](const std::string &text) {
            textInputEvent(text);
        });

        setKeyboardFocusCallback([&](bool hasFocus) {
            keyboardFocusReceive(hasFocus);
        });

        setKeyboardEvent([&](const SDL_KeyboardEvent& keyEvent) {
            keyboardInput(keyEvent);
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
        if (mCaretLocation > mText.size()) {
            mText.insert(mText.end(), (toUpperCase ? std::toupper(text[0]) : text[0]));
            mCaretLocation = mText.size();
        } else
            mText.insert(mText.begin() + mCaretLocation, (toUpperCase ? std::toupper(text[0]) : text[0]));
        if (textUpdated())
            getApplication().redrawBackground();
        ++mCaretLocation;
    }

    void TextField::keyboardFocusReceive(bool hasFocus) {
        setEditingMode(hasFocus, 0);
        mAnimationEnableState = hasFocus ? AnimationEnable::Enable : AnimationEnable::Disable;
        getApplication().redrawBackground();
    }

    void TextField::eraseChar(int location) {
        if (location < 0) {
            mCaretLocation = 0;
        } else if (location == 0) {
            mText.erase(mText.begin());
            mCaretLocation = 0;
        } else if (location >= mText.size()) {
            mText.erase(mText.end() - 1);
            mCaretLocation = mText.size();
        } else {
            mText.erase(mText.begin() + location);
            mCaretLocation = location;
        }
    }

    void TextField::keyboardInput(const SDL_KeyboardEvent &keyEvent) {
        if (keyEvent.state) {
            switch (keyEvent.keysym.sym) {
                case SDLK_BACKSPACE:
                    eraseChar(mCaretLocation - 1);
                    if (textUpdated())
                        getApplication().redrawBackground();
                    break;
                case SDLK_LEFT:
                    mCaretLocation -= 2;
                case SDLK_RIGHT:
                    ++mCaretLocation;
                    mCaretLocation = std::max(std::min(mCaretLocation, (int)mText.length()), 0);
                    break;
                case SDLK_DELETE:
                    if (keyEvent.keysym.mod & static_cast<uint>(KMOD_CTRL)) {
                        mText = "";
                    } else {
                        eraseChar(mCaretLocation);
                    }
                    if (textUpdated())
                        getApplication().redrawBackground();
                    break;
                default:
                    break;
            }
        }
    }
}
