/** @file PopupWindow.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-31
  */

#pragma once

#include "Button.h"
#include "Container.h"
#include "Frame.h"
#include "Label.h"
#include "Signals.h"

namespace rose {

    struct DialogActionButton {
        ActionButtonType mActionButton{};
        SignalToken mSignalToken{};
    };

    class Popup : public Window {
    protected:
        Size mMinimumSize;               ///< The minimum size requests for the popup if supported by derived types.
        std::shared_ptr<Frame> mFrame;      ///< The Window Frame.
        std::shared_ptr<Column> mColumn;    ///< A container for the Window Title gadget and the window Container.

        std::shared_ptr<Slot<Button::SignalType>> mActionButtonRx{};    ///< The slot to receive action button signals
        std::shared_ptr<Slot<Button::SignalType>> mDismissButtonRx{};      ///< The slot to close the menu.

    public:
        Popup() = delete;
        ~Popup() override = default;
        Popup(Popup &&) = delete;
        Popup(const Popup &) = delete;
        Popup& operator=(Popup &&) = delete;
        Popup& operator=(const Popup &) = delete;

        /**
         * @brief Construct a Popup which covers the entire screen or as determined by the size of its children.
         * @param parent the Rose application object.
         */
        explicit Popup(const shared_ptr <Rose>& parent);

        /**
         * @brief Construct a Window of specified position.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param position the Window Position.
         */
        Popup(const shared_ptr <Rose>& parent, const Position &position);

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param minimumSize the Window size.
         */
        Popup(const shared_ptr <Rose>& parent, const Position &pos, const Size &minimumSize);

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

        /**
         * @brief See Widget::initialLayout()
         */
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /**
         * @brief See Widget::draw()
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Add a child Widget to the container that will be managing children.
         * @param widget The new child Widget.
         */
        void addChild(const std::shared_ptr<Widget>& widget) override {
            mColumn->addChild(widget);
        }

        /**
         * @brief Create the DialogActionButton objects.
         * @details Iterate through actionButtonList calling the virtual function createActionButton for each item.
         * @tparam C The type of container with DialogActionButton data.
         * @param actionButtonList The DialogActionButton container
         */
        template<class C>
        void setActionButtons(C actionButtonList);

        /**
         * @brief Create a DialogActionButton on derivatives of Popup that support them.
         * @details The default is to do nothing.
         * @param dialogActionButton The data structure that describes the button.
         */
        virtual void createActionButton(const DialogActionButton &dialogActionButton) {}

        /**
         * @brief See Widget::setButtonSlot()
         * @details If buttons have been created, they are disconnected from all slots before being connected
         * to buttonSlot.
         * @param buttonSlot
         */
        void setButtonSlot(shared_ptr<Slot<ButtonSignalType>> &buttonSlot) override {
            mActionButtonRx = buttonSlot;
        }
    };

    class PopupWindow : public Popup {
    protected:
        std::string mWindowTitle{};         ///< The title of the window.
        std::shared_ptr<Label> mTitle;      ///< The Window Title Label.

    public:
        PopupWindow() = delete;
        ~PopupWindow() override = default;
        PopupWindow(PopupWindow &&) = delete;
        PopupWindow(const PopupWindow &) = delete;
        PopupWindow& operator=(PopupWindow &&) = delete;
        PopupWindow& operator=(const PopupWindow &) = delete;

        /**
         * @brief Construct a PopupWindow which covers the entire screen.
         * @param parent the Rose application object.
         */
        explicit PopupWindow(shared_ptr <Rose> parent);

        /**
         * @brief Construct a Window of specified position.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param position the Window size.
         */
        PopupWindow(shared_ptr <Rose> parent, const Position &position);

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param size the Window size.
         */
        PopupWindow(shared_ptr <Rose> parent, const Position &pos, const Size &size);

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /**
         * @brief Set the value of the Window Title.
         * @param text The Window Title.
         */
        void setTitle(const std::string &text) override {
            mTitle->setText(text);
            setNeedsDrawing();
        }

        /// Handle a mouse drag event (default implementation: do nothing)
        bool mouseDragEvent(const Position &mousePos, const Position &rel, int button, int modifiers) override;
    };

