/**
 * @file Menu.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-03
 * @brief Menus and supporting Classes
 */

#pragma once

#include <utility>

#include "Button.h"
#include "Constants.h"
#include "PopupWindow.h"
#include "Signals.h"

namespace rose {
    /**
     * @struct MenuButtonData
     * @brief Information to specify a Menu Button
     */
    struct MenuButtonData {
        std::string_view mLabelText;    ///< The MenuButton Label value
        SignalToken mSignalToken;       ///< A SignalToken to identify the MenuButton.
    };

    using MenuDataList = std::vector<MenuButtonData>;   ///< The MenuDataList type.

    /**
     * @class PopupMenu
     * @brief A Menu that can be popped up by a CascadeButton
     */
    class PopupMenu : public Popup {
    protected:
        const MenuDataList &mMenuDataList;      ///< A const reference to the MenuData items.

        std::shared_ptr<Slot<Button::SignalType>> mAppButtonRx{};   ///< The slot to action the selection.

    public:
        PopupMenu() = delete;                   ///< Deleted default constructor.
        ~PopupMenu() override = default;
        PopupMenu(PopupMenu &&) = delete;
        PopupMenu(const PopupMenu &) = delete;
        PopupMenu& operator=(PopupMenu &&) = delete;
        PopupMenu& operator=(const PopupMenu &) = delete;

        /**
         * @brief Construct a PopupMenu.
         * @param rose A reference to the Rose object.
         * @param pos The position to display the menu.
         * @param menuDataList The MenuData items.
         * @param appButtonRx The application Slot to receive Button signals.
         */
        PopupMenu(const shared_ptr <Rose>& rose, Position pos, const MenuDataList &menuDataList,
                  std::shared_ptr<Slot<Button::SignalType>> appButtonRx);

        /**
         * @brief Construct a PopupMenu.
         * @param rose A reference to the Rose object.
         * @param pos The position to display the menu.
         * @param minimumSize The width of the PopupMenu so it can match the width of the CascadeButton.
         * @param menuDataList The MenuData items.
         * @param appButtonRx The application Slot to receive Button signals.
         */
        PopupMenu(const shared_ptr<Rose>& rose, Position pos, Size minimumSize, const MenuDataList &menuDataList,
                  std::shared_ptr<Slot<Button::SignalType>> appButtonRx);

        /**
         * @brief See Widget::initializeComposite
         */
        void initializeComposite() override;

        /// Handle a mouse button event (default implementation: propagate to children)
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

    };

    /**
     * @class CascadeButton
     * @brief A Button Widget that will popup a cascade menu.
     */
    class CascadeButton : public Button {
    protected:
        CascadeButtonType mCascadeButtonType;       ///< The type of cascade button.

        std::shared_ptr<Slot<Button::SignalType>> mCascadeButtonRx{};   ///< Slot to receive CascadeButton signals.
        std::shared_ptr<Slot<Button::SignalType>> mAppButtonSlot{};     ///< Slot to receive menu Button signals.

        MenuDataList mMenuData{};      ///< List of buttons to create in the menu.

        std::weak_ptr<PopupMenu> mPopupMenu;    ///< The PopupMenu, when created.

    public:
        CascadeButton();
        ~CascadeButton() override = default;
        CascadeButton(CascadeButton &&) = delete;
        CascadeButton(const CascadeButton &) = delete;
        CascadeButton& operator=(CascadeButton &&) = delete;
        CascadeButton& operator=(const CascadeButton &) = delete;

        /**
         * @brief Constructor
         * @param labelString the text label of the button
         */
        explicit CascadeButton(const std::string &labelString);

        /**
         * @brief Constructor
         * @param id Widget id string.
         */
        explicit CascadeButton(const Id& id);

        /**
         * @brief Constructor
         * @param labelString the text label of the button
         */
        explicit CascadeButton(std::string_view &labelString) : CascadeButton(std::string{labelString}) {}

        /**
         * @brief Constructor
         * @param labelString The text label of the button
         */
        explicit CascadeButton(const char *labelString) : CascadeButton(std::string{labelString}) {}

        /**
         * @brief Construct a CascadeButton
         * @param labelString the button label
         * @param type the button type
         * @param fontSize the font size to use with the label
         */
        CascadeButton(const std::string &labelString, CascadeButtonType type, int fontSize = 0);

        /**
         * Constructor
         * @param id Widget id string.
         * @param type the button type
         * @param fontSize the font size to use with the label
         */
        CascadeButton(const Id& id, CascadeButtonType type, int fontSize = 0);

        /**
         * @brief Constructor
         * @param labelString the button label
         * @param type the button type
         * @param fontSize the font size to use with the label
         */
        CascadeButton(const char *labelString, CascadeButtonType type, int fontSize = 0)
                : CascadeButton(std::string{labelString}, type, fontSize) {}

        /**
         * @brief See Widget::initializeComposite
         */
        void initializeComposite() override;

        /**
         * @brief See Widget::draw
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Set the data needed to create the menu Buttons.
         * @tparam C The type of container with DialogActionButton data.
         * @param actionButtonList The DialogActionButton container
         */
        template<class C>
        void addMenuData(C menuButtonList) {
            for (auto &button : menuButtonList) {
                mMenuData.emplace_back(button);
            }
        }

        /// See Widget::setButtonSlot()
        void setButtonSlot(shared_ptr<Slot<ButtonSignalType>> &buttonSlot) override { mAppButtonSlot = buttonSlot; }
    };
}

/**
 * @addtogroup WidgetManip
 * @{
 */

/**
 * @brief Inserter to set MenuButtonData on a CascadeButton
 * @tparam N The number of Buttons
 * @param widget The CascadeButton
 * @param container The container of MenuButtonData, a std::array in this case.
 * @return the Widget
 */
template<size_t N>
inline std::shared_ptr<rose::CascadeButton> operator << (std::shared_ptr<rose::CascadeButton> widget,
                                                const std::array<rose::MenuButtonData,N> &container) {
    widget->addMenuData(container);
    return widget;
}

/**
 * @brief Inserter to set MenuButtonData on a CascadeButton
 * @param widget The CascadeButton
 * @param container The container of MenuButtonData, a std::vector in this case.
 * @return the Widget
 */
inline std::shared_ptr<rose::CascadeButton> operator << (std::shared_ptr<rose::CascadeButton> widget,
                                                std::vector<rose::MenuButtonData> &container) {
    widget->addMenuData(container);
    return widget;
}

/** @} */
