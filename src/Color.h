/** @file Color.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2021-01-20
  *
  * Another significant redesign to update the coding standards to C++17,
  * reduce the amount of bare pointer handling (especially in user code),
  * and focus on the RaspberryPi environment.
  *
  * License terms for the changes as well as the base nanogui-sdl code are
  * contained int the LICENSE.txt file.
  *
  * A significant redesign of this code was contributed by Christian Schueller.
  *
  * Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
  * Adaptation for SDL by Dalerank <dalerankn8@gmail.com>
  *
  * All rights reserved. Use of this source code is governed by a
  * BSD-style license that can be found in the LICENSE.txt file.
  *
  * @brief The Color Class, and supporting types.
  */

#pragma once

#include <algorithm>
#include <array>
#include <SDL.h>
#include "Configuration.h"

/**
 * @namespace rose::color
 * @brief Color management.
 */
namespace rose::color {

    /**
     * @struct Value
     * @brief The actual value of a color.
     * @details The value of a color is an array of four float values. How these values are interpreted
     * depends of the type interpretation of the value. All for values are in the range [0..1],
     * For RBBA they are Red, Green, Blue and Alpha. For HSVA they are Hue, Saturation, Value and Alpha but
     * the Hue value is mapped into [0..360] before use.
     */
    struct Value : public std::array<float, 4> {
        /**
         * @brief Constructor, initialize color to transparent black.
         */
        constexpr Value() noexcept: std::array<float, 4>() {
            for (auto &v : *this) {
                v = 0.f;
            }
        }

        constexpr explicit Value(std::array<float, 4> v) noexcept: std::array<float, 4>(v) {}
    };

    struct HSVA;
    /**
     * @class RGBA
     * @brief Red Green Blue Alpha representation of a color.
     */
    struct RGBA : public Value {

        static const RGBA TransparentBlack;

        constexpr RGBA() noexcept = default;

        /// Construct RGBA from a std::array of floats in range [0.0 ... 1.0]
        constexpr explicit RGBA(std::array<float, 4> v) noexcept: Value(v) {}

        /// Construct RGBA from unsigned integer components in range [0 ... 255]
        constexpr RGBA(uint r, uint g, uint b, uint a) noexcept {
            operator[](0) = (float) (r & 0xffu) / 255.f;
            operator[](1) = (float) (g & 0xffu) / 255.f;
            operator[](2) = (float) (b & 0xffu) / 255.f;
            operator[](3) = (float) (a & 0xffu) / 255.f;
        }

        /// Construct RGBA from float components in range [0.0 ... 1.0]
        constexpr RGBA(float r, float g, float b, float a) noexcept {
            operator[](0) = r;
            operator[](1) = g;
            operator[](2) = b;
            operator[](3) = a;
        }

        /// Construct RGBA from HSVA.
        constexpr explicit RGBA(const HSVA &hsva) noexcept;

        /// Convert this colour to an SDL_Color
        [[nodiscard]] constexpr SDL_Color toSdlColor() const noexcept {
            SDL_Color color{};
            color.r = (uint8_t)(r() * 255.f);
            color.g = (uint8_t)(g() * 255.f);
            color.b = (uint8_t)(b() * 255.f);
            color.a = (uint8_t)(a() * 255.f);
            return color;
        }

        /// Reference access to the Red value
        constexpr float &r() noexcept { return operator[](0); }

        /// Value access to the red value
        [[nodiscard]] constexpr float r() const noexcept { return operator[](0); }

        /// Reference access to the Green value
        constexpr float &g() noexcept { return operator[](1); }

        /// Value access to the red value
        [[nodiscard]] constexpr float g() const noexcept { return operator[](1); }

        /// Reference access to the Blue value
        constexpr float &b() noexcept { return operator[](2); }

        /// Value access to the Blue value
        [[nodiscard]] constexpr float b() const noexcept { return operator[](2); }

        /// Reference access to the Alpha value
        constexpr float &a() noexcept { return operator[](3); }

        /// Value access to the Alpha value
        [[nodiscard]] constexpr float a() const noexcept { return operator[](3); }

        /// Multiply a color by a constant.
        constexpr RGBA operator*(float m) const noexcept {
            return RGBA{r() * m, g() * m, b() * m, a() * m};
        }

        /// Add two RGBA values together.
        constexpr RGBA operator+(const RGBA color) const noexcept {
            return RGBA{r() + color.r(), g() + color.g(), b() + color.b(), a() + color.a()};
        }

        /// Set the Alpha channel

        HSVA toHSVA() const;
    };

    template<typename T>
    static constexpr T mod(T x, T y) noexcept {
        if (y == 0)
            return 0;
        if constexpr (std::is_integral_v<T>) {
            return x % y;
        } else {
            auto trunc = (float) ((int) (x / y));
            auto r = x - trunc * y;
            return r;
        }
    }

