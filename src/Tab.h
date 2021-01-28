/**
 * @file Tab.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 2.0
 * @date 2021-01-10
 */

#pragma once

#include <utility>

#include "Border.h"
#include "Button.h"
#include "Container.h"
#include "Frame.h"
#include "ScrollArea.h"
#include "Signals.h"

namespace rose {

    /**
     * @class TabPage
     * @brief A TabPage manages a single Widget which displays the page contents.
     */
    class TabPage : public Border {
    protected:
        std::string mTabLabel{};        ///< The identity of the TabPage, will be the TabButton Label.

    public:
        TabPage() = delete;
        ~TabPage() override = default;
        TabPage(TabPage &&) = delete;
        TabPage(const TabPage &) = delete;
        TabPage& operator=(TabPage &&) = delete;
        TabPage& operator=(const TabPage &) = delete;

        /// Constructor
        explicit TabPage(std::string label) : Border(), mTabLabel(std::move(label)) {}

        /// Constructor
        explicit TabPage(std::string_view label) : TabPage(std::string(label)) {}

        /// Constructor
        explicit TabPage(const char *label) : TabPage(std::string(label)) {}

        /**
         * @brief Access the TabPage label.
         * @return const std::string&
         */
        const std::string& tabLabel() const noexcept { return mTabLabel; }
    };

    /**
     * @class TabBody
     * @brief A TabBody manages a number of TabPages.
     */
    class TabBody : public Container {
    protected:

        /**
         * @brief Find the active TabPage.
         * @details First this method finds that active child TabPage. It is then returned after casting to
         * WidgetType.
         * @tparam WidgetType The desired return type.
         * @return std::shared_ptr<WidgetType> If there is an active child TabPage and it is derived from WidgetType.
         */
        template<class WidgetType>
        std::shared_ptr<WidgetType> activeChild() {
            for (auto &child : mChildren) {
                if (child->mVisible)
                    return child->as<WidgetType>();
            }
            return nullptr;
        }

    public:
        TabBody() = default;
        ~TabBody() override = default;
        TabBody(TabBody &&) = delete;
        TabBody(const TabBody &) = delete;
        TabBody& operator=(TabBody &&) = delete;
        TabBody& operator=(const TabBody &) = delete;

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

        /**
         * @brief See Widget::draw()
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief See Widget::initialLayout()
         */
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /// Handle a mouse button event (default implementation: propagate to children)
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

        /// Handle a mouse motion event (default implementation: propagate to children)
        bool mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse drag event (default implementation: do nothing)
        bool mouseDragEvent(const Position &p, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse enter/leave event (default implementation: record this fact, but do nothing)
        bool mouseEnterEvent(const Position &p, bool enter) override;

        /// Handle a mouse scroll event (default implementation: propagate to children)
        bool scrollEvent(const Position &p, double x, double y) override;

        /// Handle a focus change event (default implementation: record the focus status, but do nothing)
        bool focusEvent(bool focused) override;

        /// Handle a keyboard event (default implementation: do nothing)
        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

        /// Handle text input (UTF-32 format) (default implementation: do nothing)
        bool keyboardCharacterEvent(unsigned int codepoint) override;

        /**
         * @brief Find the Widget which uniquely contains the position.
         * @param pos the position the Widget must contain.
         * @return A reference to the Widget found, if any.
         */
        std::shared_ptr<Widget> findWidget(const Position &pos) override;

        /**
         * @brief Find a Widget with the specified Id.
         * @details The Widget tree is searched top down, front to back for the first Widget marked with the
         * specified Id.
         * @param id The Id to search for.
         * @return a std::shared_ptr<Widget>, empty if the Id is not found.
         */
        std::shared_ptr<Widget> findWidget(const Id &id) override;
    };

    using TabHeader = Row;      ///< The type to use as the TabHeader.

    /**
     * @class Tab
     * @brief A Tab Widget
     * @details Hosts a number of children TabPage objects in a TabBody, each with a TabButton in the
     * TabHeader. Only ont TabPage is visible at a time the other being, conceptually behind the visible TabPage.
     */
    class Tab : public Column {
    protected:
        std::size_t mActiveTab{};               ///< The currently active TabPage.
        std::shared_ptr<TabHeader> mHdr{};      ///< Convenience pointer to the TabHeader.
        std::shared_ptr<Frame> mFrame{};        ///< Convenience pointer to the Frame surrounding the TabBody.
        std::shared_ptr<Container> mBody{};     ///< Convenience pointer to the TabBody.

        std::shared_ptr<Slot<ButtonSignalType>> rxState{};      ///< Slot to receive TabButton state signals.
        std::shared_ptr<Slot<ButtonSignalType>> rxPushed{};     ///< Slot to receive TabButton pushed signals.

    public:
        Tab() = default;
        ~Tab() override = default;

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

        /**
         * @brief See Widget::draw()
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief See Widget::initialLayout()
         */
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /**
         * @brief Add a TabPage to the Tab widget
         * @param widget The TabPage to add.
         */
        void addChild(const std::shared_ptr <Widget> &widget) override;

        /**
         * @brief Add a button to the TabHeader.
         * @param label The Button Label
         * @return std::shared_ptr<Button>
         */
        void addTabButton(const string &label);

        /**
         * @brief Set the active TabPage
         * @param tabIdx The index of the TabPage to activate.
         */
        void activeTab(size_t tabIdx);
    };
}

