/**
 * @file Keyboard.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#include "Keyboard.h"
#include "Manipulators.h"

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
                    KeySpec{SDLK_LALT,SDLK_LALT,SDLK_LALT,SDLK_LALT},
                    KeySpec{KSS{"    "}},
                    KeySpec{SDLK_LEFT,SDLK_LEFT,SDLK_LEFT,SDLK_LEFT},
                    KeySpec{SDLK_RIGHT,SDLK_RIGHT,SDLK_RIGHT,SDLK_RIGHT},
                    KeySpec{SDLK_RALT,SDLK_RALT,SDLK_RALT,SDLK_RALT},
                    KeySpec{KSS{"\0\0\0\0"}},
            }
    };

    void QUERTY::build(shared_ptr <Keyboard> frame, Size keySize, shared_ptr <Slot<Button::SignalType>> &slot) const {

    }

    Rectangle Keyboard::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        return Frame::initialLayout(renderer, available);
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

        mKeyPressRx = std::make_shared<Slot<Button::SignalType>>();
        mKeyPressRx->setCallback([=](uint32_t, Button::SignalType signalType) {
        });

        mBorder = BorderStyle::Notch;
        mKeysGrid = getWidget<Keyboard>() << wdg<Column>();
        mKeysGrid->containerLayoutHints().internalSpace = 4;
        mKeysGrid->containerLayoutHints().startOffset = 4;

        mKeyboardPlugin.build(getWidget<Keyboard>(), mKeySize, mKeyPressRx);
    }
}
