/**
 * @file Keyboard.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-26
 */

#pragma once

#include "Button.h"
#include <array>

namespace rose {


    class LetterKey : public TextButton {
    protected:
        std::array<char, 4> mFaceData;
        uint mKeyState{0};

    public:
        LetterKey() = delete;

        ~LetterKey() override = default;

        explicit LetterKey(std::array<char,4> faceData) : mFaceData(faceData) {}
    };

    /**
     * @class Keyboard
     * @brief
     */
    class Keyboard {
    protected:
        static constexpr char CR{'\r'};
        static constexpr char BS{'\b'};
        static constexpr char CapLock{'\001'};
        static constexpr char RightSh{'\002'};
        static constexpr char LeftSh{'\003'};
        static constexpr char KB{'\004'};
        static constexpr char Dots2{'\005'};
        static constexpr char Dots3{'\006'};
        static constexpr char LeftArw{'\007'};
        static constexpr char RightArw{'\013'};
        static constexpr std::array<char,4> Enter {CR, CR, CR, CR};
        static constexpr std::array<char,4> Backspace {BS, BS, BS, BS};
        static constexpr std::array<char,4> CapsLock {CapLock, CapLock, CapLock, CapLock};
        static constexpr std::array<char,4> RightShift {RightSh, RightSh, KB, KB};
        static constexpr std::array<char,4> LeftShift{ LeftSh, LeftSh, KB, KB};
        static constexpr std::array<char,4> KeyState{ KB, KB, Dots2, Dots3};
        static constexpr std::array<char,4> LeftArrow{ LeftArw, LeftArw, LeftArw, LeftArw};
        static constexpr std::array<char,4> RightArrow{ RightArw, RightArw, RightArw, RightArw};

        static constexpr std::array<std::array<char, 4>, 11> keyDataRow0{{
             {'q', 'Q', '1', '1'},
             {'w', 'W', '2', '2'},
             {'e', 'E', '3', '3'},
             {'r', 'R', '4', '4'},
             {'t', 'T', '5', '5'},
             {'y', 'Y', '6', '6'},
             {'u', 'U', '7', '7'},
             {'i', 'I', '8', '8'},
             {'o', 'O', '9', '9'},
             {'p', 'P', '0', '0'},
             Backspace,
                                                                         }};

        static constexpr std::array<std::array<char, 4>, 10> keyDataRow1{{
             {'a', 'A', '@', '!'},
             {'s', 'S', '#', '~'},
             {'d', 'D', '$', '\\'},
             {'f', 'F', '%', '_'},
             {'g', 'G', '^', '['},
             {'h', 'H', '&', ']'},
             {'j', 'J', '*', '{'},
             {'k', 'K', '(', '}'},
             {'l', 'L', ')', '|'},
             Enter
                                                                         }};

        static constexpr std::array<std::array<char, 4>, 11> keyDataRow2{{
             CapsLock,
             {'z', 'S', '#', '~'},
             {'x', 'D', '$', '\\'},
             {'c', 'F', '%', '_'},
             {'v', 'G', '^', '['},
             {'b', 'H', '&', ']'},
             {'n', 'J', '*', '{'},
             {'m', 'K', '(', '}'},
             {',', '?', ',', ':'},
             {'.', '/', '.', ';'},
             RightShift,
                                                                        }};
        static constexpr std::array<std::array<char, 4>, 6> keyDataRow3{{
            LeftShift,
            KeyState,
            { ' ', ' ' , ' ', ' '},
            LeftArrow,
            RightArrow,
            KeyState
        }};

    };
}

