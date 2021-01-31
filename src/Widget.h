/** @file Widget.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-20
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
  * @brief The Widget Class.
  * Widgets form the body of the user interface rendering the view as a scene on
  * graphics display, providing a controller through active elements in the user
  * interface, and modeling the data used to create the user experience.
  *
  * Widgets may place and size themselves, but a more pleasing and useful arrangement
  * may be had if the placement and size follows an organized layout structure. This
  * structure is provided by Container objects. Widgets do not have children, Containers
  * have children.
  */

#pragma once

#include <memory>
#include <iostream>
#include <utility>
#include <vector>
#include "Color.h"
#include "Renderer.h"
#include "ScreenMetrics.h"
#include "Signals.h"
#include "Theme.h"
#include "Utilities.h"

/**
 * @page widget_hierarchy
 * @section memory_structure Memory Structure
 *
 * Widgets are display elements that are organized in a tree structure from parent through children.
 * The root parent is the application, which must be derived from the rose::Rose class even though
 * the rose::Rose class is not derived from the rose::Widget class.
 *
 * @section class_structure Class Inheritance Structure
 *
 * The inheritance structure of Widgets starts with the pure virtual base rose::Widget class.
 */

/**
 * @defgroup WidgetManip Widget Manipulator Functions
 */

namespace rose {

    class Rose;

    class Window;

    class Widget;

    class Container;

    class SingleChild;

    /**
     * @class LayoutHints
     * @brief Data used by Container widgets that manage layout.
     * @details This information is used to position a Widget within the Rectangle allocated for it if there is
     * extra space.
     */
    struct LayoutHints {
        bool mElastic{false};                           ///< True if the Widget can expand and manage its layout withing a larger box like a Label
        bool mShrinkable{false};                        ///< True if the Widget is easily shrinkable like an ImageView
        HorizontalAlignment mHorAlign{};                ///< Horizontal alignment.
        VerticalAlignment mVerAlign{};                  ///< Vertical alignment.
        LabelHorizontalAlignment mLabelHorAlign{};      ///< Horizontal text alignment within a Label.
        LabelVerticalAlignment mLabelVerAlign{};        ///< Vertical text alignment within a Label.
        std::optional<Rectangle> mAssignedRect{};       ///< If valid the assigned render location relative to parent.
        int mBaseLine{};                                ///< Pixels from the top of the texture to an alignment baseline, for example in a Label.
    };

    /**
     * @class Id
     * @brief A class derived from std::string that may be initialized by anything convertable to a std::string.
     */
    class Id : public std::string {
    public:
        Id() = default;

        /**
         * @brief Constructor
         * @tparam String The type of initializer
         * @param id The value of intializer
         */
        template<typename String>
        explicit Id(String id) : std::string(id) {}

        /**
         * @brief Get the value of the Id.
         * @return The value in a std::string.
         */
        [[nodiscard]] auto value() const { return static_cast<std::string>(*this);}
    };

    /**
     * @class Widget
     * @brief An element in the scene graph that describes a rectangle.
     * The rose::Widget class is the base class of all visual elements in the library.
     */
    class Widget : public std::enable_shared_from_this<Widget> {
    protected:
        friend class Container;

        friend class SingleChild;

        friend class TabBody;

        std::weak_ptr<Rose> mRose;                      ///< A weak_ptr to the Rose object.
        std::optional<Size> mSize;                      ///< The provided size, from Rose::build() or similar source.
        std::optional<Position> mPos;                   ///< The provided position, from Rose::build() or similar source.

        std::weak_ptr<Container> mParent{};             ///< The parent of this Widget
        bool mHasParent{};                              ///< True of mParent points to a widget

        bool mFocused{};                                ///< True if this Widget has focus
        bool mVisible{true};                            ///< True if this Widget is visible
        bool mMouseFocus{};                             ///< True if the mouse is in this Widget
        bool mFillBackground{};                         ///< True if the Widget should fill with background color

        color::RGBA mBackgroundColor{};                 ///< The widget background color::RGBA

        Id mId{};                     ///< An identifier string used to find the widget and for settings.

        std::string_view mClassName;    ///< The class name of the Widget.

        SignalToken mSignalToken{};     ///< The SignalToken of the Widget.

