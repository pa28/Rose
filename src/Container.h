/** @file Container.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-16
  * @brief The Container Classes
  */

#pragma once

#include <utility>
#include <iostream>
#include <exception>

#include "Widget.h"

namespace rose {

    /**
     * @class ContainerOversizeException
     * @brief Thrown when a Container lays out to be over sized.
     */
    class ContainerOversizeException : public std::runtime_error {
    public:
        ContainerOversizeException() = delete;

        /// Constructor
        explicit ContainerOversizeException(const std::string &what) : std::runtime_error(what) {}

        /// Constructor
        explicit ContainerOversizeException(const char *what) : std::runtime_error(what) {}
    };

    /**
     * @struct ContainerLayoutHints
     * @brief LayoutHints specific to Containers.
     */
    struct ContainerLayoutHints {
        bool labelVerAlignBaseLine{};       ///< If true, all child Labels are aligned to a common base line.
        bool fillToEnd{};                   ///< If true, expand elastic children to fill to end of available space.
        int internalSpace{};                ///< Space between children.
        int startOffset{};                  ///< Space before the first child.
        int endOffset{};                    ///< Space after the last child.
        float verticalElastic{};            ///< How much extra vertical space should go between children.
        float horizontalElastic{};          ///< How much extra horizontal space should go between children.
    };

    /**
     * @class Container
     * @brief Container is the base class of all Widgets which have children.
     * @details The base container does not provide any layout management of its children. Children are
     * self managed.
     */
    class Container : public Widget {
    protected:
        bool mLayoutReverse{};                          ///< True if the layout should be done in reverse order.

        ContainerLayoutHints mContainerHints{};         ///< Hints to use in management of children.

        std::vector<std::shared_ptr<Widget>> mChildren{};             ///< The children of this Container

//        SizeInt limitSize(const std::shared_ptr<Widget>& child);             ///< Limit the size of a child to fit on the screen.

    public:
        Container();

        ~Container() override = default;

        Container(Container &&) = delete;

        Container(const Container &) = delete;

        Container &operator=(Container &&) = delete;

        Container &operator=(const Container &) = delete;

        /**
         * @brief Add a child widget to this container
         * @details This container is set as the widget's parent, the widget is added to this containers child list,
         * and the widgets initializeComposite method is called.
         * @param widget The child widget to add
         */
        virtual void addChild(const std::shared_ptr<Widget> &widget) {
            auto wParent = widget->weak_from_this();
            widget->mParent = as<Container>();
            widget->mHasParent = true;
            widget->mRose = mRose;
            mChildren.push_back(widget);
            widget->initializeComposite();
        }

        /**
         * @brief Add a child of type WidgetType to this Widget with arguments forwarded to the constructor.
         * @details The parent of the new widget will be this widget.
         * @tparam WidgetType The type of Widget to add
         * @tparam Args The types of arguments
         * @param args The arguments
         * @return A shared pointer to the new Widget.
         */
        template<class WidgetType, typename ... Args>
        std::shared_ptr<WidgetType> add(Args &&... args) {
            auto widget = std::make_shared<WidgetType>(std::forward<Args>(args)...);
            widget->mParent = as<Container>();
            widget->mHasParent = true;
            widget->mRose = mRose;
            mChildren.emplace_back(widget);
            widget->initializeComposite();
            return widget;
        }

        /**
         * @brief Standard container access to children.
         * @return The iterator of the first child.
         */
        auto begin() noexcept { return mChildren.begin(); }

        /**
         * @brief Standard container access to children.
         * @return The iterator one past the last child.
         */
        auto end() noexcept { return mChildren.end(); }

        /**
         * @brief Standard const container access to children.
         * @return The iterator of the first child.
         */
        auto begin() const noexcept { return mChildren.cbegin(); }

        /**
         * @brief Standard const container access to children.
         * @return The iterator one past the last child.
         */
        auto end() const noexcept { return mChildren.cend(); }

        /**
         * @brief Standard const container access to children.
         * @return A reference to the first item in the container.
         */
        auto front() const noexcept { return mChildren.front(); }

        /**
         * @brief Determine if the container is empty.
         * @return true if empty.
         */
        bool empty() const { return mChildren.empty(); }

        /**
         * @brief Determine the number of children in a Container
         * @return
         */
        auto size() const { return mChildren.size(); }

        /**
         * @brief Access to ContainerLayoutHints.
         * @return ContainerLayoutHints&
         */
        ContainerLayoutHints &containerLayoutHints() noexcept { return mContainerHints; }

