//
// Created by richard on 2020-10-27.
//

#include "Border.h"
#include "Button.h"
#include "Manipulators.h"

namespace rose {

    ButtonFrame::ButtonFrame() : Frame() {
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
                updateStateSetting(state.first ? ButtonSetState::ButtonOn : ButtonSetState::ButtonOff);
                txState.transmit(sn, state);
            }
        });
    }

    ButtonFrame::ButtonFrame(const Id& id) : ButtonFrame() {
        setId(id);
    }

    ButtonFrame::ButtonFrame(const Id& id, ButtonType type, int fontSize) : ButtonFrame(id) {
        mLabelFontSize = fontSize;
        mButtonType = type;
    }

    ButtonFrame::ButtonFrame(const std::string &labelString) : ButtonFrame() {
        mLabelText = labelString;
    }

    ButtonFrame::ButtonFrame(const std::string &labelString, ButtonType type, int fontSize)
            : ButtonFrame(labelString) {
        mLabelFontSize = fontSize;
        mButtonType = type;
    }

    ButtonFrame::ButtonFrame(RoseImageId imageId, ButtonType type) : ButtonFrame() {
        mLabelFontSize = 0;
        mButtonType = type;
        mBadge = imageId;
    }

    void ButtonFrame::initializeComposite() {
        Frame::initializeComposite();
        mAcceptsFocus = true;
        auto sRose = rose();

        if (rose()->hasSettings()) {
            mSettingsUpdateRx = std::make_shared<Slot<std::string>>();
            mSettingsUpdateRx->setCallback([&](uint32_t,const std::string& name){
                if (!name.empty()) {
                    if (name == mStateId.value()) {
                        setInvert(rose()->settings()->getValue(mStateId.value(), 0));
                    } else if (name == mId.value())
                        setText(rose()->settings()->getValue(mId, std::string{}));
                }
            });
        }

        if ((!mId.empty() || !mStateId.empty()) && rose()->hasSettings()) {
            rose()->settings()->dataChangeTx.connect(mSettingsUpdateRx);
        }

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

            getWidget<ButtonFrame>() << wdg<Label>(mLabelText, mBadge)
                    << FontSize(mLabelFontSize);
        }

        mClassName = "ButtonFrame";
    }

    bool ButtonFrame::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
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
                            txState.transmit(mSignalSerialNumber.serialNumber(),
                                             std::pair<bool, SignalToken>{mButtonSelectState, mSignalToken});
                            updateStateSetting(mButtonSelectState);
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

    bool ButtonFrame::clickTransactionCancel(const Position &mousePos, int button, bool down, int modifiers) {
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

    void ButtonFrame::setImageId(ImageId imageId) {
        if (auto label = getLabel(); label) {
            label->setImageId(imageId);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    std::shared_ptr<Label> ButtonFrame::getLabel() {
        if (!mChildren.empty()) {
            if (auto label = front()->as<Label>(); label) {
                return label;
            }
        }
        return nullptr;
    }

    void ButtonFrame::setText(const string &text) {
        if (auto label = getLabel(); label) {
            label->setText(text);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void ButtonFrame::setFontName(string &fontName) {
        if (auto label = getLabel(); label) {
            label->setFontName(fontName);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void ButtonFrame::setFontSize(int fontSize) {
        if (auto label = getLabel(); label) {
            label->setFontSize(fontSize);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void ButtonFrame::setSize(Size size) {
        if (auto label = getLabel(); label) {
            mSize = size;
            auto labelSize = size;
            auto borderSize = mLayoutHints.totalBorderSize();
            labelSize.width() -= borderSize.width();
            labelSize.height() -= borderSize.height();
            label->setSize(labelSize);
        }
    }

    void ButtonFrame::setRenderFlip(sdl::RenderFlip renderFlip) {
        if (auto label = getLabel(); label) {
            label->setRenderFlip(renderFlip);
            return;
        }
        throw RoseLogicError(
                StringCompositor("Program logic error ", __PRETTY_FUNCTION__, ' ', __FILE__, __LINE__));
    }

    void ButtonFrame::updateStateSetting(ButtonSetState state) {
        if ((mButtonType == ButtonType::ToggleButton || mButtonType == ButtonType::RadioButton) &&
                                                       !mStateId.empty() && rose()->hasSettings()) {
            rose()->settings()->setValue(mStateId.value(), state == ButtonSetState::ButtonOn);
        }
    }
}