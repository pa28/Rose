/**
 * @file MapProjections.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#pragma once

#include "Cache.h"
#include "Math.h"
#include "Rose.h"

namespace rose {

    enum class ProjectionType {
        Mercator,
        StationMercator,
        StationAzmuthal,
    };

    /**
     * @enum MapDataType
     * @brief Map type ins order with the Night map after and one greater than the Day map.
     */
    enum class MapDataType : size_t {
        TerrainDay = 0,
        TerrainNight = 1,
        CountriesDay = 0x2,
        CountriesNight = 0x3,
        MapCount,
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
        static constexpr double GrayLineCos = -0.258;   ///< Sets the width of the dawn/dusk period.
        static constexpr double GrayLinePow = .99;      ///< Sets the speed of transitions, smaller is sharper.

        std::future<bool> mFutureAziProj{};       ///< A future for the map azimuthal projections.
        std::future<bool> mFutureSun{};           ///< A future for the map sun illumination.
        std::atomic_bool mNewSurfaces{};          ///< True when there are new Surfaces to render into Textures.

        ProjectionType mProjection{ProjectionType::StationAzmuthal};   ///< The desired projection display
        std::shared_ptr<WebFileCache> mMapCache{};
        ImageId mDayMapImage{};         ///< The base day Mercator map image.
        ImageId mNightMapImage{};       ///< The base night Mercator map image.
        GeoPosition mQth{};             ///< The station location.
        GeoPosition mQthRad{};          ///< The station location in radians.
        GeoPosition mAntipode{};        ///< The station antipode in radians.
        Size mMapSize{};                ///< The size of the base maps in pixels.

        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)> mMapSurface{};
        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)> mAzSurface{};

        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)/2> mMercatorTemp;
        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)/2> mAzimuthalTemp;

        std::array<sdl::Texture,2> mMercator{}; ///< The Mercator projection background and foreground maps.
        std::array<sdl::Texture,2> mAzimuthal{};    ///< The Azimuthal projection background and foreground maps.

        sdl::Texture mNightAz{};        ///< The Texture with the Night Azimuthal map.
        sdl::Texture mDayAz{};          ///< The Texture with the Day Azimuthal map.

        /**
         * @brief Compute Azimuthal map projections.
         * @details This should be called by creating the std::future mAzimuthalProjection which will be
         * completed when the projection is done, and return true if the projection was successful. This
         * method only generates the Surfaces which are then used to create Textures that are displayed.
         * The Surface to Texture conversion must happen on the main thread so locking is not an issue and
         * the normal render cycle can continue as long as the last Texture is valid.
         */
        bool computeAzimuthalMaps();

        /**
         * @brief Compute the sun illumination pattern.
         * @details This creates a background foreground map pair. The background is the night map, the
         * foreground is the day map which has had the Alpha channel modified to match the projected
         * illumination from the sun. This method only generates the Surfaces which are then used to create
         * Textures that are displayed. The Surface to Texture conversion must happen on the main thread so
         * locking is not an issue and the normal render cycle can continue as long as the last Texture is valid.
         */
        bool setForegroundBackground();

    public:
        MapProjection() = delete;

        ~MapProjection() override = default;

        MapProjection(std::shared_ptr<WebFileCache> mapCache, GeoPosition qth, Size mapSize);

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

        std::shared_ptr<Slot<uint32_t>> mapFileRx{};    ///< Slot to receive notification of map files ready.
        std::shared_ptr<Slot<int>> secondRx{};          ///< Slot to receive one-per-second signals from SecondTick.
        std::shared_ptr<Slot<int>> minuteRx{};          ///< Slot to receive one-per-minute signals from SecondTick.

        /**
         * Compute an antipode
         * @param posRadians A GeoPosition in radians.
         * @return The GeoPosition of the Antipode in radians.
         */
        static GeoPosition antipode(const GeoPosition posRadians) {
            return GeoPosition{-posRadians.lat(), (posRadians.lon() < 0. ? 1. : -1.) * (M_PI - abs(posRadians.lat()))};
        }

        /**
         * @brief Convert a map coordinate (x,y) into a GeoPosition(lat,lon) in radians.
         * @param x The x coordinate in pixels
         * @param y The y coordinate in pixels
         * @param mapSize The Size of the map.
         * @return The GeoPosition.
         */
        static GeoPosition geoPosition(int x, int y, Size mapSize) {
            double lon = (2. * M_PI * (double)x / (double)mapSize.width()) - M_PI;
            double lat = M_PI_2 - (double)y / (double)mapSize.height() * M_PI;
            return GeoPosition{lat,lon};
        }

        /**
         * @brief Convert a GeoPosition(lat,lon) in radians to a map Position(x,y) in pixels.
         * @param map The GeoPosition.
         * @param mapSize The Size of the map.
         * @return The Position.
         */
        static Position mapPosition(GeoPosition map, Size mapSize) {
            int x = util::roundToInt((map.lon() + M_PI) / (2. * M_PI) * (double)mapSize.width());
            int y = util::roundToInt((M_PI_2 - map.lat()) / M_PI * (double)mapSize.height());
            return Position{x, y};
        }
    };
}