        LayoutHints mLayoutHints{};     ///< The layout hints for this widget. These are managed by the parent Container.

        bool mSupportsDrag{};                           ///< True if the Widget supports dragging contents.
        bool mAcceptsFocus{};                           ///< True if the Widget accepts focus.
        bool mSupportsScrollWheel{};                    ///< True if the Widget supports mouse scroll wheel events.

    public:
        Widget();
        virtual ~Widget() = default;
        Widget(Widget &&) = delete;
        Widget(const Widget &) = delete;
        Widget& operator=(Widget &&) = delete;
        Widget& operator=(const Widget &) = delete;

        /**
         * @brief Get the class name.
         * @return The class name.
         */
        virtual std::string_view getClassName() const { return mClassName; }

        /**
         * @brief Set the Widget id
         * @param id the Widget id string.
         */
        void setId(const Id& id) { mId = id; }

        /**
         * @brief Get the Widget id
         * @return The Widget id string.
         */
        const Id& getId() const { return mId; }

        /**
         * @brief Called immediately after construction of a Widget.
         * @details There are a number of things that can not, or should not be done in the constructor of a
         * Widget. These things should be done in this virtual function which is called once the constructor
         * returns.
         */
        virtual void initializeComposite() {
            auto pw = parent<Widget>();
            if (pw->mRose.expired())
                throw std::bad_weak_ptr();
            mRose = parent<Widget>()->mRose;
        }

        /**
         * @brief Set the Rose needs drawing flag.
         * @details This flag should be set whenever the widget changes its display so that the Rose object
         * will re-draw the scene graph. If no widgets set this flag the scene is not redrawn during the frame
         * saving system resources.
         */
        void setNeedsDrawing();

        /**
         * @brief Get a shared pointer to this Widget.
         * @return a new shared pointer.
         */
        std::shared_ptr<Widget> getWidget() {
            return shared_from_this();
        }

        /**
         * @brief Access Widget layout hints
         * @return the layout hints object.
         */
        LayoutHints &layoutHints() { return mLayoutHints; }

        /**
         * @brief Get a shared pointer to this Widget cast to WidgetClass
         * @tparam WidgetClass
         * @return a shared pointer to the Widget as a WidgetClass, or empty if the cast fails.
         */
        template<class WidgetClass>
        std::shared_ptr<WidgetClass> getWidget() {
            return shared_from_this()->as<WidgetClass>();
        }

        /**
         * @brief Provide access to the Widget's parent
         * @details If the widget has no parent the return pointer is empty.
         * @return A pointer to the parent as a std::shared_ptr<Container>.
         */
        std::shared_ptr<Container> parent();

        /**
         * @brief Determine the Rectangle to layout a Widget.
         * @details All values come from the parent Container during layout and drawing. The parent may choose to
         * pass these values, other values, or none depending on the type of layout management and the phase of
         * the rendering cycle.</p>
         * If the Position is provided, the origin of the available area is moved relative to its
         * position by the widget Position. See Rectangle::moveOrigin(). Otherwise the position is set to (0,0).</p>
         * If the Size is provided, the smaller of the remaining available Size or the requested widget Size is selected.
         * If neither the Position nor the Size are provided the available rectangle is returned.</p>
         * The method is static so it can be tested independently of the rest of the framework.
         * @param available The Rectangle available for layout, provided by the parent.
         * @param position The requested Position provided by the Widget.
         * @param size The requested Size provided by the Widget.
         * @return The layout Rectangle to use.
         */
        static constexpr Rectangle
        clampAvailableArea(const Rectangle &available, const std::optional<Position> &position,
                           const std::optional<Size> &size) {
            auto res = available;
            if (position)
                res = res.moveOrigin(position.value());
            else
                res = Position::Zero;

            if (size && res.getSize() > size.value())
                res = size.value();

            return res;
        }

        /**
         * @brief Convenience version of clampAvailableArea().
         * @param available The Rectangle available for layout, provided by the parent.
         * @param rectangle A std::optional<Rectangle> containing the Position and Size.
         * @return The result of the main version of clampAvailableArea().
         */
        static constexpr Rectangle
        clampAvailableArea(const Rectangle &available, const std::optional<Rectangle> &rectangle) {
            if (rectangle)
                return clampAvailableArea(available, std::optional<Position>{rectangle->getPosition()},
                                          std::optional<Size>{rectangle->getSize()});
            else
                return clampAvailableArea(available, std::nullopt, std::nullopt);
        }

