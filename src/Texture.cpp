/**
 * @file Texture.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-12
 */

#include "Texture.h"
#include "Types.h"
#include "GraphicsModel.h"
#include <SDL.h>

namespace rose::gm {

    Texture
    CreateTexture(Context &context, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height) {
        Texture texture{};
        texture.reset(SDL_CreateTexture(context.get(), format, access, width, height));
        return std::move(texture);
    }

    Texture
    CreateTexture(Context &context, Size size) {
        Texture texture{};
        texture.reset(SDL_CreateTexture(context.get(), SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, size.w, size.h));
        return std::move(texture);
    }

    int TextureSetBlendMode(Texture &texture, SDL_BlendMode blendMode) {
        return SDL_SetTextureBlendMode(texture.get(), blendMode);
    }

    Size TextureGetSize(Texture &texture) {
        Size size{};
        SDL_QueryTexture(texture.get(), nullptr, nullptr, &size.w, &size.h);
        return size;
    }
}