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
     * @brief Type specifier to pass a Font size to a Widget through a manipulator.
     */
    using FontSize = int;

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
     * @struct StartOffset
     * @brief A manipulator to specify the an offset for the first child of a Container along the primary axis.
     */
    struct StartOffset {
        int mStartOffset;       ///< The start offset value, pixels.
    };

    /**
     * @struct FillToEnd
     * @brief A manipulator to set the fill to end flag on a Box widget or subclasses.
     */
    struct FillToEnd {
        bool mFillToEnd;        ///< The value of the fill to end flag.
    };

    /**
     * @struct InternalSpace
     * @brief A manipulator to specify the internal spacing between children along the primary axis.
     */
    struct InternalSpace {
        int mInternalSpace;     ///< The internal spacing value, pixels.
    };

    /**
     * @struct MinimumOrthogonal
     * @brief A manipulator to specify the minimum orthogonal size of a Widget, if supported.
     */
    struct MinimumOrthogonal {
        int mMinimumOrthogonal; ///< The minimum orthogonal size.
    };

    /**
     * @class GeoPosition
     * @brief A geographic position holding Latitude and Longitude.
     */
    class GeoPosition : public std::array<double, 2> {
    public:
        constexpr GeoPosition() noexcept: std::array<double,2>({0., 0.}) {}

        constexpr GeoPosition(double lat, double lon) noexcept
            : std::array<double,2>({std::clamp(lat,-90.,90.),
                                    std::clamp(lon,-180.,180.)}) {}

        constexpr explicit GeoPosition(std::tuple<double,double> geo) : GeoPosition(std::get<0>(geo), std::get<1>(geo)) {}

        constexpr double& lat() noexcept {
            return at(0);
        }

        constexpr double& lon() noexcept {
            return at(1);
        }

        [[nodiscard]] constexpr double lat() const noexcept {
            return at(0);
        }

        [[nodiscard]] constexpr double lon() const noexcept {
            return at(1);
        }
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

        /// Construct an elastic value from an Orientation
        explicit Elastic(Orientation orientation) : mElastic(orientation) {}

        /// Test for elasticity along the Horizontal axis
        bool horizontal() { return mElastic == Orientation::Horizontal || mElastic == Orientation::Both; }

        /// Test for elasticity along the Vertical axis
        bool vertical() { return mElastic == Orientation::Vertical || mElastic == Orientation::Both; }

        /// Test for elasticity along the Primary axis
        bool primary(Orientation orientation) { return mElastic == Orientation::Both || mElastic == orientation; }

        /// Test for elasticity along the Secondary axis
        bool secondary(Orientation orientation) {
            if (mElastic == Orientation::Both)
                return true;
            if (orientation == Orientation::Horizontal && mElastic == Orientation::Vertical)
                return true;
            if (orientation == Orientation::Vertical && mElastic == Orientation::Horizontal)
                return true;
            return false;
        }
    };

    /**
     * @struct FontName
     * @brief A structure to pass a Font name to a Widget through a manipulator.
     */
    struct FontName {
        std::string fontName{};
        FontName() : fontName() {}
        explicit FontName(const char *string) : fontName(string) {}
        explicit FontName(const std::string_view stringView) : fontName(stringView) {}
        explicit FontName(std::string string) : fontName(std::move(string)) {}
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
