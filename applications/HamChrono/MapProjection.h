/**
 * @file MapProjections.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#pragma once

#include <mutex>
#include <Button.h>

#include "AntiAliasedDrawing.h"
#include "Cache.h"
#include "Math.h"
#include "Rose.h"
#include "Ephemeris.h"
#include "Plan13.h"
#include "SettingsNames.h"

namespace rose {

    static constexpr double GRAYLINE_COS = -0.208;      ///< cos(90 + grayline angle), we use 12 degs
    static constexpr double GRAYLINE_POW =	0.75;       ///< cos power exponent, sqrt is too severe, 1 is too gradual

    enum class EphemerisFile : size_t {
        ClearSkyMoon,
        ClearSkyAll,
        CTAmateur,
        CTCube,
        CTVisual,
    };

    enum class MapProjectionType {
        Mercator,
        StationMercator,
        StationAzimuthal,
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

    struct SatelliteMetaData {
        std::string name;
        ImageId imageId;
        bool passRiseOk{}, passSetOk{};
        DateTime riseTime{};
        DateTime setTime{};
        double elevation{}, azimuth{}, range{}, rangeRate{};

        bool operator<(const SatelliteMetaData &other) const {
            if (passRiseOk && other.passRiseOk)
                return riseTime < other.riseTime;
            else
                return setTime < other.setTime;
        }

        /**
         * @brief Set the pass times for the Satellite.
         * @param rise The rise time.
         * @param set The set time.
         */
        void setPassData(bool riseOk, bool setOk, DateTime rise, DateTime set) {
            passRiseOk = riseOk;
            passSetOk = setOk;
            riseTime = rise;
            setTime = set;
        }

        /**
         * @brief Get the data relative to
         * @return
         */
        [[nodiscard]] std::tuple<bool, bool, DateTime, DateTime> getPassData() const { return std::make_tuple(passRiseOk, passSetOk, riseTime, setTime); }

        /**
         * @brief Create a std::string that describes the pass.
         * @details If relative == 0 then times that are converted are converted to absolute dates and times in GMT.
         * If the rise time is valid and in the future it is entered into the string first followed by " - " and the
         * set time. If relative is not 0 the set time is converted relative to the rise time. This provides the
         * rise time followed by the pass duration.<p/>
         * If the rise time is not valid only the set time is converted providing the set time if relative is 0 or
         * the duration if relative is not 0.
         * @param relative A time_t to make the strings relative to.
         * @return a std::string with the formatted pass timing data.
         */
        [[nodiscard]] std::string passTimeString(time_t relative = 0) const;
    };

    struct TrackedSatellite {
        SatelliteMetaData metaData; ///< Minimal data for display of the satellite status.
        Satellite satellite;        ///< The tracked Satellite.

        void predict(DateTime dateTime) {
            satellite.predict(dateTime);
        }

        void updateMetaData(Observer observer) {
            auto [elevation, azimuth, range, range_rate] = satellite.topo(observer);
            metaData.elevation = elevation;
            metaData.azimuth = azimuth;
            metaData.range = range;
            metaData.rangeRate = range_rate;
        }
    };

    /**
     * @class PartitionedLine
     * @brief Partition lines across discontinuities introduced by the current map projection.
     */
    class PartitionedLine : public std::vector<Position> {
    protected:
        /// The type of a line segment.
        using LineSegment = std::vector<Position>;

        /// The type of a partitioned line.
        using LineParts = std::vector<LineSegment>;

        /// The partitioned line segments
        LineParts mLineParts{};

    public:
        PartitionedLine() : std::vector<Position>() {}

        ~PartitionedLine() = default;

        /**
         * @brief A predicate which returns true if two Positions belong in the same partition.
         */
        using Predicate = std::function<bool(Position &, Position &)>;

        void partition() {
            LineSegment segment{};
            std::copy(begin(), end(), std::back_inserter(segment));
            mLineParts.emplace_back(segment);
            clear();
        }

        void partition(Predicate p) {
            mLineParts.clear();
            if (!empty()) {
                auto first = begin();
                auto last = end();
                LineSegment work{};
                work.emplace_back(*first);
                ++first;
                while (first != last) {
                    if (p(work.back(), *first)) {
                        work.emplace_back(*first);
                    } else {
                        if (work.size() > 1)
                            mLineParts.emplace_back(work);
                        work.clear();
                        work.emplace_back(*first);
                    }
                    ++first;
                }
                if (work.size() > 1)
                    mLineParts.emplace_back(work);
                work.clear();
            }
            clear();
        }

        bool draw(std::function<bool(const Position &, const Position &)> p) {
            for (const auto &segment : mLineParts) {
                Position p0 = segment.front();
                std::for_each(segment.begin()+1, segment.end(), [&p,&p0](const Position &p1){
                    if (!p(p0,p1))
                        return false;
                    p0 = p1;
                    return true;
                });
            }
            return true;
        }
    };

    /**
     * @class MapProjection
     * @brief
     */
    class MapProjection : public Widget {
    protected:
        SignalSerialNumber mSignalSerialNumber{};
        Rose::IconFileItem mMoonIconSpec{static_cast<ImageId>(set::AppImageId::Moon), Size{0, 0}, "full_moon.png"};

        /// Twilight specs: civil, nautical, astronomical.
        static constexpr std::array<double, 3> GrayLineCos = {-0.105, -0.208,
                                                              -0.309};   ///< Sets the width of the dawn/dusk period.
        static constexpr double GrayLinePow = .80;      ///< Sets the speed of transitions, smaller is sharper. (.75)

        /// A list of satellite icons to initialize a stack that will be used to manage the icons.
        static constexpr std::array<set::AppImageId,5> mSatelliteIconArray{
                set::AppImageId::DotPurple, set::AppImageId::DotYellow,
                set::AppImageId::DotGreen, set::AppImageId::DotBlue, set::AppImageId::DotRed, };

        /// A stack for managing the satellite icons.
        std::stack<set::AppImageId> mSatelliteIconStack{};

        bool mTerrestrialMode{};                  ///< True when map is in Terrestrial mode, excludes SatelliteMode
        bool mSatelliteMode{};                    ///< True when map is in SatelliteMode, excludes TerrestrialMode
        bool mCelestialMode{};                    ///< True when map is displaying Celestial objects.
        bool mAnnotationMode{};                   ///< True when map is displaying Annotations.

        std::string mSatelliteFavorite{};         ///< User selected favorite Satellite.

        std::future<bool> mFutureAziProj{};       ///< A future for the map azimuthal projections.
        std::future<bool> mFutureSun{};           ///< A future for the map sun illumination.
        std::atomic_bool mNewSurfaces{};          ///< True when there are new Surfaces to render into Textures.
        std::atomic_bool mAbortFuture{};          ///< Set to true to abort a running std::future.

        MapProjectionType mProjection{MapProjectionType::Mercator};   ///< The desired projection display
        std::shared_ptr<WebFileCache> mMapCache{};
        ImageId mDayMapImage{};         ///< The base day Mercator map image.
        ImageId mNightMapImage{};       ///< The base night Mercator map image.
        GeoPosition mQth{};             ///< The station location.
        GeoPosition mQthRad{};          ///< The station location in radians.
        GeoPosition mAntipode{};        ///< The station antipode in radians.
        Size mMapImgSize{};                ///< The size of the base maps in pixels.
        int mSelectedSatellite{};       ///< The Satellite to display tracking for.

        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)> mMapSurface{};
        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)> mAzSurface{};

        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)/2> mMercatorTemp;
        std::array<sdl::Surface,static_cast<std::size_t>(MapDataType::MapCount)/2> mAzimuthalTemp;

        std::atomic_bool mUpdateEphemeris{false};
        std::array<std::filesystem::path, 5> mEphemerisFilePath{};      ///< The set of cached ephemeris files.
        EphemerisFile mEphemerisFile{EphemerisFile::ClearSkyAll};       ///< The index to the ephemeris in use.
        void updateEphemerisFile();                 ///< Update ephemeris and start tracking.
        Observer mObserver{};                       ///< The QTH Observer data.

        std::vector<TrackedSatellite> mSatelliteList{};    ///< The list of Satellites being tracked.
        std::mutex mSatListMutex{};                 ///< A mutex to protect write access to mSatelliteList.
        double mMinimumElevation{15.};              ///< The minimum pass elevation to track satellite.

        /// Find the next pass of Satellite over Observer.
        static std::tuple<bool, bool, double, double, double, DateTime, DateTime>
        findNextPass(const Satellite &satellite, const Observer &observer);

        std::array<sdl::Texture,2> mMercator{}; ///< The Mercator projection background and foreground maps.
        std::array<sdl::Texture,2> mAzimuthal{};    ///< The Azimuthal projection background and foreground maps.

        sdl::Texture mNightAz{};        ///< The Texture with the Night Azimuthal map.
        sdl::Texture mDayAz{};          ///< The Texture with the Day Azimuthal map.

        std::shared_ptr<Slot<std::string>> mSettingsUpdateRx{};  ///< Slot to receive settings updates on.

        /**
         * @brief Draw a Satellite orbital path.
         * @param satellite The Satellite
         */
        void drawOrbitalPath(sdl::Renderer &renderer, TrackedSatellite &satellite, Position mapPos, int splitPixel);

        void drawFootprint(sdl::Renderer &renderer, TrackedSatellite &satellite, Position mapPos, int splitPixel);

        std::unique_ptr<AntiAliasedDrawing> mDrawingContext{};

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

        /**
         * @struct MapItem
         * @brief The information needed to render an Icon on the map.
         */
        struct MapIcon {
            ImageId imageId{RoseImageInvalid};    ///< The ImageId of the icon to render.
            GeoPosition geo{};                    ///< The GeoPosition in radians.
        };

        std::array<MapIcon,2> mStationIcons{};      ///< Icons for QTH and Antipode.
        std::array<MapIcon,2> mCelestialIcons{};    ///< Near earth objects (Sun, Moon, etc.).

        Satellite mMoon{};                          ///< Orbital data for the moon.

        void setStationIcons(GeoPosition qth) {
            mQth = qth;
            mQthRad = GeoPosition{deg2rad(qth.lat()), deg2rad(qth.lon())};
            mAntipode = antipode(mQthRad);
            mStationIcons[0].geo = mQthRad;
            mStationIcons[1].geo = mAntipode;
            mStationIcons[0].imageId = static_cast<ImageId>(set::AppImageId::RingGreen);
            mStationIcons[1].imageId = static_cast<ImageId>(set::AppImageId::RingRed);
        }

        void setMoonPhase();
        void setCelestialIcons();

        /**
         * @brief Render a single icon on the map.
         * @param mapItem The MapItem data.
         * @param renderer The Renderer.
         * @param projection True if the projection is Azimuthal.
         * @param splitPixel The split location for Mercator station centric projections.
         */
        void drawMapItem(const MapIcon &mapItem, sdl::Renderer &renderer, Rectangle mapRectangle, MapProjectionType projection,
                         int splitPixel);

        /**
         * @brief Render a container of icons [first ... last) on the map.
         * @param first The first item in the container.
         * @param last One past the end of the container.
         * @param renderer The Renderer.
         * @param projection True if the projection is Azimuthal.
         * @param splitPixel The split location for Mercator station centric projections.
         */
        template<typename InputIterator>
        void drawMapItems(InputIterator first, InputIterator last, sdl::Renderer &renderer, Rectangle mapRect,
                          MapProjectionType projection, int splitPixel = 0) {
            while (first != last) {
                drawMapItem(*first, renderer, mapRect, projection, splitPixel);
                ++first;
            }
        }

    public:
        MapProjection() = delete;

        ~MapProjection() override = default;

        MapProjection(std::shared_ptr<WebFileCache> mapCache, Size mapSize);

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
            return GeoPosition{-posRadians.lat(), (posRadians.lon() < 0. ? 1. : -1.) * (M_PI - abs(posRadians.lon()))};
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

        /**
         * @brief Convert a GeoPosition in radians to a map Position in pixels.
         * @param projection True if the desired map is an Azimuthal projection.
         * @return The map Position.
         */
        Position geoToMap(GeoPosition geo, MapProjectionType projection, int splitPixel);

        /**
         * @brief Set the Moon ephemeris
         * @param fileName The ephemeris.
         */
        void setMoonEphemerisFile(EphemerisFile item, const std::filesystem::path &filePath) {
            mEphemerisFilePath[static_cast<std::size_t>(item)] = filePath;
            if (item == EphemerisFile::ClearSkyMoon) {
                // If the file is the Moon ephemeris set or update the Moon TLE.
                Ephemeris ephemeris{filePath};
                mMoon.setEphemeris(ephemeris["Moon"]);
                setMoonPhase();
                setCelestialIcons();
            } else if (mEphemerisFile == item){
                // If the ephemeris in use, update the the satellite ephemeris processor.
                updateEphemerisFile();
            }
        }

        using SignalType = std::vector<TrackedSatellite>&;

        /// Transmit satellite tracking data.
        Signal<SignalType> trackedSatelliteTx{};

        /// Receive satellite tracking selection.
        std::shared_ptr<Slot<RadioBehavior::SignalType>> satelliteSelectRx{};
    };
}

