/**
 * @file GraphicsModel.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include "GraphicsModel.h"

namespace rose::gm {

    Texture Context::createTexture(Size size) {
        Texture texture{*this, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                        size.w, size.h};
        return std::move(texture);
    }

    int Context::renderCopy(const Texture &texture, Rectangle dst) {
        SDL_Rect dstRect{dst.x, dst.y, dst.w, dst.h};

        return SDL_RenderCopy(mRenderer.get(), texture.get(), nullptr, &dstRect);
    }

    int Context::renderCopy(const Texture &texture, Rectangle src, Rectangle dst) {
        SDL_Rect srcRect{src.x, src.y, src.w, src.h};
        SDL_Rect dstRect{dst.x, dst.y, src.w, src.h};
        return SDL_RenderCopy(mRenderer.get(), texture.get(), &srcRect, &dstRect);
    }

    int Context::renderCopy(const Texture &texture) {
        return SDL_RenderCopy(mRenderer.get(), texture.get(), nullptr, nullptr);
    }

    void Context::copyFullTexture(Texture &source, Texture &destination) {
        RenderTargetGuard renderTargetGuard(*this, destination);
        renderCopy(source);
        destination.setBlendMOde(SDL_BLENDMODE_BLEND);
    }

    int Context::renderCopyEx(Texture &texture, Rectangle src, Rectangle dst, double angle, RenderFlip renderFlip,
                               std::optional<Position> point) const {
        SDL_Rect srcRect{src.x, src.y, src.w, src.h};
        SDL_Rect dstRect{dst.x, dst.y, dst.w, dst.h};
        if (point) {
            SDL_Point sdlPoint;
            sdlPoint.x = point->x;
            sdlPoint.y = point->y;
            return SDL_RenderCopyEx(get(), texture.get(), &srcRect, &dstRect, angle, &sdlPoint, renderFlip.mFlip);
        } else {
            return SDL_RenderCopyEx(get(), texture.get(), &srcRect, &dstRect, angle, nullptr, renderFlip.mFlip);
        }
    }


    Texture::Texture(Context &context, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height)
            : Texture() {
        reset(SDL_CreateTexture(context.get(), format, access, width, height));
//        if (!operator bool()) {
//            throw RoseRuntimeError(
//                    util::StringCompositor("SDL_CreateTexture: (", width, 'x', height, ") -- ", SDL_GetError()));
//        }
    }

    Texture::Texture(Context &context, Size size) : Texture() {
        reset(SDL_CreateTexture(context.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.w,
                                size.h));
//        if (!operator bool())
//            throw RoseRuntimeError(
//                    util::StringCompositor("SDL_CreateTexture: ", size.w, 'x', size.h, ") -- ",
//                                           SDL_GetError()));
    }

    RenderTargetGuard::RenderTargetGuard(Context &context, Texture &texture) : mContext(context) {
        mLastTexture = context.mCurrentRenderTarget;
        context.mCurrentRenderTarget = texture.get();
        status = SDL_SetRenderTarget(context.get(), context.mCurrentRenderTarget);
    }

    int RenderTargetGuard::setRenderTarget(Texture &texture) {
        mContext.mCurrentRenderTarget = texture.get();
        return SDL_SetRenderTarget(mContext.get(), mContext.mCurrentRenderTarget);
    }

    RenderTargetGuard::~RenderTargetGuard() noexcept(false) {
        mContext.mCurrentRenderTarget = mLastTexture;
        status = SDL_SetRenderTarget(mContext.get(), mContext.mCurrentRenderTarget);
    }
}
