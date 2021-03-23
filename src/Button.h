/**
 * @file Button.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#pragma once

#include "Frame.h"
#include "PointerInteractions.h"

namespace rose {

    /**
     * @class ButtonFrame
     * @brief A frame that supports ButtonSemantics
     */
    class ButtonFrame : public Frame {
    protected:
        ButtonSemantics mButtonSemantics;

        ButtonDisplayCallback mButtonDisplayCallback{};
        ButtonStateChangeCallback mButtonStateChangeCallback{};

    public:
        ButtonFrame() noexcept;

        ~ButtonFrame() override = default;

        ButtonFrame(const ButtonFrame&) = delete;

        ButtonFrame(ButtonFrame&&) = delete;

        ButtonFrame& operator=(const ButtonFrame&) = delete;

        ButtonFrame& operator=(ButtonFrame&&) = delete;

        explicit ButtonFrame(int padding) noexcept;

        /// Draw the screen contents.
        void draw(gm::Context &context, const Position &containerPosition) override {
            std::cout << __PRETTY_FUNCTION__ << '\n';
            Frame::drawAnimate(context, containerPosition);
        }

        /// Layout the screen contents.
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
            std::cout << __PRETTY_FUNCTION__ << '\n';
            return Frame::layout(context, screenRect);
        }

    };

    /**
     * @class Button
     * @brief
     */
    class Button {

    };
}

