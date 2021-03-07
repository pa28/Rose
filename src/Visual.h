/**
 * @file Visual.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 * @brief User Interface Visual types.
 */

#pragma once

#include <iostream>
#include <utility>
#include "StructuredTypes.h"
#include "Types.h"

namespace rose {

    class Widget;
    class Window;
    class Manager;
    class Widget;

    using FocusTree = std::pair<std::vector<std::shared_ptr<Manager>>,std::shared_ptr<Widget>>;

    /**
     * @struct SemanticGesture
     * @brief The type of semantic gesture supported by a Widget.
     */
    struct SemanticGesture {
        uint32_t value;
        static SemanticGesture None;
        static SemanticGesture Click;
        static SemanticGesture Scroll;
        static SemanticGesture Multi;

        [[nodiscard]] bool supports(SemanticGesture semanticGesture) const {
            return (value & semanticGesture.value) == semanticGesture.value;
        }
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
            mScreenRect = getScreenRectangle(containerPosition);
        }

        [[nodiscard]] Rectangle getScreenRectangle(const Position &containerPosition) const {
            return Rectangle{containerPosition + mPos, mSize};
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
    class Screen : public Visual, public Container {
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
    class Window : public Visual, public Container {
        bool mModalWindow{};

    public:
        Window() = default;
        ~Window() override = default;

        bool isModal() const { return mModalWindow; }

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

        std::optional<FocusTree> focusTree(Position mousePosition);
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

        void focusTree(const Position &containerPosition, const Position &mousePosition, FocusTree &result);
    };

    class Widget : public Visual, Node {
    protected:
        SemanticGesture mSemanticGesture{};

    public:
        Widget() = default;
        ~Widget() override = default;

        Rectangle layout(const Rectangle &screenRect) override {
            mPos = mPreferredPos;
            mSize = mPreferredSize;
            return Rectangle{mPos, mSize};
        }

        void draw(const Position &containerPosition) override;

        SemanticGesture supportedSemanticGestures() const { return mSemanticGesture; }
    };
}
