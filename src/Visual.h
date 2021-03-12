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
#include <optional>
#include "StructuredTypes.h"
#include "Types.h"

namespace rose {

    namespace gm {
        class Context;
    }

    class Widget;
    class Window;
    class Manager;
    class Widget;

    struct Id {
        std::string_view idString;
    };

    struct State {
        std::string_view stateString;
    };

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
        Rectangle mScreenRect{};    ///< The screen Rectangle computed at drawing time.
        Id mId{};                   ///< The object Id string.
        State mState{};             ///< The object state Id string.

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
        virtual void draw(gm::Context &context, const Position &containerPosition) = 0;

        /// Layout the visual.
        virtual Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) = 0;

        /// Set preferred size.
        void set(const Size& size) { mPreferredSize = size; }

        /// Set preferred position.
        void set(const Position& position) {
            mPreferredPos = position;
        }
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
        void draw(gm::Context &context, const Position &containerPosition) override;

        /// Layout the screen contents.
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;
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
            if (std::dynamic_pointer_cast<Manager>(node))
                Container::add(node);
            else
                throw NodeTypeError("A Window may only contain Manager objects.");
        }

        /// Draw the contents of the Window
        void draw(gm::Context &context, const Position &containerPosition) override;

        /// Layout the contents of teh Window
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

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
        virtual Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) = 0;
    };

    class SimpleLayout : public LayoutManager {
    public:
        SimpleLayout() = default;
        ~SimpleLayout() override = default;

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };

    class Manager : public Visual, public Container {
    protected:
        std::unique_ptr<LayoutManager> mLayoutManager{};

    public:
        Manager() = default;
        ~Manager() override = default;

        void add(const std::shared_ptr<Node> &node) override {
            if (std::dynamic_pointer_cast<Widget>(node) || std::dynamic_pointer_cast<Manager>(node))
                Container::add(node);
            else
                throw NodeTypeError("A Manager may only contain Manager or Widget objects.");
        }

        void draw(gm::Context &context, const Position &containerPosition) override;

        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        void focusTree(const Position &containerPosition, const Position &mousePosition, FocusTree &result);

        auto container() const { return mContainer.lock(); }
    };

    class Widget : public Visual, public Node {
    protected:
        SemanticGesture mSemanticGesture{};

    public:
        Widget() = default;
        ~Widget() override = default;

        Widget(const Widget&) = delete;
        Widget(Widget &&) = delete;
        Widget& operator=(const Widget&) = delete;
        Widget& operator=(Widget &&) = delete;

        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
            mPos = mPreferredPos;
            mSize = mPreferredSize;
            return Rectangle{mPos, mSize};
        }

        void draw(gm::Context &context, const Position &containerPosition) override;

        SemanticGesture supportedSemanticGestures() const { return mSemanticGesture; }

        auto container() { return mContainer.lock(); }

        auto container() const { return mContainer.lock(); }
    };

    struct Parent {};

    template<class WidgetClass, typename ... Args>
    inline std::shared_ptr<WidgetClass> wdg(Args ... args) {
        return std::make_shared<WidgetClass>(args ...);
    }
}

template<class ContainerClass, class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<ContainerClass> container, std::shared_ptr<WidgetClass> widget) {
    static_assert(std::is_base_of_v<rose::Visual, ContainerClass> && std::is_base_of_v<rose::Container, ContainerClass>,
                  "ContainerClass must be derived from both rose::Visual and rose::Container.");
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass> && std::is_base_of_v<rose::Node, WidgetClass>,
                  "WidgetClass must be derived from both rose::visual and rose::Node.");
    container->add(widget);
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator >>(std::shared_ptr<WidgetClass> widget, std::shared_ptr<rose::Widget> &store) {
    static_assert(std::is_base_of_v<rose::Widget,WidgetClass>, "WidgetClass must be derived from rose::Widget." );
    store = widget;
    return widget;
}

template<class ManagerClass>
inline std::shared_ptr<ManagerClass> operator >>(std::shared_ptr<ManagerClass> manager, std::shared_ptr<rose::Manager> &store) {
    static_assert(std::is_base_of_v<rose::Manager,ManagerClass>, "ManagerClass must be derived from rose::Manager." );
    store = manager;
    return manager;
}

template<class WidgetClass>
inline std::shared_ptr<rose::Manager> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Parent &) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
                  "WidgetClass must be derived from rose::Widget or rose::Manager.");
    return std::dynamic_pointer_cast<rose::Manager>(widget->container());
}

template<typename WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Size& size) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
            "WidgetClass must be derived from rose::Widget or rose::Manager.");
    widget->set(size);
    return widget;
}

template<typename WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Position& position) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
                  "WidgetClass must be derived from rose::Widget or rose::Manager.");
    widget->set(position);
    return widget;
}