        /**
         * @brief See Widget::draw
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Determine the desired size of the child widgets.
         * @details If not overridden the default is to call widgetLayout() for each child and return
         * the current space available to the Container.
         * @param renderer
         * @return SizeInt
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override {
            auto containerAvailable = clampAvailableArea(available, mPos, mSize);
            for (int localLayoutStage = 0; localLayoutStage < 2; ++localLayoutStage)
                for (auto &child : mChildren) {
                    auto layout = child->widgetLayout(renderer, containerAvailable, localLayoutStage);
                    if (auto position = child->getPos(); position)
                        layout = position.value();
                    child->mLayoutHints.mAssignedRect = layout;
                }
            return available;
        }

        /**
         * @brief Find the Widget which uniquely contains the position.
         * @param pos the position the Widget must contain.
         * @return A reference to the Widget found, if any.
         */
        virtual std::shared_ptr<Widget> findWidget(const Position &pos);

        /**
         * @brief Find a Widget with the specified Id.
         * @details The Widget tree is searched top down, front to back for the first Widget marked with the
         * specified Id.
         * @param id The Id to search for.
         * @return a std::shared_ptr<Widget>, empty if the Id is not found.
         */
        virtual std::shared_ptr<Widget> findWidget(const Id &id);

        /**
         * @brief Find the first Container above a Widget that supports dragging contents.
         * @param container The parent of the found widget.
         * @return A container that supports dragging or nullptr.
         */
        static std::shared_ptr<Container> findDragContainer(std::shared_ptr<Container> container);

        /**
         * @brief Calculate the ratio to shrink an object and maintain aspect ratio.
         * @param size The size of the object.
         * @param width The desired Width.
         * @return
         */
        static Size wdigetRatioWidth(Size size, int width);

        /**
         * @brief Calculate the ratio to shrink an object and maintain aspect ratio.
         * @param size The size of the object.
         * @param height The desired Height.
         * @return
         */
        static Size widgetRatioHeight(Size size, int height);

        /// Handle a mouse enter event.
        bool mouseEnterEvent(const Position &p, bool enter) override;

        /// Handle a mouse button event (default implementation: propagate to children)
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

        /// Handle a mouse motion event (default implementation: propagate to children)
        bool mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse drag event (default implementation: do nothing)
        bool mouseDragEvent(const Position &p, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse scroll event (default implementation: propagate to children)
        bool scrollEvent(const Position &p, int32_t relX, int32_t relY) override;

//        /// Handle a keyboard event (default implementation: do nothing)
//        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

        /// Handle text input (UTF-32 format) (default implementation: do nothing)
        bool keyboardCharacterEvent(unsigned int codepoint) override;

    };

    /**
     * @brief Type convenience for a shared pointer to a Container.
     */
    using ContainerPtr = std::shared_ptr<Container>;

    /**
     * @class Window
     * @brief A simple unmanaged container interface between the Screen and a branch of the scene tree.
     */
    class Window : public Container {
    protected:
        Modality mModal{};    ///< See Modality.

    public:
        ~Window() override = default;

        Window() = default;

        Window(Window &&) = delete;

        Window(const Window &) = delete;

        Window &operator=(Window &&) = delete;

        Window &operator=(const Window &) = delete;

        /**
         * @brief Construct a Window which covers the entire screen.
         * @param parent the Rose application object.
         */
        explicit Window(const std::shared_ptr<Rose> &parent);

        /**
         * @brief Construct a Window of specified size. Default position attempts to centre the Window on the Screen.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param size the Window size.
         */
        Window(const std::shared_ptr<Rose> &parent, const Size &size);

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param size the Window size.
         */
        Window(const std::shared_ptr<Rose> &parent, const Position &pos, const Size &size);

        /**
         * @brief Provide a reference to the Rose application object.
         * @return
         */
        std::shared_ptr<Rose> rose() override {
            if (mRose.expired())
                throw std::bad_weak_ptr();
            return mRose.lock();
        }

        /**
         * @brief Proved the modal status of the Window.
         * @return true if the Window is modal.
         */
        Modality getModal() const { return mModal; }
    };

    /**
     * @class Box
     * @brief A linear layout Container that can layout children in a Row or a Column.
     */
    class Box : public Container {
    protected:
        int mMinOrthogonal{};                    ///< The minimum size along the secondary axis.
        Orientation mOrientation{};              ///< The box Orientation.

