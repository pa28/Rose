/**
 * @file MapProjections.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#pragma once

#include "Rose.h"

namespace rose {

    enum class ProjectionType {
        Mercator,
        StationMercator,
        StationAzmuthal,
    };

    /**
     * @class GeoPosition
     * @brief A geographic position holding Latitude and Longitude.
     */
    class GeoPosition : public std::array<double, 2> {
    public:
        constexpr GeoPosition() noexcept: std::array<double,2>({0., 0.}) {}

        constexpr GeoPosition(double lat, double lon) noexcept : std::array<double,2>({lat,lon}) {}

        constexpr double& lat() noexcept {
            return at(0);
        }

        constexpr double& lon() noexcept {
            return at(1);
        }

        [[nodiscard]] constexpr double lat() const noexcept {
            return at(0);
        }

        [[nodiscard]] constexpr double lon() const noexcept {
            return at(1);
        }
    };

    /**
     * @class MapProjections
     * @brief
     */
    class MapProjection : public Widget {
    protected:
        ProjectionType mProjection{ProjectionType::StationMercator};   ///< The desired projection display
        ImageId mDayMapImage{};         ///< The base day Mercator map image.
        ImageId mNightMapImage{};       ///< The base night Mercator map image.
        GeoPosition mQth{};             ///< The station location.
        Size mMapSize{};                ///< The size of the base maps in pixels.

        sdl::Texture mGeoChron{};

    public:
        MapProjection() = delete;

        ~MapProjection() override = default;

        MapProjection(std::optional<ImageId> day, std::optional<ImageId> night, GeoPosition qth, Size mapSize);

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /**
         * @brief Determine the desired size of the label with text and badge.
         * @param renderer the Renderer to use if needed.
         * @param available The Rectangle available for layout.
         * @return Rectangle The layout Rectangle.
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /**
         * @brief Draw the Label
         * @param renderer the renderer used to draw.
         * @param parentRect The layout size computed for this widget
         * @param parentPosition The layout position computed for this widget
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;
    };
}

