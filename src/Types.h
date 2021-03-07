/**
 * @file Types.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-07
 */

#pragma once

#include <iostream>
#include <tuple>
#include <utility>

namespace rose {

    namespace set {
        static constexpr std::string_view SetAppSize{"SetAppSize"};
        static constexpr std::string_view SetAppPosition{"SetAppPosition"};
        static constexpr std::string_view SetAppState{"SetAppState"};
    }

    /**
     * @struct Position
     * @brief A position in integer (x, y) co-ordinates.
     * @details For screen objects (0, 0) is the top left corner, x increases to the right, y increases toward
     * the bottom.
     */
    struct Position {
        int x{0}, y{0};

        constexpr Position() noexcept = default;
        constexpr Position(int X, int Y) noexcept : x(X), y(Y) {}
        constexpr Position(const Position &p) = default;
        constexpr Position(Position &&p) = default;
        constexpr Position& operator=(const Position &p) = default;
        constexpr Position& operator=(Position &&p) = default;

        /// Add two positions together.
        constexpr Position operator+(const Position &p) const noexcept {
            return Position{x + p.x, y + p.y};
        }

        bool operator!=(const Position &other) const noexcept {
            return x != other.x || y != other.y;
        }

        bool operator==(const Position &other) const noexcept {
            return x == other.x && y == other.y;
        }

        static Position Zero;
        static Position Undefined;
    };

    /**
     * @struct Size
     * @brief A size in integer dimensions.
     * @details The width (w) is in the direction of the x axis, the height (h) is in the direction of the y axis.
     */
    struct Size {
        int w{0}, h{0};

        constexpr Size() noexcept = default;
        constexpr Size(int W, int H) noexcept : w(W), h(H) {}
        constexpr Size(const Size &p) = default;
        constexpr Size(Size &&p) = default;
        constexpr Size& operator=(const Size &p) = default;
        constexpr Size& operator=(Size &&p) = default;

        bool operator!=(const Size &other) const noexcept {
            return w != other.w || h != other.h;
        }

        bool operator==(const Size &other) const noexcept {
            return w == other.w && h == other.h;
        }

        static Size Zero;
    };

    /**
     * @struct Rectangle
     * @brief A composite of a Position and a Size.
     */
    struct Rectangle {
        int x{0}, y{0}, w{0}, h{0};

        constexpr Rectangle() noexcept = default;
        constexpr Rectangle(int X, int Y, int W, int H) noexcept : x(X), y(Y), w(W), h(H) {}
        constexpr Rectangle(const Position &p, const Size &s) noexcept : x(p.x), y(p.y), w(s.w), h(s.h) {}
        constexpr Rectangle(const Rectangle &p) = default;
        constexpr Rectangle(Rectangle &&p) = default;
        constexpr Rectangle& operator=(const Rectangle &p) = default;
        constexpr Rectangle& operator=(Rectangle &&p) = default;

        constexpr Rectangle& operator=(const Size &s) {
            w = s.w;
            h = s.h;
            return *this;
        }

        constexpr Rectangle& operator=(const Position &p) {
            x = p.x;
            y = p.y;
            return *this;
        }

        /// Add a Position to a Rectangle.
        constexpr Rectangle operator + (const Position &p) const noexcept {
            return Rectangle{x+p.x, y+p.y, w, h};
        }

        /// Get a Position from a Rectangle.
        [[nodiscard]] Position position() const noexcept {
            return Position{x,y};
        }

        /// Get a Size from a Rectangle
        [[nodiscard]] Size size() const noexcept {
            return Size{w,h};
        }

        /// Get the Positions of top-left and bottom-right corners defined by the Rectangle
        [[nodiscard]] std::pair<Position,Position> primeCorners() const noexcept {
            return std::make_pair(Position{x,y}, Position{x+w,y+h});
        }

        /// Get the Positions of the top-right and bottom-left corners defined by the Rectangle
        [[nodiscard]] std::pair<Position,Position> crossCorners() const noexcept {
            return std::make_pair(Position{x+w,y}, Position{x,y+h});
        }

        /// Get the Positions of all four corners defined by the Rectangle, top to bottom, left to right.
        [[nodiscard]] std::tuple<Position,Position,Position,Position> corners() const noexcept {
            auto [tl,br] = primeCorners();
            auto [bl,tr] = crossCorners();
            return std::make_tuple(tl,tr,bl,br);
        }

        /// Determine if a Rectangle contains a Position.
        [[nodiscard]] constexpr bool contains(Position pos) const noexcept {
            return pos.x >= x && pos.x < x + w && pos.y >= y && pos.y < y + h;
        }

        /**
         * @brief Determine if there is no overlap between two Rectangle objects.
         * @param o The other Rectangle.
         * @return True if there is no overlap of this Rectangle and the other Rectangle.
         */
        [[nodiscard]] constexpr bool noOverlap(const Rectangle& o) const noexcept {
            return x > o.x + o.w || o.x > x + w || y > o.y + o.h || o.y > y + h;
        }

        /**
         * @brief Determine if there is overlap between to Rectangle objects by inverting noOverlap().
         * @param o The other Rectangle.
         * @return True if there is overlap of this Rectangle and the other Rectangle.
         */
        [[nodiscard]] constexpr bool overlap(const Rectangle& o) const noexcept {
            return !noOverlap(o);
        }

        [[nodiscard]] Rectangle intersection(const Rectangle &o) const {
            // gives bottom-left point
            // of intersection rectangle

            auto x5 = std::max(x, o.x);
            auto y5 = std::max(y, o.y);

            // gives top-right point
            // of intersection rectangle
            auto x6 = std::min(x+w, o.x+o.w);
            auto y6 = std::min(y+h, o.y+o.h);

            // no intersection
            if (x5 > x6 || y5 > y6) {
                return Rectangle{0,0,0,0};
            }

            return Rectangle{x5, y5, x6 - x5, y6 - y5};
        }

        static Rectangle Zero;
    };

}

inline std::ostream& operator<<(std::ostream& strm, const rose::Size &size) {
    strm << '(' << size.w << ',' << size.h << ')';
    return strm;
}

inline std::ostream& operator<<(std::ostream& strm, const rose::Position &pos) {
    strm << '(' << pos.x << ',' << pos.y << ')';
    return strm;
}

inline std::ostream& operator<<(std::ostream& strm, const rose::Rectangle &rec) {
    strm << '(' << rec.x << ',' << rec.y << ',' << rec.w << ',' << rec.h << ')';
    return strm;
}