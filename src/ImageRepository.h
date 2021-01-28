#pragma once

/** @file ImageRepository.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-02
  * @brief The Sprite Class.
  *
  * A sprite is a non-layout, self positioning object that moves at the application frame rate following
  * movement commands or splines.
  *
  */



#include <chrono>
#include <future>
#include <iterator>
#include <map>
#include <vector>
#include <type_traits>
#include "Renderer.h"
#include "Surface.h"
#include "Texture.h"
#include "Utilities.h"

namespace rose {
    using namespace std;

    /**
     * @class ImageRepository
     * @brief A cache of texture data structures that may be shared by other widgets to
     * render textures.
     */
    class ImageRepository {
    public:

        /**
         * @brief The store of TextureData objects.
         */
        using ImageStore = map<ImageId, sdl::TextureData>;

        /**
         * @brief The store of surfaces to be made into textures.
         */
        using SurfaceStore = map<ImageId, sdl::Surface>;

        static constexpr ImageId InvalidImageId = 0x0u;     ///< An invalid image ID which may be used for initialization

    protected:
        static constexpr ImageId FirstUserImageId = 0x100u; ///< This and all following image IDs are available to user applications
        ImageId mNextUserImageId;                           ///< A variable to keep track of used image IDs

        ImageStore mImageStore{};       ///< The texture cache
        SurfaceStore mSurfaceStore{};   ///< The surface cache

        /**
         * @brief Check for a future for an image and get the data if available.
         * @param renderer The renderer used to convert the generated surface to a texture.
         * @param imageId the image identifier
         */
        void getSurface(sdl::Renderer &renderer, ImageId imageId) {
            if (!mSurfaceStore.empty()) {
                if (auto surface = mSurfaceStore.find(imageId); surface != mSurfaceStore.end()) {
                    if (!mImageStore.empty() && mImageStore.find(imageId) == mImageStore.end()) {
                        sdl::TextureData textureData{surface->second.toTexture(renderer)};
                        textureData.setLoadedTime(std::chrono::system_clock::now());
                        mImageStore[imageId] = std::move(textureData);
                    } else {
                        mImageStore.at(imageId) = surface->second.toTexture(renderer).release();
                        mImageStore.at(imageId).setLoadedTime(std::chrono::system_clock::now());
                    }
                    mSurfaceStore.erase(imageId);
                }
            }
        }

    public:
        ImageRepository() : mNextUserImageId{FirstUserImageId} {}

        ~ImageRepository() = default;

        /**
         * @brief Get the next available ImageId.
         * @return the next ImageId.
         */
        ImageId getImageId() {
            return mNextUserImageId++;
        }

        /**
         * @brief Access the TextureData object for an ImageId.
         * @details Throws a std::out_of_range if no such ImageId is in the cache.
         * @param imageId the image identifier
         * @return a reference to the TextureData object.
         */
        sdl::TextureData &image(ImageId imageId) {
            return mImageStore.at(imageId);
        }

        /**
         * @brief Const access the TextureData object for an ImageId.
         * @details Throws a std::out_of_range if no such ImageId is in the cache.
         * @param imageId the image identifier
         * @return a const reference to the TextureData object.
         */
        [[nodiscard]] const sdl::TextureData &image(ImageId imageId) const {
            return (mImageStore.at(imageId));
        }

        /**
         * @brief Access the name associated with the TextureData object.
         * @details Throws a std::out_of_range if no such ImageId is in the cache.
         * @param imageId the image identifier.
         * @return a const std::string_view of the image name.
         */
        [[nodiscard]] string_view imageName(ImageId imageId) const {
            return mImageStore.at(imageId).getName();
        }

        /**
         * @brief Access the file path associated with the TextureData object.
         * @details Throws a std::out_of_range if no such ImageId is in the cache.
         * @param imageId the image identifier.
         * @return a const std::string_view of the image path.
         */
        [[nodiscard]] string_view imagePath(ImageId imageId) const {
            return mImageStore.at(imageId).getPath();
        }

