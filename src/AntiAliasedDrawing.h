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
        enum DrawingType {
            SimpleLine,
#if 0
            SimpleRectangle,
            AntiAliased,
#endif
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
#if 0
                case SimpleRectangle: {
                    auto dx = p1.x - p0.x;
                    auto dy = p1.y - p0.y;
                    auto length = util::roundToInt(sqrt((double) (dx * dx) + (double) (dy * dy)));
                    if (length == 0) {
                        return true;
                    }
                    length += 2;

                    float angleRad = atan2((float) dy, (float) dx);
                    float angle = util::rad2deg(angleRad);

                    if (mTexture) {
                        Rectangle src{Position<int>{}, Size{length, mTexture.getSize().h}};
                        Rectangle dst{p0, src.size()};
                        dst.x -= util::roundToInt(cos(angleRad));
                        dst.y -= util::roundToInt(sin(angleRad));
                        return context.renderCopyEx(mTexture, src, dst, angle, gm::RenderFlip{SDL_FLIP_NONE},
                                                    Position{0, src.h / 2}) == 0;
                    } else {
                        return false;
                    }
                }
                case AntiAliased: {
                    auto dx = p1.x - p0.x;
                    auto dy = p1.y - p0.y;
                    auto length = sqrt((double) (dx * dx) + (double) (dy * dy));

                    float angle = util::rad2deg(atan2((float) dy, (float) dx));

                    Rectangle src{0, 0, NubWidth, NubHeight};
                    Rectangle dst{p0.x, p0.y - mWidth * NubHeight, util::roundToInt(length) + 1, mWidth * NubHeight * 2};
                    return context.renderCopyEx(mTexture, src, dst, angle, gm::RenderFlip{SDL_FLIP_NONE},
                                                Position{0, dst.h / 2}) == 0;
                }
#endif
            }
            return false;
        }
    };
}