    /**
     * @class HSVA
     * @brief Hue Saturation Value (or Brightness) representation of a color
     */
    struct HSVA : public Value {
        constexpr HSVA() noexcept = default;

        constexpr explicit HSVA(std::array<float, 4> v) noexcept: Value(v) {
            operator[](0) /= 360.f;
        }

        constexpr HSVA(float h, float s, float v, float a) noexcept: Value() {
            operator[](0) = h / 360.f;
            operator[](1) = s;
            operator[](2) = v;
            operator[](3) = a;
        }

        [[nodiscard]] constexpr HSVA modValue(float dValue) const noexcept {
            HSVA result{*this};
            result[2] += dValue;
            return result;
        }

        /**
         * @brief Get a new colour from this HSL colour by modifying the hue value.
         * @param hue The new hue value
         * @return The new HSL colour.
         */
        [[nodiscard]] constexpr HSVA withHue(uint32_t hue) const {
            return HSVA{{(float) (hue % 360) / 360.f, saturation(), value(), alpha()}};
        }

        /**
         * @brief Get a new colour from this HSL colour by modifying the hue value.
         * @param hue The new hue value
         * @return The new HSL colour.
         */
        [[nodiscard]] constexpr HSVA withMinSaturation(float sat) const {
            return HSVA{{hue(), std::max(saturation(),sat), value(), alpha()}};
        }

        /**
         * @brief Get a new contrasting colour to this HSL colour.
         * @return A new contrasting HSL colour.
         */
        [[nodiscard]] constexpr HSVA contrasting() const {
            float value = 0;
            if (operator[](2) < 0.5f)
                value = operator[](2) + 0.4f;
            else
                value = operator[](2) - 0.4f;
            return HSVA{{operator[](0), operator[](1), value, operator[](3)}};
        }

        /**
         * @brief Convert to RGBA in an array of floats.
         * @details See <a href="https://www.rapidtables.com/convert/color/hsl-to-rgb.html">hsl to rgb</a>
         * @return RGBA colour Value.
         */
        [[nodiscard]] constexpr RGBA toRGBA() const noexcept {
            auto H = mod(operator[](0) * 360.0f, 360.f);
            float S = operator[](1);
            float V = operator[](2);
            float C = S * V;
            float X = C * (1 - abs(mod(H / 60.0f, 2.f) - 1));
            float m = V - C;
            float r = 0.f, g = 0.f, b = 0.f;
            if (H >= 0 && H < 60) {
                r = C, g = X, b = 0;
            } else if (H >= 60 && H < 120) {
                r = X, g = C, b = 0;
            } else if (H >= 120 && H < 180) {
                r = 0, g = C, b = X;
            } else if (H >= 180 && H < 240) {
                r = 0, g = X, b = C;
            } else if (H >= 240 && H < 300) {
                r = X, g = 0, b = C;
            } else {
                r = C, g = 0, b = X;
            }

            return RGBA{std::array<float, 4>{r + m, g + m, b + m, operator[](3)}};
        }

        constexpr float &hue() noexcept { return operator[](0); }

        [[nodiscard]] constexpr float hue() const noexcept { return operator[](0); }

        constexpr float &saturation() noexcept { return operator[](1); }

        [[nodiscard]] constexpr float saturation() const noexcept { return operator[](1); }

        constexpr float &value() noexcept { return operator[](2); }

        [[nodiscard]] constexpr float value() const noexcept { return operator[](2); }

        constexpr float &alpha() noexcept { return operator[](3); }

        [[nodiscard]] constexpr float alpha() const noexcept { return operator[](3); }
    };

    constexpr RGBA::RGBA(const HSVA &hsva) noexcept : RGBA(hsva.toRGBA()) {
    }

    /**
     * @brief Set the Alpha value in a pixel color.
     * @param pixel The pixel color
     * @param a The alpha value
     * @return The modified pixel color.
     */
    static constexpr uint32_t set_a_value(uint32_t pixel, uint32_t a) { return (pixel & cmask) | a << ashift; }


    /**
     * @class Interpolator
     * @brief Provide linear interpolation between two color Values.
     * @tparam Representation The Representation type of the Value.
     */
    template<class Representation>
    class Interpolator {
    public:
        using color_type = Representation;
        using index_type = ssize_t;

    protected:
        color_type mStart;      ///< The starting color
        color_type mFinish;     ///< The finishing color
        index_type mSteps;      ///< The number of steps in the interpolation
        Value mIncrement;       ///< The amount to increment

        /**
         * @class Iterator
         * @brief An iterator over the range of the interpolator.
         */
        class Iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = index_type;
            using value_type = color_type;
            using pointer = color_type *;
            using reference = color_type &;

        protected:
            const Interpolator<color_type> *mInterpolator{nullptr};    ///< A pointer to the Interpolator.
            color_type mValue{};                                       ///< The current color Value.
            index_type mIndex{0};                                      ///< The index of the current position.

        public:
            Iterator() = default;

