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
    Position Position::Undefined{SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED};
    Size Size::Zero{};
    Rectangle Rectangle::Zero{};

}
