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
        static constexpr int NubWidth = 1;
        static constexpr int NubHeight = 3;
        static constexpr int NubColorSize = 1;

        color::RGBA mColor{};
        int mWidth{};
        int mScaledWidth{};
        sdl::Texture mTexture{};

    public:
        AntiAliasedDrawing() = default;
        ~AntiAliasedDrawing() = default;

        AntiAliasedDrawing(sdl::Renderer& renderer, int width, color::RGBA rgba);

        void setWidthColor(sdl::Renderer& renderer, int width, color::RGBA rgba);

        void setColor(sdl::Renderer& renderer, color::RGBA rgba) {
            setWidthColor(renderer, mWidth, rgba);
        }

        bool renderLine(sdl::Renderer& renderer, Position p0, Position p1);
    };
}

