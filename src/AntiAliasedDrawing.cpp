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

    AntiAliasedDrawing::AntiAliasedDrawing(gm::Context &context, int width, color::RGBA rgba) {
        setWidthColor(context, width, rgba);
    }

    void AntiAliasedDrawing::setWidthColor(gm::Context &context, int width, color::RGBA rgba) {
        mColor = rgba;
        mWidth = width;

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

    bool AntiAliasedDrawing::renderLine(gm::Context &context, Position p0, Position p1) {
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
