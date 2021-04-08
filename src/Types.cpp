/**
 * @file Types.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-07
 */

#include "Types.h"
#include <SDL.h>

namespace rose {

    Position Position::Zero{};
    Position Position::Undefined{WINDOWPOS_UNDEFINED, WINDOWPOS_UNDEFINED};
    Size Size::Zero{};
    Rectangle Rectangle::Zero{};

    std::array<char, 8> utf8(int c) {
        auto uc = (uint) c;
        std::array<char, 8> seq{};
        int n = 0;
        if (uc < 0x80) n = 1;
        else if (uc < 0x800) n = 2;
        else if (uc < 0x10000) n = 3;
        else if (uc < 0x200000) n = 4;
        else if (uc < 0x4000000) n = 5;
        else if (uc <= 0x7fffffff) n = 6;
        seq[n] = '\0';
        switch (n) {
            case 6:
                seq[5] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x4000000u;
            case 5:
                seq[4] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x200000u;
            case 4:
                seq[3] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x10000u;
            case 3:
                seq[2] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x800u;
            case 2:
                seq[1] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0xc0u;
            case 1:
                seq[0] = uc;
            default:
                break;
        }
        return seq;
    }
}
