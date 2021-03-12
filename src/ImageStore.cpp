/**
 * @file ImageStore.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#include <array>
#include "ImageStore.h"
#include "Font.h"
#include "Surface.h"
#include "Texture.h"

namespace rose {

    inline ImageId toImageId(int id) { return static_cast<ImageId>(id); }
    inline int fmImageId(ImageId id) { return static_cast<int>(id); }

    static constexpr std::array<IconImage,3> IconImageList{{
                                                                   {ImageId::Compass,       ENTYPO_ICON_COMPASS,        color::DarkTextColour},
                                                                   {ImageId::Heart,         ENTYPO_ICON_HEART,          color::DarKRed},
                                                                   {ImageId::HeartEmpty,    ENTYPO_ICON_HEART_EMPTY,    color::DarkTextColour},
                                                           }};


    void ImageStore::createIcon(gm::Context &context, IconImage iconImage) {

        FontCache &fontCache{FontCache::getFontCache()};
        auto font = fontCache.getFont("entypo", 50);

        if (!font)
            throw (std::runtime_error(StringCompositor("Can not find font '", "entypo", "'.")));

        gm::Surface surface{TTF_RenderUTF8_Blended(font.get(), utf8(iconImage.code).data(), iconImage.color.toSdlColor())};

        int minX = surface->w;
        int minY = surface->h;
        int maxX = 0, maxY = 0;

        for (auto y = 0; y < surface->h; ++y) {
            for (auto x = 0; x < surface->w; ++x) {
                auto rgba = gm::getRGBA(surface->format, surface.pixel(x,y));
                if (rgba.a() > 0) {
                    minX = std::min(minX, x);
                    minY = std::min(minY, y);
                    maxX = std::max(maxX, x);
                    maxY = std::max(maxY, y);
                }
            }
        }

        gm::Surface minimal{maxX - minX + 1, maxY - minY + 1};
        for (auto y = 0; y < minimal->h; ++y) {
            for (auto x = 0; x < minimal->w; ++x) {
                auto rgba = gm::getRGBA(surface->format, surface.pixel(minX + x, minY + y));
                minimal.pixel(x, y) = gm::mapRGBA(minimal->format, rgba);
            }
        }

        gm::Texture texture{minimal.toTexture(context)};
        mImageMap.emplace(iconImage.key, std::move(texture));
    }

    void ImageStore::initialize(gm::Context &context) {
        if (mInitialized)
            return;

        createIcons(context, IconImageList.begin(), IconImageList.end());
        mInitialized = true;
    }

#if 0
    Size ImageStore::size(ImageId imageId) {
        Size size{};
        if (auto image = mImageMap.find(imageId); image != mImageMap.end()) {
            size = TextureGetSize(image->second);
        }

        return size;
    }

    int ImageStore::renderCopy(gm::Context &context, ImageId imageId, Rectangle dst) {
        if (auto image = mImageMap.find(imageId); image != mImageMap.end()) {
            return context.renderCopy(image->second, dst);
        }
        return 0;
    }
#endif
}
