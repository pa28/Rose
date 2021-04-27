/**
 * @file Popup.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-24
 */

#pragma once

#include "Callbacks.h"
#include "StructuredTypes.h"
#include "Types.h"
#include "Visual.h"
#include "GraphicsModel.h"
#include "Theme.h"
#include "Frame.h"

namespace rose {

    class PopupWindow : public Window {
    protected:
        std::shared_ptr<Frame> mContentFrame{};
        bool mRemovePopup{false};

    public:
        PopupWindow() {
            mModalWindow = true;
        }

        ~PopupWindow() override = default;

        PopupWindow(const PopupWindow&) = delete;

        PopupWindow(PopupWindow&&) = delete;

        PopupWindow& operator=(const PopupWindow&) = delete;

        PopupWindow& operator=(PopupWindow&&) = delete;

        static constexpr std::string_view id = "PopupWindow";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
            auto rect = screenRect;
            for (auto &content : (*this)) {
                if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                    rect = manager->layout(context, screenRect);
                    manager->setScreenRectangle(rect);
                }
            }
            rect = mPreferredPos;
            return rect;
        }

        constexpr bool removePopup() const noexcept {
            return mRemovePopup;
        }
    };

    class Dialog : public PopupWindow {
    protected:

    public:
        Dialog() = default;

        ~Dialog() override = default;

        Dialog(const PopupWindow&) = delete;

        Dialog(PopupWindow&&) = delete;

        explicit Dialog(const Position& position) : PopupWindow() {
            setPosition(position);
        }

        static constexpr std::string_view id = "Dialog";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        void addedToContainer() override;

        template<class WidgetClass>
        void addWidget(std::shared_ptr<WidgetClass> widget) {
            mContentFrame << widget;
        }
    };
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<rose::Dialog> dialog, std::shared_ptr<WidgetClass> widget) {
    dialog->addWidget(widget);
    return widget;
}

inline std::shared_ptr<rose::Dialog> operator<<(std::shared_ptr<rose::Dialog> dialog, const rose::Position& position) {
    dialog->setPosition(position);
    return dialog;
}

/**
 * @brief An insertion operator to set the preferred Position of a Widget.
 * @tparam WidgetClass The class of the Widget.
 * @param widget The Widget.
 * @param position The Position.
 * @return The Widget.
 */
inline std::shared_ptr<rose::PopupWindow> operator<<(std::shared_ptr<rose::PopupWindow> widget, const rose::Position& position) {
    widget->setPosition(position);
    return widget;
}
