/**
 * @file Surface.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include "Renderer.h"
#include "Surface.h"
#include "Texture.h"

namespace rose::sdl {

    Surface::Surface(std::filesystem::path &path) : Surface() {
        reset(IMG_Load(path.c_str()));
    }

    Surface::Surface(int width, int height, int depth, SDL_PixelFormatEnum format) : Surface() {
        reset(SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format));
    }

    Surface::Surface(int width, int height, int depth, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask) : Surface() {
        reset(SDL_CreateRGBSurface(0, width, height, depth, rmask, gmask, bmask, amask));
    }

    uint32_t &Surface::pixel(int x, int y) {
        auto *pixels = (Uint32 *) get()->pixels;
        return pixels[(y * get()->w) + x];
    }

    bool Surface::createWithFormat(int width, int height, int depth, SDL_PixelFormatEnum format) {
        reset(SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format));
        return operator bool();
    }

    int Surface::fillRectangle(const Rectangle &rect, const color::RGBA &color) {
        auto c = color.toSdlColor();
        auto r = rect.toSdlRect();
        return SDL_FillRect(get(), &r, SDL_MapRGBA(get()->format, c.r, c.g, c.b, c.a));
    }

    int Surface::fillRectangle(const color::RGBA &color) {
        auto c = color.toSdlColor();
        return SDL_FillRect(get(), nullptr, SDL_MapRGBA(get()->format, c.r, c.g, c.b, c.a));
    }

    bool Surface::textureFromSurface(Renderer &renderer, Texture &texture) {
        texture.reset(SDL_CreateTextureFromSurface(renderer.get(), get()));
        return texture.operator bool();
    }

    Texture Surface::toTexture(Renderer &renderer) {
        return std::move(Texture{SDL_CreateTextureFromSurface(renderer.get(), get())});
    }

    int Surface::setBlendMode(SDL_BlendMode blendMode) noexcept {
        return SDL_SetSurfaceBlendMode(get(), blendMode);
    }

    int Surface::blitSurface(Surface &source) {
        return SDL_BlitSurface(source.get(), nullptr, get(), nullptr);
    }

    SurfaceLock::~SurfaceLock() {
        SDL_UnlockSurface(mSurface);
    }

    SurfaceLock::SurfaceLock(SDL_Surface *surface) : mSurface(surface) {
        status = SDL_LockSurface(mSurface);
    }
}
