/**
 * @file Image.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#pragma once

#include "ImageStore.h"
#include "Theme.h"

namespace rose {

    /**
     * @class Image
     * @brief
     */
    class Image {
    protected:
        ImageId mImageId{ImageId::NoImage};

    public:
        Image() = default;
        virtual ~Image() = default;

        explicit Image(ImageId id) : mImageId(id) {
            std::cout << __PRETTY_FUNCTION__ << '\n';
        }

    };

    class ImageLabel : public Widget, protected Image {
    protected:
        Size mRequestedSize{};

    public:
        explicit ImageLabel(ImageId imageId = ImageId::ThreeDots) : Image(imageId) {
            mRequestedSize = Theme::getTheme().ImageLabelSize;
        }

        ~ImageLabel() override = default;

        ImageLabel(const ImageLabel&) = delete;
        ImageLabel(ImageLabel &&) = delete;
        ImageLabel& operator=(const ImageLabel&) = delete;
        ImageLabel& operator=(ImageLabel &&) = delete;

        ImageLabel(ImageId imageId, Size size);

        ImageLabel(ImageId imageId, int size) : ImageLabel(imageId, Size{size}) {}

        /**
         * @brief Layout the label.
         * @param context The graphics Context to use.
         * @param screenRect The available screen Rectangle.
         * @return The requested screen Rectangle.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override {
            mSize = ImageStore::getStore().size(mImageId);
            if (mPreferredSize)
                mSize = mPreferredSize;
            mPos = mPreferredPos;

            if (mSize < mRequestedSize) {
                auto space = mRequestedSize - mSize;
                mPadding.l = space.w / 2;
                mPadding.r = space.w - mPadding.l;
                mPadding.t = space.h / 2;
                mPadding.b = space.h - mPadding.t;
            }

            return layoutPadding(Rectangle{mPos, mSize});
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