        /**
         * @brief Access the URI associated with the TextureData object.
         * @details Throws a std::out_of_range if no such ImageId is in the cache.
         * @param imageId the image identifier.
         * @return a const std::string_view of the image URI.
         */
        [[nodiscard]] string_view imageURI(ImageId imageId) const {
            return mImageStore.at(imageId).getPath();
        }

        /**
         * @brief Render the texture
         * @details The texture is rendered using the supplied renderer, source and paint rectangles.
         * Throws a std::out_of_range if no such ImageId is in the cache.
         * @param renderer the Renderer to use.
         * @param imageId the image identifier.
         * @param imgSrcRect the source SDL_Rectangle
         * @param imgPaintRect the destination SDL_Rectangle
         */
        void renderCopy(sdl::Renderer &renderer, ImageId imageId, SDL_Rect &imgSrcRect, SDL_Rect &imgPaintRect) {
            getSurface(renderer, imageId);
            auto tex = mImageStore.at(imageId).get();
            SDL_RenderCopy(renderer.get(), tex, &imgSrcRect, &imgPaintRect);
        }

        /**
         * @brief Render the full texture to the destination rectangle
         * @param renderer The Renderer to use.
         * @param imageId The ImageId to render.
         * @param imgPaintRect The destination rectangle.
         */
        void renderCopy(sdl::Renderer &renderer, ImageId imageId, SDL_Rect &imgPaintRect) {
            getSurface(renderer, imageId);
            auto tex = mImageStore.at(imageId).get();
            SDL_RenderCopy(renderer.get(), tex, nullptr, &imgPaintRect);
        }

        /**
         * @brief Render the texture
         * @details The texture is rendered using the supplied renderer, source and paint rectangles.
         * Throws a std::out_of_range if no such ImageId is in the cache.
         * @param renderer the Renderer to use.
         * @param imageId the image identifier.
         * @param imgSrcRect the source rose::Rectangle
         * @param imgDstRect the destination rose::Rectangle
         */
        void renderCopy(sdl::Renderer &renderer, ImageId imageId, Rectangle &imgSrcRect, Rectangle &imgDstRect) {
            auto src = imgSrcRect.toSdlRect();
            auto dst = imgDstRect.toSdlRect();
            renderCopy(renderer, imageId, src, dst);
        }

        /**
         * @brief Render the full source of the texture to the destination.
         * @param renderer The Renderer to use.
         * @param imageId The ImageId to render.
         * @param imgDstRect The destination rectangle.
         */
        void renderCopy(sdl::Renderer &renderer, ImageId imageId, Rectangle &imgDstRect) {
            auto dst = imgDstRect.toSdlRect();
            renderCopy( renderer, imageId, dst);
        }

        /**
         * @brief Set an image in the cache replacing any existing image with the provided imageId
         * @details Throws a std::out_of_range if no such ImageId is in the cache.
         * @param imageId the image identifier.
         * @param textureData the TextureData to store.
         */
        void setImage(ImageId imageId, sdl::TextureData textureData) {
            mImageStore[imageId] = std::move(textureData);
        }

        /**
         * @brief Set a Surface to be used to create or update the image Texture.
         * @param imageId The ImageId to set or update.
         * @param surface The Surface.
         */
        void setImageSurface(ImageId imageId, sdl::Surface &surface) {
            mSurfaceStore[imageId] = std::move(surface);
        }

        /**
         * @brief Determine if the image store is empty.
         * @return true if empty.
         */
        [[nodiscard]] bool empty() const { return mImageStore.empty(); }

        /**
         * @brief Determine the size of the image store.
         * @return the number of images in the store.
         */
        [[nodiscard]] auto size() const { return mImageStore.size(); }

        /**
         * @brief Determine if an image id retrieves an image.
         * @param imageId The ImageId to test.
         * @return true if there is an image for the id.
         */
        bool isValidImage(sdl::Renderer &renderer, ImageId imageId) {
            if (!mImageStore.empty() && mImageStore.find(imageId) != mImageStore.end())
                return true;
            if (!mSurfaceStore.empty() && mSurfaceStore.find(imageId) != mSurfaceStore.end()) {
                getSurface(renderer, imageId);
                return true;
            }
            return false;
        }
    };
}



