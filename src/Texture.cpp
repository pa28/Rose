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
    }

    Texture::Texture(Context &context, Size size) {
        reset(SDL_CreateTexture(context.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.w, size.h));
    }
}