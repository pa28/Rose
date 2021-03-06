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
#include "Color.h"
#include "Math.h"

namespace rose {

    template<typename I = int>
    constexpr I WINDOWPOS_UNDEFINED_DISPLAY(uint display=0) {
        return static_cast<I>(SDL_WINDOWPOS_UNDEFINED_MASK | display);
    }

    template<typename I = int>
    constexpr I WINDOWPOS_CENTERED_DISPLAY(uint display=0) {
        return static_cast<I>(SDL_WINDOWPOS_CENTERED_MASK | display);
    }

    static constexpr int WINDOWPOS_UNDEFINED = WINDOWPOS_UNDEFINED_DISPLAY();
    static constexpr int WINDOWPOS_CENTERED = WINDOWPOS_CENTERED_DISPLAY();

    /**
     *  \brief Flags used when creating a rendering context
     */
    enum RendererFlags : uint32_t {
        RENDERER_SOFTWARE = static_cast<uint32_t>(SDL_RENDERER_SOFTWARE),         /**< The renderer is a software fallback */
        RENDERER_ACCELERATED = static_cast<uint32_t>(SDL_RENDERER_ACCELERATED),   /**< The renderer uses hardware acceleration */
        RENDERER_PRESENTVSYNC = static_cast<uint32_t>(SDL_RENDERER_PRESENTVSYNC), /**< Present is synchronized with the refresh rate */
        RENDERER_TARGETTEXTURE = static_cast<uint32_t>(SDL_RENDERER_TARGETTEXTURE) /**< The renderer supports rendering to texture */
    };

    enum class Orientation {
        Horizontal,
        Vertical,
    };

    /**
     * @enum BorderStyle
     * @brief The types of border supported.
     */
    enum class BorderStyle {
        Unset,      ///< Not set to a valid value.
        None,       ///< No border
        BevelOut,   ///< A beveled out border
        BevelIn,    ///< A beveled in border
        NotchOut,   ///< A notch out border
        NotchIn,    ///< A notch in border
    };

    /**
     * @enum CornerStyle
     * @brief Types of corners supported.
     */
    enum class CornerStyle {
        Unset,      ///< Not set to a valid value.
        Square,     ///< Square corners
        Round,      ///< Round corners.
    };

    namespace set {
        static constexpr std::string_view SetAppSize{"SetAppSize"};
        static constexpr std::string_view SetAppPosition{"SetAppPosition"};
        static constexpr std::string_view SetAppState{"SetAppState"};
    }

    struct FrameSettings {
        std::pair<color::RGBA,color::RGBA>  backgroundPair;
        std::pair<BorderStyle,BorderStyle>  borderStylePair;

        color::RGBA background(bool invert) {
            return invert ? backgroundPair.second : backgroundPair.first;
        }

        BorderStyle borderStyle(bool invert) {
            return invert ? borderStylePair.second : borderStylePair.first;
        }
    };

    /**
     * @struct Position
     * @brief A position in integer (x, y) co-ordinates.
     * @details For screen objects (0, 0) is the top left corner, x increases to the right, y increases toward
     * the bottom.
     */
    template <typename T>
    struct Position {
        T x{0}, y{0};

        constexpr Position() noexcept = default;

        template<typename O>
        constexpr Position(O X, O Y) noexcept {
            if constexpr (std::is_same_v<T,O>) {
                x = X;
                y = Y;
            } else if constexpr (std::is_integral_v<T> && std::is_floating_point_v<O>) {
                x = util::roundToInt(X);
                y = util::roundToInt(Y);
            } else {
                x = static_cast<T>(X);
                y = static_cast<T>(Y);
            }
        }

        constexpr Position(const Position &p) = default;
        constexpr Position(Position &&p)  noexcept = default;
        constexpr Position& operator=(const Position &p) = default;
        constexpr Position& operator=(Position &&p) = default;

        constexpr explicit Position(T p) noexcept : x(p), y(p) {}

        /// Add two positions together.
        template <typename O>
        constexpr Position operator+(const Position<O> &p) const noexcept {
            if constexpr (std::is_same_v<T,O>) {
                return Position{x + p.x, y + p.y};
            } else if constexpr (std::is_integral_v<T> && std::is_floating_point_v<O>) {
                return Position{x + util::roundToInt(p.x), y + util::roundToInt(p.y)};
            } else {
                return Position{x + static_cast<T>(p.x), y + static_cast<T>(p.y)};
            }
        }

        /// Subtract position other from this.
        template <typename O>
        constexpr Position operator-(const Position<O> &p) const noexcept {
            if constexpr (std::is_same_v<T,O>) {
                return Position{x - p.x, y - p.y};
            } else if constexpr (std::is_integral_v<T> && std::is_floating_point_v<O>) {
                return Position{x - util::roundToInt(p.x), y - util::roundToInt(p.y)};
            } else {
                return Position{x - static_cast<T>(p.x), y - static_cast<T>(p.y)};
            }
        }

