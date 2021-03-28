/**
 * @file MapProjection.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-27
 */

#pragma once

#include <filesystem>
#include "Visual.h"
#include "WebCache.h"
#include "GraphicsModel.h"
#include "Texture.h"
#include "Surface.h"

namespace rose {

    enum class MapProjectionType {
        Mercator,
        StationMercator,
        StationAzimuthal,
    };

    enum class MapDepiction {
        Terrain,
        Countries,
        Last
    };

    enum class MapIllumination {
        Day,
        Night,
        Last
    };

    enum class MapSize {
        Small,
        Medium,
        Large,
        ExtraLarge,
        Last
    };

    static constexpr Size MapImageSize(MapSize mapSize) {
        switch (mapSize) {
            case MapSize::Small:
                return Size{660,330};
            case MapSize::Medium:
                return Size{1320, 660};
            case MapSize::Large:
                return Size{1980, 990};
            case MapSize::ExtraLarge:
                return Size{2640,1320};
            default:
                break;
        }
        return Size{};
    }

    static constexpr uint32_t MapImageId(MapDepiction mapDepiction, MapSize mapSize, MapIllumination illumination) {
        return (static_cast<uint32_t>(mapSize) << 2u) | (static_cast<uint32_t>(mapDepiction) << 1u) | static_cast<uint32_t>(illumination);
    }

    static std::string MapFileName(MapDepiction mapDepiction, MapSize mapSize, MapIllumination illumination) {
        std::stringstream sstrm{};

        sstrm << "map-";
        switch (illumination) {
            case MapIllumination::Day:
                sstrm << "D-";
                break;
            case MapIllumination::Night:
                sstrm << "N-";
                break;
            default:
                break;
        }

        auto size = MapImageSize(mapSize);
        sstrm << size.w << 'x' << size.h << '-';

        switch (mapDepiction) {
            case MapDepiction::Terrain:
                sstrm << "Terrain";
                break;
            case MapDepiction::Countries:
                sstrm << "Countries";
                break;
            default:
                break;
        }

        sstrm << ".bmp";
        return sstrm.str();
    }

    template<typename T>
    static constexpr T rad2deg(T r) noexcept {
        return 180. * (r/M_PI);
    }

    template<typename T>
    static constexpr T deg2rad(T d) noexcept {
        return M_PI * (d/180.);
    }

    struct GeoPosition {
        double lat{0.}, lon{0.};
        bool radians{false};

        GeoPosition() = default;

        ~GeoPosition() = default;

        GeoPosition(double latitude, double longitude, bool rad = false) {
            radians = rad;
            if (radians) {
                lat = std::clamp(latitude, -M_PI_2, M_PI_2);
                lon = std::clamp(longitude, -M_PI, M_PI);
            } else {
                lat = std::clamp(latitude, -90., 90.);
                lon = std::clamp(longitude, -180., 180.);
            }
        }

        [[nodiscard]] GeoPosition toRadians() const {
            if (radians)
                return GeoPosition{*this};
            else {
                GeoPosition g{};
                g.radians = true;
                g.lat = deg2rad(lat);
                g.lon = deg2rad(lon);
                return g;
            }
        }

        [[nodiscard]] GeoPosition toDegrees() const {
            if (radians) {
                GeoPosition g{};
                g.radians = false;
                g.lat = rad2deg(lat);
                g.lon = rad2deg(lon);
                return g;
            } else
                return GeoPosition{*this};
        }
    };

    /**
     * @class MapProjection
     * @brief
     */
    class MapProjection : public Widget {
    protected:
        WebCache mMapCache;
        WebCacheProtocol::slot_type mMapSlot{};
        MapProjectionType mProjection{MapProjectionType::StationAzimuthal};
        MapDepiction mMapDepiction{MapDepiction::Terrain};
        MapSize mMapSize{MapSize::Small};
        bool mNewSurfaces{};
        size_t mSelectedSize{0};

        Size mMapImgSize{};

        std::array<gm::Surface,2> mMapSurface{};
        std::array<gm::Surface,2> mAzSurface{};

        std::array<gm::Surface,2> mMercatorTemp{};
        std::array<gm::Surface,2> mAzimuthalTemp{};

        std::array<gm::Texture,2> mMercator{};     ///< The Mercator projection background and foreground maps.
        std::array<gm::Texture,2> mAzimuthal{};    ///< The Azimuthal projection background and foreground maps.

        std::atomic_bool mAbortFuture{};
        GeoPosition mQth{45.,-75.};
        GeoPosition mQthRad{mQth.toRadians()};

        /// Twilight specs: civil, nautical, astronomical.
        static constexpr std::array<double, 3> GrayLineCos = {-0.105, -0.208,
                                                              -0.309};   ///< Sets the width of the dawn/dusk period.
        static constexpr double GrayLinePow = .80;      ///< Sets the speed of transitions, smaller is sharper. (.75)

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

        MapProjection(const MapProjection &) = delete;

        MapProjection(MapProjection &&) = delete;

        MapProjection &operator=(const MapProjection &) = delete;

        MapProjection &operator=(MapProjection &&) = delete;

        explicit MapProjection(const std::filesystem::path& configHome);

        void draw(gm::Context &context, const Position &containerPosition) override;

        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

    };
}

