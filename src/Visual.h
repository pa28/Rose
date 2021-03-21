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
#include <limits>
#include "Callbacks.h"
#include "StructuredTypes.h"
#include "Types.h"
#include "Utilities.h"

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

    /**
     * @struct SemanticGesture
     * @brief The type of semantic gesture supported by a Widget.
     */
    struct SemanticGesture {
        uint32_t value;
        static SemanticGesture None;
        static SemanticGesture Click;
        static SemanticGesture Drag;
        static SemanticGesture Scroll;
        static SemanticGesture Key;
        static SemanticGesture Multi;

        [[nodiscard]] bool supports(SemanticGesture semanticGesture) const {
            return (value & semanticGesture.value) != 0;
        }

        SemanticGesture operator|(const SemanticGesture& other) {
            SemanticGesture result{};
            result.value = value | other.value;
            return result;
        }
    };

    class LayoutHint {
    public:
        enum Attachment : int {
            None,           ///< No attachment.
            TopLeft,        ///< Attach to the top left corner of the container.
            TopRight,       ///< Attach to the top right corner of the container.
            BottomLeft,     ///< Attach to the bottom left corner of the container.
            BottomRight,    ///< Attach to the bottom right corner of the container.
            Top,            ///< Attach top to the top of the container.
            Left,           ///< Attach left to the left of the container.
            TopTo,          ///< Attach top to the bottom of indexed object.
            LeftTo,         ///< Attach left to the right of indexed object.
            Bottom,         ///< Attach bottom to the bottom of the container.
            Right,          ///< Attach right to the right of the container.
            BottomTo,       ///< Attach bottom to the top of indexed object.
            RightTo,        ///< Attach right to the left of indexed object.
            TopWith,        ///< Set top even with top of indexed object.
            LeftWith,       ///< Set left even with left of index object.
            BottomWith,     ///< Set bottom even with bottom of indexed object.
            RightWith,      ///< Set right even with right of indexed object.
        };

        static constexpr size_t RefIndexNone = std::numeric_limits<size_t>::max();

    protected:
        Attachment mAttachment{None};
        size_t mRefIndex{RefIndexNone};
        Id mRefId{};

    public:
        LayoutHint() = default;

        virtual ~LayoutHint() = default;

        explicit LayoutHint(Attachment attachment) {
            mAttachment = attachment;
        }

        LayoutHint(Attachment attachment, size_t refIndex) {
            mAttachment = attachment;
            mRefIndex = refIndex;
        }

        LayoutHint(Attachment attachment, Id refId) {
            mAttachment = attachment;
            mRefId = refId;
        }

        [[nodiscard]] Attachment attachment() const {
            return mAttachment;
        }

        [[nodiscard]] size_t refIndex() const {
            return mRefIndex;
        }

        bool operator<(const LayoutHint& other) const noexcept {
            return static_cast<int>(mAttachment) < static_cast<int>(other.mAttachment);
        }
    };

    /**
     * @class Visual
     * @brief The properties common to all visual objects on the screen.
     * @details Preferred values may be set by the user application or specific widgets prior to layout. They
     * may or may not be honoured.
     */
    class Visual {
        friend class LayoutManager;

    protected:
        SemanticGesture mSemanticGesture{};
        Position mPos{};            ///< Position relative to the container, arrived at by layout.
        Size mSize{};               ///< The size on screen, arrived at by layout.
        Position mPreferredPos{};   ///< The preferred position.
        Size mPreferredSize{};      ///< The preferred size.
        Rectangle mScreenRect{};    ///< The screen Rectangle computed at drawing time.
        Padding mPadding{};         ///< Immediately around the Visual, used for separation and alignment.
        Id mId{};                   ///< The object Id string.
        State mState{};             ///< The object state Id string.
        bool mVisible{true};        ///< If true the object is visible.

        std::vector<LayoutHint> mLayoutHints{};     ///< A list of LayoutHints for the LayoutManager.

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

        /// Pad the drawing location.
        Position drawPadding(const Position &containerPosition) {
            return containerPosition + mPadding.position();
        }

        /// Pad the layout.
        Rectangle layoutPadding(const Rectangle &screenRect) {
            return Rectangle{screenRect.position(), screenRect.size() + mPadding.size()};
        }

        /// Set preferred Size.
        void setSize(const Size& size) {
            mPreferredSize = size;
        }

        [[nodiscard]] Size getSize() const {
            return mPreferredSize;
        }

        /// Set preferred Position.
        void setPosition(const Position& position) {
            mPreferredPos = position;
        }

        [[nodiscard]] Position getPosition() const {
            return mPreferredPos;
        }

        /// Set Padding.
        void setPadding(const Padding &padding) {
            mPadding = padding;
        }

        /// Set Screen Rectangle
        void setScreenRectangle(const Rectangle &screenRect) {
            mPos = screenRect.position();
            mSize = screenRect.size();
        }

        /// Check visibility.
        [[nodiscard]] constexpr bool isVisible() const noexcept {
            return mVisible;
        }

        /// Set visibility.
        void setVisible(bool visible) noexcept {
            mVisible = visible;
        }

        /// Set Id
        void setId(const Id& id) noexcept {
            mId = id;
        }

        /// Add a LayoutHint
        void addLayoutHint(const LayoutHint &hint) {
            mLayoutHints.push_back(hint);
        }

        /// Get supported SemanticGestures.
        [[nodiscard]] SemanticGesture supportedSemanticGestures() const { return mSemanticGesture; }

    };

    class Application;

    /**
     * @class Screen
     * @brief An abstraction of the available display screen.
     */
    class Screen : public Visual, public Container {
        Application& mApplication;

    public:
        explicit Screen(Application &application);
        ~Screen() override = default;

        Screen(const Screen&) = delete;
        Screen(Screen &&) = delete;
        Screen& operator=(const Screen&) = delete;
        Screen& operator=(Screen &&) = delete;

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

        Application& getApplication() {
            return mApplication;
        }

        const Application& getApplication() const {
            return mApplication;
        }
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
            if (empty()) {
                if (std::dynamic_pointer_cast<Manager>(node))
                    Container::add(node);
                else
                    throw NodeTypeError("A Window may only contain one Manager object.");
            } else {
                throw NodeRangeError("A Window may only contain one Manager object.");
            }
        }

        /// Draw the contents of the Window
        void draw(gm::Context &context, const Position &containerPosition) override;

        /// Layout the contents of the Window
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        std::shared_ptr<Widget> focusWidget(SemanticGesture gesture, Position position, Position containerPosition);

        std::shared_ptr<Widget> pointerWidget(Position position);

        std::shared_ptr<Screen> getScreen() {
            if (auto screen = std::dynamic_pointer_cast<Screen>(container()))
                return screen;
            return nullptr;
        }

        std::shared_ptr<Screen> getScreen() const {
            if (auto screen = std::dynamic_pointer_cast<Screen>(container()))
                return screen;
            return nullptr;
        }
    };

    /**
     * @class LayoutManager
     * @brief A pure virtual base class for layout managers.
     */
    class LayoutManager {
    protected:
        size_t mMaxContent{0};

        static std::vector<LayoutHint>& getLayoutHints(std::shared_ptr<Visual> &visual) {
            return visual->mLayoutHints;
        }

        static Rectangle getScreenRectangle(std::shared_ptr<Visual> &visual) {
            return Rectangle{visual->mPos, visual->mSize};
        }

    public:
        LayoutManager() = default;
        virtual ~LayoutManager() = default;
        using Itr = Container::iterator;

        /// Layout the contents of the associated manager.
        virtual Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) = 0;

        [[nodiscard]] size_t maximumContent() const { return mMaxContent; }
    };

    template<class Layout, typename ... Args>
    std::unique_ptr<Layout> makeLayout(Args ... args) {
        static_assert(std::is_base_of_v<LayoutManager, Layout>, "Layout must be derived from LayoutManager.");
        return std::make_unique<Layout>(args ...);
    }

    class SimpleLayout : public LayoutManager {
    public:
        SimpleLayout() = default;
        ~SimpleLayout() override = default;

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };

    class LayoutManagerError : public std::runtime_error {
    public:
        LayoutManagerError() = delete;
        explicit LayoutManagerError(const std::string &what) : std::runtime_error(what) {}
        explicit LayoutManagerError(const char *what) : std::runtime_error(what) {}
        LayoutManagerError(const LayoutManagerError& other) = default;
    };

    class Widget : public Visual, public Container {
    public:

    protected:
        EventCallback mEnterEventCallback{};
        EventCallback mLeaveEventCallback{};
        ButtonCallback mButtonEventCallback{};
        MouseMotionCallback mMouseMotionCallback{};
        ScrollCallback mMouseScrollCallback{};
        KeyboardShortcutCallback mKeyboardShortcutCallback{};

    public:
        Widget() = default;

        ~Widget() override = default;

        Widget(const Widget &) = delete;

        Widget(Widget &&) = delete;

        Widget &operator=(const Widget &) = delete;

        Widget &operator=(Widget &&) = delete;

//        std::shared_ptr<Widget> focusWidget(SemanticGesture gesture, Position position, Position containerPosition);

        std::shared_ptr<Widget> pointerWidget(Position position, Position containerPosition);

        /**
         * @brief Recursively compute the Widget Position on the Screen.
         * @return The computed Position.
         */
        Position computeScreenPosition();

        /**
         * @brief Determine if a given Screen Position is within the Widget Rectangle.
         * @param position The Screen Position.
         * @return True if the Position is within the Rectangle.
         */
        bool contains(const Position &position);

//        void clearFocus(const SemanticGesture &gesture) {}

        /**
         * @brief Get a reference to the Application object.
         * @return The reference.
         */
        Application& getApplication() {
            if (auto window = getWindow(); window) {
                if (auto screen = window->getScreen(); screen)
                    return screen->getApplication();
            }
            throw std::runtime_error("No path from Widget to Screen to Application.");
        }

        /// const version of getApplication()
        Application& getApplication() const {
            if (auto window = getWindow(); window) {
                if (auto screen = window->getScreen(); screen)
                    return screen->getApplication();
            }
            throw std::runtime_error("No path from Widget to Screen to Application.");
        }

        /**
         * @brief Get the Window ancestor of a Widget.
         * @return The Window.
         */
        std::shared_ptr<Window> getWindow() {
            auto c = container();
            while (c) {
                if (auto window = std::dynamic_pointer_cast<Window>(c); window)
                    return window;
                c = c->container();
            }

            return nullptr;
        }

        /// const version of getWindow().
        std::shared_ptr<Window> getWindow() const {
            auto c = container();
            while (c) {
                if (auto window = std::dynamic_pointer_cast<Window>(c); window)
                    return window;
                c = c->container();
            }

            return nullptr;
        }

        /**
         * @brief Notify Widget of mouse pointer enter event.
         * @return True if event is consumed.
         */
        bool enterEvent() {
            std::cout << __PRETTY_FUNCTION__ << '\n';
            if (mEnterEventCallback)
                return mEnterEventCallback();
            return false;
        }

        /// Set the enter callback
        void setEnterLeaveEventCallback(EventCallback eventCallback) {
            mEnterEventCallback = std::move(eventCallback);
        }

        /**
         * @brief Notify Widget of mouse pointer leave event.
         * @return True if event is consumed.
         */
        bool leaveEvent() {
            std::cout << __PRETTY_FUNCTION__ << '\n';
            if (mLeaveEventCallback)
                return mLeaveEventCallback();
            return false;
        }

        /// Set the leave callback
        void setLeaveEventCallback(EventCallback eventCallback) {
            mLeaveEventCallback = std::move(eventCallback);
        }

        /**
         * @brief Notify Widget of mouse button events.
         * @param pressed True if a button is pressed.
         * @param button The button that changed state.
         * @param clicks The number of clicks.
         * @return True if the event is consumed.
         */
        bool buttonEvent(bool pressed, uint button, uint clicks) {
            std::cout << __PRETTY_FUNCTION__ << ' ' << pressed << ' ' << button << ' ' << clicks << '\n';
            if (mButtonEventCallback)
                return mButtonEventCallback(pressed, button, clicks);
            return false;
        }

        /// Set the button callback
        void setButtonEventCallback(ButtonCallback buttonCallback) {
            mButtonEventCallback = std::move(buttonCallback);
        }

        /**
         * @brief Notify Widget of mouse motion events.
         * @details If this Widget does not consume the event, and any button is pressed, it is passed up the tree.
         * This allows a Manager above the pointer focused Widget to receive drag events.
         * @param pressed True if a button is pressed.
         * @param button A bitwise or of pressed buttons.
         * @param mousePos The screen position of the mouse.
         * @param relativePos The relative motion of the mouse.
         * @return True if the event is consumed.
         */
        bool mouseMotionEvent(bool pressed, uint button, Position mousePos, Position relativePos, bool passed);

        /// Set the mouse motion callback
        void setMouseMotionEventCallback(MouseMotionCallback mouseMotionCallback) {
            mMouseMotionCallback = std::move(mouseMotionCallback);
        }

        /**
         * @brief Notify Widget of mouse scroll wheel events.
         * @details If this Widget does not consume the event it is passed up the tree.
         * @param deltaPos The delta position of the scroll wheel motion.
         * @return True if the event is consumed.
         */
        bool mouseScrollEvent(Position deltaPos, bool passed);

        /// Set the mouse scroll callback
        void setMouseScrollEventCallback(ScrollCallback scrollCallback) {
            mMouseScrollCallback = std::move(scrollCallback);
        }

        /**
         * @brief Notify the Widget of a keyboard shortcut invocation.
         * @param keycode The keycode registered to the Widget.
         */
        bool keyboardShortcutEvent(SDL_Keycode keycode, bool state, uint repeat);

        /// Set the keyboard shortcut callback
        void setKeyboardShortcutCallback(KeyboardShortcutCallback keyboardShortcutCallback) {
            mKeyboardShortcutCallback = std::move(keyboardShortcutCallback);
        }
    };

    class Manager : public Widget {
    protected:
        std::unique_ptr<LayoutManager> mLayoutManager{};

    public:
        Manager();

        ~Manager() override = default;

        void add(const std::shared_ptr<Node> &node) override {
            if (mLayoutManager) {
                if (mLayoutManager->maximumContent() == 0 || size() < mLayoutManager->maximumContent()) {
                    if (std::dynamic_pointer_cast<Widget>(node) || std::dynamic_pointer_cast<Manager>(node))
                        Container::add(node);
                    else
                        throw NodeTypeError("A Manager may only contain Manager or Widget objects.");
                } else {
                    throw NodeRangeError(StringCompositor("Contents exceed maximum limit: ",
                                                          mLayoutManager->maximumContent()));
                }
            } else {
                throw LayoutManagerError("Can not add content without a LayoutManager.");
            }
        }

        void draw(gm::Context &context, const Position &containerPosition) override;

        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        void setLayoutManager(std::unique_ptr<LayoutManager> &&layoutManager) {
            mLayoutManager = std::move(layoutManager);
        }

        std::unique_ptr<LayoutManager>& layoutManager() { return mLayoutManager; }
    };

    struct Parent {};

    static Parent endw{};

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

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Size& size) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
            "WidgetClass must be derived from rose::Widget or rose::Manager.");
    widget->setSize(size);
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Position& position) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
                  "WidgetClass must be derived from rose::Widget or rose::Manager.");
    widget->setPosition(position);
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, std::unique_ptr<rose::LayoutManager>&& layout) {
    static_assert(std::is_base_of_v<rose::Manager,WidgetClass>, "WidgetClass must be derived from rose::Manager.");
    widget->setLayoutManager(std::move(layout));
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Padding& padding) {
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass>, "WidgetClass must be derived from rose::Visual.");
    widget->setPadding(padding);
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Id& id) {
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass>, "WidgetClass must be derived from rose::Visual.");
    widget->setId(id);
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::LayoutHint& hint) {
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass>, "WidgetClass must be derived from rose::Visual.");
    widget->addLayoutHint(hint);
    return widget;
}
