/**
 * @file Keyboard.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#pragma once

#include "Frame.h"
#include "Button.h"
#include "Container.h"

namespace rose {

    enum class KeyboardMode {
        LowerCase,
        UpperCase,
        Symbols,
    };

    enum class Keys {
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
        P0, P1, P2, P3,
        C0, C1, C2, C3,
    };

    template<std::size_t N>
    using KeyRow = std::array<Keys, N>;

    template<std::size_t R, std::size_t N>
    using KeyboardDef = std::array<KeyRow<N>,R>;

    using KeyboardFace = std::array<std::string_view, 3>;

/**
 * @class Keyboard
 * @brief Implement a touchscreen keyboard
 */
    class Keyboard : public Frame {
    protected:
        KeyboardMode mKeyboardMode{KeyboardMode::LowerCase};    ///< The mode the keyboard is in.

        char mEm{'M'};                          ///< The 'largest' glyph in the Font.
        std::shared_ptr<Column> mKeysGrid;      ///< The Column of key rows.
        Size mKeySize{};                        ///< The size of regular keys, determined by mEm.

        std::string mFontName{};
        int mFontSize{};

        static constexpr KeyRow<10> KeyRow1 = {Keys::N1, Keys::N2, Keys::N3,
                                               Keys::N4, Keys::N5, Keys::N6,
                                               Keys::N7, Keys::N8, Keys::N9,
                                               Keys::N0};

        static constexpr KeyRow<10> KeyRow2 = { Keys::Q, Keys::W,Keys::E,
                                                Keys::R, Keys::T, Keys::Y,
                                                Keys::U, Keys::I, Keys::O,
                                                Keys::P};

        static constexpr KeyRow<10> KeyRow3 = {Keys::A, Keys::S, Keys::D,
                                               Keys::F, Keys::G, Keys::H,
                                               Keys::J, Keys::K, Keys::L,
                                               Keys::P0};

        static constexpr KeyRow<10> KeyRow4 = {Keys::Z, Keys::X, Keys::C,
                                               Keys::V, Keys::B, Keys::N,
                                               Keys::M, Keys::P1, Keys::P2,
                                               Keys::P3};

        template<std::size_t R, std::size_t N>
        void buildKeyboard(std::shared_ptr<Column> &grid, KeyboardDef<R,N> keyboardDef);

        static constexpr KeyboardDef<4,10> mKeyboardDef = {
                KeyRow1, KeyRow2, KeyRow3, KeyRow4,
        };

        static std::string keyFace(KeyboardMode keyboardMode, Keys keys, KeyboardFace face);

        std::shared_ptr<Slot<Button::SignalType>> mKeyPressRx{};

    public:
        Keyboard() = default;

        ~Keyboard() override = default;

        Keyboard(Keyboard &&) = delete;

        Keyboard(const Keyboard &) = delete;

        Keyboard &operator=(Keyboard &&) = delete;

        Keyboard &operator=(const Keyboard &) = delete;

        /// See Widget::initialLayout()
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /// See Widget::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /// See Widget::initializeComposite()
        void initializeComposite() override;

    };
}