    /**
     * @class Dialog
     * @brief A Dialog is a way for the application to have a brief conversation with the user.
     * @details A Dialog has a Title, Text, Badge and one or more action buttons.
     */
    class Dialog : public PopupWindow {
    protected:
        ImageId mBadgeId{};                 ///< The ImageId of the Dialog badge.
        std::shared_ptr<Row> mMessageRow;   ///< The area where the Dialog badge and message are held.
        std::shared_ptr<Row> mButtonRow;    ///< The area where the Dialog ActionButtons are held.

    public:
        Dialog() = delete;
        ~Dialog() override = default;
        Dialog(Dialog &&) = delete;
        Dialog(const Dialog &) = delete;
        Dialog& operator=(Dialog &&) = delete;
        Dialog& operator=(const Dialog &) = delete;

        /**
         * @brief Construct a PopupWindow which covers the entire screen.
         * @param parent the Rose application object.
         */
        explicit Dialog(shared_ptr <Rose> parent);

        /**
         * @brief Construct a Window of specified position.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param position the Window size.
         */
        Dialog(shared_ptr <Rose> parent, const Position &position);

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param size the Window size.
         */
        Dialog(shared_ptr <Rose> parent, const Position &pos, const Size &size);

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

        /**
         * @brief Create a DialogActionButton.
         * @param dialogActionButton The data structure that describes the button.
         */
        void createActionButton(const DialogActionButton &dialogActionButton) override;

        /**
         * @brief See Widget::setButtonSlot()
         * @details If buttons have been created, they are disconnected from all slots before being connected
         * to buttonSlot.
         * @param buttonSlot
         */
        void setButtonSlot(shared_ptr<Slot<ButtonSignalType>> &buttonSlot) override;
    };

    class ExitDialog : public Dialog {
    protected:
        static constexpr std::string_view mExitTitle = "Exit?";
        static constexpr std::string_view mExitMessage = "Exit the program?";
        static constexpr std::array<DialogActionButton,2> mActionButtons = {
                DialogActionButton{ ActionButtonOk, ExitDialogOk },
                DialogActionButton{ ActionButtonCancel, ExitDialogCancel }
        };

    public:
        ExitDialog() = delete;
        ~ExitDialog() override = default;
        ExitDialog(ExitDialog &&) = delete;
        ExitDialog(const ExitDialog &) = delete;
        ExitDialog& operator=(ExitDialog &&) = delete;
        ExitDialog& operator=(const ExitDialog &) = delete;

        /**
         * @brief Construct a PopupWindow which covers the entire screen.
         * @param parent the Rose application object.
         */
        explicit ExitDialog(shared_ptr <Rose> parent);

        /**
         * @brief Construct a Window of specified position.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param position the Window size.
         */
        ExitDialog(shared_ptr <Rose> parent, const Position &position);

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param size the Window size.
         */
        ExitDialog(shared_ptr <Rose> parent, const Position &pos, const Size &size);

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

    };
}

/**
 * @addtogroup WidgetManip
 * @{
 */

/**
 * @brief An Inserter to place a set of DialogActionButton Widgets on a Popup.
 * @tparam WidgetType The type of Widget.
 * @tparam N The number of DialogActionButton Widgets.
 * @param widget The Popup.
 * @param container The container holding the DialogActionButton data, in this case a std::array.
 * @return The Widget.
 */
template<class WidgetType, size_t N>
inline std::shared_ptr<WidgetType> operator << (std::shared_ptr<WidgetType> &widget,
        std::array<rose::DialogActionButton,N> &container) {
    static_assert(std::is_base_of_v<rose::Popup, WidgetType>,
            "Dialog action buttons are only created on classes derrived from rose::Popup." );
    widget->setActionbuttons(container);
    return widget;
}

/**
 * @brief An Inserter to place a set of DialogActionButton Widgets on a Popup.
 * @tparam WidgetType The type of Widget.
 * @tparam N The number of DialogActionButton Widgets.
 * @param widget The Popup.
 * @param container The container holding the DialogActionButton data, in this case a std::array.
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator << (std::shared_ptr<WidgetType> &widget,
                                                std::vector<rose::DialogActionButton> &container) {
    static_assert(std::is_base_of_v<rose::Popup, WidgetType>,
                  "Dialog action buttons are only created on classes derrived from rose::Popup." );
    widget->setActionbuttons(container);
    return widget;
}

/** @} */
