//
// Created by richard on 2020-10-27.
//

#include "Border.h"
#include "Button.h"
#include "Manipulators.h"

namespace rose {

    Button::Button() : Frame() {
        rxPushed = std::make_shared<Slot<std::pair<bool,SignalToken>>>();
        rxPushed->setCallback([&](uint32_t sn, std::pair<bool,SignalToken> pushed) {
            if (txPushed && sn != mSignalSerialNumber && mButtonType == NormalButton) {
                txPushed.transmit(sn, pushed);
            }
        });

        rxState = std::make_shared<Slot<std::pair<bool,SignalToken>>>();
        rxState->setCallback([&](uint32_t sn, std::pair<bool,SignalToken> state) {
            if (sn != mSignalSerialNumber && mInvert != state.first) {
                setInvert(state.first);
                txState.transmit(sn, state);
            }
        });
    }

    Button::Button(const Id& id) : Button() {
        setId(id);
    }

    Button::Button(const Id& id, ButtonType type, int fontSize) : Button(id) {
        mLabelFontSize = fontSize;
        mButtonType = type;
    }

    Button::Button(const std::string &labelString) : Button() {
        mLabelText = labelString;
    }

    Button::Button(const std::string &labelString, ButtonType type, int fontSize)
            : Button(labelString) {
        mLabelFontSize = fontSize;
        mButtonType = type;
    }

    void Button::initializeComposite() {
        Frame::initializeComposite();
        mAcceptsFocus = true;
        auto sRose = rose();

        if (mLabelText.empty() && !getId().empty()) {
            mLabelText = sRose->settings()->getValue(getId().value(), getId().value());
        }

        auto theme = sRose->theme();
        if (mLabelFontSize == 0)
            mLabelFontSize = theme.mFontPointSize;

        if (unset(mBorder))
            mBorder = BorderStyle::Bevel;

        if (mChildren.empty()) {
            if (mBadge == RoseImageInvalid)
                switch (mButtonType) {
                    case CancelButton:
                        mBadge = IconCancel;
                        break;
                    case OkButton:
                        mBadge = IconCheck;
                        break;
                    default:
                        break;
                }

            getWidget<Button>() << wdg<Border>(sRose->theme().mButtonPadding)
                                << wdg<Label>(mLabelText, mBadge)
                                << FontSize{mLabelFontSize};
        }
        mClassName = "Button";
    }

    bool Button::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        if (button == SDL_BUTTON_LEFT) {
            if (down) {
                mSelectProgress = true;

                switch (mButtonType) {
                    case NormalButton:
                    case MenuCascade:
                    case CancelButton:
                    case OkButton:
                        setInvert(down);
                        break;
                    case ToggleButton:
                        setInvert(!mInvert);
                        break;
                    case RadioButton:
                        break;
                    case TabButton:
                        break;
                }
                return true;
            } else {
                if (mSelectProgress) {
                    switch (mButtonType) {
                        case NormalButton:
                        case MenuCascade:
                        case CancelButton:
                        case OkButton:
                            txPushed.transmit(mSignalSerialNumber.serialNumber(), std::pair<bool,SignalToken>{true,mSignalToken});
                            setInvert(false);
                            break;
                        case ToggleButton:
                            mButtonSelectState = mInvert ? ButtonSetState::ButtonOn : ButtonSetState::ButtonOff;
                            txState.transmit(mSignalSerialNumber.serialNumber(), std::pair<bool,SignalToken>{mButtonSelectState,mSignalToken});
                            break;
                        case RadioButton:
                            break;
                        case TabButton:
                            if (mButtonSelectState == ButtonOff) {
                                txPushed.transmit(mSignalSerialNumber.serialNumber(), std::pair<bool,SignalToken>{true,mSignalToken});
                                mButtonSelectState = ButtonSetState::ButtonOn;
                                txState.transmit(mSignalSerialNumber.serialNumber(), std::pair<bool,SignalToken>{mButtonSelectState,mSignalToken});
                                setInvert(!mButtonSelectState);
                            }
                            break;
                    }
                }
                return true;
            }
        }
        return false;
    }

    bool Button::clickTransactionCancel(const Position &mousePos, int button, bool down, int modifiers) {
        if (mSelectProgress) {
            switch (mButtonType) {
                case NormalButton:
                case MenuCascade:
                case CancelButton:
                case OkButton:
                    setInvert(false);
                    break;
                case ToggleButton:
                    setInvert(!mInvert);
                    break;
                case RadioButton:
                case TabButton:
                    break;
            }
            mSelectProgress = false;
        }
        return true;
    }

    void Button::setImageId(ImageId imageId) {
        mChildren.front()->as<Label>()->setBadge(imageId);
    }

    void Button::setText(const string &text) {
        if (!mChildren.empty()) {
            if (auto border = mChildren.front()->as<Border>(); border) {
                if (auto label = border->front()->as<Label>(); label) {
                    label->setText(text);
                    return;
                }
            }
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void Button::setFontName(string &fontName) {
        if (!mChildren.empty()) {
            if (auto border = mChildren.front()->as<Border>(); border) {
                if (auto label = border->front()->as<Label>(); label) {
                    label->setFontName(fontName);
                    return;
                }
            }
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void Button::setFontSize(int fontSize) {
        if (!mChildren.empty()) {
            if (auto border = mChildren.front()->as<Border>(); border) {
                if (auto label = border->front()->as<Label>(); label) {
                    label->setFontSize(fontSize);
                    return;
                }
            }
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }
}