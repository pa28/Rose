/**
 * @file Image.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#pragma once

#include "ImageStore.h"

namespace rose {

    /**
     * @class Image
     * @brief
     */
    class Image {
    protected:
        ImageId mImageKey{ImageId::NoImage};

    public:
        Image() = default;
        virtual ~Image() = default;

        explicit Image(ImageId id) : mImageKey(id) {
            std::cout << __PRETTY_FUNCTION__ << '\n';
        }

    };

    class ImageLabel : public Widget, protected Image {
    protected:

    public:
        ImageLabel() = default;
        ~ImageLabel() override = default;

        ImageLabel(const ImageLabel&) = delete;
        ImageLabel(ImageLabel &&) = delete;
        ImageLabel& operator=(const ImageLabel&) = delete;
        ImageLabel& operator=(ImageLabel &&) = delete;

        explicit ImageLabel(ImageId imageId) : Image(imageId) {}

        /**
         * @brief Layout the label.
         * @param context The graphics Context to use.
         * @param screenRect The available screen Rectangle.
         * @return The requested screen Rectangle.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
            mSize = ImageStore::getStore().size(mImageKey);
            if (mPreferredSize)
                mSize = mPreferredSize;
            mPos = mPreferredPos;
            return Rectangle{mPos, mSize};
        }

        /**
         * @brief Draw the label.
         * @details The text is rendered relative the the parent Container.
         * @param context The graphics context to use.
         * @param containerPosition The position of the parent Container.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;
    };
}

