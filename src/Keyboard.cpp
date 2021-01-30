/**
 * @file Keyboard.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#include "Keyboard.h"
#include "Manipulators.h"

namespace rose {

    static constexpr KeyboardFace keyboardFace = {
            "abcdefghijklmnopqrstuvwxyz0123456789;,./;,./",
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*():<>?:<>?",
            "1234567890!@#$%^&*()-=[]\\,./_+:\"<>?       ",
    };

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

        // 32 x 42   28 x 38
        mKeySize = Size{hmaxx - hminx + borderWidth * 2, lineSkip + borderWidth * 2};

        mKeyPressRx = std::make_shared<Slot<Button::SignalType>>();
        mKeyPressRx->setCallback([=](uint32_t, Button::SignalType signalType) {
            auto face = keyFace(mKeyboardMode, static_cast<Keys>(signalType.second), keyboardFace);
            std::cout << face << '\n';
        });

        mBorder = BorderStyle::Notch;
        mKeysGrid = getWidget<Keyboard>() << wdg<Column>();
        mKeysGrid->containerLayoutHints().internalSpace = 4;
        mKeysGrid->containerLayoutHints().startOffset = 4;

        buildKeyboard(mKeysGrid, mKeyboardDef);
    }

    template<size_t R, size_t N>
    void Keyboard::buildKeyboard(shared_ptr<Column> &column, KeyboardDef<R, N> keyboardDef) {
        int nRow = 0;
        for (auto &row : keyboardDef) {
            auto keyRow = column << wdg<Row>();
            keyRow->containerLayoutHints().internalSpace = 4;

            if (nRow == 1)
                keyRow->containerLayoutHints().startOffset = 4 + nRow * mKeySize.height() / 2;
            else
                keyRow->containerLayoutHints().startOffset = 4;

            if (nRow == 2) {
                auto btn = keyRow << wdg<Button>("L", ButtonType::ToggleButton, mFontSize)
                                  << mKeySize
                                  << FontName{mFontName};
            } else if (nRow == 3) {
                auto btn = keyRow << wdg<Button>("Sh", ButtonType::NormalButton, mFontSize)
                                  << Size{(mKeySize.width() * 3) / 2, mKeySize.height()}
                                  << FontName{mFontName};
            }

            for (auto &key : row) {
                auto face = keyFace(mKeyboardMode, key, keyboardFace);
                auto btn = keyRow << wdg<Button>(face, ButtonType::NormalButton, mFontSize)
                                  << static_cast<SignalToken>(key)
                                  << mKeySize << CornerStyle::Round
                                  << FontName{mFontName};
                btn->txPushed.connect(mKeyPressRx);
            }
            ++nRow;
        }
    }

    std::string Keyboard::keyFace(KeyboardMode keyboardMode, Keys keys, KeyboardFace face) {
        auto mode = static_cast<std::size_t>(keyboardMode);
        auto idx = static_cast<std::size_t>(keys);
        switch (keys) {
            case Keys::C0:
                return "C0";
            case Keys::C1:
                return "C1";
            case Keys::C2:
                return "C2";
            case Keys::C3:
                return "C3";
            default:
                return std::string{face[mode][idx]};
        }
    }
}