        /**
         * @brief Determine the desired size of the widget.
         * @details If not overridden the default is to return the current value of mSize.
         * @param renderer the Renderer to use if needed.
         * @param available The rectangle available for layout.
         * @return Rectangle The requested layout rectangle.
         */
        virtual Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) = 0;

        /**
         * @brief Provide access to the Widget's parent down cast to WidgetType if possible.
         * @details If the widget has no parent, or the cast fails the return pointer is empty.
         * @tparam WidgetType
         * @return a std::shared_ptr<WidgetClass> with the parent or empty.
         */
        template<class WidgetType>
        std::shared_ptr<WidgetType> parent() {
            if (mHasParent && !mParent.expired()) {
                return std::dynamic_pointer_cast<WidgetType>(mParent.lock());
            }
            return nullptr;
        }

        /**
         * @brief Request this Widget receive focus.
         */
        void requestFocus();

        /**
         * @brief Down cast the Widget to
         * @tparam WidgetType the type to cast to
         * @return A std::shared_ptr to the WidgetType which will be empty if the cast fails.
         */
        template<class WidgetType>
        std::shared_ptr<WidgetType> as() {
            return std::dynamic_pointer_cast<WidgetType>(getWidget());
        }

        /**
         * @brief Traverse the scene tree to find the root which is a Window
         * @return a WindowPtr
         */
        std::shared_ptr<Window> window();

        /**
         * @brief Traverse the scene tree to fine the root, the return the Rose
         * @return a RosePtr empty if the screen could not be found.
         */
        virtual std::shared_ptr<Rose> rose();

        /**
         * @brief Draw the widget.
         * @details If the Widget is managed but its parent container it will have a valid Rectangle in
         * the LayoutHints::mAssignedRectangle value; and it should be drawn in that rectangle after offsetting
         * by the parent position.
         * @param renderer The Renderer to use
         * @param parentRect The Rectangle available to the Parent.
         */
        virtual void draw(sdl::Renderer &renderer, Rectangle parentRect);

        /**
         * @brief Assert that the Widget has been layed out for drawing.
         */
        void assertLayout();

        /**
         * @brief Set the size of the Widget.
         * @param size the new size.
         */
        virtual void setSize(Size size) { mSize = size; }

        /**
         * @brief Get the size of the Widget.
         * @return the current size.
         */
        auto getSize() const { return mSize; }

        /**
         * @brief Set the text value if implemented by a Widget.
         * @details The default behavior is to do nothing. Widgets that implement a text member must override or
         * be derived from a Widget that does override.
         * @param text The text to set.
         */
        virtual void setText(const std::string &text) {}

        /**
         * @brief Set the title value if implemented by a Widget.
         * @details The default behavior is to do nothing. Widgets that implement a title member must override or
         * be derived from a Widget that does override.
         * @param title The title value.
         */
        virtual void setTitle(const std::string &title) {}

        /**
         * @brief Set the SignalToken value.
         * @details The SignalToken value can be sent on signals, or used in other ways to uniquely identify the
         * Widget source. The user application is responsible for enforcing what unique means in this case.
         * @param signalToken The SignalToken
         */
        void setSignalToken(SignalToken signalToken) {
            mSignalToken = signalToken;
        }

        /**
         * @brief Get the SignalToken value.
         * @return The SignalToken.
         */
        SignalToken getSignalToken() const { return mSignalToken; }

        /**
         * @brief Set the position of the Widget relative to its parent.
         * @param pos a Position
         */
        void setPos(Position pos) { mPos = pos; }

        /**
         * @brief Get the position of the Widget relative to its parent.
         * @return a Position
         */
        auto getPos() const { return mPos; }

        /**
         * @brief Get the Widget focus status.
         * @return true if the Widget is focused.
         */
        bool getFocused() const { return mFocused; }

        /**
         * @brief Set the Widget focus status.
         * @param focused true if focused.
         */
        void setFocused(bool focused) { mFocused = focused; }

        /**
         * @brief Set the visibility of the Widget
         * @param visible true if visible.
         */
        void setVisible(bool visible) { mVisible = visible; }

