/**
 * @file Texture.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#include "Color.h"
#include "Renderer.h"
#include "Texture.h"

namespace rose::sdl {

    Texture::Texture(Renderer &renderer, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height)
            : Texture() {
        reset(SDL_CreateTexture(renderer.get(), format, access, width, height));
    }

    GradientTexture::GradientTexture(Renderer &renderer, const color::RGBA &topLeft, const color::RGBA &topRight,
                                     const color::RGBA &bottomLeft, const color::RGBA &bottomRight) {
        reset(SDL_CreateTexture(renderer.get(),SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING,4,4));
        if (operator bool())
            setColors(topLeft, topRight, bottomLeft, bottomRight);
    }

    GradientTexture::GradientTexture(Renderer &renderer, const color::RGBA &start, const color::RGBA &end,
                                     Orientation orientation) {
        reset(SDL_CreateTexture(renderer.get(),SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING,4,4));

        if (operator bool())
            setColors(start, end, orientation);
    }

    void GradientTexture::setColors(const color::RGBA &topLeft, const color::RGBA &topRight, const color::RGBA &bottomLeft,
                                    const color::RGBA &bottomRight) {
        PixelFormat pixelFormat{SDL_PIXELFORMAT_RGBA8888};
        auto A = mapRGBA(pixelFormat, topLeft);
        auto B = mapRGBA(pixelFormat, topRight);
        auto C = mapRGBA(pixelFormat, bottomLeft);
        auto D = mapRGBA(pixelFormat, bottomRight);

        uint32_t * bgpixels;
        int i;
        //AABB
        //AABB
        //CCDD
        //CCDD
        SDL_LockTexture(get(), nullptr, (void**)&bgpixels, &i);
        bgpixels[0] = A;
        bgpixels[1] = A;
        bgpixels[2] = B;
        bgpixels[3] = B;
        bgpixels[4] = A;
        bgpixels[5] = A;
        bgpixels[6] = B;
        bgpixels[7] = B;
        bgpixels[8] = C;
        bgpixels[9] = C;
        bgpixels[10] = D;
        bgpixels[11] = D;
        bgpixels[12] = C;
        bgpixels[13] = C;
        bgpixels[14] = D;
        bgpixels[15] = D;
        SDL_UnlockTexture(get());
    }

    void GradientTexture::setColors(const color::RGBA &start, const color::RGBA &end, Orientation orientation) {
        switch (orientation) {
            case Orientation::Unset:
            case Orientation::Horizontal:
                return setColors(start, end, start, end);
            case Orientation::Vertical:
                return setColors(start, start, end, end);
        }
        return setColors(start, end, start, end);
    }

    uint32_t mapRGBA(SDL_PixelFormatEnum pixelFormat, const color::RGBA &color) {
        PixelFormat format{pixelFormat};
        return mapRGBA(format, color);
    }

    uint32_t mapRGBA(PixelFormat &pixelFormat, const color::RGBA &color) {
        return mapRGBA(pixelFormat.get(), color);
    }

    uint32_t mapRGBA(SDL_PixelFormat *format, const color::RGBA &color) {
        auto c = color.toSdlColor();
        return SDL_MapRGBA(format, c.r, c.g, c.b, c.a);
    }

    color::RGBA getRGBA(PixelFormat &pixelFormat, uint32_t pixel) {
        return getRGBA(pixelFormat.get(), pixel);
    }

    color::RGBA getRGBA(SDL_PixelFormat *format, uint32_t pixel) {
        uint8_t r, g, b, a;
        SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
        return color::RGBA{r, g, b, a};
    }
}
