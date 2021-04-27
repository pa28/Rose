/**
 * @file Keyboard.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-26
 */

#include "Keyboard.h"
#include "ImageStore.h"
#include "Application.h"

namespace rose {

    void Keyboard::addedToContainer() {
        Node::addedToContainer();
        auto grid = getNode<Grid>();
        for (auto &spec : keyDataRow0) {
            if (spec.imageKey)
                grid << wdg<ImageKey>(spec.command);
            else
                grid << wdg<LetterKey>(spec.command);
        }

        back()->getNode<Visual>() << LayoutHint{LayoutHint::GridLayoutHint::EndStride, 0};

        bool first = true;
        for (auto &spec : keyDataRow1) {
            if (spec.imageKey)
                grid << wdg<ImageKey>(spec.command);
            else
                grid << wdg<LetterKey>(spec.command);
            if (first) {
                back()->getNode<Visual>() << LayoutHint{LayoutHint::GridLayoutHint::AxisOffset, 50};
                first = false;
            }
        }

        back()->getNode<Visual>()
                << LayoutHint{LayoutHint::GridLayoutHint::EndStride, 0}
                << LayoutHint{LayoutHint::GridLayoutHint::AxisSize, 150};

        for (auto &spec : keyDataRow2) {
            if (spec.command[0] == SDLK_CAPSLOCK)
                grid << wdg<CapsLockKey>(spec.command);
            else if (spec.imageKey)
                grid << wdg<ImageKey>(spec.command);
            else
                grid << wdg<LetterKey>(spec.command);
        }
        back()->getNode<Visual>()
                << LayoutHint{LayoutHint::GridLayoutHint::EndStride, 0};

        for (auto &spec : keyDataRow3) {
            if (spec.imageKey)
                grid << wdg<ImageKey>(spec.command);
            else if (spec.command[0] == SDLK_SPACE)
                grid << wdg<LetterKey>(spec.command)
                     << LayoutHint{LayoutHint::GridLayoutHint::AxisSize, 600};
            else
                grid << wdg<LetterKey>(spec.command);
        }
    }

    void Keyboard::keyCommandCallback(ButtonStateChange stateChange, uint command) {
        switch (command) {
            case SDLK_CAPSLOCK:
                mCapsLock = stateChange == ButtonStateChange::On;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                if (mAltState) {
                    mAltState = 0;
                    mShiftActive = false;
                } else
                    mShiftActive = !mShiftActive;
                break;
            case SDLK_BACKSPACE:
            case SDLK_RETURN:
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                switch (mAltState) {
                    case 0:
                        mAltState = 1;
                        break;
                    case 1:
                        mAltState = 2;
                        break;
                    case 2:
                        mAltState = 1;
                        break;
                }
                break;
            case SDLK_LEFT:
            case SDLK_RIGHT:
                break;
            default:
                std::cout << __PRETTY_FUNCTION__ << " CLock: " << (mCapsLock ? " ON  " : " OFF ")
                << ", Shift: " << (mShiftActive ? " ON  " : " OFF ")
                << SDL_GetKeyName(command) << '\n';
                mShiftActive = false;
        }

        std::cout << __PRETTY_FUNCTION__ << " CLock: " << (mCapsLock ? " ON  " : " OFF ")
                  << ", Shift: " << (mShiftActive ? " ON  " : " OFF ") << '\n';

        uint newKeystate = 0;
        if (mAltState) {
            switch (mAltState) {
                case 1:
                    newKeystate = 2;
                    break;
                case 2:
                    newKeystate = 3;
                    break;
            }
        } else if ((mCapsLock && !mShiftActive) || (!mCapsLock && mShiftActive))
            newKeystate = 1;

        if (mKeyState != newKeystate) {
            mKeyState = newKeystate;
            for (auto &key : *this) {
                if (auto letterKey = key->getNode<LetterKey>(); letterKey)
                    letterKey->setKeyState(mKeyState);
                else if (auto imageKey = key->getNode<ImageKey>(); imageKey)
                    imageKey->setKeyState(mKeyState);
            }
            getApplication().redrawBackground();
        }
    }

    void ImageKey::addedToContainer() {
        setImageFromCommand();
        mButtonSemantics->setButtonStateChangeCallback([&](ButtonStateChange stateChange) {
            if (stateChange == ButtonStateChange::Pushed) {
                if ( auto keyboard = container()->getNode<Keyboard>(); keyboard) {
                    keyboard->keyCommandCallback(stateChange, mCommand[mKeyState]);
                }
            }
        });
    }

    void ImageKey::setImageFromCommand() {
        switch (mCommand[mKeyState]) {
            case SDLK_BACKSPACE:
                setImage(ImageId::Back);
                break;
            case SDLK_RETURN:
                setImage(ImageId::LevelDown);
                setRenderFlip(gm::RenderFlip{SDL_FLIP_HORIZONTAL});
                break;
            case SDLK_RSHIFT:
            case SDLK_LSHIFT:
                switch (mKeyState) {
                    case 0:
                        setImage(ImageId::UpOpenBig);
                        break;
                    case 1:
                        setImage(ImageId::DownOpenBig);
                        break;
                    default:
                        setImage(ImageId::Keyboard);
                }
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                switch (mKeyState) {
                    case 0:
                    case 1:
                        setImage(ImageId::Keyboard);
                        break;
                    case 2:
                        setImage(ImageId::TwoDots);
                        break;
                    default:
                        setImage(ImageId::ThreeDots);
                }
                break;
            case SDLK_LEFT:
                setImage(ImageId::Left);
                break;
            case SDLK_RIGHT:
                setImage(ImageId::Right);
                break;
        }
//        getApplication().redrawBackground();
    }

    void LetterKey::setTextFromCommand() {
        switch (mCommand[mKeyState]) {
            case '~':
            case '|':
            case SDLK_SPACE:
                setText(std::string{static_cast<char>(mCommand[mKeyState])});
                break;
            default:
                std::string keyName{SDL_GetKeyName(mCommand[mKeyState])};
                if (mKeyState == 0)
                    std::for_each(keyName.begin(), keyName.end(), [](char &c) {
                        c = static_cast<char>(::tolower(c));
                    });
                setText(keyName);
        }
//        getApplication().redrawBackground();
    }

    void LetterKey::addedToContainer() {
        setTextFromCommand();
        mButtonSemantics->setButtonStateChangeCallback([&](ButtonStateChange stateChange) {
            if (stateChange == ButtonStateChange::Pushed) {
                if ( auto keyboard = container()->getNode<Keyboard>(); keyboard) {
                    keyboard->keyCommandCallback(stateChange, mCommand[mKeyState]);
                }
            }
        });
    }

    void CapsLockKey::addedToContainer() {
        setImageFromLockState();
        mButtonSemantics->setButtonStateChangeCallback([&](ButtonStateChange stateChange) {
            if (stateChange == ButtonStateChange::Pushed) {
                mLockState = !mLockState;
                setImageFromLockState();
                if ( auto keyboard = container()->getNode<Keyboard>(); keyboard) {
                    keyboard->keyCommandCallback(mLockState ? ButtonStateChange::On : ButtonStateChange::Off, SDLK_CAPSLOCK);
                }
            }
        });
    }

    void CapsLockKey::setImageFromLockState() {
        if (mLockState)
            setImage(ImageId::Lock);
        else
            setImage(ImageId::LockOpen);
        getApplication().redrawBackground();
    }
}