        /**
         * @brief Return this position after converting to type O.
         * @tparam O The type of the returned values.
         * @return The converted position.
         */
        template<typename O>
        Position<O> as() const {
            if constexpr (std::is_same_v<T,O>) {
                return Position<O>{x, y};
            } else if constexpr (std::is_integral_v<O> && std::is_floating_point_v<T>) {
                return Position<O>{util::roundToInt(x), util::roundToInt(y)};
            } else {
                return Position<O>{static_cast<O>(x), static_cast<O>(y)};
            }
        }

        /// Inequality operator
        bool operator!=(const Position &other) const noexcept {
            return x != other.x || y != other.y;
        }

        /// Equality operator
        bool operator==(const Position &other) const noexcept {
            return x == other.x && y == other.y;
        }

        /// Less than or Equal to operator
        bool operator<=(const Position &other) const noexcept {
            return x <= other.x && y <= other.y;
        }

        /// Access the primary component for a given Orientation.
        T& primary(Orientation o) noexcept {
            return o == Orientation::Horizontal ? x : y;
        }

        /// Access the secondary component for the given Orientation.
        T& secondary(Orientation o) noexcept {
            return o == Orientation::Horizontal ? y : x;
        }

        /// Return the primary component for a given Orientation.
        [[nodiscard]] constexpr T primary(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? x : y;
        }

        /// Return the secondary component for the given Orientation.
        [[nodiscard]] constexpr T secondary(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? y : x;
        }

        /**
         * @brief Compute the distance squared from this position to another.
         * @param other The other position.
         * @return The distance between the two positions squared.
         */
        [[nodiscard]] constexpr T rSqr(const Position &other) const noexcept {
            auto dX = other.x - x;
            auto dY = other.y - y;
            return dX*dX + dY*dY;
        }

        /// Swap the x and y components.
        void swap() noexcept {
            std::swap(x,y);
        }

        /// Mirror Position on X axis.
        [[nodiscard]] constexpr Position<T> mirrorX() const noexcept {
            return Position<T>{-x, y};
        }

        /// Mirror Position on Y axis.
        [[nodiscard]] constexpr Position<T> mirrorY() const noexcept {
            return Position<T>{x, -y};
        }
    };

    static constexpr Position<int> UndefinedPosition;

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

        constexpr explicit Size(int size) noexcept : w(size), h(size) {}

        constexpr explicit Size(const std::tuple<int,int> &size) noexcept : Size(std::get<0>(size), std::get<1>(size)) {}

        constexpr Size& operator=(std::tuple<int,int> &size) noexcept {
            w = std::get<0>(size);
            h = std::get<0>(size);
            return *this;
        }

        explicit operator bool() const { return w > 0 && h > 0; }

        bool operator!=(const Size &other) const noexcept {
            return w != other.w || h != other.h;
        }

        bool operator==(const Size &other) const noexcept {
            return w == other.w && h == other.h;
        }

        bool operator<(const Size &other) const noexcept {
            return w < other.w && h < other.h;
        }

        bool operator<=(const Size &other) const noexcept {
            return w <= other.w && h <= other.h;
        }

        bool operator>=(const Size &other) const noexcept {
            return !(operator<(other));
        }

        Size operator+(const Size &other) const noexcept {
            return Size{w + other.w, h + other.h};
        }

        Size operator-(const Size &other) const noexcept {
            return Size{w - other.w, h - other.h};
        }

        Size operator/(const int divisor) const  {
            return Size{w / divisor, h / divisor};
        }

        int& primary(Orientation o) noexcept {
            return o == Orientation::Horizontal ? w : h;
        }

        int& secondary(Orientation o) noexcept {
            return o == Orientation::Horizontal ? h : w;
        }

        [[nodiscard]] constexpr int primary(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? w : h;
        }

