/**
 * @file ImageStore.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#pragma once

#include <cstdint>
#include <map>
#include "Color.h"
#include "Entypo.h"
#include "GraphicsModel.h"
#include "Surface.h"
#include "Texture.h"

namespace rose {

    enum class ImageId : int  {
        NoImage,
        Compass,
        Heart,
        HeartEmpty,
    };

    struct IconImage {
        ImageId key{};
        uint32_t code{};
        color::RGBA color{};
    };

    /**
    * @class ImageStore
    * @brief
    */
    class ImageStore {
    protected:
        bool mInitialized{false};

        std::map<ImageId,gm::Texture> mImageMap{};

        void createIcon(gm::Context &context, IconImage iconImage);

        template<typename Iterator>
        void createIcons(gm::Context &context, Iterator first, Iterator last) {
            while (first != last) {
                createIcon(context, *first);
                first++;
            }
        }

        void initialize(gm::Context &context);

    public:
        static ImageStore& getStore() {
            static ImageStore imageStore{};
            return imageStore;
        }

        static ImageStore& getStore(gm::Context &context) {
            ImageStore& store{getStore()};
            if (!store.mInitialized) {
                store.initialize(context);
                store.mInitialized = true;
            }
            return getStore();
        }

        bool exists(ImageId imageId) {
            return mImageMap.find(imageId) != mImageMap.end();
        }

        Size size(ImageId imageId) {
            Size size{};
            auto image = mImageMap.find(imageId);
            if (image != mImageMap.end()) {
                size = gm::TextureGetSize(image->second);
            }
            return size;
        }

        int renderCopy(gm::Context& context, ImageId imageId, Rectangle dst) {
            if (auto image = mImageMap.find(imageId); image != mImageMap.end())
                return context.renderCopy(image->second, dst);
            return 0;
        }
    };
}

