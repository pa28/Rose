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
#include "ImageStore.h"
#include "Texture.h"
#include "TimerTick.h"
#include "SatelliteModel.h"
#include "Surface.h"
#include "AntiAliasedDrawing.h"

// https://earthobservatory.nasa.gov/features/NightLights/page3.php
// https://visibleearth.nasa.gov/images/57752/blue-marble-land-surface-shallow-water-and-shaded-topography
// https://visibleearth.nasa.gov/images/57752/blue-marble-land-surface-shallow-water-and-shaded-topography/57754l
// https://commons.wikimedia.org/wiki/File:Large_World_Topo_Map_2.png
// https://commons.wikimedia.org/wiki/File:The_earth_at_night.jpg
// https://commons.wikimedia.org/wiki/File:The_earth_at_night_(2).jpg

namespace rose {

    namespace set {
        static constexpr std::string_view ChronoMapProjection{"MapProjection"};
        static constexpr std::string_view ChronoMapDepiction{"MapDepiction"};
    }

    /**
     * @enum MapProjectionType
     */
    enum class MapProjectionType {
        Mercator,           ///< Standard Mercator split a the International Date Line.
        StationMercator,    ///< Mercator split so the Station location is centred.
        StationAzimuthal,   ///< Azimuthal with the Station location centered on the left hemisphere.
    };

    /**
     * @enum MapDepiction
     */
    enum class MapDepiction {
        Terrain,            ///< The terrain map.
        Countries,          ///< The countries map.
    };

    /**
     * @enum MapIllumination
     */
    enum class MapIllumination {
        Day,                ///< The Day map.
        Night,              ///< The Night map.
    };

    /**
     * @enum The map size.
     */
    enum class MapSize {
        Small,              ///< Small 660 x 330
        Medium,             ///< Medium 1320 x 660
        Large,              ///< Large 1980 x 990
        ExtraLarge,         ///< ExtraLarger 2640 x 1320
        Last
    };

    /**
     * @brief Get a map Size from MapSize.
     * @param mapSize The MapSize value.
     * @return The size of the map.
     */
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

    /**
     * @brief Create an image ide for a specific map.
     * @param mapDepiction The MapDepiction.
     * @param mapSize The MapSize.
     * @param illumination The MapIllumination.
     * @return A composed image id.
     */
    static constexpr uint32_t MapImageId(MapDepiction mapDepiction, MapSize mapSize, MapIllumination illumination) {
        return (static_cast<uint32_t>(mapSize) << 2u) | (static_cast<uint32_t>(mapDepiction) << 1u) | static_cast<uint32_t>(illumination);
    }

    /**
     * @brief Crate a file name for a specific map.
     * @param mapDepiction The MapDepiction.
     * @param mapSize The MapSize.
     * @param illumination The MapIllumination.
     * @return The composed file name.
     */
    static std::string MapFileName(MapDepiction mapDepiction, MapSize mapSize, MapIllumination illumination) {
        std::stringstream sstrm{};

        switch (mapDepiction) {
            case MapDepiction::Terrain:
                sstrm << "World_Topo_";
                break;
            case MapDepiction::Countries:
                sstrm << "Countries";
                break;
            default:
                break;
        }

        switch (illumination) {
            case MapIllumination::Day:
                sstrm << "D_";
                break;
            case MapIllumination::Night:
                sstrm << "N_";
                break;
            default:
                break;
        }

        auto size = MapImageSize(mapSize);
        sstrm << size.w << 'x' << size.h;

        sstrm << ".png";
        return sstrm.str();
    }

    /// Convert radians to degrees.
    template<typename T>
    static constexpr T rad2deg(T r) noexcept {
        return 180. * (r/M_PI);
    }

    /// Convert degrees to radians.
    template<typename T>
    static constexpr T deg2rad(T d) noexcept {
        return M_PI * (d/180.);
    }

    /**
     * @class GeoPosition
     * @brief An abstraction of a geographic position.
     */
    struct GeoPosition {
        double lat{0.}, lon{0.};
        bool radians{false};

        GeoPosition() = default;

        /**
         * @brief Create a geographic position.
         * @param latitude The latitude.
         * @param longitude The longitude.
         * @param rad When true interpret latitude and longitude as degrees, otherwise as radians.
         */
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

        /**
         * @brief Convert the position from degrees to radians.
         * @return The position in radians.
         */
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

        /**
         * @brief Convert the position from radians to degrees.
         * @return The position in degrees.
         */
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

    enum class MapOverLayImage : size_t {
        Sun,
        Moon,
        Count,
    };

    struct OverlayImageSpec {
        MapOverLayImage mapOverLayImage;
        std::string_view fileName;
    };

    /**
     * @class MapProjection
     * @brief
     */
    class MapProjection : public Manager {
    public:
        enum ShortCutCode : uint32_t {
            MercatorProjection,
            StationMercatorProjection,
            AzimuthalProjection,
            TerrainMap,
            CountryMap,
        };

    protected:
        /// Source of timing information.
        std::shared_ptr<TimerTick> mTimerTick{};

        /// Slot to receive illumination update time signals on.
        TickProtocol::slot_type mMapIlluminationTimer{};

        /// The pointer to the map cache.
//        std::unique_ptr<WebCache> mMapCache{};

        /// The slot to receive map cache events.
        WebCacheProtocol::slot_type mMapSlot{};

        /// The map projection type.
        MapProjectionType mProjection{};

        /// The map depiction.
        MapDepiction mMapDepiction{MapDepiction::Terrain};

        /// The map size.
        MapSize mMapSize{MapSize::Small};

