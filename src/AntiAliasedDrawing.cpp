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
                                1, 3};
        sdl::RenderTargetGuard renderTargetGuard{renderer, mTexture};
        mTexture.setBlendMOde(SDL_BLENDMODE_BLEND);
        auto transparent = rgba;
        transparent.a() = 0.;
        renderer.fillRect( Rectangle{0, 0, 1, 3}, transparent);
        renderer.fillRect(Rectangle{0,1, 1, 1}, mColor);
    }

    void AntiAliasedDrawing::drawLine(sdl::Renderer& renderer, Position p0, Position p1) {
        Rectangle dst{p0.x(), p0.y(), mWidth, mWidth * 6};
        Rectangle src{0, 0, 1, 3};
        auto dx = abs(p1.x() - p0.x());
        auto sx = p0.x() < p1.x() ? 1 : -1;
        auto dy = -abs(p1.y() - p0.y());
        auto sy = p0.y() < p1.y() ? 1 : -1;
        auto err = dx + dy;
        float angle = util::rad2deg(atan2((float)(p0.x() - p1.x()), (float)(p0.y() - p1.y())));
        while (true) {
            renderer.renderCopyEx(mTexture, src, dst, angle, sdl::RenderFlip{SDL_FLIP_NONE});
//            renderer.renderCopy(mTexture, dst);
            if (dst.x() == p1.x() && dst.y() == p1.y()) break;
            auto e2 = 2 * err;

            if (e2 > dy) {
                err += dy;
                dst.x() += sx;
            }

            if (e2 <= dx) {
                err += dx;
                dst.y() += sy;
            }
        }
    }

    void AntiAliasedDrawing::renderLine(sdl::Renderer &renderer, Position p0, Position p1) {
        auto dx = p1.x() - p0.x();
        auto dy = p1.y() - p0.y();
        auto length = sqrt((double)(dx * dx) + (double)(dy * dy));

        float angle = util::rad2deg(atan2((float)dy, (float)dx));
        float angle2 = util::rad2deg(asin(double (dy) / length));

        Rectangle src{0, 0, 1, 3};
        Rectangle dst{p0.x(), p0.y() - mWidth * 3, util::roundToInt(length)+1, mWidth * 6};
        renderer.renderCopyEx(mTexture, src, dst, angle, sdl::RenderFlip{SDL_FLIP_NONE}, Position{0, dst.height()/2});
    }
}
