/**
 * @file AntiAliasedDrawing.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-22
 */

#pragma once

#include "Color.h"
#include "Math.h"
#include "Renderer.h"
#include "ScreenMetrics.h"
#include "Texture.h"
#include "Surface.h"

namespace rose {

/**
 * @class AntiAliasedDrawing
 * @brief
 */
    class AntiAliasedDrawing {
    protected:
        static constexpr int Scale = 10;
        color::RGBA mColor{};
        int mWidth{};
        int mScaledWidth{};
        sdl::Texture mTexture{};

    public:
        AntiAliasedDrawing() = default;
        ~AntiAliasedDrawing() = default;

        AntiAliasedDrawing(sdl::Renderer& renderer, int width, color::RGBA rgba);

        void setWidthColor(sdl::Renderer& renderer, int width, color::RGBA rgba);

        void drawLine(sdl::Renderer& renderer, Position p0, Position p1);

        void renderLine(sdl::Renderer& renderer, Position p0, Position p1);
    };
}

