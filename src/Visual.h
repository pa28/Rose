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
#include "Texture.h"

namespace rose {

    namespace gm {
        class Context;
    }

    class Widget;
    class Window;
    class Manager;
    class Widget;

    /**
     * @brief A type to specify a state.
     */
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

        SemanticGesture operator|(const SemanticGesture& other) const {
            SemanticGesture result{};
            result.value = value | other.value;
            return result;
        }
    };

    class LayoutHint {
        friend class Visual;

    public:
        static constexpr size_t AttachmentHint = 0;
        static constexpr size_t GridHint = 1;

        enum class GridLayoutHint : int {
            EndStride,
            AxisSize,
            AxisOffset,
        };

        enum class Attachment : int {
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

        static constexpr int RefIndexNone = std::numeric_limits<int>::max();

    protected:
        size_t mHintClass;
        int mValueType;
        int mValue;

    public:
        LayoutHint() = delete;

        LayoutHint(Attachment attachment, int index) {
            mHintClass = AttachmentHint;
            mValueType = static_cast<int>(attachment);
            mValue = index;
        }

        LayoutHint(GridLayoutHint hint, int value) {
            mHintClass = GridHint;
            mValueType = static_cast<int>(hint);
            mValue = value;
        }
    };

    /**
     * @class Visual
     * @brief The properties common to all visual objects on the screen.
     * @details Preferred values may be set by the user application or specific widgets prior to layout. They
     * may or may not be honoured.
     */
    class Visual {
    public:
        friend class LayoutManager;

        using ValueMap = std::map<int, int>;        ///< The std::map type for each hint class
        std::map<size_t, ValueMap> mHintsMap{};     ///< The std::map of hint class to ValueMap

    protected:
        SemanticGesture mSemanticGesture{};
        Position mPos{};            ///< Position relative to the container, arrived at by layout.
        Size mSize{};               ///< The size on screen, arrived at by layout.
        Position mPreferredPos{};   ///< The preferred position.
        Size mPreferredSize{};      ///< The preferred size.
        Rectangle mScreenRect{};    ///< The screen Rectangle computed at drawing time.
        Padding mPadding{};         ///< Immediately around the Visual, used for separation and alignment.
        State mState{};             ///< The object state Id string.
        bool mVisible{true};        ///< If true the object is visible.

    public:
        /**
         * @brief Compute the screen rectangle from the Container screen Position.
         * @param containerPosition The Container screen Position.
         */
        void setScreenRectangle(const Position &containerPosition) {
            mScreenRect = getScreenRectangle(containerPosition);
        }

        /**
         * @brief Get the rectangle occupied by the Visual.
         * @param containerPosition The position of the container holding the Visual.
         * @return The rectangle.
         */
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

        /// Get the preferred size.
        [[nodiscard]] Size getSize() const {
            return mPreferredSize;
        }

        /// Set preferred Position.
        void setPosition(const Position& position) {
            mPreferredPos = position;
        }

        /// Get the preferred Position.
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

        /// Add a LayoutHint
        void setLayoutHint(const LayoutHint &hint) {
            mHintsMap[hint.mHintClass][hint.mValueType] = hint.mValue;
        }

        /**
         * @brief Get an AttachmentHint value given a LayoutHint::Attachment.
         * @param attachment A value from LayoutHint::Attachment.
         * @return A std::optional with the value if found.
         */
        std::optional<int> getAttachmentHint(LayoutHint::Attachment attachment) {
            if (auto map = mHintsMap.find(LayoutHint::AttachmentHint); map != mHintsMap.end()) {
                if (auto att = (*map).second.find(static_cast<int>(attachment)); att != (*map).second.end()) {
                    return (*att).second;
                }
            }
            return std::nullopt;
        }

        /**
         * @brief Get a GridLayoutHint value given a LayoutHint::GridLayoutHint.
         * @param attachment A value from LayoutHint::GridLayoutHint.
         * @return A std::optional with the value if found.
         */
        std::optional<int> getGridHint(LayoutHint::GridLayoutHint gridLayoutHint) {
            if (auto map = mHintsMap.find(LayoutHint::GridHint); map != mHintsMap.end()) {
                if (auto gridHint = (*map).second.find(static_cast<int>(gridLayoutHint)); gridHint != (*map).second.end()) {
                    return (*gridHint).second;
                }
            }
            return std::nullopt;
        }

        template<size_t HintClass>
        [[nodiscard]] std::optional<const ValueMap> getHintMap() const {
            static_assert(HintClass == LayoutHint::AttachmentHint || HintClass == LayoutHint::GridHint,
                    "Supported maps are: LayoutHint::AttachmentHint, LayoutHint::GridHint.");
            if (auto map = mHintsMap.find(HintClass); map != mHintsMap.end()) {
                return std::make_optional<const ValueMap>((*map).second);
            }
            return std::nullopt;
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

        static constexpr std::string_view id = "Screen";
        std::string_view nodeId() const noexcept override {
            return id;
        }

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

        /// Get the Application object associated with the Screen.
        Application& getApplication() {
            return mApplication;
        }

        /// Get the Application associated with a const Screen.
        const Application& getApplication() const {
            return mApplication;
        }
    };

    /**
     * @class Window
     * @brief A Window is a visual abstraction of a number of related user interface objects.
     */
    class Window : public Visual, public Container {
    protected:
        bool mModalWindow{};
        gm::Texture mBaseTexture{};     ///< The base texture which animations draw over.

    public:
        ~Window() override = default;

        static constexpr std::string_view id = "Window";
        std::string_view nodeId() const noexcept override {
            return id;
        }

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

        bool baseTextureNeeded(const Position &containerPosition) {
            setScreenRectangle(containerPosition);
            return !mBaseTexture || mBaseTexture.getSize() != mScreenRect.size();
        }
        /**
         * @brief Create a Texture for the Window that can be drawn over by animations.
         * @param context The gm::Context to use.
         * @param containerPosition The container position.
         */
        void generateBaseTexture(gm::Context &context, const Position &containerPosition);

        /**
         * @brief Draw the base texture for the window.
         * @param context The gm::Context to use.
         * @param containerPosition The container position.
         */
        void drawBaseTexture(gm::Context &context, const Position &containerPosition);

        /// Layout the contents of the Window
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /// The the Widget which contains the Position.
        std::shared_ptr<Widget> pointerWidget(Position position);

        /// Get the Screen which supports the Window.
        std::shared_ptr<Screen> getScreen() {
            if (auto screen = std::dynamic_pointer_cast<Screen>(container()))
                return screen;
            return nullptr;
        }

        /// Get the Screen which support the const Window.
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
        size_t mMaxContent{UnlimitedContent};   //The maximum number of objects the LayoutManager supports.

        /// Get the screen rectangle of a Visual.
        static Rectangle getScreenRectangle(std::shared_ptr<Visual> &visual) {
            return Rectangle{visual->mPos, visual->mSize};
        }

    public:
        /// The value for 'unlimited' managed objects.
        static constexpr size_t UnlimitedContent = std::numeric_limits<size_t>::max();

        virtual ~LayoutManager() = default;
        using Itr = Container::iterator;

        /// Layout the contents of the associated manager.
        virtual Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) = 0;

        /// Get the maximum content of the LayoutManager.
        [[nodiscard]] size_t maximumContent() const { return mMaxContent; }
    };

    template<class Layout, typename ... Args>
    std::unique_ptr<Layout> makeLayout(Args ... args) {
        static_assert(std::is_base_of_v<LayoutManager, Layout>, "Layout must be derived from LayoutManager.");
        return std::make_unique<Layout>(args ...);
    }

    /**
     * @class SimpleLayout.
     * @brief A Simple layout manager.
     */
    class SimpleLayout : public LayoutManager {
    public:
        ~SimpleLayout() override = default;

        /// Layout the content.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };

    /**
     * @class LayoutManagerError
     * @brief An exception to indicate runtime error during layout.
     */
    class LayoutManagerError : public std::runtime_error {
    public:
        LayoutManagerError() = delete;
        explicit LayoutManagerError(const std::string &what) : std::runtime_error(what) {}
        explicit LayoutManagerError(const char *what) : std::runtime_error(what) {}
        LayoutManagerError(const LayoutManagerError& other) = default;
    };

    /**
     * @class Widget
     * @brief An element of the application user interface.
     */
    class Widget : public Visual, public Container {
    protected:
        /// The pointer has entered the Widget.
        EventCallback mEnterEventCallback{};

        /// The pointer has left the Widget.
        EventCallback mLeaveEventCallback{};

        /// The pointer button has been pressed with the pointer inside the Widget.
        ButtonCallback mButtonEventCallback{};

        /// The pointer is in motion, inside the Widget with a button pressed.
        MouseMotionCallback mMouseMotionCallback{};

        /// The mouse scroll wheel has moved with the pointer inside the Widget.
        ScrollCallback mMouseScrollCallback{};

        /// A keyboard button event while the Widget has keyboard focus.
        KeyboardShortcutCallback mKeyboardShortcutCallback{};

    public:
        Widget() = default;

        ~Widget() override = default;

        Widget(const Widget &) = delete;

        Widget(Widget &&) = delete;

        Widget &operator=(const Widget &) = delete;

        Widget &operator=(Widget &&) = delete;

        static constexpr std::string_view id = "Widget";
        std::string_view nodeId() const noexcept override {
            return id;
        }

//        std::shared_ptr<Widget> focusWidget(SemanticGesture gesture, Position position, Position containerPosition);

        /**
         * @brief Search for the Widget within which the position lies
         * @param position The position.
         * @param containerPosition The position of the container holding the Widget.
         * @return The Widget if found, otherwise a null pointer.
         */
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
        bool buttonEvent(bool pressed, uint button, uint clicks, bool passed);

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
         * @param shortcutCode The keycode registered to the Widget.
         */
        bool keyboardShortcutEvent(uint32_t shortcutCode, bool state, uint repeat);

        /// Set the keyboard shortcut callback
        void setKeyboardShortcutCallback(KeyboardShortcutCallback keyboardShortcutCallback) {
            mKeyboardShortcutCallback = std::move(keyboardShortcutCallback);
        }
    };

    /**
     * @class Manager
     * @brief A Widget which manages contained Widgets.
     */
    class Manager : public Widget {
    protected:
        /// The layout manager which determines the presentation of contained Widgets.
        std::unique_ptr<LayoutManager> mLayoutManager{};

    public:
        Manager();

        ~Manager() override = default;

        Manager(const Manager&) = delete;

        Manager(Manager&&) = delete;

        Manager& operator=(const Manager&) = delete;

        Manager& operator=(Manager&&) = delete;

        static constexpr std::string_view id = "Manager";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /**
         * @brief Add a Node to the contents of the Manager.
         * @param node The Node added.
         */
        void add(const std::shared_ptr<Node> &node) override {
            if (mLayoutManager) {
                if (mLayoutManager->maximumContent() == LayoutManager::UnlimitedContent || size() < mLayoutManager->maximumContent()) {
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

        /**
         * @brief Draw the manager and contents.
         * @param context The graphics context used to draw the manager and contents.
         * @param containerPosition The Position of the Container that holds the Manager.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;

        /**
         * @brief Layout the Manager and contents.
         * @param context The context that will be used to draw the Manager and contents.
         * @param screenRect The screen rectangle available to the Manager.
         * @return The rectangle occupied by the Manager.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /**
         * @brief Set the layout manager.
         * @param layoutManager The LayoutManager.
         */
        void setLayoutManager(std::unique_ptr<LayoutManager> &&layoutManager) {
            mLayoutManager = std::move(layoutManager);
        }

        /**
         * @brief Get the LayoutManager.
         * @return A reference to the LayoutManager.
         */
        std::unique_ptr<LayoutManager>& layoutManager() { return mLayoutManager; }
    };

    /**
     * @class Parent
     * @brief A type used to extract the parent from a Widget.
     */
    struct Parent {};

    /**
     * @brief A manipulator to extract the parent from a Widget.
     */
    static Parent endw{};

    /**
     * @brief Create a Widget.
     * @tparam WidgetClass The type of the Widget to create.
     * @tparam Args The parameter pack type.
     * @param args The creation arguments parameter pack.
     * @return A std::shared_ptr to the Widget or null.
     */
    template<class WidgetClass, typename ... Args>
    inline std::shared_ptr<WidgetClass> wdg(Args ... args) {
        return std::make_shared<WidgetClass>(args ...);
    }
}

/**
 * @brief An extraction operator to store a Widget in a std::shared_ptr<Widget>.
 * @details Acting like a 'Tee' operation the Widget is both saved and passed on to be used in further processing.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The widget.
 * @param store The variable that will store the std::shared_ptr<Widget>.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator >>(std::shared_ptr<WidgetClass> widget, std::shared_ptr<rose::Widget> &store) {
    static_assert(std::is_base_of_v<rose::Widget,WidgetClass>, "WidgetClass must be derived from rose::Widget." );
    store = widget;
    return widget;
}

/**
 * @brief An extraction operator to store a Widget in a specifically typed std::shared_ptr<WidgetClass>.
 * @details Acting like a 'Tee' operation the Widget is both saved and passed on to be used in further processing.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The widget.
 * @param store The variable that will store the std::shared_ptr<WidgetClass>.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator >>(std::shared_ptr<WidgetClass> widget, std::shared_ptr<WidgetClass> &store) {
    static_assert(std::is_base_of_v<rose::Widget,WidgetClass>, "WidgetClass must be derived from rose::Widget." );
    store = widget;
    return widget;
}

//template<class ManagerClass>
//inline std::shared_ptr<ManagerClass> operator >>(std::shared_ptr<ManagerClass> manager, std::shared_ptr<rose::Manager> &store) {
//    static_assert(std::is_base_of_v<rose::Manager,ManagerClass>, "ManagerClass must be derived from rose::Manager." );
//    store = manager;
//    return manager;
//}

/**
 * @brief An insertion operator that take a Parent value and returns the parent of a Widget.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @return The Manager of the Widget, or an empty pointer if the Widget has no Manager.
 */
template<class WidgetClass>
inline std::shared_ptr<rose::Manager> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Parent &) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
                  "WidgetClass must be derived from rose::Widget or rose::Manager.");
    return std::dynamic_pointer_cast<rose::Manager>(widget->container());
}

/**
 * @brief An insertion operator to set the preferred size of a Widget.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param size The Size.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Size& size) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
            "WidgetClass must be derived from rose::Widget or rose::Manager.");
    widget->setSize(size);
    return widget;
}

/**
 * @brief An insertion operator to set the preferred Position of a Widget.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param position The Position.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Position& position) {
    static_assert(std::is_base_of_v<rose::Widget, WidgetClass> || std::is_base_of_v<rose::Manager, WidgetClass>,
                  "WidgetClass must be derived from rose::Widget or rose::Manager.");
    widget->setPosition(position);
    return widget;
}

/**
 * @brief An insertion operator to set the LayoutManager of a Manager.
 * @tparam WidgetClass The class of the Manager.
 * @param widget The Manager.
 * @param layout The LayoutManger.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, std::unique_ptr<rose::LayoutManager>&& layout) {
    static_assert(std::is_base_of_v<rose::Manager,WidgetClass>, "WidgetClass must be derived from rose::Manager.");
    widget->setLayoutManager(std::move(layout));
    return widget;
}

/**
 * @brief An insertion operator to set the Padding of a Widget.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param padding The Padding.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Padding& padding) {
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass>, "WidgetClass must be derived from rose::Visual.");
    widget->setPadding(padding);
    return widget;
}

/**
 * @brief An insertion operator to set a LyoutHing on a Widget.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param hint The LayoutHint.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::LayoutHint& hint) {
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass>, "WidgetClass must be derived from rose::Visual.");
    widget->setLayoutHint(hint);
    return widget;
}
