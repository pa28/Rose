/**
 * @file AntiAliasedDrawing.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-22
 */

#pragma once

#include "Color.h"
#include "Math.h"
#include "Surface.h"
#include "Texture.h"

namespace rose {

    /**
     * @class AntiAliasedDrawing
     * @brief
     */
    class AntiAliasedDrawing {
    public:
        enum DrawingType {
            SimpleLine,
            SimpleRectangle,
            AntiAliased,
        };

    protected:
        static constexpr int Scale = 10;
        static constexpr int NubWidth = 1;
        static constexpr int NubHeight = 3;
        static constexpr int NubColorSize = 1;

        DrawingType mDrawingType{SimpleLine};

        color::RGBA mColor{color::RGBA::OpaqueBlack};
        int mWidth{1};
        int mScaledWidth{};
        gm::Texture mTexture{};

    public:
        AntiAliasedDrawing() = default;
        ~AntiAliasedDrawing() = default;

        /**
         * @brief Create an anti-aliased drawing context with a given width and colour.
         * @param context The graphics context to use.
         * @param width The width of the line in pixels.
         * @param rgba The line colour RGBA.
         */
        AntiAliasedDrawing(gm::Context &context, DrawingType drawingType);

        /**
         * @brief Set the width and colour of a line to be drawn.
         * @param context The graphics context to use.
         * @param width The width of the line in pixels.
         * @param rgba The line colour RGBA.
         */
        void setWidthColor(gm::Context &context, int width, color::RGBA rgba);

        /**
         * @brief Set the colour of the line to be drawn.
         * @param context The graphics context to use.
         * @param rgba The line colour RGBA.
         */
        void setColor(gm::Context &context, color::RGBA rgba) {
            setWidthColor(context, mWidth, rgba);
        }

        /**
         * @brief Draw an anti-aliased line.
         * @param context The graphics context to use.
         * @param p0 The start point.
         * @param p1 The end point.
         * @return True if the rendering operation returned success.
         */
        bool renderLine(gm::Context &context, Position p0, Position p1);
    };
}