        [[nodiscard]] constexpr int secondary(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? h : w;
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
        constexpr Rectangle(const Position<int> &p, const Size &s) noexcept : x(p.x), y(p.y), w(s.w), h(s.h) {}
        constexpr Rectangle(const Rectangle &p) = default;
        constexpr Rectangle(Rectangle &&p) = default;
        constexpr Rectangle& operator=(const Rectangle &p) = default;
        constexpr Rectangle& operator=(Rectangle &&p) = default;

        constexpr Rectangle& operator=(const Size &s) {
            w = s.w;
            h = s.h;
            return *this;
        }

        constexpr Rectangle& operator=(const Position<int> &p) {
            x = p.x;
            y = p.y;
            return *this;
        }

        /// Add a Position to a Rectangle.
        constexpr Rectangle operator + (const Position<int> &p) const noexcept {
            return Rectangle{x+p.x, y+p.y, w, h};
        }

        /// Add a Size to a Rectangle.
        constexpr Rectangle operator + (const Size &s) const noexcept {
            return Rectangle{ x, y, w + s.w, h + s.h};
        }

        /// Subtract a Size from a Rectangle.
        constexpr Rectangle operator - (const Size &s) const noexcept {
            return Rectangle{ x, y, w - s.w, h - s.h};
        }

        /// Get a Position from a Rectangle.
        [[nodiscard]] Position<int> position() const noexcept {
            return Position<int>{x,y};
        }

        /// Get a Size from a Rectangle
        [[nodiscard]] Size size() const noexcept {
            return Size{w,h};
        }

        /// Get the Positions of top-left and bottom-right corners defined by the Rectangle
        [[nodiscard]] std::pair<Position<int>,Position<int>> primeCorners() const noexcept {
            return std::make_pair(Position<int>{x,y}, Position<int>{x+w,y+h});
        }

        /// Get the Positions of the top-right and bottom-left corners defined by the Rectangle
        [[nodiscard]] std::pair<Position<int>,Position<int>> crossCorners() const noexcept {
            return std::make_pair(Position<int>{x+w,y}, Position<int>{x,y+h});
        }

        /// Get the Positions of all four corners defined by the Rectangle, top to bottom, left to right.
        [[nodiscard]] std::tuple<Position<int>,Position<int>,Position<int>,Position<int>> corners() const noexcept {
            auto [tl,br] = primeCorners();
            auto [bl,tr] = crossCorners();
            return std::make_tuple(tl,tr,bl,br);
        }

        /// Determine if a Rectangle contains a Position.
        [[nodiscard]] constexpr bool contains(Position<int> pos) const noexcept {
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

        int& sizePri(Orientation o) noexcept {
            return o == Orientation::Horizontal ? w : h;
        }

        int& sizeSec(Orientation o) noexcept {
            return o == Orientation::Horizontal ? h : w;
        }

        int& posPri(Orientation o) noexcept {
            return o == Orientation::Horizontal ? x : y;
        }

        int& posSec(Orientation o) noexcept {
            return o == Orientation::Horizontal ? y : x;
        }

        [[nodiscard]] constexpr int sizePri(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? w : h;
        }

        [[nodiscard]] constexpr int sizeSec(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? h : w;
        }

        [[nodiscard]] constexpr int posPri(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? x : y;
        }

        [[nodiscard]] constexpr int posSec(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? y : x;
        }

        static Rectangle Zero;
    };

    /**
     * @struct Padding
     * @brief Abstraction of space consumed around an object for space, borders, etc.
     */
    struct Padding {
        int t{0}, b{0}, l{0}, r{0};

        constexpr Padding() noexcept = default;

        constexpr explicit Padding(int p) noexcept : t(p), b(p), l(p), r(p) {}

        constexpr Padding(int h, int v) noexcept : t(v), b(v), l(h), r(h) {}

        constexpr Padding(int top, int bot, int left, int right) noexcept : t(top), b(bot), l(left), r(right) {}

        [[nodiscard]] constexpr int vertical() const noexcept { return t + b; }

        [[nodiscard]] constexpr int horizontal() const noexcept { return l + r; }

        [[nodiscard]] constexpr Position<int> position() const noexcept { return Position<int>{l, t}; }

        [[nodiscard]] constexpr Size size() const noexcept { return Size{horizontal(), vertical()}; }

        int& priLead(Orientation o) noexcept {
            return o == Orientation::Horizontal ? l : t;
        }

        int& priLag(Orientation o) noexcept {
            return o == Orientation::Horizontal ? r : b;
        }

        int& secLead(Orientation o) noexcept {
            return o == Orientation::Horizontal ? t : l;
        }

        int& secLag(Orientation o) noexcept {
            return o == Orientation::Horizontal ? b : r;
        }

        [[nodiscard]] constexpr int priLead(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? l : t;
        }

        [[nodiscard]] constexpr int priLag(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? r : b;
        }

        [[nodiscard]] constexpr int secLead(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? t : l;
        }

        [[nodiscard]] constexpr int secLag(Orientation o) const noexcept {
            return o == Orientation::Horizontal ? b : r;
        }
    };

    std::array<char, 8> utf8(int c);
}

/// Stream insertion operator for Size.
inline std::ostream& operator<<(std::ostream& strm, const rose::Size &size) {
    strm << '(' << size.w << ',' << size.h << ')';
    return strm;
}

/// Stream insertion operator for Position.
template<typename T>
inline std::ostream& operator<<(std::ostream& strm, const rose::Position<T> &pos) {
    strm << '(' << pos.x << ',' << pos.y << ')';
    return strm;
}

/// Stream insertion operator for Rectangle.
inline std::ostream& operator<<(std::ostream& strm, const rose::Rectangle &rec) {
    strm << '(' << rec.x << ',' << rec.y << ',' << rec.w << ',' << rec.h << ')';
    return strm;
}

/// Stream insertion operator for Padding.
inline std::ostream& operator<<(std::ostream& strm, const rose::Padding &pad) {
    strm << '[' << pad.t << ',' << pad.l << ',' << pad.b << ',' << pad.r << ']';
    return strm;
}
