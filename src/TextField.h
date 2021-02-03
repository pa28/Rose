/**
 * @file TextField.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#pragma once

#include <regex>
#include "Frame.h"

namespace rose {

/**
 * @class TextField
 * @brief
 */
    class TextField : public Frame {
        std::regex mValidationPattern{};                    ///< Regular expression to validate content.

    public:
        TextField() = default;
        ~TextField() override = default;
        TextField(TextField &&) = delete;
        TextField(const TextField &) = delete;
        TextField& operator=(TextField &&) = delete;
        TextField& operator=(const TextField &) = delete;

        /// See Frame::widgetLayout()
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /// See Frame::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /// See Widget::initializeComposite();
        void initializeComposite() override;
    };
}

