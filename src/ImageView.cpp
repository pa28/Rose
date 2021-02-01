/** @file ImageView.cpp
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2020-12-24
  * @brief An image view Widget.
  */

#include "ImageView.h"
#include "Math.h"
#include "Rose.h"
#include "Utilities.h"

namespace rose {
    ImageView::ImageView() {
        mImageId = ImageRepository::InvalidImageId;
    }

    ImageView::ImageView(ImageId imageId) : ImageView() {
        mImageId = imageId;
    }

    ImageView::ImageView(ImageId imageId, bool shrinkable) : ImageView(imageId) {
        mLayoutHints.mShrinkable = shrinkable;
    }

    void ImageView::initializeComposite() {
        Widget::initializeComposite();
        mClassName = "ImageView";
    }

    Rectangle ImageView::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        if (mImageId == ImageRepository::InvalidImageId) {
            mPos = Position::Zero;
            mSize = Size::Zero;
        } else if (rose()->imageRepository().isValidImage(renderer, mImageId)) {
            mPos = Position::Zero;
            mSize = rose()->imageRepository().image(static_cast<RoseImageId>(mImageId)).getSize();
        } else {
            mPos = Position::Zero;
            mSize = Size::Zero;
        }

        auto imageRect = clampAvailableArea(available, mPos, mSize);

        if (imageRect.getSize() > available.getSize()) {
            float wRatio = (float) available.width() / (float) mSize->width();
            float hRatio = (float) available.height() / (float) mSize->height();
            if (wRatio < hRatio) {
                imageRect.width() = roundToInt((float) mSize->width() * wRatio);
                imageRect.height() = roundToInt((float) mSize->height() * wRatio);
            } else {
                imageRect.width() = roundToInt((float) mSize->width() * hRatio);
                imageRect.height() = roundToInt((float) mSize->height() * hRatio);
            }
        } else if (imageRect.width() == 0 || imageRect.height() == 0) {
            imageRect = available.getSize();
        } else {
            float wRatio = (float)imageRect.width() / (float) mSize->width();
            float hRatio = (float)imageRect.height() / (float) mSize->height();
            if (wRatio < hRatio) {
                imageRect.width() = roundToInt((float) mSize->width() * wRatio);
                imageRect.height() = roundToInt((float) mSize->height() * wRatio);
            } else {
                imageRect.width() = roundToInt((float) mSize->width() * hRatio);
                imageRect.height() = roundToInt((float) mSize->height() * hRatio);
            }
        }

        return imageRect;
    }

    void ImageView::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            Rectangle widgetRect{};

            // The Widget rectangle will have the parent origin offset by the Widget position
            widgetRect = parentRect.getPosition() + mLayoutHints.mAssignedRect->getPosition();
            // And the Widget size.
            widgetRect = mLayoutHints.mAssignedRect->getSize();

            if (mImageId != ImageRepository::InvalidImageId) {
                if (rose()->imageRepository().isValidImage(renderer, mImageId)) {
                    Rectangle dst{parentRect.getPosition() + mLayoutHints.mAssignedRect->getPosition(),
                                     mLayoutHints.mAssignedRect->getSize()};
                    rose()->imageRepository().renderCopy(renderer, mImageId, dst);
                }
            }
        }
    }

    void ImageView::setImageId(ImageId imageId) {
        mImageId = imageId;
        std::cout << __PRETTY_FUNCTION__ << '\n';
        if (auto newRect = rose()->imageRepository().image(mImageId).getSize(); newRect != mSize.value()) {
            mSize = newRect;
            rose()->needsLayout();
        }
    }
}