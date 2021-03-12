/**
 * @file Image.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#include "Image.h"

namespace rose {

    void ImageLabel::draw(gm::Context &context, const Position &containerPosition) {
        ImageStore& imageStore{ImageStore::getStore()};
        if (imageStore.exists(mImageKey)) {
            Rectangle dst{containerPosition + mPos, imageStore.size(mImageKey)};
            imageStore.renderCopy(context, mImageKey, dst);
        }
    }
}
