//
// Created by richard on 2020-10-27.
//

#include "Border.h"
#include "Button.h"
#include "Manipulators.h"

namespace rose {

    ButtonFrame::ButtonFrame(Padding padding) : Frame(padding) {
        rxPushed = std::make_shared<Slot<std::pair<bool, SignalToken>>>();
        rxPushed->setCallback([&](uint32_t sn, std::pair<bool, SignalToken> pushed) {
            if (txPushed && sn != mSignalSerialNumber && mButtonType == NormalButton) {
                txPushed.transmit(sn, pushed);
            }
        });

        rxState = std::make_shared<Slot<std::pair<bool, SignalToken>>>();
        rxState->setCallback([&](uint32_t sn, std::pair<bool, SignalToken> state) {
            if (sn != mSignalSerialNumber && mInvert != state.first) {
                setInvert(state.first);
                updateStateSetting(state.first ? ButtonSetState::ButtonOn : ButtonSetState::ButtonOff);
                txState.transmit(sn, state);
            }
        });
    }

    void ButtonFrame::initializeComposite() {
        Frame::initializeComposite();
        mAcceptsFocus = true;
        auto sRose = rose();

        if (sRose->hasSettings()) {
            mSettingsUpdateRx = std::make_shared<Slot<std::string>>();
            mSettingsUpdateRx->setCallback([&](uint32_t, const std::string &name) {
                if (!name.empty()) {
                    if (name == mStateId.value())
                        setInvert(rose()->settings()->getValue(mStateId.value(), 0));
                }
            });
        }

        if (!mStateId.empty() && sRose->hasSettings()) {
            sRose->settings()->dataChangeTx.connect(mSettingsUpdateRx);
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

    void ButtonFrame::updateStateSetting(ButtonSetState state) {
        if ((mButtonType == ButtonType::ToggleButton || mButtonType == ButtonType::RadioButton) &&
            !mStateId.empty() && rose()->hasSettings()) {
            rose()->settings()->setValue(mStateId.value(), state == ButtonSetState::ButtonOn);
        }
    }

    Button::Button(const string &labelString, ButtonType type, int fontSize) : ButtonFrame(0) {
        mLabelText = labelString;
        mButtonType = type;
        mLabelFontSize = fontSize;
    }

    Button::Button(const Id &id, ButtonType type, int fontSize) : ButtonFrame(0) {
        mId = id;
        mButtonType = type;
        mLabelFontSize = fontSize;
    }

    Button::Button(RoseImageId imageId, ButtonType type) : ButtonFrame(0) {
        mImageId = imageId;
        mButtonType = type;
    }

    void Button::initializeComposite() {
        ButtonFrame::initializeComposite();
        setPadding(rose()->theme().mButtonPadding);

        if (!mId.empty() && rose()->hasSettings()) {
            mLabelText = rose()->settings()->getValue(mId.value(), mId.value());
        }

        auto label = getWidget<ButtonFrame>() << wdg<Label>(mLabelText, RoseImageId{mImageId});

        auto theme = rose()->theme();
        if (mLabelFontSize == 0)
            mLabelFontSize = theme.mFontPointSize;

        label->setFontSize(mLabelFontSize);

        if (unset(mBorder))
            mBorder = BorderStyle::Bevel;

        mClassName = "Button";
    }

    void Button::setSize(Size size) {
        if (auto label = getSingleChild<Label>(); label) {
            mSize = size;
            auto labelSize = size;
            auto borderSize = mLayoutHints.totalBorderSize();
            labelSize.width() -= borderSize.width();
            labelSize.height() -= borderSize.height();
            label->setSize(labelSize);
        }
    }
}