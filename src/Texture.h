/**
 * @file Texture.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-12
 */

#pragma once

#include <memory>
#include <map>
#include <SDL.h>
#include "Types.h"

namespace rose::gm {

    class TextureRuntimeError : public std::runtime_error {
    public:
        ~TextureRuntimeError() override = default;

        explicit TextureRuntimeError(const std::string &what) : std::runtime_error(what) {}
        explicit TextureRuntimeError(const char *what) : std::runtime_error(what) {}
    };

    /**
     * @brief A functor to destroy an SDL_Texture in a std::unique_ptr (rose::sdl::Texture)
     */
    class TextureDestroy {
    public:
        /**
         * @brief Call the SDL API to destroy an SDL_Texture.
         * @param sdlTexture A pointer to the SDL_Texture to destroy.
         */
        void operator()(SDL_Texture *sdlTexture) {
            if (sdlTexture != nullptr)
                SDL_DestroyTexture(sdlTexture);
        }
    };

    class Context;

    /**
     * @class Texture
     * @brief Abstraction of SDL_Texture
     */
    class Texture : public std::unique_ptr<SDL_Texture,TextureDestroy> {
    public:
        Texture() = default;

        Texture(const Texture&) = delete;
        Texture(Texture &&) = default;
        Texture& operator=(const Texture &) = delete;
        Texture& operator=(Texture &&) = default;

        /**
         * @breif Create a Texture
         * @param context The Context to use.
         * @param format The pixel format from SDL_PixelFormatEnum.
         * @param access The Texture access from SDL_TextureAccess.
         * @param width The width of the texture.
         * @param height The height of the texture.
         */
        Texture(Context& context, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height);

        /**
         * @brief Create a Texture
         * @details Builds a Texture compatible with building up textures within Rose . The pixel format is
         * SDL_PIXELFORMAT_RGBA8888, the texture access is SDL_TEXTUREACCESS_TARGET.
         * @param context The renderer to use.
         * @param size The size of the texture.
         */
        Texture(Context &context, Size size);

        int setBlendMode(SDL_BlendMode blendMode) {
            return SDL_SetTextureBlendMode(get(), blendMode);
        }

        [[nodiscard]] Size getSize() const {
            Size size{};
            SDL_QueryTexture(get(), nullptr, nullptr, &size.w, &size.h);
            return size;
        }

        int setAlphaMod(float alpha);
    };
}
