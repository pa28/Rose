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
}

