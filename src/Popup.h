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
            setSize(Size{200,200});
            setPosition(Position{300, 100});
            mModalWindow = true;
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

        void addedToContainer() override;

        constexpr bool removePopup() const noexcept {
            return mRemovePopup;
        }

        ~PopupWindow() override = default;
    };
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
