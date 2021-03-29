/**
 * @file Border.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-07
 */

#pragma once

#include "SingleChild.h"

namespace rose {

    /**
     * @class Border
     * @brief Manage a single child Widget with individual padding on each side.
     */
    class Border : public SingleChild {
    protected:
        std::optional<Padding> mPadding{};    ///< The padding around the child.

    public:
        Border();
        Border(const Border &) = delete;
        Border(Border &&) = delete;
        Border& operator=(const Border &) = delete;
        Border& operator=(Border &&) = delete;
        ~Border() override = default;

        /**
         * @brief Constructor
         * @details Set the padding to the same value on all sides.
         * @param padding The padding value.
         */
        explicit Border(int padding) noexcept : mPadding(padding) {}

        /**
         * @brief Constructor
         * @details Set the padding to the provided Padding object value.
         * @param padding The padding value.
         */
        explicit Border(Padding padding) noexcept : mPadding(padding) {}

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /// See Widget::widgetLayout()
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /// See Widget::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /// Set Padding
        void setPadding(Padding padding) { mPadding = padding; }

        /// Access Border Padding.
        auto getPadding() const noexcept { return mPadding; }

        /**
         * @brief Get the interior (managed) size of a Container
         * @return
         */
//        Rectangle interiorRectangle() override {
//            Rectangle rect{};
//            if (mLayoutHints.mAssignedRect) {
//                rect = mLayoutHints.mAssignedRect.value();
//            } else if (mSize) {
//                rect = mSize.value();
//                if (mPos)
//                    rect = mPos.value();
//            }
//
//            rect.x() += mPadding->left();
//            rect.y() += mPadding->top();
//            rect.width() -= mPadding->size();
//            rect.height() -= mPadding->height();
//            return rect;
//        }
    };
}

