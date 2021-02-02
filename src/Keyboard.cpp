/**
 * @file Keyboard.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#include "Keyboard.h"
#include "Manipulators.h"
#include "Types.h"

namespace rose {

    constexpr KeyboardSpec<4, 11> QUERTY::QWERTYData = {
            KeySpecRow<11>{
                    KeySpec{KSS{"qQ11"}},
                    KeySpec{KSS{"wW22"}},
                    KeySpec{KSS{"eE33"}},
                    KeySpec{KSS{"rR44"}},
                    KeySpec{KSS{"tT55"}},
                    KeySpec{KSS{"yY66"}},
                    KeySpec{KSS{"uU77"}},
                    KeySpec{KSS{"iI88"}},
                    KeySpec{KSS{"oO99"}},
                    KeySpec{KSS{"pP00"}},
                    KeySpec{KSS{"\b\b\b\b"}},
            },
            KeySpecRow<11>{
                    KeySpec{KSS{"aA@!"}},
                    KeySpec{KSS{"sS#~"}},
                    KeySpec{KSS{"dD$\\"}},
                    KeySpec{KSS{"fF%_"}},
                    KeySpec{KSS{"gG^["}},
                    KeySpec{KSS{"hH&]"}},
                    KeySpec{KSS{"jJ*{"}},
                    KeySpec{KSS{"kK(}"}},
                    KeySpec{KSS{"lL)|"}},
                    KeySpec{KSS{"\r\r\r\r"}},
                    KeySpec{KSS{"\0\0\0\0"}}
            },
            KeySpecRow<11>{
                    KeySpec{SDLK_CAPSLOCK, SDLK_CAPSLOCK, SDLK_CAPSLOCK, SDLK_CAPSLOCK},
                    KeySpec{KSS{"zZ+1"}},
                    KeySpec{KSS{"xX-2"}},
                    KeySpec{KSS{"cC'3"}},
                    KeySpec{KSS{"vV\"4"}},
                    KeySpec{KSS{"bB55"}},
                    KeySpec{KSS{"nN66"}},
                    KeySpec{KSS{"mM77"}},
                    KeySpec{KSS{",?88"}},
                    KeySpec{KSS{"./99"}},
                    KeySpec{SDLK_RSHIFT, SDLK_RSHIFT, SDLK_RSHIFT, SDLK_RSHIFT},
            },
            KeySpecRow<11>{
                    KeySpec{SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT},
                    KeySpec{SDLK_LALT, SDLK_LALT, SDLK_LALT, SDLK_LALT},
                    KeySpec{KSS{"    "}},
                    KeySpec{SDLK_LEFT, SDLK_LEFT, SDLK_LEFT, SDLK_LEFT},
                    KeySpec{SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT},
                    KeySpec{SDLK_RALT, SDLK_RALT, SDLK_RALT, SDLK_RALT},
                    KeySpec{KSS{"\0\0\0\0"}},
            }
    };

    auto QUERTY::controlKeyData(const KeySpec &keyData) {
        RoseImageId imageId = RoseImageId::RoseImageInvalid;
        auto keyIdx = static_cast<std::size_t>(mKeyboardMode);
        auto buttonType = ButtonType::NormalButton;
        bool elastic = false;
        bool isChar = false;
        sdl::RenderFlip renderFlip{};
        switch (keyData[keyIdx]) {
            case SDLK_SPACE:
                elastic = true;
                isChar = true;
                break;
            case SDLK_BACKSPACE:
                imageId = RoseImageId::IconBack;
                elastic = true;
                isChar = true;
                break;
            case SDLK_TAB:
                imageId = RoseImageId::IconToEnd;
                isChar = true;
                break;
            case SDLK_RETURN:
                imageId = RoseImageId::IconLevelDown;
                elastic = true;
                isChar = true;
                renderFlip = sdl::RenderFlip{SDL_FLIP_HORIZONTAL};
                break;
            case SDLK_ESCAPE:
                isChar = true;
                break;
            case SDLK_CAPSLOCK:
                imageId = RoseImageId::IconLock;
                buttonType = ButtonType::ToggleButton;
                break;
            case SDLK_RSHIFT:
            case SDLK_LSHIFT:
                imageId = RoseImageId::IconUpBold;
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                switch (mSymbolState) {
                    case SymbolState::None:
                        imageId = Icon1Dot;
                        break;
                    case SymbolState::Symbols1:
                        imageId = Icon2Dots;
                        break;
                    case SymbolState::Symbols2:
                        imageId = Icon3Dots;
                        break;
                }
                break;
            case SDLK_LEFT:
                imageId = RoseImageId::IconLeft;
                break;
            case SDLK_RIGHT:
                imageId = RoseImageId::IconRight;
                break;
            default:
                break;
        }

        return std::make_tuple(isChar, imageId, buttonType, elastic, renderFlip);
    }

    void QUERTY::build(shared_ptr<Keyboard> keyboard, Size keySize, int fontSize, const std::string &fontName) {
        mKeyboard = keyboard;
        rxKey = std::make_shared<Slot<Button::SignalType>>();
        rxKey->setCallback([=](uint32_t, Button::SignalType signal) {
            SDL_Event event;
            event.type = SDL_TEXTINPUT;
            event.text.timestamp = SDL_GetTicks();
            event.text.windowID = 0;
            event.text.text[0] = static_cast<char>(signal.second & 0xFFu);
            event.text.text[1] = '\0';
            SDL_ClearError();
            if (auto status = SDL_PushEvent(&event); !status)
                std::cout << __PRETTY_FUNCTION__ << ' ' << status << SDL_GetError() << '\n';
            if (mTempShiftKey) {
                mTempShiftKey = 0;
                setKeyboardMode();
            }
        });

        rxCtl = std::make_shared<Slot<Button::SignalType>>();
        rxCtl->setCallback([=](uint32_t, Button::SignalType signal) {
            switch (signal.second) {
                case SDLK_RSHIFT:
                case SDLK_LSHIFT:
                    if (mSymbolState == SymbolState::None) {
                        if (mTempShiftKey)
                            mTempShiftKey = 0;
                        else
                            mTempShiftKey = signal.second;
                    } else
                        mSymbolState = SymbolState::None;
                    break;
                case SDLK_LALT:
                case SDLK_RALT:
                    switch (mSymbolState) {
                        case SymbolState::None:
                            mSymbolState = SymbolState::Symbols1;
                            break;
                        case SymbolState::Symbols1:
                            mSymbolState = SymbolState::Symbols2;
                            break;
                        case SymbolState::Symbols2:
                            mSymbolState = SymbolState::None;
                            break;
                    }
                    break;
                case SDLK_LEFT:
                case SDLK_RIGHT:
                default:
                    return;
            }

            setKeyboardMode();
        });

        rxToggle = std::make_shared<Slot<Button::SignalType>>();
        rxToggle->setCallback([=](uint32_t, Button::SignalType signal) {
            if (signal.second == SDLK_CAPSLOCK) {
                mCapsLock = signal.first;
                setKeyboardMode();
            }
        });

        size_t rowIdx = 0;
        size_t keyIdx = 0;
        auto column = keyboard << wdg<Column>();
        column->containerLayoutHints().startOffset = 4;
        column->containerLayoutHints().endOffset = 4;
        column->containerLayoutHints().internalSpace = 4;
        for (const auto &keyRow : QWERTYData) {
            auto row = column << wdg<Row>() << Elastic{Orientation::Horizontal};
            row->containerLayoutHints().startOffset = 4;
            row->containerLayoutHints().endOffset = 4;
            row->containerLayoutHints().internalSpace = 4;
            row->containerLayoutHints().fillToEnd = true;
            if (rowIdx == 1)
                row->containerLayoutHints().startOffset += keySize.width()/2 + 4;
            for (const auto &keyData : keyRow) {
                if (keyData[keyIdx] == SDLK_UNKNOWN)
                    break;
                if (keyData[keyIdx] <= SDLK_SPACE || keyData[keyIdx] >= SDLK_CAPSLOCK) {
                    // Control key.
                    auto[isChar, imageId, buttonType, elastic, renderFlip] = controlKeyData(keyData);
                    auto key = row << wdg<Button>(imageId, buttonType)
                                   << renderFlip
                                   << CornerStyle::Round;
                    if (!elastic)
                        key << Elastic{Orientation::Vertical};
                    else
                        key << Elastic{Orientation::Both};

                    if (buttonType == ButtonType::ToggleButton)
                        key->txState.connect(rxToggle);
                    else {
                        if (isChar)
                            key->txPushed.connect(rxKey);
                        else
                            key->txPushed.connect(rxCtl);
                    }
                    key->setSignalToken(keyData[keyIdx]);
                } else {
                    // Character key.
                    auto key =
                            row << wdg<Button>(std::string{(char) keyData[keyIdx]}, ButtonType::NormalButton, fontSize)
                                << CornerStyle::Round << FontName{fontName} << keySize;
                    key->txPushed.connect(rxKey);
                    key->setSignalToken(keyData[keyIdx]);
                }
            }
            ++rowIdx;
        }
    }

    void QUERTY::setKeyboardMode() {
        KeyboardMode mode = KeyboardMode::LowerCase;
        if (mSymbolState != SymbolState::None) {
            if (mSymbolState == SymbolState::Symbols1)
                mode = KeyboardMode::Numeric;
            else
                mode = KeyboardMode::Symbols;
        } else {
            if (bxor(mTempShiftKey != 0, mCapsLock))
                mode = KeyboardMode::UpperCase;
            else
                mode = KeyboardMode::LowerCase;
        }

        if (mKeyboardMode != mode) {
            mKeyboardMode = mode;
            changeKeyboardFace();
        }
    }

    void QUERTY::changeKeyboardFace() {
        auto keyIdx = static_cast<std::size_t>(mKeyboardMode);
        if (auto keyboard = mKeyboard.lock(); keyboard) {
            if (auto column = keyboard->front()->as<Column>(); column) {
                auto columnItr = column->begin();
                for (const auto &keyRow : QWERTYData) {
                    if (columnItr == column->end())
                        break;
                    if (auto rowWidget = (*columnItr)->as<Row>(); rowWidget) {
                        auto rowItr = rowWidget->begin();
                        for (const auto &keyData : keyRow) {
                            if (rowItr == rowWidget->end())
                                break;
                            if (auto btn = (*rowItr)->as<Button>(); btn) {
                                if (keyData[keyIdx] <= SDLK_SPACE || keyData[keyIdx] >= SDLK_CAPSLOCK) {
                                    // Control key.
                                    auto[isChar, imageId, buttonType, elastic, renderFlip] = controlKeyData(keyData);
                                    btn->setImageId(imageId);
                                    btn->setSignalToken(keyData[keyIdx]);
                                } else {
                                    btn->setText(std::string{static_cast<char>(keyData[keyIdx])});
                                    btn->setSignalToken(keyData[keyIdx]);
                                }
                            } else
                                break;
                            ++rowItr;
                        }
                    } else
                        break;
                    ++columnItr;
                }
            }
            keyboard->setNeedsDrawing();
        }
    }

    Rectangle Keyboard::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Frame::widgetLayout(renderer, available, 0);
    }

    void Keyboard::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Frame::draw(renderer, parentRect);
    }

    void Keyboard::initializeComposite() {
        Frame::initializeComposite();

        mFontName = rose()->theme().mKeyboardFont;
        mFontSize = rose()->theme().mKeyboardFontPointSize;
        auto borderWidth = rose()->theme().mBorderWidth + rose()->theme().mButtonPadding;

        auto fontPtr = rose()->fontCache().getFont(mFontName, mFontSize);
        auto[hminx, hmaxx, hminy, hmaxy, hadvance] = getGlyphMetrics(fontPtr, mEm);
        auto lineSkip = TTF_FontLineSkip(fontPtr.value().get());

        mKeySize = Size{hmaxx - hminx + borderWidth * 2, lineSkip + borderWidth * 2};

        mBorder = BorderStyle::Notch;

        mKeyboardPlugin->build(getWidget<Keyboard>(), mKeySize, mFontSize, mFontName);
    }
}
