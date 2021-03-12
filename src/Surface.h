/**
 * @file Surface.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#pragma once

#include <exception>
#include <filesystem>
#include <memory>
#include <SDL.h>
#include "Color.h"
#include "Texture.h"
#include "Types.h"

namespace rose::gm {

    class Context;

    class SurfaceRuntimeError : public std::runtime_error {
    public:
        ~SurfaceRuntimeError() override = default;

        explicit SurfaceRuntimeError(const std::string &what) : std::runtime_error(what) {}
        explicit SurfaceRuntimeError(const char *what) : std::runtime_error(what) {}
    };

    /**
* @brief A functor to destroy an SDL_Surface
*/
    class SurfaceDestroy {
    public:
        /**
         * @brief Call the SDL API to free the SDL_Surface.
         * @param sdlSurface A pointer to the SDL_Surface to free.
         */
        void operator()(SDL_Surface *sdlSurface) {
            SDL_FreeSurface(sdlSurface);
        }
    };

    /**
     * @class Surface
     * @brief A wrapper class for SDL_Surface pointers.
     * @details Provide a std::unique_ptr storage type for SDL_Surface pointers.
     */
    class Surface : public std::unique_ptr<SDL_Surface, SurfaceDestroy> {
    public:
        /**
         * @brief Create an empty Surface.
         */
        Surface() = default;

        /**
         * @brief Create a Surface initialized with an SDL_Surface pointer.
         * @param surface The SDL_Surface pointer.
         */
        explicit Surface(SDL_Surface *surface) : std::unique_ptr<SDL_Surface, SurfaceDestroy>(surface) {}

        /**
         * @brief Create a surface from a file.
         * @param path The path to the file to load.
         */
        explicit Surface(std::filesystem::path &path);

        /**
         * @brief Constructor. Create a surface using SDL_CreateRGBSurfaceWithFormat()
         * @param width The width of the surface.
         * @param height The height of the surface.
         * @param depth The pixel depth, defaults to 32.
         * @param format The format, defaults to SDL_PIXELFORMAT_RGBA8888
         */
        Surface(int width, int height, int depth = 32, SDL_PixelFormatEnum format = SDL_PIXELFORMAT_RGBA8888);

        explicit Surface(Size size, int depth = 32, SDL_PixelFormatEnum format = SDL_PIXELFORMAT_RGBA8888)
                : Surface(size.w, size.h, depth, format) {}

        Surface(int width, int height, int depth, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask);

        /**
         * @brief Provide access to a pixel of the Surface.
         * @details The co-ordinates are not checked for out of range values.
         * @param x The X co-ordinate.
         * @param y The Y co-ordinate.
         * @return A reference to the pixel.
         */
        uint32_t &pixel(int x, int y);

        /**
         * @brief Get a pixel color of the Surface.
         * @param x The x co-ordinate.
         * @param y The y co-ordinate.
         * @return The color as an RGBA.
         */
        color::RGBA color(int x, int y);

        /**
         * @brief Set a pixel color of the Surface.
         * @param x The x co-ordinate.
         * @param y The y co-ordinate.
         * @param color the color as an RGBA.
         */
        void setColor(int x, int y, color::RGBA color);

        /**
         * @brief Create a surface using SDL_CreateRGBSurfaceWithFormat()
         * @param width The width of the surface.
         * @param height The height of the surface.
         * @param depth The pixel depth, defaults to 32.
         * @param format The format, defaults to SDL_PIXELFORMAT_RGBA8888
         * @return True if the surface was created, false otherwise.
         */
        bool
        createWithFormat(int width, int height, int depth = 32, SDL_PixelFormatEnum format = SDL_PIXELFORMAT_RGBA8888);

        /**
         * @brief Fill a rectangle using SDL_FillRect()
         * @param rect The Rectangle to fill.
         * @param color The fill Color.
         * @return The return status value.
         */
        int fillRectangle(const Rectangle &rect, const color::RGBA &color);

        /**
         * @brief Fill the Surface using SDL_FillRect()
         * @param color The fill Color.
         * @return The return status value.
         */
        int fillRectangle(const color::RGBA &color);

        /**
         * @brief Create an SDL_Texture from the Surface and set it in the Texture object.
         * @param texture The Texture object.
         * @return True if the SDL_Texture was created.
         */
        bool textureFromSurface(Context &context, Texture &texture);

        /**
         * @brief Create an SDL_Texture from the surface.
         * @param context The Renderer used.
         * @return The generatoed Texture.
         */
        Texture toTexture(Context &context);

        /**
         * @brief Set the Surfacle SDL_BlendMode.
         * @param blendMode The blend mode, a value from SDL_BlendMode enum.
         * @return The return status from SDL_SetSurfaceBlendMode().
         */
        int setBlendMode(SDL_BlendMode blendMode) noexcept;

        /**
         * @brief Blit the contents of the source Surface to this surface.
         * @param source The source Surface.
         * @return the SDL_Status return code.
         */
        int blitSurface(Surface &source);
    };

    /**
     * @class SurfaceLock
     * @brief A helper class to wrap the SDL_LockSurface and SDL_UnlockSurface API calls.
     */
    class SurfaceLock {
    protected:
        int status{0};          ///< The return status returned by SDL_LockSurface.
        SDL_Surface *mSurface;  ///< The surface being locked.

    public:
        /**
         * @brief Unlock the surface on destruction.
         */
        ~SurfaceLock();

        SurfaceLock() = delete; ///< Deleted default constructor.

        /**
         * @brief Construct a SurfaceLock.
         * @param surface The surface that will be locked.
         */
        explicit SurfaceLock(SDL_Surface *surface);

        /**
         * @brief Check the validity of the surface lock
         * @return True if SDL_LockSurface returned 0.
         */
        explicit operator bool() const noexcept {
            return status == 0;
        }
    };

}

