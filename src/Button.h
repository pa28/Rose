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
        ButtonFrame();

        ~ButtonFrame() override = default;

        ButtonFrame(const ButtonFrame&) = delete;

        ButtonFrame(ButtonFrame&&) = delete;

        ButtonFrame& operator=(const ButtonFrame&) = delete;

        ButtonFrame& operator=(ButtonFrame&&) = delete;

    };

    /**
     * @class Button
     * @brief
     */
    class Button {

    };
}