        /**
         * @brief Get the visibility of the Widget
         * @return true if visible.
         */
        bool getVisible() const { return mVisible; }

        /**
         * @brief Set the visibility of the Widget and return WidgetPtr.
         * @param visible
         * @return and return rose::ref<Widget>{this}.
         */
        std::shared_ptr<Widget> withVisible(bool visible) {
            setVisible(visible);
            return getWidget();
        }

        /**
         * @brief Enable or disable filling background
         * @param fill true if background should be filled.
         */
        void setFillBackground(bool fill) { mFillBackground = fill; }

        /**
         * @brief Get the fill background state of the Widget
         * @return true if widget will be filled.
         */
        bool getFillBackground() const { return mFillBackground; }

        /**
         * @brief Enable or disable filling background of the Widget and return WidgetPtr. DEPRECATED
         * @param fill True if the background should be filled.
         * @return and return rose::ref<Widget>{this}.
         */
        std::shared_ptr<Widget> withFillBackground(bool fill) {
            setFillBackground(fill);
            return getWidget();
        }

        /**
         * @brief Set the background color of the Widget and enable filling background
         * @param color
         */
        void setBackgroundColor(const color::RGBA &color) {
            mBackgroundColor = color;
            mFillBackground = true;
        }

        /**
         * @brief Get the background color of the Widget
         * @return color
         */
        color::RGBA getBackgroundColor() const { return mBackgroundColor; }

        /**
         * @brief Set the background color of the Widget, enable filling and return rose::ref<Widget>{this}.
         * @param color
         * @return and return rose::ref<Widget>{this}.
         */
        std::shared_ptr<Widget> withBackgroundColor(const color::RGBA &color) {
            setBackgroundColor(color);
            return getWidget();
        }

        /**
         * @brief Determine if the Widget supports dragging contents.
         * @return True if dragging contents is supported.
         */
        bool supportsDrag() const { return mSupportsDrag; }

        /**
         * @brief Determine if the Widget accepts focus.
         * @return True if the Widget accpets focus.
         */
         bool acceptsFocus() const { return mAcceptsFocus; }

         /**
          * @brief Determine if the Widget supports mouse scroll wheel events.
          * @return
          */
         bool supportsScrollWheel() const { return mSupportsScrollWheel; }

        /**
         * @brief Set a color on a Widget.
         * @details This is a virtual method, each widget can redefine the what to do with the color. The default
         * behavior provided by the Widget class is to set the background color.
         * @param color
         */
        virtual void setColor(const color::RGBA &color) { setBackgroundColor(color); }

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        virtual void setHorizontalAlignment(HorizontalAlignment alignment) { mLayoutHints.mHorAlign = alignment; }

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        virtual void setVerticalAlignment(VerticalAlignment alignment) { mLayoutHints.mVerAlign = alignment; }

        /**
         * @brief Set the orientation of Widgets that support it.
         * @details The default action is to do nothing. Widgets that support Orientation must override this
         * method.
         * @param orientation The desired orientation.
         */
        virtual void setOrientation(Orientation orientation) {}

        /**
         * @brief Get the orientation of Widgets that support it.
         * @details The default action is to return Orientation::Unset. Widgets that support Orientation must
         * override this method.
         * @return The actual orientation if supported, Orientation::Unset if not.
         */
        virtual Orientation getOrientation() const { return Orientation::Unset; }

        /**
         * @brief Check if the widget contains a certain position
         * @param p the Position to check.
         * @return true if the Widget contains p
         */
        bool contains(const Position &p) const {
            if (mLayoutHints.mAssignedRect && mLayoutHints.mAssignedRect->contains(p))
                return true;
            return false;
        }

        /**
         * @brief Get the position, on the screen, where the top left corner of the widget is located.
         * @return a Position
         */
        Position getScreenPosition() const;

        /**
         * @brief Set the widget ImageId. Default is to do nothing because not all widgets use images with ImageId.
         * @param imageId The ImageId.
         */
        virtual void setImageId(ImageId imageId) {}

        /**
         * @brief Get the widget ImageId. Default is to return RoseImageId::RoseImageInvalid because not all widgets
         * use ImageId.
         * @return The Widget ImageId.
         */
        virtual ImageId getImageId() const noexcept;

