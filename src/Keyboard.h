/**
 * @file Keyboard.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-26
 */

#pragma once

#include "Button.h"
#include "Manager.h"
#include <array>
#include <utility>

namespace rose {

    struct KeySpec {
        bool imageKey;
        std::array<uint, 4> command;

        constexpr KeySpec(bool imgKey, std::array<uint, 4> cmd) : command(cmd), imageKey(imgKey) {
        }
    };

    static constexpr std::array<KeySpec, 1> K0{{
                                                       {true, {SDLK_CAPSLOCK}}
                                               }};

    /**
     * @class Keyboard
     * @brief
     */
    class Keyboard : public Grid {
    protected:
        bool mCapsLock{false};
        bool mShiftActive{false};
        uint mKeyState{0};
        uint mAltState{0};

        static constexpr std::array<KeySpec, 11> keyDataRow0
                {{
                         {false, {SDLK_q, SDLK_q, SDLK_1, SDLK_1}},
                         {false, {SDLK_w, SDLK_w, SDLK_2, SDLK_2}},
                         {false, {SDLK_e, SDLK_e, SDLK_3, SDLK_3}},
                         {false, {SDLK_r, SDLK_r, SDLK_4, SDLK_4}},
                         {false, {SDLK_t, SDLK_t, SDLK_5, SDLK_5}},
                         {false, {SDLK_y, SDLK_y, SDLK_6, SDLK_6}},
                         {false, {SDLK_u, SDLK_u, SDLK_7, SDLK_7}},
                         {false, {SDLK_i, SDLK_i, SDLK_8, SDLK_8}},
                         {false, {SDLK_o, SDLK_o, SDLK_9, SDLK_9}},
                         {false, {SDLK_p, SDLK_p, SDLK_0, SDLK_0}},
                         {true, {SDLK_BACKSPACE, SDLK_BACKSPACE, SDLK_BACKSPACE, SDLK_BACKSPACE}}
                 }};

        static constexpr std::array<KeySpec, 10> keyDataRow1
                {{
                         {false, {SDLK_a, SDLK_a, SDLK_AT, SDLK_AT}},
                         {false, {SDLK_s, SDLK_s, SDLK_HASH, SDLK_HASH}},
                         {false, {SDLK_d, SDLK_d, SDLK_DOLLAR, SDLK_DOLLAR}},
                         {false, {SDLK_f, SDLK_f, SDLK_PERCENT, SDLK_PERCENT}},
                         {false, {SDLK_g, SDLK_g, SDLK_CARET, SDLK_CARET}},
                         {false, {SDLK_h, SDLK_h, SDLK_AMPERSAND, SDLK_AMPERSAND}},
                         {false, {SDLK_j, SDLK_j, SDLK_ASTERISK, SDLK_ASTERISK}},
                         {false, {SDLK_k, SDLK_k, SDLK_LEFTPAREN, SDLK_LEFTPAREN}},
                         {false, {SDLK_l, SDLK_l, SDLK_RIGHTPAREN, SDLK_RIGHTPAREN}},
                         {true, {SDLK_RETURN, SDLK_RETURN, SDLK_RETURN, SDLK_RETURN}}
                 }};

        static constexpr std::array<KeySpec, 11> keyDataRow2
                {{
                         {true, {SDLK_CAPSLOCK, SDLK_CAPSLOCK, SDLK_CAPSLOCK, SDLK_CAPSLOCK}},
                         {false, {SDLK_z, SDLK_z, '~', '~'}},
                         {false, {SDLK_x, SDLK_x, SDLK_BACKSLASH, SDLK_BACKSLASH}},
                         {false, {SDLK_c, SDLK_c, SDLK_UNDERSCORE, SDLK_UNDERSCORE}},
                         {false, {SDLK_v, SDLK_v, SDLK_LEFTBRACKET, SDLK_BACKQUOTE}},
                         {false, {SDLK_b, SDLK_b, SDLK_RIGHTBRACKET, SDLK_COMMA}},
                         {false, {SDLK_n, SDLK_n, '{', SDLK_PERIOD}},
                         {false, {SDLK_m, SDLK_m, '}', SDLK_QUESTION}},
                         {false, {SDLK_COMMA, SDLK_QUESTION, SDLK_COLON, SDLK_COLON}},
                         {false, {SDLK_PERIOD, SDLK_SLASH, SDLK_SEMICOLON, SDLK_SEMICOLON}},
                         {true, {SDLK_RSHIFT, SDLK_RSHIFT, SDLK_RSHIFT, SDLK_RSHIFT}}
                 }};
        static constexpr std::array<KeySpec, 6> keyDataRow3
                {{
                         {true, {SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT}},
                         {true, {SDLK_LALT, SDLK_LALT, SDLK_LALT, SDLK_LALT}},
                         {false, {SDLK_SPACE, SDLK_SPACE, SDLK_SPACE, SDLK_SPACE}},
                         {true, {SDLK_LEFT, SDLK_LEFT, SDLK_LEFT, SDLK_LEFT}},
                         {true, {SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT}},
                         {true, {SDLK_RALT, SDLK_RALT, SDLK_RALT, SDLK_RALT}}
                 }};

    public:
        Keyboard() : Grid() {}

        ~Keyboard() override = default;

        void addedToContainer() override;

        void keyCommandCallback(ButtonStateChange stateChange, uint command);
    };

    class LetterKey : public TextButton {
    protected:
        std::array<uint, 4> mCommand;
        uint mKeyState{0};

        void setTextFromCommand();

        std::function<void()> mKeyCallback;

    public:
        LetterKey() = delete;

        ~LetterKey() override = default;

        explicit LetterKey(std::array<uint, 4> cmd) : TextButton(), mCommand(cmd) {
            mCentreVertical = true;
            mCentreHorizontal = true;
        }

        void addedToContainer() override;

        void setKeyState(uint keyState) {
            mKeyState = keyState;
            setTextFromCommand();
        }
    };

    class ImageKey : public ImageButton {
    protected:
        std::array<uint, 4> mCommand;
        uint mKeyState{0};

        void setImageFromCommand();

    public:
        ImageKey() = delete;

        ~ImageKey() override = default;

        explicit ImageKey(std::array<uint, 4> cmd) : ImageButton(), mCommand(cmd) {
            mCentreVertical = true;
            mCentreHorizontal = true;
        }

        void addedToContainer() override;

        void setKeyState(uint keyState) {
            mKeyState = keyState;
            setImageFromCommand();
        }
    };

    class CapsLockKey : public ImageKey {
    protected:
        bool mLockState{false};

        void setImageFromLockState();

    public:
        CapsLockKey() = delete;

        explicit CapsLockKey(std::array<uint, 4> cmd) : ImageKey(cmd) {
        }

        void addedToContainer() override;
    };
}

