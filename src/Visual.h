/**
 * @file Visual.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 * @brief User Interface Visual types.
 */

#pragma once

#include <utility>
#include "StructuredTypes.h"

namespace rose {

    class Widget;
    class Window;
    class Manager;
    class Widget;

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

    /**
     * @class Visual
     * @brief The properties common to all visual objects on the screen.
     * @details Preferred values may be set by the user application or specific widgets prior to layout. They
     * may or may not be honoured.
     */
    class Visual {
    protected:
        Position mPos{};            ///< Position relative to the container, arrived at by layout.
        Size mSize{};               ///< The size on screen, arrived at by layout.
        Position mPreferredPos{};   ///< The preferred position.
        Size mPreferredSize{};      ///< The preferred size.
        Rectangle mScreenRect{};    ///< The screen Rectangle computed at drawomg time.

    public:
        /**
         * @brief Compute the screen rectangle from the Container screen Position.
         * @param containerPosition The Container screen Position.
         */
        void setScreenRectangle(const Position &containerPosition) {
            mScreenRect = Rectangle{containerPosition + mPos, mSize};
        }

        /// Draw the visual.
        virtual void draw(const Position &containerPosition) = 0;

        /// Layout the visual.
        virtual Rectangle layout(const Rectangle &screenRect) = 0;
    };

    /**
     * @class Screen
     * @brief An abstraction of the available display screen.
     */
    class Screen : public Visual, Container {
    public:
        Screen() = default;
        ~Screen() override = default;

        /**
         * @brief Add a Window to the screen.
         * @param node The Window to add.
         */
        void add(const std::shared_ptr<Node> &node) override {
            if (auto window = std::dynamic_pointer_cast<Window>(node); window)
                Container::add(node);
            else
                throw NodeTypeError("A Screen may only contain Window objects.");
        }

        /// Draw the screen contents.
        void draw(const Position &containerPosition) override;

        /// Layout the screen contents.
        Rectangle layout(const Rectangle &screenRect) override;
    };

    /**
     * @class Window
     * @brief A Window is a visual abstraction of a number of related user interface objects.
     */
    class Window : public Visual, Container {
    public:
        Window() = default;
        ~Window() override = default;

        /**
         * @brief Add a Manager to the Window.
         * @details Normally only one manager is required for a window, but more than one could be used.
         * @param node The manager.
         */
        void add(const std::shared_ptr<Node> &node) override {
            if (std::dynamic_pointer_cast<Widget>(node))
                Container::add(node);
            else
                throw NodeTypeError("A Window may only contain Manager objects.");
        }

        /// Draw the contents of the Window
        void draw(const Position &containerPosition) override;

        /// Layout the contents of teh Window
        Rectangle layout(const Rectangle &screenRect) override;
    };

    /**
     * @class LayoutManager
     * @brief A pure virtual base class for layout managers.
     */
    class LayoutManager {
    protected:

    public:
        LayoutManager() = default;
        virtual ~LayoutManager() = default;
        using Itr = Container::iterator;

        /// Layout the contents of the associated manager.
        virtual Rectangle layoutContent(const Rectangle &screenRect, Itr first, Itr last) = 0;
    };

    class SimpleLayout : public LayoutManager {
    public:
        SimpleLayout() = default;
        ~SimpleLayout() override = default;

        Rectangle layoutContent(const Rectangle &screenRect, Itr first, Itr last) override;
    };

    class Manager : public Visual, Container {
    protected:
        std::unique_ptr<LayoutManager> mLayoutManager{};

    public:
        Manager() = default;
        ~Manager() override = default;

        void add(const std::shared_ptr<Node> &node) override {
            if (std::dynamic_pointer_cast<Manager>(node) || std::dynamic_pointer_cast<Manager>(node))
                Container::add(node);
            else
                throw NodeTypeError("A Manager may only contain Manager or Widget objects.");
        }

        void draw(const Position &containerPosition) override;

        Rectangle layout(const Rectangle &screenRect) override;
    };

    class Widget : public Visual, Node {
    public:
        Widget() = default;
        ~Widget() override = default;

        Rectangle layout(const Rectangle &screenRect) override {
            mPos = mPreferredPos;
            mSize = mPreferredSize;
            return Rectangle{mPos, mSize};
        }
    };
}

