/**
 * @file LinearScale.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-16
 */

#pragma once

#include "Border.h"
#include "Frame.h"
#include "ImageView.h"
#include "Signals.h"

namespace rose {

    using namespace std;

    /**
     * @class LinearScale
     * @brief A base class for Widgets that are base on a linear value input or output eg) Slider and Gauge.
     * @details A linearScale is made up of nested Widgets: Frame; Border; and an optional ImageView which is
     * used as a value indicator. Also included is a background Texture that can be used as a background scale
     * behind the ImageView, or when the ImageView is not in use, partially rendered to indicate the value.
     */
    class LinearScale : public Frame {
    protected:
        SignalSerialNumber mSignalSerialNumber{};       ///< The button serial number
        Orientation mOrientation{Orientation::Unset};   ///< Scale Orientation.

        float mLowerBound{0.f};                 ///< The lower bound of the scale.
        float mUpperBound{1.f};                 ///< The upper bound of the scale.
        float mValue{0.f};                      ///< The current value of the scale.

        ImageId mImageId{RoseImageInvalid};     ///< ImageId of the indicator.
        ImageId mCenterId{RoseImageInvalid};    ///< ImageId of the thumb center.

        /**
         * @class LinearScaleBorder
         * @brief The border around a LinearScale background.
         */
        class LinearScaleBorder : public Border {
        public:
            LinearScaleBorder() = default;
            ~LinearScaleBorder() override = default;
            LinearScaleBorder(const LinearScaleBorder &) = delete;
            LinearScaleBorder(LinearScaleBorder &&) = delete;
            LinearScaleBorder& operator=(const LinearScaleBorder &) = delete;
            LinearScaleBorder& operator=(LinearScaleBorder &&) = delete;

            /**
             * @brief Constructor
             * @param border The size of the border in pixes.
             */
            explicit LinearScaleBorder(int border) : Border(border) {}

            /**
             * @brief See Widget::initializeComposite()
             */
            void initializeComposite() override;

            /**
             * @brief See Widget widgetLayout()
             */
            Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

            /**
             * @brief See Widget::draw()
             */
            void draw(sdl::Renderer &renderer, Rectangle parentRect) override;
        };

        /**
         * @class LinearScaleImage
         * @brief The Indicator image of the LinearScale.
         */
        class LinearScaleImage : public ImageView {
        public:
            LinearScaleImage() = default;
            ~LinearScaleImage() override = default;
            LinearScaleImage(const LinearScaleImage &) = delete;
            LinearScaleImage(LinearScaleImage &&) = delete;
            LinearScaleImage& operator=(const LinearScaleImage &) = delete;
            LinearScaleImage& operator=(LinearScaleImage &&) = delete;

            /**
             * @brief Constructor
             * @param imageId The ImageId of the indicator
             */
            explicit LinearScaleImage(ImageId imageId) : ImageView(imageId) {}

            /**
             * @brief See Widget::initializeComposite()
             */
            void initializeComposite() override;

            /**
             * @brief See Widget widgetLayout()
             */
            Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

            /**
             * @brief See Widget::draw()
             */
            void draw(sdl::Renderer &renderer, Rectangle parentRect) override;
        };

        shared_ptr<LinearScaleBorder> mBorder{};       ///< The Border inside the Frame.
        shared_ptr<LinearScaleImage> mIndicator{}; ///< The optional indicator.

    public:
        ~LinearScale() override = default;               ///< Default destructor
        LinearScale(const LinearScale &) = delete;
        LinearScale& operator=(const LinearScale &) = delete;
        LinearScale(LinearScale &&) = delete;
        LinearScale& operator=(LinearScale &&) = delete;

        /**
         * @brief Constructor
         * @param imageId The ImageId for the indicator which is omitted if imageId == RoseImageInvalid.
         */
        explicit LinearScale(ImageId imageId = RoseImageInvalid);

        /**
         * @brief Constructor
         * @param lowerBound The lower bound of the scale.
         * @param upperBound The upper bound of the scale.
         * @param value The initial value of the scale.
         * @param imageId The ImageId for the indicator which is omitted if imageId == RoseImageInvalid.
         */
        LinearScale(float lowerBound, float upperBound, float value, ImageId imageId = RoseImageInvalid);

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

        /**
         * @brief See Widget widgetLayout()
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /**
         * @brief See Widget::draw()
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Called by LinearScaleBorder after it calls Border::initializeComposite().
         */
        virtual void initializeBorderComposite();

        /**
         * @brief Called by LinearScaleBorder::initialLayout() after it calls Border::widgetLayout().
         * @details The return value of this method is returned by LinearScaleBorder::widgetLayout().
         * @param renderer The Renderer used.
         * @param available The available screen rectangle.
         * @param borderLayout The layout Rectangle returned by Border::widgetLayout().
         * @return The final layout rectangle.
         */
        virtual Rectangle initialBorderLayout(sdl::Renderer &renderer, Rectangle available, Rectangle borderLayout);

        /**
         * @brief Called by LinearScaleBoarder::draw() after it calls Border::draw().
         * @param renderer The Renderer to use.
         * @param available The available screen rectangle.
         */
        virtual void drawBorder(sdl::Renderer &renderer, Rectangle available);

        /**
         * @brief Called by LinearScaleImage after it calls ImageView::initializeComposite().
         */
        virtual void initializeImageComposite();

        /**
         * @brief Called by LinearScaleImage::initialLayout() after it calls ImageView::widgetLayout().
         * @details The return value of this method is returned by LinearScaleImage::widgetLayout().
         * @param renderer The Renderer used.
         * @param available The available screen rectangle.
         * @param imageLayout The layout Rectangle returned by ImageView::widgetLayout().
         * @return The final layout rectangle.
         */
        virtual Rectangle initialImageLayout(sdl::Renderer &renderer, Rectangle available, Rectangle imageLayout);

        /**
         * @brief Called by LinearScaleImage::draw() after it calls ImageView::draw().
         * @param renderer The Renderer to use.
         * @param available The available screen rectangle.
         */
        virtual void drawImage(sdl::Renderer &renderer, Rectangle available);

        /// Local definition of SignalType
        using SignalType = LinearScaleSignalType;

        /// Signal to transmit value changes.
        Signal<SignalType> valueTx{};

        /// Slot to receive value changes.
        std::shared_ptr<Slot<SignalType>> valueRx{};

        /**
         * @brief Set the current value of the LinearScale.
         * @param value The new value.
         * @param transmit If true this value is transmitted on Signal valueTx.
         */
        void setValue(float value, bool transmit);
    };
}