        /// True when new surfaces have been created.
        bool mNewSurfaces{};

        /// The size of the map image.
        Size mMapImgSize{};

        std::array<gm::Surface,2> mMapSurface{};
        std::array<gm::Surface,2> mAzSurface{};

        std::array<gm::Surface,2> mMercatorTemp{};
        std::array<gm::Surface,2> mAzimuthalTemp{};

        std::array<gm::Texture,2> mMercator{};     ///< The Mercator projection background and foreground maps.
        std::array<gm::Texture,2> mAzimuthal{};    ///< The Azimuthal projection background and foreground maps.

        std::atomic_bool mAbortFuture{};           ///< A flag to abort background processing.

        /// The station location in degrees.
        GeoPosition mQth{45.,-75.};

        /// The station location in radians.
        GeoPosition mQthRad{mQth.toRadians()};

        /// Twilight specs: civil, nautical, astronomical.
        static constexpr std::array<double, 3> GrayLineCos = {-0.105,
                                                              -0.208,
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

        /// The std::future result of computeAzimuthalMaps()
        std::future<bool> mComputeAzimuthalMapsFuture;

        /// True when base maps have not been loaded or projected for use.
        bool mMapProjectionsInvalid{true};

        /// The std::future result of setForegroundBackground()
        std::future<bool> mForegroundBackgroundFuture;

        /**
         * @brief Compute the sun illumination pattern.
         * @details This creates a background foreground map pair. The background is the night map, the
         * foreground is the day map which has had the Alpha channel modified to match the projected
         * illumination from the sun. This method only generates the Surfaces which are then used to create
         * Textures that are displayed. The Surface to Texture conversion must happen on the main thread so
         * locking is not an issue and the normal render cycle can continue as long as the last Texture is valid.
         */
        bool setForegroundBackground();

        /**
         * @brief Convert a GeoPosition in radians to a map Position in pixels.
         * @param geo The surface geographic position.
         * @param projection The map projection type.
         * @param splitPixel The horizontal pixel to split SationMercator maps.
         * @param mapRect The screen rectanble of the map.
         * @return The map Position.
         */
        Position geoToMap(GeoPosition geo, MapProjectionType projection, int splitPixel, Rectangle &mapRect) const;

    public:
        MapProjection() = delete;

        explicit MapProjection(std::shared_ptr<TimerTick> timerTick, std::filesystem::path& xdgDataPath);

        ~MapProjection() override = default;

        MapProjection(const MapProjection &) = delete;

        MapProjection(MapProjection &&) = delete;

        MapProjection &operator=(const MapProjection &) = delete;

        MapProjection &operator=(MapProjection &&) = delete;

        static constexpr std::string_view id = "MapProjection";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /// Draw the MapProjection
        void draw(gm::Context &context, const Position &containerPosition) override;

        /// Layout the MapProjection
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /**
         * @brief Called when added to a container.
         * @details Connects the map slot receiver for notification when maps are loaded, then calls
         * cacheCurrentMaps().
         */
        void addedToContainer() override;

        /**
         * @brief Add the selected maps to the current cache list and invoke a load if required.
         */
        void cacheCurrentMaps();

        /**
         * Compute the sub-solar geographic coordinates, used in plotting the solar illumination.
         * @return a tuple with the latitude, longitude in radians
         */
        static std::tuple<double, double> subSolar();

        /*
         * @brief Accessor for Qth
         */
        auto getQth() const {
            return mQth;
        }

        /**
         * @brief Determine if map projections are valid.
         * @return Return true if valid, false otherwise.
         */
        bool mapProjectionsValid() const {
            return !mMapProjectionsInvalid; // && !mForegroundBackgroundFuture.valid();
        }

        /**
         * @brief Accessor for the type of projection.
         * @return A MapProjectionType.
         */
        auto getProjection() const {
            return mProjection;
        }

        /**
         * @brief Render a single icon on the map.
         * @param mapItem The MapItem data.
         * @param renderer The Renderer.
         * @param projection True if the projection is Azimuthal.
         * @param splitPixel The split location for Mercator station centric projections.
         */
        void drawMapItem(const ImageId &mapItem, gm::Context& context, Rectangle mapRectangle, GeoPosition& geoPosition,
                         MapProjectionType projection, int splitPixel);

        /**
         * @brief Computer the StationMercator split pixel given a map drawing size.
         * @param drawSize The size of the drawing area in use for the map.
         * @return The split pixel.
         */
        int projectionSplitPixel(Size drawSize) const {
            int splitPixel = util::roundToInt((double) drawSize.w * ((mQth.lon) / 360.));
            if (splitPixel < 0)
                splitPixel += drawSize.w;
            return splitPixel;
        }

        void drawLongitude(gm::Context &context, AntiAliasedDrawing &drawing, double longitude, Rectangle mapRect) {
            GeoPosition geoPosition{0., longitude, true};
            auto p0 = geoToMap(geoPosition, mProjection, projectionSplitPixel(mapRect.size()), mapRect);
            p0.y = mapRect.y;
            auto p1 = p0;
            p1.y += mapRect.h;
            drawing.renderLine(context, p0, p1);
        }

        void drawLatitude(gm::Context &context, AntiAliasedDrawing &drawing, double latitude, Rectangle mapRect) {
            GeoPosition geoPosition{latitude, 0., true};
            auto p0 = geoToMap(geoPosition, mProjection, projectionSplitPixel(mapRect.size()), mapRect);
            p0.x = mapRect.x;
            auto p1 = p0;
            p1.x += mapRect.w;
            drawing.renderLine(context, p0, p1);
        }
    };
}

