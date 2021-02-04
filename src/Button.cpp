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

    Button::Button(RoseImageId imageId, ButtonType type) : Button() {
        mLabelFontSize = 0;
        mButtonType = type;
        mBadge = imageId;
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

            setPadding(sRose->theme().mButtonPadding);

            getWidget<Button>() << wdg<Label>(mLabelText, mBadge)
                    << FontSize(mLabelFontSize);
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
                mSelectProgress = false;
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
        if (auto label = getLabel(); label) {
            label->setImageId(imageId);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    std::shared_ptr<Label> Button::getLabel() {
        if (!mChildren.empty()) {
            if (auto label = front()->as<Label>(); label) {
                return label;
            }
        }
        return nullptr;
    }

    void Button::setText(const string &text) {
        if (auto label = getLabel(); label) {
            label->setText(text);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void Button::setFontName(string &fontName) {
        if (auto label = getLabel(); label) {
            label->setFontName(fontName);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void Button::setFontSize(int fontSize) {
        if (auto label = getLabel(); label) {
            label->setFontSize(fontSize);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void Button::setSize(Size size) {
        if (auto label = getLabel(); label) {
            mSize = size;
            auto labelSize = size;
            auto borderSize = mLayoutHints.totalBorderSize();
            labelSize.width() -= borderSize.width();
            labelSize.height() -= borderSize.height();
            label->setSize(labelSize);
        }
    }

    void Button::setRenderFlip(sdl::RenderFlip renderFlip) {
        if (auto label = getLabel(); label) {
            label->setRenderFlip(renderFlip);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }
}