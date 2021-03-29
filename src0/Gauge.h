/**
 * @file Gauge.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-14
 */

#pragma once

#include "Frame.h"
#include "Signals.h"
#include "Utilities.h"

namespace rose {

    /**
     * @class Gauge
     * @brief Display a numeric value as a graphic indicator.
     * @details The Gauge supports five bands defined by six values. If fewer bands are needed the band defining
     * value can be set to be equal. The full Gauge strip, as it would be displayed when set to UpperBound, is
     * created. The indicated value can be displayed by render copying the strip from LowerBound to mStripLength;
     * or by render copying the full strip and placing an indicator image centered on mStripLength.
     */

    enum class GaugeIndex : unsigned long {
        LowerBound = 0,     ///< Index to the minimum displayable value and LowEmergency color
        LowWarning,         ///< The bottom of the warning band value and and LowWarning color
        LowNormal,          ///< The bottom of the normal band value and Normal color
        HighNormal,         ///< The top of the normal band value and HighWarning color
        HighWarning,        ///< The top of the warning band value and HighEmergency color
        UpperBound,         ///< Index to the maximum displayable value
    };

    constexpr inline auto indexValue(const GaugeIndex &g) {
        using IntType = typename std::underlying_type<GaugeIndex>::type;
        return static_cast<IntType>(g);
    }

    /**
     * @brief GaugeIndex pre-increment operator.
     * @details Throws std::domain_error if the resultant value is outside the define range.
     * @param gaugeIndex The GaugeIndex to increment.
     * @return A reference to the GaugeIndex.
     */
    inline GaugeIndex& operator++( GaugeIndex &gaugeIndex ) {
        using IntType = typename std::underlying_type<GaugeIndex>::type;
        if (gaugeIndex == GaugeIndex::UpperBound)
            throw std::domain_error("Incrementing GaugeIndex beyond UpperBound.");
        gaugeIndex = static_cast<GaugeIndex>( static_cast<IntType>(gaugeIndex) + 1 );
        return gaugeIndex;
    }

    /**
     * @brief Add an unsigned integer to a GaugeIndex.
     * @details Throws std::domain_error if the resultant value is outside the define range.
     * @param gaugeIndex The GaugeIndex to add to.
     * @param increment The amount to increment it by.
     * @return The GaugeIndex that results from the increment.
     */
    inline GaugeIndex operator+(const GaugeIndex &gaugeIndex, unsigned long increment) {
        using IntType = typename std::underlying_type<GaugeIndex>::type;
        if (static_cast<IntType>(gaugeIndex) + increment > indexValue(GaugeIndex::UpperBound))
            throw std::domain_error("Incrementing GaugeIndex beyond UpperBound.");
        return static_cast<GaugeIndex>(static_cast<IntType>(gaugeIndex) + increment);
    }

    using GaugeValueLimits = std::array<float,6>;   ///< Type of Gauge values.
    using GaugeValueHues = std::array<uint32_t,5>;   ///< Type of Gauge colors.

    /**
     * @class Gauge
     * @brief Display an analogue value as the position of an indicator on a background scale.
     */
    class Gauge : public Frame {
    protected:
        /**
         * @class GaugeInterior
         * @brief Manages the interior of the Gauge.
         */
        class GaugeInterior : public Widget {
        protected:
            friend class Gauge;
            bool mTextureValid{false};                          ///< True if the texture is valid.
            sdl::Texture mTexture{};                            ///< The Texture that draws the Gauge scale.
            Orientation mOrientation{Orientation::Unset};       ///< Orientation passed in from Gauge.

            static array<int, 6>
            computeValueBands(GaugeValueLimits gaugeValueLimits, Rectangle rect, Orientation orientation);              ///< Translate the value bands to the interior size.

        public:
            GaugeInterior() = default;
            ~GaugeInterior() override = default;                ///< Default destructor
            GaugeInterior(const Gauge &) = delete;
            GaugeInterior& operator=(const GaugeInterior &) = delete;
            GaugeInterior(GaugeInterior &&) = delete;
            GaugeInterior& operator=(GaugeInterior &&) = delete;

            /**
             * @brief See Widget widgetLayout()
             */
            Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

            /// See Widget::draw();
            void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

            /// See Widget::setOrientation();
            void setOrientation(Orientation orientation) override { mOrientation = orientation; }
        };

        friend class GaugeInterior;

        /**
         * @brief The default limits.
         * @details The first and last values must be the lower and upper bounds respectively. Other values must
         * increase or remain the same. For example when the LowerBound is also the LowNormal value the
         * GaugeValueLimits could be : { 0.f, 0.f, 0.f, 0.75f, 0.8f, 1.0f }
         */
        static constexpr GaugeValueLimits defaultLimits = {0.f, 0.1f, 0.2f,
                                                           0.8f, 0.9f, 1.f};

        /**
         * @brief The default band Hues.
         * @details These Hues are combined with the Theme base Color Saturation and Lightness to derive band
         * colors.
         */
        static constexpr GaugeValueHues defaultHues = { 300, 240, 120,
                                                        60, 10};
        float mValue{0.f};                          ///< The current value displayed by the Gauge.
        sdl::Texture mTexture{};                    ///< The Texture with the full Gauge strip.
        int mStripLength{0};                        ///< The current value converted to strip length in pixels.
        ImageId mIndicator{RoseImageInvalid};       ///< The Gauge indicator, when a valid ImageId.

        /**
         * @brief The limits and color areas of the Gauge.
         * @details The six values are, in order: LowerBound, LowWarning, LowNormal, HighNormal, HighWarning, Upperbound
         */
        GaugeValueLimits mValueLimits;

        /**
         * @brief The colors for the five possible value bands.
         * @details The five values are, in order: LowEmergency, LowWarning, Normal, HighWarning, HighEmergency
         */
        GaugeValueHues mValueHues;

        Orientation mOrientation{Orientation::Unset};   ///< Orientation of the gauge. Unset is treated as Horizontal.

    public:
        ~Gauge() override = default;               ///< Default destructor
        Gauge(const Gauge &) = delete;
        Gauge& operator=(const Gauge &) = delete;
        Gauge(Gauge &&) = delete;
        Gauge& operator=(Gauge &&) = delete;

        /**
         * @brief Constructor
         * @param imageId The ImageId of the indicator image.
         */
        explicit Gauge(ImageId imageId = RoseImageInvalid);

        /**
         * @brief Constructor
         * @param gaugeValueLimits The desired value limits.
         * @param gaugeValueHuess The desired value color hues.
         */
        Gauge(const GaugeValueLimits &gaugeValueLimits, const GaugeValueHues &gaugeValueHuess, ImageId imageId = RoseImageInvalid);

        /**
         * @brief Constructor
         * @param gaugeValueLimits The desired value limits.
         * @param gaugeValueHues The desired value color hues.
         */
        Gauge(GaugeValueLimits &&gaugeValueLimits, GaugeValueHues &&gaugeValueHues, ImageId imageId = RoseImageInvalid);

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

        std::shared_ptr<Slot<float>> valueRx;       ///< A Slot to receive values on.

        /**
         * @brief Set the value to be displayed.
         * @param value The value which will be clamped between LowerBound and UpperBound.
         */
        void setValue(float value);

        void setOrientation(Orientation orientation) override;
    };
}

