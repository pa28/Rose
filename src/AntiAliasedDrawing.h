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
#include "Math.h"
#include "GraphicsModel.h"

namespace rose {

    /**
     * @class AntiAliasedDrawing
     * @brief
     */
    class AntiAliasedDrawing {
    public:
        /**
         * @enum DrawingType
         * @brief The drawing style used by the drawing engine.
         */
        enum DrawingType {
            SimpleLine,         ///< Simple lines, no anti-aliasing.
            AntiAliased,        ///< Modified Wu's algorithm anti-aliaseing.
        };

    protected:
        DrawingType mDrawingType{SimpleLine};

        color::RGBA mColor{color::RGBA::OpaqueBlack};
        int mWidth{1};
        gm::Texture mTexture{};
        Size mWidgetSize{};

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
        void setWidthColor(gm::Context &context, int width, color::RGBA rgba, Size& widgetSize);

        /**
         * @brief Set the colour of the line to be drawn.
         * @param context The graphics context to use.
         * @param rgba The line colour RGBA.
         */
        void setColor(gm::Context &context, color::RGBA rgba, Size& widgetSize) {
            setWidthColor(context, mWidth, rgba, widgetSize);
        }

        /**
         * @brief Implement a modified Xiaolin Wu anti aliasing algorithm.
         * @details Implements the Xiaolin Wu anti aliased line algorithm but inserts interiorWidth stuffing pixels
         * between the two controlled brightness pixes. When interiorWidth is 0 the normal Wu algorithm is used.
         * @param context The Graphics Context used to draw.
         * @param p0 The starting point for the line.
         * @param p1 The ending point for the line.
         * @param interiorWidth Stuffing width of the line.
         */
        void drawLine(gm::Context &context, Position<float> p0, Position<float> p1, int interiorWidth = 0);

        /**
         * @brief Draw an anti-aliased line.
         * @param context The graphics context to use.
         * @param p0 The start point.
         * @param p1 The end point.
         * @return True if the rendering operation returned success.
         */
        template<typename T>
        bool renderLine(gm::Context &context, Position<T> p0, Position<T> p1) {
            switch (mDrawingType) {
                case SimpleLine: {
                    gm::DrawColorGuard drawColorGuard{context, mColor};
                    if constexpr (std::is_integral_v<T>)
                        return SDL_RenderDrawLine(context.get(), p0.x, p0.y, p1.x, p1.y) == 0;
                    else
                        return SDL_RenderDrawLineF(context.get(), p0.x, p0.y, p1.x, p1.y) == 0;
                }
                case AntiAliased: {
                    if constexpr (std::is_integral_v<T>)
                        drawLine(context, p0.template as<float>(), p1.template as<float>(), mWidth);
                    else
                        drawLine(context, p0, p1, mWidth);
                    return true;
                }
            }
            return false;
        }
    };
}

