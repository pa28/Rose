/**
 * @file Image.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#include "Image.h"

namespace rose {
#if 0
    void ImageLabel::draw(gm::Context &context, const Position &containerPosition) {
        ImageStore& imageStore{ImageStore::getStore()};
        if (imageStore.exists(mImageId)) {
            Rectangle dst{drawPadding(containerPosition + mPos), imageStore.size(mImageId)};
            imageStore.renderCopy(context, mImageId, dst);
        }
    }

    ImageLabel::ImageLabel(ImageId imageId, Size size) : ImageLabel(imageId) {
        mRequestedSize = size;
    }
#endif
}
