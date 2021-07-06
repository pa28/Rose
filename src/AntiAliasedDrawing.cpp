/**
 * @file AntiAliasedDrawing.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-22
 */

#include "AntiAliasedDrawing.h"
#include "Math.h"
#include "GraphicsModel.h"

namespace rose {

    AntiAliasedDrawing::AntiAliasedDrawing(gm::Context &context, DrawingType drawingType) {
        mDrawingType = drawingType;
    }

    void AntiAliasedDrawing::setWidthColor(gm::Context &context, int width, color::RGBA rgba, Size& widgetSize) {
        if (mColor != rgba || mWidth != width || mWidgetSize != widgetSize) {
            mTexture.reset();
        }

        mColor = rgba;
        mWidth = width;
        mWidgetSize = widgetSize;


        switch (mDrawingType) {
            case SimpleLine:
                break;
            case SimpleRectangle: {
                mTexture = gm::Texture{context, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                       std::max(widgetSize.w, widgetSize.h) + 2, mWidth + 2};
                gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                auto transparent = rgba;
                transparent.a() = 0.;
                context.fillRect(Rectangle{0, 0, mTexture.getSize().w, mTexture.getSize().h}, transparent);
                context.fillRect(Rectangle{0, 1, mTexture.getSize().w, mTexture.getSize().h - 2},
                                 mColor);
            }
                break;
            case AntiAliased: {
                mTexture = gm::Texture{context, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                       NubWidth, NubHeight};
                gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                auto transparent = rgba;
                transparent.a() = 0.;
                context.fillRect(Rectangle{0, 0, NubWidth, NubHeight}, transparent);
                context.fillRect(Rectangle{0, (NubHeight - NubColorSize) / 2, NubWidth, (NubHeight - NubColorSize) / 2},
                                 mColor);
            }
                break;
        }
    }

    bool AntiAliasedDrawing::renderLine(gm::Context &context, Position p0, Position p1) {
        switch (mDrawingType) {
            case SimpleLine: {
                gm::DrawColorGuard drawColorGuard{context, mColor};
                return SDL_RenderDrawLine(context.get(), p0.x, p0.y, p1.x, p1.y) == 0;
            }
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
                    Rectangle src{Position::Zero, Size{length, mTexture.getSize().h}};
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
        }
        return false;
    }
}
