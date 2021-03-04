/** @file Sprite.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-20
  *
  * @brief The Sprite Class.
  *
  * A sprite is a non-layout, self positioning object that moves at the application frame rate following
  * movement commands or splines.
  *
  */

#pragma once

#include "Widget.h"
#include "Types.h"
#include "ImageRepository.h"

namespace rose {
    /**
     * @class Sprite
     * @brief A sprite is a free roaming texture that is directed using Signals and Slots.
     * @details A Sprite is a self managing visual object that can be directed to move in various
     * ways at a rate lower than the frame rate of the application. It will continue to move
     * until it has carried out the last instruction, or a new instruction is received.
     */
    class Sprite : public Widget {
    public:
        Sprite() = default;
        ~Sprite() override = default;
        Sprite(Sprite &&) = delete;
        Sprite(const Sprite &) = delete;
        Sprite& operator=(Sprite &&) = delete;
        Sprite& operator=(const Sprite &) = delete;

        /**
         * @brief Basic constructor
         * @param rose the Rose object
         * @param parent the parent Widget.
         */
        Sprite(Rose &rose, Widget *parent);

        /**
         * @brief Constructor
         * @param rose the Rose object
         * @param parent the parent Widget
         * @param imageId the ImageId of the image to use as the sprite.
         */
        Sprite(std::shared_ptr<Widget> parent, ImageId imageId) : Sprite(parent) {
            mImageId = imageId;
        }

        /**
         * @brief Draw the sprite.
         * @param renderer
         */
        void draw(sdl::Renderer &renderer, RectangleInt parentRect) override;

        /**
         * @brief Give the sprite a vector to travel and a time in milliseconds that it should take to travers.
         * @param vector
         * @param time
         */
        void setVector(Size<float> vector, uint32_t time) {
            mCurrentVector = vector;
            mVectorTicks = (float) time;
        }

        /**
         * @brief Set the ImageRepository::ImageId of the texture to use as the sprite.
         * @param imageId
         */
        void setImageId(ImageId imageId) { mImageId = imageId; }

    protected:
        ImageId mImageId{ImageRepository::InvalidImageId};

        PositionInt mTextureFrame{};
        PositionInt mTextureOffset{};

        uint32_t mTicks;

        Position<float> mCurrentPosition;
        Size<float> mCurrentVector;
        float mVectorTicks{};
        int mFrameTicks{};
        int mFrameRate{};
    };
}
