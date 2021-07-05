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

    void AntiAliasedDrawing::setWidthColor(gm::Context &context, int width, color::RGBA rgba) {
        mColor = rgba;
        mWidth = width;

        switch (mDrawingType) {
            case SimpleLine:
            case SimpleRectangle:
                if (mTexture)
                    mTexture.reset();
                break;
            case AntiAliased:
                mTexture = gm::Texture{context, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                       NubWidth, NubHeight};
                gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                auto transparent = rgba;
                transparent.a() = 0.;
                context.fillRect(Rectangle{0, 0, NubWidth, NubHeight}, transparent);
                context.fillRect(Rectangle{0, (NubHeight - NubColorSize) / 2, NubWidth, (NubHeight - NubColorSize) / 2},
                                 mColor);
                break;
        }
    }

    bool AntiAliasedDrawing::renderLine(gm::Context &context, Position p0, Position p1) {
        std::cout << __PRETTY_FUNCTION__ << "Start\n";
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

                Size textureSize{length, mWidth + 2};
                if (mTexture) {
                    auto size = mTexture.getSize();
                    if (textureSize.w != size.w || textureSize.h != size.w)
                        mTexture.reset();
                }

                if (!mTexture) {
                    mTexture = gm::Texture{context, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                           length, mWidth + 2};
                    if (mTexture) {
                        mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                        gm::RenderTargetGuard renderTargetGuard{context, mTexture};

                        auto transparent = mColor;
                        transparent.a() = 0.;

                        gm::DrawColorGuard drawColorGuard{context, transparent};
                        context.renderClear();

                        context.fillRect(Rectangle{0, 1, length, mWidth}, mColor);
                    } else {
                        std::cout << __PRETTY_FUNCTION__ << "End\n";
                        return false;
                    }
                }

                Rectangle src{Position::Zero, textureSize};
                Rectangle dst{p0, textureSize};
                dst.x -= cos(angleRad);
                dst.y -= sin(angleRad);
                std::cout << __PRETTY_FUNCTION__ << "End\n";
                return context.renderCopyEx(mTexture, src, dst, angle, gm::RenderFlip{SDL_FLIP_NONE},
                                            Position{0, src.h / 2}) == 0;
            }
            case AntiAliased: {
                auto dx = p1.x - p0.x;
                auto dy = p1.y - p0.y;
                auto length = sqrt((double) (dx * dx) + (double) (dy * dy));

                float angle = util::rad2deg(atan2((float) dy, (float) dx));

                Rectangle src{0, 0, NubWidth, NubHeight};
                Rectangle dst{p0.x, p0.y - mWidth * NubHeight, util::roundToInt(length) + 1, mWidth * NubHeight * 2};
                std::cout << __PRETTY_FUNCTION__ << "End\n";
                return context.renderCopyEx(mTexture, src, dst, angle, gm::RenderFlip{SDL_FLIP_NONE},
                                            Position{0, dst.h / 2}) == 0;
            }
        }
        std::cout << __PRETTY_FUNCTION__ << "End\n";
        return false;
    }
}
