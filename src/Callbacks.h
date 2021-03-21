/**
 * @file Callbacks.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 */

#pragma once

#include <functional>
#include "Types.h"

namespace rose {

    using EventCallback = std::function<bool()>;
    using ScrollCallback = std::function<bool(Position deltaPos)>;
    using ButtonCallback = std::function<bool(bool pressed, uint button, uint clicks)>;
    using MouseMotionCallback = std::function<bool(bool pressed, uint state, Position mousePosition,
            Position relativePosition)>;

}
