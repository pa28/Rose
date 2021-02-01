/** @file Types.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-23
  * @brief Types ond constants associated with the SDL library.
  */

#pragma once

#include <chrono>
#include <exception>
#include <memory>
#include <sstream>
#include <functional>
#include <map>
#include <tuple>
#include <variant>
#include <cstdint>
#include <uuid/uuid.h>
#include <SDL2/SDL.h>
#include "Constants.h"

namespace rose {
#ifdef DISPLAY800X480
    static constexpr int DISPLAY_WIDTH = 800;
    static constexpr int DISPLAY_HEIGHT = 480;
    static constexpr int EARTH_BIG_W = 660;
    static constexpr int EARTH_BIG_H = 330;
    static constexpr std::string_view EARTH_BIG_S = "660x330";
#define EARTH_BIG_S 660x330
#else
    static constexpr int DISPLAY_WIDTH = 800;
    static constexpr int DISPLAY_HEIGHT = 480;
    static constexpr int EARTH_BIG_W = 660;
    static constexpr int EARTH_BIG_H = 330;
#define EARTH_BIG_S 660x330                         ///< Size of world map for a screen 800x480
#endif

    /**
     * @brief An opaque type used to refer to a TextureData object in the cache.
     */
    using ImageId = uint32_t;

    /**
     * @brief A type definition for SignalToken used to identify the source of a Signal.
     */
    using SignalToken = uint32_t;

    /// Type definition for the LinearScale Signal type.
    using LinearScaleSignalType = std::pair<float, SignalToken>;

    /**
     * @struct VerticalSpacing
     * @brief A manipulator type for VerticalSpacing.
     */
    struct StartOffset {
        int mStartOffset;       ///< The start offset value, pixels.
    };

    /**
     * @struct HorizontalSpacing
     * @brief A manipulator type for HorizontalSpacing.
     */
    struct InternalSpace {
        int mInternalSpace;     ///< The internal spacing value, pixels.
    };

    /**
     * @struct Elastic
     * @brief A stream manipulator to set a Widget elastic value.
     */
    class Elastic {
    protected:
        Orientation mElastic{Orientation::Unset};   ///< The elastic value.

    public:

        /// Default constructor
        Elastic() = default;

        Elastic(Orientation orientation) : mElastic(orientation) {}

        bool horizontal() { return mElastic == Orientation::Horizontal || mElastic == Orientation::Both; }

        bool vertical() { return mElastic == Orientation::Vertical || mElastic == Orientation::Both; }
    };

    /**
     * @class RoseLogicError
     * @brief Thrown when a structural logic error is detected at runtime.
     */
    class RoseLogicError : public std::logic_error {
    public:
        /// Constructor
        explicit RoseLogicError(const std::string &what) : std::logic_error(what) {}

        /// Constructor
        explicit RoseLogicError(std::string_view what) : std::logic_error(std::string(what)) {}

        /// Constructor
        explicit RoseLogicError(const char *what) : std::logic_error(std::string(what)) {}
    };

    /**
     * @class RoseRuntimeError
     * @brief Thrown when a runtime error has occurred.
     */
    class RoseRuntimeError : public std::runtime_error {
    public:
        /// Constructor
        explicit RoseRuntimeError(const std::string &what) : std::runtime_error(what) {}

        /// Constructor
        explicit RoseRuntimeError(std::string_view what) : std::runtime_error(std::string(what)) {}

        /// Constructor
        explicit RoseRuntimeError(const char *what) : std::runtime_error(std::string(what)) {}
    };
}