    public:
        ~Box() override = default;

        Box(Box &&) = delete;

        Box(const Box &) = delete;

        Box &operator=(Box &&) = delete;

        Box &operator=(const Box &) = delete;

        /**
         * @brief Constructor see: Widget constructor.
         * @param parent
         */
        Box() : Container() {
            mClassName = "Box";
        }

        /**
         * @brief See Widget::draw.
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief See Widget::widgetLayout.
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /**
         * @brief Set the minimum width of the Column
         * @param minimumWidth The minimum width.
         */
        void setMinimumOrthogonal(int minimumOrthogonal) { mMinOrthogonal = minimumOrthogonal; }

        /// Set the orientation of the Box
        void setOrientation(Orientation orientation) override { mOrientation = orientation; }
    };

    /**
     * @class Column
     * @brief A container that manages children in a vertical arrangement.
     */
    class Column : public Box {
    public:
        ~Column() override = default;

        Column(Column &&) = delete;

        Column(const Column &) = delete;

        Column &operator=(Column &&) = delete;

        Column &operator=(const Column &) = delete;

        /**
         * @brief Constructor see: Widget constructor.
         * @param parent
         */
        Column() : Box() {
            mClassName = "Column";
            mOrientation = Orientation::Vertical;
        }
    };

    /**
     * @class Row
     * @brief A container that manages children in a horizontal arragement.
     */
    class Row : public Box {
    public:
        ~Row() override = default;

        Row(Row &&) = delete;

        Row(const Row &) = delete;

        Row &operator=(Row &&) = delete;

        Row &operator=(const Row &) = delete;

        /**
         * @brief Constructor see: Widget constructor.
         * @param parent
         */
        Row() : Box() {
            mClassName = "Row";
            mOrientation = Orientation::Horizontal;
        }
    };

    /**
     * @class Grid
     * @brief A container than manages its children in a grid arrangement.
     */
    class Grid : public Container {
    protected:
        size_t mStride;         ///< The stride (objects along the primary axis) of the grid
        std::optional<Size> mSingleSize;       ///< The size of all objects are the same.
        Orientation mOrientation{Orientation::Horizontal};             ///< Grid orientation

    public:
        using size_type = std::vector<LayoutHints>::size_type;  ///< The size type for vectors of LayoutHints.

        Grid() = delete;

        ~Grid() override = default;

        Grid(Grid &&) = delete;

        Grid(const Grid &) = delete;

        Grid &operator=(Grid &&) = delete;

        Grid &operator=(const Grid &) = delete;

        /**
         * @brief Constructor
         * @param stride The stride in the orientation direction
         * @param orientation The orientation direction
         */
        explicit Grid(size_t stride, const Size &size, Orientation orientation = Orientation::Horizontal);

        /**
         * @brief See Widget::draw.
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief See Widget::widgetLayout.
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;
    };

    /**
     * @brief Construct a WidgetType widget with the provided arguments.
     * @details This method is intended to be used with operator<< to add the widget to a container.
     * @tparam WidgetType The type of the Widget to create
     * @tparam Args The types in the parameter pack.
     * @param args The parameter pack
     * @return std::sharded_ptr<WidgetType>
     */
    template<class WidgetType, typename... Args>
    std::shared_ptr<WidgetType> wdg(Args... args) {
        std::shared_ptr<WidgetType> widget = std::make_shared<WidgetType>(std::forward<Args>(args)...);
        return widget;
    }
}
/**
 * @addtogroup WidgetManip
 * @{
 */

/**
 * @brief Set the VerticalSpacing on a Container ContainerLayoutHints.
 * @param container The Container.
 * @param startOffset The VerticalSpacing.
 * @return The Container.
 */
inline std::shared_ptr<rose::Container>
operator<<(std::shared_ptr<rose::Container> container, rose::StartOffset startOffset) {
    container->containerLayoutHints().startOffset = startOffset.mStartOffset;
    return container;
}

/**
 * @brief Set the HorizontalSpacing on a Container ContainerLayoutHints.
 * @param container The Container.
 * @param horizontalSpacing The HorizontalSpacing.
 * @return The Container.
 */
inline std::shared_ptr<rose::Container>
operator<<(std::shared_ptr<rose::Container> container, rose::InternalSpace internalSpace) {
    container->containerLayoutHints().internalSpace = internalSpace.mInternalSpace;
    return container;
}

/** @} */

