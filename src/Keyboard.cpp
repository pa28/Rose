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

    constexpr KeyboardSpec<NumberPad::Rows, NumberPad::KeysPerRow, 1> NumberPad::NumberData = {
            KeySpecRow<NumberPad::KeysPerRow, 1>{
                    makeKeySpec<1>(","),
                    makeKeySpec<1>("/"),
                    makeKeySpec<1>("*"),
            },
            KeySpecRow<NumberPad::KeysPerRow, 1>{
                    makeKeySpec<1>("7"),
                    makeKeySpec<1>("8"),
                    makeKeySpec<1>("9"),
            },
            KeySpecRow<NumberPad::KeysPerRow, 1>{
                    makeKeySpec<1>("4"),
                    makeKeySpec<1>("5"),
                    makeKeySpec<1>("6"),
            },
            KeySpecRow<NumberPad::KeysPerRow, 1>{
                    makeKeySpec<1>("1"),
                    makeKeySpec<1>("2"),
                    makeKeySpec<1>("3"),
            },
            KeySpecRow<NumberPad::KeysPerRow, 1>{
                    makeKeySpec<1>("0"),
                    makeKeySpec<1>("."),
                    makeKeySpec<1>(0),
            },
    };

    constexpr KeyboardSpec<NumberPad::SideCols, NumberPad::SideKeysPerCol, 1> NumberPad::SideNumberData = {
            KeySpecRow<NumberPad::SideKeysPerCol, 1>{
                    makeKeySpec<1>("-"),
                    makeKeySpec<1>("+"),
                    makeKeySpec<1>("_"),
                    makeKeySpec<1>("\r"),
                    makeKeySpec<1>(0)
            }
    };

    void NumberPad::build(std::shared_ptr<Keyboard> keyboard, Size keySize, int fontSize, const string &fontName) {
        auto row0 = keyboard << wdg<Row>();
        row0->containerLayoutHints().endOffset = 4;

        auto column = row0 << wdg<Column>();
        column->containerLayoutHints().startOffset = 4;
        column->containerLayoutHints().endOffset = 4;
        column->containerLayoutHints().internalSpace = 4;

        for (auto &keyRow : NumberData) {
            auto row = column << wdg<Row>() << Elastic{Orientation::Horizontal};
            row->containerLayoutHints().startOffset = 4;
            row->containerLayoutHints().endOffset = 4;
            row->containerLayoutHints().internalSpace = 4;
            row->containerLayoutHints().fillToEnd = true;
            for (auto keyData : keyRow) {
                if (keyData[0] == 0)
                    break;
                // Character key.
                auto key = row << wdg<Button>(std::string{(char) keyData[0]},
                                              ButtonType::NormalButton, fontSize)
                               << CornerStyle::Round
                               << FontName{fontName}
                               << keySize;
                key->setSignalToken(keyData[0]);
                if (keyData[0] == '0')
                    key << Elastic(Orientation::Both);
                else
                    key << Elastic(Orientation::Vertical);
            }
        }

        auto col = row0 << wdg<Column>();
        col->containerLayoutHints().startOffset = 4;
        col->containerLayoutHints().endOffset = 4;
        col->containerLayoutHints().internalSpace = 4;
        col->containerLayoutHints().fillToEnd = true;
        for (auto &keyRow : SideNumberData) {
            for (auto &keyData : keyRow) {
                if (keyData[0] == 0)
                    break;
                if (keyData[0] == '\r') {
                    auto renderFlip = sdl::RenderFlip{SDL_FLIP_HORIZONTAL};
                    auto key = col << wdg<Button>(RoseImageId::IconLevelDown, ButtonType::NormalButton)
                                   << renderFlip
                                   << Elastic(Orientation::Both)
                                   << CornerStyle::Round;
                    key->setSignalToken(keyData[0]);
                } else {
                    auto key = col << wdg<Button>(std::string{(char) keyData[0]},
                                                  ButtonType::NormalButton, fontSize)
                                   << CornerStyle::Round
                                   << Elastic(Orientation::Horizontal)
                                   << FontName{fontName}
                                   << keySize;
                    key->setSignalToken(keyData[0]);
                }
            }
        }
    }

    constexpr KeyboardSpec<QUERTY::Rows, QUERTY::KeysPerRow, QUERTY::Depth> QUERTY::QWERTYData = {
            KeySpecRow<QUERTY::KeysPerRow, QUERTY::Depth>{
                    makeKeySpec<QUERTY::Depth>("qQ11"),
                    makeKeySpec<QUERTY::Depth>("wW22"),
                    makeKeySpec<QUERTY::Depth>("eE33"),
                    makeKeySpec<QUERTY::Depth>("rR44"),
                    makeKeySpec<QUERTY::Depth>("tT55"),
                    makeKeySpec<QUERTY::Depth>("yY66"),
                    makeKeySpec<QUERTY::Depth>("uU77"),
                    makeKeySpec<QUERTY::Depth>("iI88"),
                    makeKeySpec<QUERTY::Depth>("oO99"),
                    makeKeySpec<QUERTY::Depth>("pP00"),
                    makeKeySpec<QUERTY::Depth>("\b\b\b\b"),
            },
            KeySpecRow<QUERTY::KeysPerRow, QUERTY::Depth>{
                    makeKeySpec<QUERTY::Depth>("aA@!"),
                    makeKeySpec<QUERTY::Depth>("sS#~"),
                    makeKeySpec<QUERTY::Depth>("dD$\\"),
                    makeKeySpec<QUERTY::Depth>("fF%_"),
                    makeKeySpec<QUERTY::Depth>("gG^["),
                    makeKeySpec<QUERTY::Depth>("hH&]"),
                    makeKeySpec<QUERTY::Depth>("jJ*{"),
                    makeKeySpec<QUERTY::Depth>("kK()"),
                    makeKeySpec<QUERTY::Depth>("lL)|"),
                    makeKeySpec<QUERTY::Depth>("\r\r\r\r"),
                    makeKeySpec<QUERTY::Depth>(0),
            },
            KeySpecRow<QUERTY::KeysPerRow, QUERTY::Depth>{
                    makeKeySpec<QUERTY::Depth>(SDLK_CAPSLOCK, SDLK_CAPSLOCK, SDLK_CAPSLOCK, SDLK_CAPSLOCK),
                    makeKeySpec<QUERTY::Depth>("zZ+1"),
                    makeKeySpec<QUERTY::Depth>("xX-2"),
                    makeKeySpec<QUERTY::Depth>("cC'3"),
                    makeKeySpec<QUERTY::Depth>("vV\"4"),
                    makeKeySpec<QUERTY::Depth>("bB_5"),
                    makeKeySpec<QUERTY::Depth>("nN?6"),
                    makeKeySpec<QUERTY::Depth>("mM/7"),
                    makeKeySpec<QUERTY::Depth>(",?,8"),
                    makeKeySpec<QUERTY::Depth>("./.9"),
                    makeKeySpec<QUERTY::Depth>(SDLK_RSHIFT, SDLK_RSHIFT, SDLK_RSHIFT, SDLK_RSHIFT),
            },
            KeySpecRow<QUERTY::KeysPerRow, QUERTY::Depth>{
                    makeKeySpec<QUERTY::Depth>(SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT),
                    makeKeySpec<QUERTY::Depth>(SDLK_LALT, SDLK_LALT, SDLK_LALT, SDLK_LALT),
                    makeKeySpec<QUERTY::Depth>("    "),
                    makeKeySpec<QUERTY::Depth>(SDLK_LEFT, SDLK_LEFT, SDLK_LEFT, SDLK_LEFT),
                    makeKeySpec<QUERTY::Depth>(SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT),
                    makeKeySpec<QUERTY::Depth>(SDLK_RALT, SDLK_RALT, SDLK_RALT, SDLK_RALT),
                    makeKeySpec<QUERTY::Depth>(0),
            }
    };

    auto QUERTY::controlKeyData(const KeySpec<Depth> &keyData) {
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
                switch (mSymbolState) {
                    case SymbolState::None:
                        imageId = IconUpBold;
                        break;
                    case SymbolState::Symbols1:
                    case SymbolState::Symbols2:
                        imageId = IconKeyboard;
                        break;
                }
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                switch (mSymbolState) {
                    case SymbolState::None:
                        imageId = IconKeyboard;
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
                row->containerLayoutHints().startOffset += keySize.width() / 2 + 4;
            for (const auto &keyData : keyRow) {
                if (keyData[keyIdx] == SDLK_UNKNOWN)
                    break;
                if (keyData[keyIdx] <= SDLK_SPACE || keyData[keyIdx] >= SDLK_CAPSLOCK) {
                    // Control key.
                    auto[isChar, imageId, buttonType, elastic, renderFlip] = controlKeyData(keyData);
                    auto key = row << wdg<Button>(imageId, buttonType)
                                   << renderFlip
                                   << CornerStyle::Round
                                   << keySize;
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
        mKeySize.width() = std::max(mKeySize.width(), mKeySize.height());
        mKeySize.height() = mKeySize.width();

        mBorder = BorderStyle::Notch;

        mKeyboardPlugin->build(getWidget<Keyboard>(), mKeySize, mFontSize, mFontName);
    }
}
