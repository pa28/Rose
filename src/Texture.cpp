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

    Texture::Texture(Context &context, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height) {
        reset(SDL_CreateTexture(context.get(), format, access, width, height));
        if (!operator bool()) {
            throw TextureRuntimeError(
                    StringCompositor("SDL_CreateTexture: (", width, 'x', height, ") -- ",
                                     SDL_GetError()));
        }
    }

    Texture::Texture(Context &context, Size size) {
        reset(SDL_CreateTexture(context.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.w, size.h));
        if (!operator bool()) {
            throw TextureRuntimeError(
                    StringCompositor("SDL_CreateTexture: (", size.w, 'x', size.h, ") -- ",
                                     SDL_GetError()));
        }
    }

    int Texture::setAlphaMod(float alpha) {
        uint8_t alphaMod = static_cast<uint8_t>(255.f * std::clamp(alpha, 0.f, 1.f));
        return SDL_SetTextureAlphaMod(get(), alphaMod);
    }
}
