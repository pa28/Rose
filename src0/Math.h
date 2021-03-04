/** @file Math.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-26
  * @brief Miscellaneous utility classes and functions.
  */

#pragma once

#include <type_traits>
#include <limits>
#include <cmath>

namespace rose::util {

    template<typename T>
    constexpr T deg2rad(T deg) {
        return M_PI * (deg / 180.);
    }

    template<typename T>
    constexpr T rad2deg(T rad) {
        return 180. * (rad / M_PI);
    }

    /**
    * @brief Round a floating point value to an integer
    * @tparam T the floating point type
    * @param value the value to round
    * @param multiplier a multiplier which can change the where the rounding takes place
    * @return the integer part of the value after rounding
    */
    template<typename T>
    int roundToInt(T value, T multiplier = 1.) {
        static_assert(std::is_floating_point<T>::value, "value must be floating point.");
        return (int) (round(value * multiplier) / multiplier);
    }

    /**
    * @brief Round a floating point value to an floating point value
    * @tparam T the floating point type
    * @param value the value to round
    * @param multiplier a multiplier which can change the where the rounding takes place
    * @return the rounded value
    */
    template<typename T>
    float roundToFloat(T value, T multiplier = 1.) {
        static_assert(std::is_floating_point<T>::value, "value must be floating point.");
        return (float) (round(value * multiplier) / multiplier);
    }

    /**
    * @brief Round a value to an unsigned integer and mask to the maximum value of the target type.
    * @tparam R return value type, must be an unsigned type
    * @tparam V type to round, must be a floating point type
    * @param value the value to round
    * @return the result of rounding and masking.
    */
    template<typename R, typename V>
    constexpr R roundTo(V value) {
        static_assert(std::is_unsigned<R>::value, "return value must be an unsigned type.");
        static_assert(std::is_floating_point<V>::value, "value must be floating point.");
        return (R) round(value) & std::numeric_limits<R>::max();
    }

}
