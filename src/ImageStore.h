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

    enum class ImageId : int {
        NoImage,
        Compass,
        Heart,
        HeartEmpty,
        Cancel,
        Help,
        Info,
        Check,
        Alert,
        Lock,
        UpBold,
        UpOpenBig,
        Left,
        Right,
        ToEnd,
        Back,
        LevelDown,
        OneDot,
        TwoDots,
        ThreeDots,
        UpDir,
        DownDir,
        LeftDir,
        RightDir,
        Keyboard,
        Globe,
        Network,
        Rocket,
        Location,
        Camera,
        ScaleNeedleUp,
        ScaleNeedleDown,
        ScaleNeedleLeft,
        ScaleNeedleRight,
        IconTarget,
        // ImageIds for Frames
        BevelOutSquareCorners,
        BevelInSquareCorners,
        NotchOutSquareCorners,
        NotchInSquareCorners,
        BevelOutRoundCorners,
        BevelInRoundCorners,
        NotchOutRoundCorners,
        NotchInRoundCorners,
        RoundCornerTrim,
        // ImageIds for Centres
        CenterRoundBaseColor,
        CenterRoundInvertColor,
        CenterRoundRedColor,
        CenterRoundGreenColor,
        CenterRoundBlueColor,
        CenterRoundYellowColor,
        CenterSquareBaseColor,
        CenterSquareInvertColor,
        // Following value must be last.
        DynamicIdStart,
    };

    /**
     * @struct IconImage
     * @brief The information required to create an Icon from the Entypo Font.
     */
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
        /// True after initialization.
        bool mInitialized{false};

        /// The next ImageId for storage of dynamic images.
        int mNextImageId{static_cast<int>(ImageId::DynamicIdStart)};

        /// The Texture storage map.
        std::map<ImageId,gm::Texture> mImageMap{};

        /// Create an Icon using the Entypo font.
        void createIcon(gm::Context &context, IconImage iconImage);

        /// Create many Icons using a container of IconImage structures.
        template<typename Iterator>
        void createIcons(gm::Context &context, Iterator first, Iterator last) {
            while (first != last) {
                createIcon(context, *first);
                first++;
            }
        }

        /// Initialize the ImageStore
        void initialize(gm::Context &context);

        void createSquareCorners(gm::Context &context, int radius, int borderWidth,
                                 color::RGBA top, color::RGBA bot,
                                 color::RGBA left, color::RGBA right);

        void createRoundCorners(gm::Context &context, int scale, int radius, int borderWidth,
                                color::RGBA top, color::RGBA bot,
                                color::RGBA left, color::RGBA right);

        void createCenters(gm::Context &context, int scale, int radius);

    public:

        /**
         * @brief Get access to the Singleton ImageStore.
         * @return A reference to the ImageStore.
         */
        static ImageStore &getStore() {
            static ImageStore imageStore{};
            return imageStore;
        }

        /**
         * @brief Get access to, and initialize, the Singleton ImageStore.
         * @details This must be called by the application after the GraphicModel and Context have been initialized
         * but before any access to the image storage.
         * @param context The graphics Context used to create some icons.
         * @return A reference to the ImageStore.
         */
        static ImageStore& getStore(gm::Context &context) {
            ImageStore& store{getStore()};
            if (!store.mInitialized) {
                store.initialize(context);
                store.mInitialized = true;
            }
            return getStore();
        }

        /// Get the next dyname ImageId.
        ImageId nextImageId() {
            return static_cast<ImageId>(mNextImageId++);
        }

        /// Set, or reset the Texture associated with the ImageId.
        void setImage(ImageId imageId, gm::Texture &&texture);

        /// Test to see if a Texture is associated with an ImageId.
        bool exists(ImageId imageId) {
            return mImageMap.find(imageId) != mImageMap.end();
        }

        /**
         * @brief Get the size of the Texture associated with an ImageId.
         * @details It is not an error for there to be no associated Texture. When this is the case Size::Zero
         * is returned.
         * @param imageId The ImageId.
         * @return A Size object.
         */
        Size size(ImageId imageId) {
            Size size{};
            auto image = mImageMap.find(imageId);
            if (image != mImageMap.end()) {
                size = image->second.getSize();
            }
            return size;
        }

        /**
         * @brief Render the Texture associated with an Image Id.
         * @details The source rectangle is set to the full Texture,
         * @param context The Context to use.
         * @param imageId The ImageId.
         * @param dst The destination Rectangle.
         * @return The return status code from the SDL API.
         */
        int renderCopy(gm::Context& context, ImageId imageId, Rectangle dst) {
            if (auto image = mImageMap.find(imageId); image != mImageMap.end())
                return context.renderCopy(image->second, dst);
            return 0;
        }

        /**
         * @brief Render the Texture associated with an Image Id.
         * @param context The Context to use.
         * @param imageId The ImageId.
         * @param src The source Rectangel.
         * @param dst The destination Rectangle.
         * @return The return status code from the SDL API.
         */
        int renderCopy(gm::Context& context, ImageId imageId, Rectangle src, Rectangle dst) {
            if (auto image = mImageMap.find(imageId); image != mImageMap.end())
                return context.renderCopy(image->second, src, dst);
            return 0;
        }
    };
}

