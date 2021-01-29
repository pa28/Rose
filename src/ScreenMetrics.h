/**
 * @file ScreenMetrics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#pragma once

#include <array>
#include <iostream>
#include <iomanip>
#include <SDL.h>
#include "Constants.h"
#include "Utilities.h"

namespace rose {

    /**
     * @class Size
     * @brief The size of an object on the screen
     */
    class Size : public std::array<int, 2> {
    public:

        static const Size Zero;     ///< A zero size.

        /// Constructor sets value to contents of an array.
        constexpr explicit Size(std::array<int, 2> value) noexcept: std::array<int, 2>(value) {}

        /// Default constructor initializes width and height to 0 or to a specified value.
        constexpr explicit Size(int value = 0) noexcept: Size(std::array<int, 2>({value, value})) {}

        /// Constructor sets value to descrete values.
        constexpr Size(int width, int height) noexcept: Size(std::array<int, 2>({width, height})) {}

        /// Copy constructor
        constexpr Size(const Size &) = default;

        /// Move constructor
        constexpr Size(Size &&) = default;

        /// Copy assignment
        constexpr Size& operator=(const Size &) = default;

        /// Move assignment
        constexpr Size& operator=(Size &&) = default;

        /// Reference accessor for width
        constexpr int &width() { return operator[](0); }

        /// Value accessor for width
        [[nodiscard]] constexpr int width() const noexcept { return operator[](0); }

        /// Reference accessor for height
        constexpr int &height() { return operator[](1); }

        /// Value accessor for height.
        [[nodiscard]] constexpr int height() const noexcept { return operator[](1); }

        /**
         * @brief Reference for primary axis value.
         * @details The primary access differs depending on the orientation
         */
        int &primary(Orientation orientation) {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return width();
                case Orientation::Vertical:
                    return height();
            }
        }

        /**
         * @brief Value for primary axis value.
         * @details The primary access differs depending on the orientation
         */
        [[nodiscard]] constexpr int primary(Orientation orientation) const noexcept {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return width();
                case Orientation::Vertical:
                    return height();
            }
        }

        /**
         * @brief Reference for secondary axis value.
         * @details The secondary access differs depending on the orientation
         */
        int &secondary(Orientation orientation) {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return height();
                case Orientation::Vertical:
                    return width();
            }
        }

        /**
         * @brief Value for secondary axis value.
         * @details The secondary access differs depending on the orientation
         */
        [[nodiscard]] constexpr int secondary(Orientation orientation) const noexcept {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return height();
                case Orientation::Vertical:
                    return width();
            }
        }

        /// Addition operator
        constexpr Size operator+(const Size &size) const noexcept {
            return Size{width() + size.width(), height() + size.height()};
        }

        /// Subtraction operator
        constexpr Size operator-(const Size &size) const noexcept {
            return Size{width() - size.width(), height() - size.height()};
        }
    };

    /**
     * @class Position
     * @brief The position, or relative position of an object on the screen
     */
    class Position : public std::array<int, 2> {
    public:

        static const Position Zero;     ///< A zero position.

        /// Constructor initialize to the value of an array
        constexpr explicit Position(std::array<int, 2> value) noexcept: std::array<int, 2>(value) {}

        /// Default constructor initializes x and y to 0 or to a specified value
        constexpr explicit Position(int value = 0) noexcept: Position(std::array<int, 2>({value, value})) {}

        /// Constructor initialize to descrete x and y values.
        constexpr Position(int x, int y) noexcept: Position(std::array<int, 2>({x, y})) {}

        /// Copy constructor
        constexpr Position(const Position &) = default;

        /// Move constructor
        constexpr Position(Position &&) = default;

        /// Copy assignment
        constexpr Position& operator=(const Position &) = default;

        /// Move assignment
        constexpr Position& operator=(Position &&) = default;

        /// Reference access to x
        constexpr int &x() { return operator[](0); }

        /// Value access to x
        [[nodiscard]] constexpr int x() const noexcept { return operator[](0); }

        /// Reference access to y
        constexpr int &y() { return operator[](1); }

        /// Value access to y
        [[nodiscard]] constexpr int y() const noexcept { return operator[](1); }

        /**
         * @brief Reference for primary axis value.
         * @details The primary access differs depending on the orientation
         */
        int &primary(Orientation orientation) {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return x();
                case Orientation::Vertical:
                    return y();
            }
        }

        /**
         * @brief Value for primary axis value.
         * @details The primary access differs depending on the orientation
         */
        [[nodiscard]] constexpr int primary(Orientation orientation) const noexcept {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return x();
                case Orientation::Vertical:
                    return y();
            }
        }

        /**
         * @brief Reference for secondary axis value.
         * @details The secondary access differs depending on the orientation
         */
        int &secondary(Orientation orientation) {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return y();
                case Orientation::Vertical:
                    return x();
            }
        }

        /**
         * @brief Value for secondary axis value.
         * @details The secondary access differs depending on the orientation
         */
        [[nodiscard]] constexpr int secondary(Orientation orientation) const noexcept {
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    return y();
                case Orientation::Vertical:
                    return x();
            }
        }

        /// Addition operator
        constexpr Position operator+(const Position &position) const noexcept {
            return Position{x() + position.x(), y() + position.y()};
        }

        /// Subtraction operator
        constexpr Position operator-(const Position &position) const noexcept {
            return Position{x() - position.x(), y() - position.y()};
        }
    };

    /**
     * @class Rectangle
     * @brief Describes area on the screen. Position may be relative or absolute.
     */
    class Rectangle : public std::array<int, 4> {
    public:
        static const Rectangle Zero;        ///< A rectangle with position and size of zero.

        /// Constructor initialize to the value of an array
        constexpr explicit Rectangle(std::array<int, 4> value) noexcept: std::array<int, 4>(value) {}

        /// Default constructor initializes x and y to 0 or to a specified value
        constexpr explicit Rectangle(int value = 0) noexcept: Rectangle(
                std::array<int, 4>({value, value, value, value})) {}

        /// Constructor initialize to descrete x, y, width, and height values.
        constexpr Rectangle(int x, int y, int width, int height) noexcept: Rectangle(
                std::array<int, 4>({x, y, width, height})) {}

        /// Copy constructor
        constexpr Rectangle(const Rectangle &) = default;

        /// Move constructor
        constexpr Rectangle(Rectangle &&) = default;

        /// Copy assignment
        constexpr Rectangle &operator=(const Rectangle &) = default;

        /// Move assignment
        constexpr Rectangle &operator=(Rectangle &&) = default;

        /// Constructor initialize to the value of a Position and Size.
        constexpr Rectangle(const Position &pos, const Size &size) : Rectangle(pos.x(), pos.y(), size.width(),
                                                                               size.height()) {}

        /// Constructor initialize to the value of std::optional Position and Size.
        constexpr Rectangle(const std::optional<Position> &pos, const std::optional<Size> &size) : Rectangle() {
            if (pos) {
                x() = pos->x();
                y() = pos->y();
            }
            if (size) {
                width() = size->width();
                height() = size->height();
            }
        }

        /// Reference access to x
        constexpr int &x() { return operator[](0); }

        /// Value access to x
        [[nodiscard]] constexpr int x() const noexcept { return operator[](0); }

        /// Reference access to y
        constexpr int &y() { return operator[](1); }

        /// Value access to y
        [[nodiscard]] constexpr int y() const noexcept { return operator[](1); }

        /// Reference access to width
        constexpr int &width() { return operator[](2); }

        /// Value access to width
        [[nodiscard]] constexpr int width() const noexcept { return operator[](2); }

        /// Reference access to height
        constexpr int &height() { return operator[](3); }

        /// Value access to height
        [[nodiscard]] constexpr int height() const noexcept { return operator[](3); }

        /// Get the Size of a Rectangle
        [[nodiscard]] constexpr Size getSize() const noexcept {
            return Size{width(), height()};
        }

        /// Get the Position of a Rectangle
        [[nodiscard]] constexpr Position getPosition() const noexcept {
            return Position{x(), y()};
        }

        [[nodiscard]] constexpr SDL_Rect toSdlRect() const noexcept {
            SDL_Rect rect{};
            rect.x = x();
            rect.y = y();
            rect.w = width();
            rect.h = height();
            return rect;
        }

        /// Assign a Size to a Rectangle
        constexpr Rectangle &operator=(const Size &size) {
            width() = size.width();
            height() = size.height();
            return *this;
        }

        /// Assign a Position to a Rectangle
        constexpr Rectangle &operator=(const Position &position) {
            x() = position.x();
            y() = position.y();
            return *this;
        }

        /// Move a Rectangle by a delta Position.
        constexpr Rectangle &operator+=(const Position &deltaPos) {
            x() += deltaPos.x();
            y() += deltaPos.y();
            return *this;
        }

        /// Add a Position to a Rectangle.
        constexpr Rectangle operator+(const Position &deltaPos) {
            Rectangle r{*this};
            r.x() += deltaPos.x();
            r.y() += deltaPos.y();
            return r;
        }

        /// Move the origin of a Rectangle by an optional deltaPos and reduce the size of the rectangle by a corresponding amount.
        [[nodiscard]] constexpr Rectangle moveOrigin(const std::optional<Position> &deltaPos) const {
            Rectangle r{*this};
            if (deltaPos) {
                r += deltaPos.value();
                r.width() -= deltaPos->x();
                r.height() -= deltaPos->y();
            }
            return r;
        }

        /// Determine if a Rectangle contains a Position.
        [[nodiscard]] constexpr bool contains(Position pos) const noexcept {
            return pos.x() >= x() && pos.x() < x() + width() && pos.y() >= y() && pos.y() < y() + height();
        }

        /**
         * @brief Determine if there is no overlap between two Rectangle objects.
         * @param o The other Rectangle.
         * @return True if there is no overlap of this Rectangle and the other Rectangle.
         */
        [[nodiscard]] constexpr bool noOverlap(const Rectangle& o) const noexcept {
            return x() > o.x() + o.width() ||
                   o.x() > x() + width() ||
                   y() > o.y() + o.height() ||
                   o.y() > y() + height();
        }

        /**
         * @brief Determine if there is overlap between to Rectangle objects by inverting noOverlap().
         * @param o The other Rectangle.
         * @return True if there is overlap of this Rectangle and the other Rectangle.
         */
        [[nodiscard]] constexpr bool overlap(const Rectangle& o) const noexcept {
            return !noOverlap(o);
        }

    };

    /**
     * @class Padding
     * @brief Padding values for Border Widget objects
     */
    class Padding : public std::array<int,4> {
    public:
        static const Padding Zero;        ///< A rectangle with position and size of zero.

        /// Constructor initialize to the value of an array
        constexpr explicit Padding(std::array<int, 4> value) noexcept: std::array<int, 4>(value) {}

        /// Default constructor initializes x and y to 0 or to a specified value
        constexpr explicit Padding(int value = 0) noexcept: Padding(
                std::array<int, 4>({value, value, value, value})) {}

        /// Constructor initialize to descrete x, y, width, and height values.
        constexpr Padding(int left, int top, int right, int bottom) noexcept: Padding(
                std::array<int, 4>({left, top, right, bottom})) {}


        [[nodiscard]] constexpr int left() const noexcept { return (*this)[0]; }      ///< Get the left padding.
        [[nodiscard]] constexpr int right() const noexcept { return (*this)[1]; }     ///< Get the right padding.
        [[nodiscard]] constexpr int top() const noexcept { return (*this)[2]; }       ///< Get the top padding.
        [[nodiscard]] constexpr int bottom() const noexcept { return (*this)[3]; }    ///< Get the bottom padding.
        [[nodiscard]] constexpr int width() const noexcept { return left() + right(); }    ///< Get the width of the padding.
        [[nodiscard]] constexpr int height() const noexcept { return top() + bottom(); }    ///< Get the height of the padding.

        /// Get the total size of the padding.
        [[nodiscard]] constexpr Size padSize() const noexcept { return Size{width(),height()}; }
        /// Get the position of the padding.
        [[nodiscard]] constexpr Position padPos() const noexcept { return Position{left(), top()}; }

        int& left() noexcept { return (*this)[0]; }          ///< Access the left padding.
        int& right() noexcept { return (*this)[1]; }         ///< Access the right padding.
        int& top() noexcept { return (*this)[2]; }           ///< Access the top padding.
        int& bottom() noexcept { return (*this)[3]; }        ///< Access the bottom padding.
    };

    /**
     * @class Line
     * @brief An abstraction of a line defined by two end points.
     */
    class Line {
    protected:
        Position    mPoint0;            ///< One endpoint of the line.
        Position    mPoint1;            ///< The other endpoint of the line.

    public:
        /**
         * @brief Construct line of zero length.
         */
        constexpr Line() noexcept : mPoint0(), mPoint1() {}

        /**
         * @brief Construct a line from p0 to p1.
         * @param p0 An end point.
         * @param p1 An end point.
         */
        constexpr Line(const Position &p0, const Position &p1) : mPoint0(p0), mPoint1(p1) {}

        /// Copy constructor.
        constexpr Line(const Line &) = default;

        /// Move Constructor.
        constexpr Line(Line &&) = default;

        /// Copy assignment.
        constexpr Line& operator=(const Line &) = default;

        /// Move assignment.
        constexpr Line &operator=(Line &&) = default;

    };
}

/**
 * @brief An output stream inserter to print ScreenMetrics.
 * @tparam T The underlying type.
 * @tparam N The size
 * @param strm The output stream.
 * @param array The ScreenMetric
 * @return The output stream.
 */
template<typename T, size_t N>
inline std::ostream &operator<<(std::ostream &strm, const std::array<T, N> &array) {
    return rose::util::printScreenMetric<T, N>(strm, array);
}

/**
 * @brief An output stream inserter to print a std::optional Rectangle, Position or Size.
 * @tparam T The type to print.
 * @param strm The output stream.
 * @param opt The std::optional
 * @return The output stream.
 */
template<typename T>
inline std::ostream &operator<<(std::ostream &strm, const std::optional<T> &opt) {
    if (opt) {
        if constexpr (std::is_same_v<T, rose::Rectangle>)
            rose::util::printScreenMetric<int, 4>(strm, opt.value());
        if constexpr (std::is_same_v<T, rose::Position> || std::is_same_v<T, rose::Size>)
            rose::util::printScreenMetric<int,2>(strm, opt.value());
    } else
        strm << "(empty)";
    return strm;
}

