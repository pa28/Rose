/**
 * @file AntiAliasedDrawing.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-22
 */

#include "AntiAliasedDrawing.h"
#include "Math.h"

namespace rose {

    AntiAliasedDrawing::AntiAliasedDrawing(sdl::Renderer &renderer, int width, color::RGBA rgba) {
        setWidthColor(renderer, width, rgba);
    }

    void AntiAliasedDrawing::setWidthColor(sdl::Renderer &renderer, int width, color::RGBA rgba) {
        mColor = rgba;
        mWidth = width;

        mTexture = sdl::Texture{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                NubWidth, NubHeight};
        sdl::RenderTargetGuard renderTargetGuard{renderer, mTexture};
        mTexture.setBlendMOde(SDL_BLENDMODE_BLEND);
        auto transparent = rgba;
        transparent.a() = 0.;
        renderer.fillRect(Rectangle{0, 0, NubWidth, NubHeight}, transparent);
        renderer.fillRect(Rectangle{0, (NubHeight - NubColorSize) / 2, NubWidth, (NubHeight - NubColorSize) / 2},
                          mColor);
    }

    bool AntiAliasedDrawing::renderLine(sdl::Renderer &renderer, Position p0, Position p1) {
        auto dx = p1.x() - p0.x();
        auto dy = p1.y() - p0.y();
        auto length = sqrt((double) (dx * dx) + (double) (dy * dy));

        float angle = util::rad2deg(atan2((float) dy, (float) dx));

        Rectangle src{0, 0, NubWidth, NubHeight};
        Rectangle dst{p0.x(), p0.y() - mWidth * NubHeight, util::roundToInt(length) + 1, mWidth * NubHeight * 2};
        return renderer.renderCopyEx(mTexture, src, dst, angle, sdl::RenderFlip{SDL_FLIP_NONE},
                                     Position{0, dst.height() / 2}) == 0;
    }
}