            /**
             * @brief Constructor, set the iterator to begin.
             * @param interpolator
             */
            explicit Iterator(const Interpolator *interpolator) : mInterpolator(interpolator) {
                mValue = interpolator->mStart;
            }

            /**
             * @brief Constructor, set the iterator to a random location, or end if index is out of range.
             * @param interpolator
             * @param index
             */
            Iterator(const Interpolator *interpolator, difference_type index) : Iterator(interpolator) {
                if (index >= 0 && index < interpolator->mSteps)
                    mIndex = index;
                else
                    mIndex = interpolator->mSteps;
                mValue = (*mInterpolator)(mIndex);
            }

            /**
             * @brief Copy constructor
             * @param iterator
             */
            Iterator(const Iterator &iterator) : Iterator(iterator.mInterpolator) {
                mIndex = iterator.mIndex;
                mValue = iterator.mValue;
            }

            /**
             * @brief Move constructor
             * @param iterator
             */
            Iterator(Iterator &&iterator) noexcept: Iterator(iterator.mInterpolator) {
                mIndex = iterator.mIndex;
                mValue = iterator.mValue;
                iterator.mInterpolator = nullptr;
            }

            /**
             * @brief Assignment operator.
             * @param iterator
             * @return
             */
            Iterator &operator=(const Iterator &iterator) {
                mInterpolator = iterator.mInterpolator;
                mIndex = iterator.mIndex;
                mValue = iterator.mValue;
                return *this;
            }

            /**
             * @brief Move assignment operator.
             * @param iterator
             * @return
             */
            Iterator &operator=(Iterator &&iterator) noexcept {
                mInterpolator = iterator.mInterpolator;
                mIndex = iterator.mIndex;
                mValue = iterator.mValue;
                iterator.mInterpolator = nullptr;
                return *this;
            }

            /**
             * @brief Return a reference to the iterator value.
             * @return
             */
            reference operator*() const { return mValue; }

            /**
             * @brief Return a pointer to the iterator value.
             * @return
             */
            pointer operator->() const { return &mValue; }

            /// Pre-increment operator
            Iterator &operator++() {
                if (mIndex < mInterpolator->mSteps) {
                    ++mIndex;
                    mValue = (*mInterpolator)(mIndex);
                }
                return *this;
            }

            /// Post-increment operator
            Iterator operator++(int) {
                Iterator tmp{*this};
                operator++();
                return tmp;
            }

            bool operator==(const Iterator &other) const {
                return mInterpolator == other.mInterpolator && mIndex == other.mIndex;
            }

            bool operator!=(const Iterator &other) const {
                return mInterpolator != other.mInterpolator || mIndex != other.mIndex;
            }
        };

    public:
        Interpolator() = delete;

        Interpolator(Representation start, Representation finish, size_t steps)
                : mStart(start), mFinish(finish), mSteps(steps) {
            auto fSteps = static_cast<float>(mSteps);
            std::transform(mStart.begin(), mStart.end(), mFinish.begin(), mIncrement.begin(),
                           [=](float start, float finish) {
                               return (finish - start) / fSteps;
                           });
        }

        /**
         * @brief Get the interpolated value at index.
         * @param index The index to interpolate at [0..steps)
         * @return The interpolated color Value.
         */
        color_type operator()(index_type index) {
            color_type result{};
            auto fIndex = static_cast<float>(index);
            std::transform(mStart.begin(), mIncrement.begin(), result.begin(),
                           [=](float start, float increment) {
                               return start + increment * fIndex;
                           });
            return result;
        }

        /**
         * @brief Get an iterator pointing to the start of the interpolated range.
         * @return an Iterator
         */
        Iterator begin() {
            return Iterator{*this};
        }

        /**
         * @brief Get an iterator pointing one past the end of the interpolated range.
         * @return on Iterator
         */
        Iterator end() {
            Iterator iterator{*this};
            iterator.mIndex = mSteps;
            return iterator;
        }
    };

    static constexpr color::HSVA DarkBaseColorHSVA{{200.f, .00, .15, 1.0}};
    static constexpr color::RGBA DarkBaseColor{DarkBaseColorHSVA};
    static constexpr color::RGBA DarkTopColor{DarkBaseColorHSVA.modValue(0.2)};
    static constexpr color::RGBA DarkBotColor{DarkBaseColorHSVA.modValue(-0.15)};
    static constexpr color::RGBA DarkLeftColor{DarkBaseColorHSVA.modValue(0.1)};
    static constexpr color::RGBA DarkRightColor{DarkBaseColorHSVA.modValue(-0.15)};
    static constexpr color::RGBA DarkInvertColor{DarkBaseColorHSVA.modValue(-0.075)};
    static constexpr color::RGBA DarkTextColour{DarkBaseColorHSVA.contrasting()};
    static constexpr color::RGBA DarKRed{ 1.f, 0.f, 0.f, 1.f};
}