        /// Provide a Slot for the widget to direct Signals transmitted from child Button Widgets.
        virtual void setButtonSlot(std::shared_ptr<Slot<ButtonSignalType>> &buttonSlot) {}

        /// Handle a mouse button event (default implementation: propagate to children)
        virtual bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers);

        /// Handle a click transaction cancel event (default implementation: propagate to children)
        virtual bool clickTransactionCancel(const Position &mousePos, int button, bool down, int modifiers);

        /// Handle a mouse motion event (default implementation: propagate to children)
        virtual bool mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers);

        /// Handle a mouse drag event (default implementation: do nothing)
        virtual bool mouseDragEvent(const Position &p, const Position &rel, int button, int modifiers);

        /// Handle a mouse enter/leave event (default implementation: record this fact, but do nothing)
        virtual bool mouseEnterEvent(const Position &p, bool enter);

        /// Handle a mouse scroll event (default implementation: propagate to children)
        virtual bool scrollEvent(const Position &p, int32_t relX, int32_t relY);

        /// Handle a focus change event (default implementation: record the focus status, but do nothing)
        virtual bool focusEvent(bool focused);

        /// Handle a keyboard event (default implementation: do nothing)
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

        /// Handle text input (UTF-32 format) (default implementation: do nothing)
        virtual bool keyboardCharacterEvent(unsigned int codepoint);
    };

    /**
     * @addtogroup WidgetManip
     * @{
     */

    /**
     * @class BackgroundColor
     * @brief A manipulator to set the background color on a widget.
     */
    struct BackgroundColor : public color::RGBA {
        BackgroundColor() = delete;

        /**
         * @brief Create a BackgroundColor manipulator.
         * @param color the color to use.
         */
        explicit BackgroundColor(const color::RGBA &color) : color::RGBA(color) {}
    };

    /**
     * @class FillBackground
     * @brief A manipulator to set the FillBackground flag on a widget.
     */
    struct FillBackground {
        bool value;                     ///< The flag value.
        FillBackground() = delete;

        /**
         * @brief Create a FillBackground manipulator.
         * @param fill the boolean value to set on the flag.
         */
        explicit FillBackground(bool fill) : value(fill) {}
    };

    /**
     * @class Parent
     * @brief A Widget manipulator to climb the tree looking for a particular type of parent.
     * @tparam WidgetClass The type of parent to look for.
     */
    template<class WidgetClass>
    struct Parent {};

    /**
     * @class WidgetTitle
     * @brief A Widget manipulator to set the Widget title.
     */
    struct WidgetTitle {
        std::string mTitle{};   ///< The title.

        WidgetTitle() = default;    ///< Default constructor

        /// Constructor
        explicit WidgetTitle(std::string title) : mTitle(std::move(title)) {}

        /// Constructor
        explicit WidgetTitle(std::string_view title) : mTitle(title) {}

        /// Constructor
        explicit WidgetTitle(const char *title) : mTitle(title) {}
    };

    /**
     * @class WidgetText
     * @brief A Widget manipulator to set the Widget text.
     */
    struct WidgetText {
        std::string mText{};       ///< The title.

        WidgetText() = default;    ///< Default constructor

        /// Constructor
        explicit WidgetText(std::string text) : mText(std::move(text)) {}

        /// Constructor
        explicit WidgetText(std::string_view text) : mText(text) {}

        /// Constructor
        explicit WidgetText(const char *text) : mText(text) {}

        /// Set the widget text value
        WidgetText& operator=(std::string text) { mText = std::move(text); return *this; }

        /// Set the widget text value
        WidgetText& operator=(std::string_view text) { mText = text; return *this; }

        /// Set the widget text value
        WidgetText& operator=(const char *text) { mText = text; return *this; }
    };

    /** @} */
}

/**
 * @addtogroup WidgetManip
 * @{
 */

/**
 * @brief The insertion operator to process Parent<WidgetClass> objects
 * @tparam WidgetClass The type of Widget operated on.
 * @tparam ParentClass The type of parent Widget to look for.
 * @param widget The widget.
 * @param parentClass The manipulator.
 * @return std::shared_ptr<ParentClass> containing the parent of that type if found, otherwise empty.
 */
