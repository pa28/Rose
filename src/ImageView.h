/** @file ImageView.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2020-12-24
  * @brief An image view Widget.
  */

#pragma once

#include "Widget.h"
#include "ImageRepository.h"
#include "Utilities.h"

namespace rose {

    /**
     * @class ImageView
     * @brief Display an image texture.
     */
    class ImageView : public Widget {
    protected:
        ImageId mImageId;

    public:
        ImageView();
        ~ImageView() override = default;
        ImageView(ImageView &&) = delete;
        ImageView(const ImageView &) = delete;
        ImageView& operator=(ImageView &&) = delete;
        ImageView& operator=(const ImageView &) = delete;

        /**
         * @brief Constructor
         * @param imageId The ImageId to display from the ImageRepository.
         */
        explicit ImageView(ImageId imageId);

        /**
         * @brief Constructor
         * @param imageId The ImageId to display from the ImageRepository.
         * @param shrinkable True of the image may be shrunk to fit.
         */
        ImageView(ImageId imageId, bool shrinkable);

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /**
         * @brief Determine the desired size of the label with text and badge.
         * @param renderer the Renderer to use if needed.
         * @param available The Rectangle available for layout.
         * @return Rectangle The layout Rectangle.
         */
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /**
         * @brief Draw the Label
         * @param renderer the renderer used to draw.
         * @param parentRect The layout size computed for this widget
         * @param parentPosition The layout position computed for this widget
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Set the ImageId of the image texture to display.
         * @param imageId The ImageId.
         */
        void setImageId(ImageId imageId) override;

    };

}
