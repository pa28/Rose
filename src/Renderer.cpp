/**
 * @file Renderer.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#include "Renderer.h"
#include "Texture.h"

namespace rose::sdl {

    Renderer::Renderer(Window &window, int index, Uint32 flags)
            : mRenderer(SDL_CreateRenderer(window.get(), index, flags)) {}

    Texture Renderer::createTexture(Size size) {
        Texture texture{*this, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                        size.width(), size.height()};
        return std::move(texture);
    }

    int Renderer::pushRenderTarget(Texture &texture) {
        mTargetTextureStack.push(texture.get());
        return SDL_SetRenderTarget(mRenderer.get(), texture.get());
    }

    int Renderer::renderCopy(const Texture &texture, Rectangle dst) {
        SDL_Rect dstRect = dst.toSdlRect();
        return SDL_RenderCopy(mRenderer.get(), texture.get(), nullptr, &dstRect);
    }

    int Renderer::renderCopy(const Texture &texture) {
        return SDL_RenderCopy(mRenderer.get(), texture.get(), nullptr, nullptr);
    }

    int Renderer::fillRect(Rectangle rectangle, color::RGBA color) {
        DrawColorGuard drawColorGuard(*this, color);
        auto rect = rectangle.toSdlRect();
        auto res = SDL_RenderFillRect(mRenderer.get(), &rect);
        return res;
    }

    int Renderer::renderCopy(GradientTexture &texture, Rectangle dst) {
        SDL_Rect dstRect = dst.toSdlRect();
        SDL_Rect srcRect{1, 1, 2, 2};
        return SDL_RenderCopy(mRenderer.get(), texture.get(), &srcRect, &dstRect);
    }

    void Renderer::copyFullTexture(sdl::Texture &source, sdl::Texture &destination) {
        sdl::RenderTargetGuard renderTargetGuard(*this, destination);
        renderCopy(source);
        destination.setBlendMOde(SDL_BLENDMODE_BLEND);
    }

    int Renderer::renderCopyEx(Texture &texture, Rectangle src, Rectangle dst, double angle, RenderFlip renderFlip,
                               std::optional<Position> point) {
        auto srcRect = src.toSdlRect();
        auto dstRect = dst.toSdlRect();
        if (point) {
            SDL_Point sdlPoint;
            sdlPoint.x = point->x();
            sdlPoint.y = point->y();
            return SDL_RenderCopyEx(get(), texture.get(), &srcRect, &dstRect, angle, &sdlPoint, renderFlip.mFlip);
        } else {
            return SDL_RenderCopyEx(get(), texture.get(), &srcRect, &dstRect, angle, nullptr, renderFlip.mFlip);
        }
    }

    DrawColorGuard::DrawColorGuard(Renderer &renderer, SDL_Color color) : mRenderer(renderer) {
        mStatus = 0;
        if (int status = SDL_GetRenderDrawColor( mRenderer.get(), &mOldColor.r, &mOldColor.g,
                                                 &mOldColor.b, &mOldColor.a); status == 0 ) {
            mStatus = SDL_SetRenderDrawColor( mRenderer.get(), color.r, color.g, color.b, color.a);
        } else {
            mStatus = status;
        }
    }

    DrawColorGuard::DrawColorGuard(Renderer &renderer, color::RGBA color) : DrawColorGuard(renderer,
                                                                                           color.toSdlColor()) {}

    int DrawColorGuard::setDrawColor(SDL_Color color) {
        return SDL_SetRenderDrawColor(mRenderer.get(), color.r, color.g, color.b, color.a);
    }

    int RenderTargetGuard::setRenderTarget(Texture &texture) {
        status = SDL_SetRenderTarget(mRenderer.get(), texture.get());
        return status;
    }
}