template<class WidgetClass,class ParentClass>
inline std::shared_ptr<ParentClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::Parent<ParentClass> parentClass) {
    auto parent = widget->parent();
    while (parent) {
        if (auto parentAs = parent->template as<ParentClass>(); parentAs)
            return parentAs;
        parent = parent->parent();
    }
    return nullptr;
}

/**
 * @brief A Widget manipulator to set the size.
 * @tparam WidgetClass The type of widget.
 * @param widget The widget
 * @param size The new size.
 * @return a std::shared_ptr to this widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::Size size) {
    widget->setSize(size);
    return widget;
}

/**
 * @brief A Widget manipulator to set the position.
 * @tparam WidgetClass The type of widget.
 * @param widget The widget
 * @param pos The new position.
 * @return a std::shared_ptr to this widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::Position pos) {
    widget->setPos(pos);
    return widget;
}

/**
 * @brief A Widget manipulator to set size treatment value.
 * @tparam WidgetClass The type of widget.
 * @param widget The widget
 * @param widgetSize The size treatment value.
 * @return a std::shared_ptr to this widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::WidgetSize widgetSize) {
    widget->setWidgetSize(widgetSize);
    return widget;
}

/**
 * @brief Insertion operator for a color.
 * @tparam WidgetClass The widget type
 * @param widget The Widget
 * @param color The color::RGBA
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::color::RGBA &color) {
    widget->setColor(color);
    return widget;
}

/**
 * @brief Insertion operator for BackgroundColor
 * @tparam WidgetClass The widget type
 * @param widget The Widget
 * @param color The color to set as background.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass>
operator<<(std::shared_ptr<WidgetClass> widget, const rose::BackgroundColor &color) {
    widget->setBackgroundColor(color);
    return widget;
}

/**
 * @brief Insertion operator for FillBackground flag
 * @tparam WidgetClass The widget type
 * @param widget The Widget
 * @param fill The value to set on the flag
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::FillBackground &fill) {
    widget->setFillBackground(fill);
    return widget;
}

/**
 * @brief A Widget manipulator to set how the Widget size is handled.
 * @tparam WidgetClass The type of Widget.
 * @param widget The Widget.
 * @param widgetSize The kind of handling applied to the Widget size.
 * @return The Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass>
operator<<(std::shared_ptr<WidgetClass> widget, const rose::WidgetSize &widgetSize) {
    widget->setWidgetSize(widgetSize);
    return widget;
}

/**
 * @brief Widget manipulator inserter for setting Orientation.
 * @tparam WidgetClass The type of widget manipulated
 * @param widget The widget manipulated
 * @param orientation The orientation to set on the Widget
 * @return The widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const rose::Orientation &orientation) {
    widget->setOrientation(orientation);
    return widget;
}

/**
 * @brief Set the Widget title.
 * @tparam WidgetType The Widget type.
 * @param widget The Widget
 * @param title The title
 * @return The Widget
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::WidgetTitle title) {
    widget->setTitle(title.mTitle);
    return widget;
}

/**
 * @brief Set the Widget text.
 * @tparam WidgetType The Widget type.
 * @param widget The Widget
 * @param text The text
 * @return The Widget
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::WidgetText text) {
    widget->setText(text.mText);
    return widget;
}

/**
 * @brief Set the Widget Slot to receive Signals from child Button Widgets, if implemented.
 * @tparam WidgetType The Widget type.
 * @param widget The Widget.
 * @param buttonRxSlot The Slot<rose::ButtonSignalType>.
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget,
        std::shared_ptr<rose::Slot<rose::ButtonSignalType>> buttonRxSlot) {
    widget->setButtonSlot(buttonRxSlot);
    return widget;
}

/**
 * @brief Set the HorizontalAlignment of a Widget.
 * @tparam WidgetType The Widget type.
 * @param widget The Widget.
 * @param horizontalAlignment The HorizontalAlignment value.
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::HorizontalAlignment horizontalAlignment) {
    widget->setHorizontalAlignment(horizontalAlignment);
    return widget;
}

/**
 * @brief Set the VerticalAlignment of a Widget.
 * @tparam WidgetType  The Widget type.
 * @param widget The Widget.
 * @param verticalAlignment The VerticalAlignment value.
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::VerticalAlignment verticalAlignment) {
    widget->setVerticalAlignment(verticalAlignment);
    return widget;
}

/** @} */
