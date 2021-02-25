/**
 * @file MapProjections.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#include "MapProjection.h"

#include <algorithm>
#include <filesystem>
#include <utility>
#include "Color.h"
#include "Math.h"
#include "SettingsNames.h"

namespace rose {

    MapProjection::MapProjection(std::shared_ptr<WebFileCache> mapCache, Size mapSize) {
        mMapCache = std::move(mapCache);
        mMapSize = mapSize;
    }

    void MapProjection::initializeComposite() {
        mapFileRx = std::make_shared<Slot<uint32_t>>();
        setStationIcons(rose()->settings()->getValue(set::QTH, GeoPosition{0.,0.}));
        setCelestialIcons();

        mObserver = Observer{mQth.lat(), mQth.lon(), 0.};

        mapFileRx->setCallback([&](uint32_t, uint32_t map){
            std::filesystem::path filePath(mMapCache->cacheRootPath());
            filePath.append(mMapCache->at(map).objectSrcName());
            sdl::Surface bmp{filePath};
            mMapSurface[map] = sdl::Surface{mMapSize.width(),mMapSize.height()};
            mMapSurface[map].blitSurface(bmp);

            mAzSurface[map] = sdl::Surface{mMapSurface[map]->w, mMapSurface[map]->h};
            for (auto &surface : mMapSurface)
                if (!surface)
                    return;

            mFutureAziProj = std::async(std::launch::async, &MapProjection::computeAzimuthalMaps, this);
        });

        secondRx = std::make_shared<Slot<int>>();
        secondRx->setCallback([&](uint32_t, int second){
            updateEphemerisFile();
            setNeedsDrawing();

            if (second % 5 == 1) {
                DateTime now{true};
                for (auto & satellite : mSatelliteList) {
                    satellite.satellite.predict(now);
                }
            }
            std::chrono::milliseconds span(1);      // The length of time to wait for a future.

            if (mFutureAziProj.valid()) {
                if (auto status = mFutureAziProj.wait_for(span); status == std::future_status::ready) {
                    if (mFutureAziProj.get()) {
                        mFutureSun = std::async(std::launch::async, &MapProjection::setForegroundBackground, this);
                    } else {
                        std::cerr << "Future Azimuth Projection failed.\n";
                    }
                }
            }

            if (mFutureSun.valid()) {
                if (auto status = mFutureSun.wait_for(span); status == std::future_status::ready) {
                    if (mNewSurfaces = mFutureSun.get(); mNewSurfaces) {
                        setNeedsDrawing();
                    } else {
                        std::cerr << "Future Sun Illumination failed.\n";
                    }
                }
            }
        });

        minuteRx = std::make_shared<Slot<int>>();
        minuteRx->setCallback([&](uint32_t, int minute){
            setCelestialIcons();
            mUpdateEphemeris = true;
            if (!mFutureSun.valid())
                mFutureSun = std::async(std::launch::async, &MapProjection::setForegroundBackground, this);
        });

        mMapCache->itemFetched.connect(mapFileRx);

        mSettingsUpdateRx = std::make_shared<Slot<std::string>>();
        mSettingsUpdateRx->setCallback([&](uint32_t, const std::string& name){
            if (name == set::QTH) {
                mAbortFuture = true;
                if (mFutureAziProj.valid()) {
                    mFutureAziProj.get();
                } else if (mFutureSun.valid()) {
                    mFutureSun.get();
                }
                mAbortFuture = false;
                setStationIcons(rose()->settings()->getValue(set::QTH, GeoPosition{0.,0.}));
                mFutureAziProj = std::async(std::launch::async, &MapProjection::computeAzimuthalMaps, this);
            } else if (name == set::AzimuthalMode) {
                mProjection = rose()->settings()->getValue(set::AzimuthalMode, 0) ? ProjectionType::StationAzmuthal :
                        ProjectionType::StationMercator;
                setNeedsDrawing();
            } else if (name == set::CelestialMode) {
                mCelestialMode = rose()->settings()->getValue(set::CelestialMode, 0) != 0;
                setNeedsDrawing();
            } else if (name == set::SatelliteMode) {
                mSatelliteMode = rose()->settings()->getValue(set::SatelliteMode, 0) != 0;
                if (mSatelliteMode)
                    rose()->settings()->setValue(set::TerrestrialMode, 0);
                setNeedsDrawing();
            } else if (name == set::TerrestrialMode) {
                mTerrestrialMode = rose()->settings()->getValue(set::TerrestrialMode, 0) != 0;
                if (mTerrestrialMode)
                    rose()->settings()->setValue(set::SatelliteMode, 0);
                setNeedsDrawing();
            } else if (name == set::AnnotationMode) {
                mAnnotationMode = rose()->settings()->getValue(set::AnnotationMode, 0) != 0;
                setNeedsDrawing();
            }
        });

        mProjection = rose()->settings()->getValue(set::AzimuthalMode, 0) ? ProjectionType::StationAzmuthal :
                      ProjectionType::StationMercator;
        mCelestialMode = rose()->settings()->getValue(set::CelestialMode, 0) != 0;
        mSatelliteMode = rose()->settings()->getValue(set::SatelliteMode, 0) != 0;
        mAnnotationMode = rose()->settings()->getValue(set::AnnotationMode, 0) != 0;
        rose()->settings()->dataChangeTx.connect(mSettingsUpdateRx);

        mClassName = "MapProjection";
    }

    Rectangle MapProjection::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Rectangle{0, 0, mMapSize.width(), mMapSize.height()};
    }

    void MapProjection::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Rectangle widgetRect{};
        widgetRect = parentRect.getPosition() + mLayoutHints.mAssignedRect->getPosition();
        widgetRect = mLayoutHints.mAssignedRect->getSize();

        if (mNewSurfaces) {
            mNewSurfaces = false;
            for (size_t i = 0; i < mMercatorTemp.size(); ++i) {
                mMercator[i*2+1] = mMercatorTemp[i].toTexture(rose()->getRenderer());
                mMercator[i*2+1].setBlendMOde(SDL_BLENDMODE_BLEND);
                mMercatorTemp[i].reset(nullptr);
                mMercator[i*2] = mMapSurface[i*2+1].toTexture(rose()->getRenderer());
                mMercator[i*2].setBlendMOde(SDL_BLENDMODE_BLEND);
            }

            for (size_t i = 0; i < mAzimuthalTemp.size(); ++i) {
                mAzimuthal[i*2+1] = mAzimuthalTemp[i].toTexture(rose()->getRenderer());
                mAzimuthal[i*2+1].setBlendMOde(SDL_BLENDMODE_BLEND);
                mAzimuthalTemp[i].reset(nullptr);
                mAzimuthal[i*2] = mAzSurface[i*2+1].toTexture(rose()->getRenderer());
                mAzimuthal[i*2].setBlendMOde(SDL_BLENDMODE_BLEND);
            }
        }

        if (!mMercator[0] || !mAzimuthal[0]) {
            return;
        }

        int splitPixel = 0;
        switch (mProjection) {
            case ProjectionType::Mercator:
                renderer.renderCopy(mMercator[0], widgetRect);
                renderer.renderCopy(mMercator[1], widgetRect);
                break;
            case ProjectionType::StationMercator: {
                auto lon = mQth.lon();
                splitPixel = util::roundToInt((double)mMapSize.width() * ((lon) / 360.));
                if (splitPixel < 0)
                    splitPixel += mMapSize.width();

                Rectangle src{splitPixel, 0, mMapSize.width()-splitPixel, mMapSize.height()};
                Rectangle dst{widgetRect};
                dst.width() = src.width();
                dst.height() = src.height();
                renderer.renderCopy(mMercator[0], src, dst);
                renderer.renderCopy(mMercator[1], src, dst);

                src.x() = 0;
                dst.x() += src.width();
                src.width() = splitPixel;
                dst.width() = splitPixel;
                renderer.renderCopy(mMercator[0], src, dst);
                renderer.renderCopy(mMercator[1], src, dst);
            }
                break;
            case ProjectionType::StationAzmuthal:
                renderer.renderCopy(mAzimuthal[0], widgetRect);
                renderer.renderCopy(mAzimuthal[1], widgetRect);
                break;
        }

        drawMapItems(mStationIcons.begin(), mStationIcons.end(), renderer,
                     widgetRect, mProjection, splitPixel);

        if (mSatelliteMode) {
            if (!mDrawingContext)
                mDrawingContext = std::make_unique<AntiAliasedDrawing>(renderer, 2,
                                                                       color::RGBA{1.f, 0.f, 0.f, 1.f});
            std::lock_guard<std::mutex> lockGuard{mSatListMutex};
            for (auto &satellite : mSatelliteList) {
                GeoPosition geo{satellite.satellite.geo()};
                MapIcon mapItem{static_cast<ImageId>(satellite.dataStub.imageId), geo};
                drawMapItem(mapItem, renderer, widgetRect, mProjection, splitPixel);
            }

            if (mAnnotationMode) {
                drawOrbitalPath(renderer, mSatelliteList.front(), widgetRect.getPosition(), splitPixel);
                drawFootprint(renderer, mSatelliteList.front(), widgetRect.getPosition(), splitPixel);
            }
        }

        if (mCelestialMode)
            drawMapItems(mCelestialIcons.begin(), mCelestialIcons.end(), renderer,
                         widgetRect, mProjection, splitPixel);
    }

    void MapProjection::drawMapItem(const MapIcon &mapItem, sdl::Renderer &renderer, Rectangle mapRectangle, ProjectionType projection,
                                    int splitPixel) {
        if (mapItem.imageId == RoseImageInvalid)
            return;

        auto mapPos = geoToMap(mapItem.geo, projection, splitPixel);

        auto iconSize = rose()->imageRepository(mapItem.imageId).getSize();
        mapPos.x() -= iconSize.width() / 2;
        mapPos.y() -= iconSize.height() / 2;

        sdl::ClipRectangleGuard clipRectangleGuard(renderer, mapRectangle);

        mapPos = mapPos + mapRectangle.getPosition();
        Rectangle dst{mapPos, iconSize};
        int h = 0;
        if (dst.y() < mapRectangle.y()) {
            // Top hang
            h = mapRectangle.y() - dst.y();
        } else if (dst.y() + dst.height() > mapRectangle.y() + mapRectangle.height()) {
            // Bottom hang
            h = mapRectangle.y() + mapRectangle.height() - dst.y();
        }

        int w = 0;
        if (dst.x() < mapRectangle.x()) {
            w = mapRectangle.x() - dst.x();
        } else if (dst.x() + dst.width() > mapRectangle.x() + mapRectangle.width()) {
            w = mapRectangle.x() + mapRectangle.width() - dst.x();
        }

        if (h == 0 && w == 0){
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            return;
        }

        if (h == 0 && w > 0) {
            // Left/Right hang - draw left side on right of map
            dst.x() = mapRectangle.x() - w;
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            dst.x() += mapRectangle.width();
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
        } else if (h > 0 && w == 0) {
            if (projection != ProjectionType::StationAzmuthal || h < iconSize.height() / 2) {
                dst.y() = mapRectangle.y() - h;
                rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            }
            if (projection != ProjectionType::StationAzmuthal || h >= iconSize.height() / 2) {
                dst.y() += mapRectangle.height();
                rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            }
        } else if (h > 0 && w > 0 && projection != ProjectionType::StationAzmuthal) {
            dst.x() = mapRectangle.x() - w;
            dst.y() = mapRectangle.y() - h;
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            dst.x() += mapRectangle.width();
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            dst.y() += mapRectangle.height();
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            dst.x() -= mapRectangle.width();
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
        }
    }

    /* solve a spherical triangle:
     *           A
     *          /  \
     *         /    \
     *      c /      \ b
     *       /        \
     *      /          \
     *    B ____________ C
     *           a
     *
     * given A, b, c find B and a in range -PI..B..PI and 0..a..PI, respectively..
     * cap and Bp may be NULL if not interested in either one.
     * N.B. we pass in cos(c) and sin(c) because in many problems one of the sides
     *   remains constant for many values of A and b.
     */
    void solveSphere(double A, double b, double cc, double sc, double &cap, double &Bp) {
        double cb = cos(b), sb = sin(b);
        double sA, cA = cos(A);
        double x, y;
        double ca;
        double B;

        ca = cb * cc + sb * sc * cA;
        if (ca > 1.0F) ca = 1.0F;
        if (ca < -1.0F) ca = -1.0F;
        cap = ca;

        if (sc < 1e-7F)
            B = cc < 0 ? A : M_PI - A;
        else {
            sA = sin(A);
            y = sA * sb * sc;
            x = cb - ca * cc;
            B = y != 0.0 ? (x != 0.0 ? atan2(y, x) : (y > 0.0 ? M_PI_2 : -M_PI_2)) : (x >= 0.0 ? 0.0 : M_PI);
        }

        Bp = B;
    }

    GeoPosition projected(GeoPosition location, double angDist, double bearing) {
        auto lat = asin(sin(location.lat()) * cos(angDist) + cos(location.lat()) * sin(angDist) * cos(bearing));
        auto lon = location.lon() + atan2(sin(bearing) * sin(angDist) * cos(location.lat()),
                                          cos(angDist) - sin(location.lat()) * sin(lat));
        return GeoPosition{lat, lon};
    }

    double range(GeoPosition p0, GeoPosition p1) {
        auto sinSqLat = sin((p1.lat() - p0.lat()) / 2.);
        sinSqLat *= sinSqLat;
        auto sinSqLon = sin((p1.lon() - p0.lon()) / 2.);
        sinSqLon *= sinSqLon;
        auto a = sinSqLat + cos(p0.lat()) * cos(p1.lat()) * sinSqLon;
        auto c = 2. * atan2(sqrt(a), sqrt(1. - a));
        return c;
    }

    /**
     * Transform a Mercator map pixel into an Azimuthal map latitude and longitude in radians
     * @param x The map x pixel location 0 on the left
     * @param y The map y pixel location 0 at the top
     * @param mapSize the width (x) and height (y) of the map in pixels
     * @param location the longitude (x) and latitude (y) of the center of the projection
     * @param siny pre-computed sine of the latitude
     * @param cosy pre-computed cosine of the latitude
     * @return [valid, latitude, longitude ], valid if the pixel is on the Earth,
     * latitude -PI..+PI West to East, longitude +PI/2..-PI/2 North to South
     */
    std::tuple<bool, double, double>
    xyToAzLatLong(int x, int y, const Size &mapSize, const GeoPosition &location, double siny, double cosy) {
        bool onAntipode = x > mapSize.width() / 2;
        auto w2 = (mapSize.height() / 2) * (mapSize.height() / 2);
        auto dx = onAntipode ? x - (3 * mapSize.width()) / 4 : x - mapSize.width() / 4;
        auto dy = mapSize.height() / 2 - y;
        auto r2 = dx * dx + dy * dy;    // radius squared

        if (r2 <= w2) {
            auto b = sqrt((double ) r2 / (double ) w2) * M_PI_2;    // Great circle distance.
            auto A = M_PI_2 - atan2((double ) dy, (double ) dx);       // Azimuth
            double ca, B;
            solveSphere(A, b, (onAntipode ? -siny : siny), cosy, ca, B);
            auto lat = (float) M_PI_2 - acos(ca);
            auto lon = location.lon() + B + (onAntipode ? 6. : 5.) * (double ) M_PI;
            lon = fmod(location.lon() + B + (onAntipode ? 6. : 5.) * (double ) M_PI, 2 * M_PI) - (double ) M_PI;
            return std::make_tuple(true, lat, lon);
        }
        return std::make_tuple(false, 0., 0.);
    }

    bool MapProjection::computeAzimuthalMaps() {
        // Compute Azmuthal maps from the Mercator maps
        auto siny = sin(mQthRad.lat());
        auto cosy = cos(mQthRad.lat());
        for (int y = 0; y < mMapSize.height(); y += 1) {
            for (int x = 0; x < mMapSize.width(); x += 1) {
                if (mAbortFuture) {
                    mAbortFuture = false;
                    return false;
                }

                auto[valid, lat, lon] = xyToAzLatLong(x, y, mMapSize, mQthRad, siny, cosy);
                if (valid) {
                    GeoPosition position{lat, lon};
                    auto xx = min(mMapSize.width() - 1,
                                  (int) round((double) mMapSize.width() * ((lon + M_PI) / (2 * M_PI))));
                    auto yy = min(mMapSize.height() - 1,
                                  (int) round((double) mMapSize.height() * ((M_PI_2 - lat) / M_PI)));
                    mAzSurface[0].pixel(x, y) = sdl::mapRGBA(mAzSurface[0]->format,
                                                             sdl::getRGBA(mMapSurface[0]->format,
                                                                          mMapSurface[0].pixel(xx, yy)));
                    mAzSurface[1].pixel(x, y) = sdl::mapRGBA(mAzSurface[1]->format,
                                                             sdl::getRGBA(mMapSurface[1]->format,
                                                                          mMapSurface[1].pixel(xx, yy)));
                }
            }
        }
        return true;
    }

    /**
     * Compute the sub-solar geographic coordinates, used in plotting the solar ilumination.
     * @return a tuple with the latitude, longitude in radians
     */
    std::tuple<double, double> subSolar() {
        using namespace std::chrono;
        auto epoch = system_clock::now();
        time_t tt = system_clock::to_time_t(epoch);

        double JD = (tt / 86400.0) + 2440587.5;
        double D = JD - 2451545.0;
        double g = 357.529 + 0.98560028 * D;
        double q = 280.459 + 0.98564736 * D;
        double L = q + 1.915 * sin(M_PI / 180 * g) + 0.020 * sin(M_PI / 180 * 2 * g);
        double e = 23.439 - 0.00000036 * D;
        double RA = 180 / M_PI * atan2(cos(M_PI / 180 * e) * sin(M_PI / 180 * L), cos(M_PI / 180 * L));
        auto lat = asin(sin(M_PI / 180 * e) * sin(M_PI / 180 * L));
        auto lat_d = rad2deg(lat);
        double GMST = fmod(15 * (18.697374558 + 24.06570982441908 * D), 360.0);
        auto lng_d = fmod(RA - GMST + 36000.0 + 180.0, 360.0) - 180.0;
        auto lng = deg2rad(lng_d);

        return std::make_tuple(lat, lng);
    }

    bool MapProjection::setForegroundBackground() {
        for (size_t i = 0; i < mMercatorTemp.size(); ++i) {
            mMercatorTemp[i] = sdl::Surface{mMapSize};
            mAzimuthalTemp[i] = sdl::Surface{mMapSize};

            mMercatorTemp[i].setBlendMode(SDL_BLENDMODE_BLEND);
            mAzimuthalTemp[i].setBlendMode(SDL_BLENDMODE_BLEND);
            mMercatorTemp[i].blitSurface(mMapSurface[i*2]);
            mAzimuthalTemp[i].blitSurface(mAzSurface[i*2]);
        }

        auto[latS, lonS] = subSolar();

        auto siny = sin(mQthRad.lat());
        auto cosy = cos(mQthRad.lat());

        // Three loops for: Longitude, Latitude, and map form (Mercator, Azimuthal).
        // This lets us use the common calculations without repeating them, easier than
        // debugging two areas with the same computation.
        for (int x = 0; x < mMapSize.width(); x += 1) {
            for (int y = 0; y < mMapSize.height(); y += 1) {
                for (int az = 0; az < 2; ++az) {
                    float alpha = 1.;
                    bool valid;
                    float latE;
                    float lonE;

                    if (mAbortFuture) {
                        mAbortFuture = false;
                        return false;
                    }

                    if (az == 1) {
                        // The Azimuthal coordinates that correspond to a map pixel
                        auto tuple = xyToAzLatLong(x, y, mMapSize,
                                                   mQthRad, siny, cosy);
                        valid = get<0>(tuple);
                        latE = get<1>(tuple);
                        lonE = get<2>(tuple);
                    } else {
                        // The Mercator coordinates for the same map pixel
                        valid = true;
                        lonE = (float) ((float) x - (float) mMapSize.width() / 2.f) * (float) M_PI /
                               (float) ((float) mMapSize.width() / 2.);
                        latE = (float) ((float) mMapSize.height() / 2.f - (float) y) * (float) M_PI_2 /
                               (float) ((float) mMapSize.height() / 2.);
                    }
                    if (valid) {
                        // Compute the amont of solar illumination and use it to compute the pixel alpha value
                        // GrayLineCos sets the interior angle between the sub-solar point and the location.
                        // GrayLinePower sets how fast it gets dark.
                        auto cosDeltaSigma = sin(latS) * sin(latE) + cos(latS) * cos(latE) * cos(abs(lonS - lonE));
                        double fract_day;
                        if (cosDeltaSigma < 0) {
                            if (cosDeltaSigma > GrayLineCos[1]) {
                                fract_day = 1.0 - pow(cosDeltaSigma / GrayLineCos[1], GrayLinePow);
                                alpha = std::clamp((float)fract_day, 0.0313f, 1.f);
                            } else
                                alpha = 0.0313;  // Set the minimun alpha to keep some daytime colour on the night side
                        }
                    } else
                        alpha = 0;

                    // Set the alpha channel in the appropriate map.
                    if (az == 1) {
                        for (auto & azimuthalTemp : mAzimuthalTemp) {
                            auto pixel = sdl::getRGBA(azimuthalTemp->format, azimuthalTemp.pixel(x, y));
                            pixel.a() = alpha;
                            azimuthalTemp.pixel(x, y) = sdl::mapRGBA(azimuthalTemp->format, pixel);
                        }
                    } else {
                        for (auto & mercatorTemp : mMercatorTemp) {
                            auto pixel = sdl::getRGBA(mercatorTemp->format, mercatorTemp.pixel(x, y));
                            pixel.a() = alpha;
                            mercatorTemp.pixel(x, y) = sdl::mapRGBA(mercatorTemp->format, pixel);
                        }
                    }
                }
            }
        }
        return true;
    }

    void MapProjection::setMoonPhase() {
//        auto[sLat, sLon] = subSolar();
//
//        if (mMoon) {
//            DateTime predictTime{true};
//            mMoon.predict(predictTime);
//            auto[mLat, mLon] = mMoon.geo();
//
//            auto s2lat = sin((sLat - mLat) / 2.);
//            s2lat *= s2lat;
//            auto s2lon = sin((sLon - mLon) / 2.);
//            s2lon *= s2lon;
//            auto a = s2lat + cos(sLat) * cos(mLat) * s2lon;
//            auto sC = 2 * atan2(sqrt(a), sqrt(1. - a));
//            // A simple Moon phase calculation. Not astronomically accurate but should vie pleasing results.
//            // 0 deg == New ... waxing 180 == full ... waning ... 0
//
//            // Unit vector from the Earth to the Sun.
//            auto sx = sin(sLon);
//            auto sy = cos(sLon);
//            auto sz = sin(0);
//
//            // Unit vector from the Moon to the Earth
//            auto mx0 = -sin(mLon);
//            auto my0 = -cos(mLon);
//            auto mz0 = -sin(0);
//
//            auto dot0 = sx * mx0 + sy * my0 + sz * mz0;
//
////            std::cout << __PRETTY_FUNCTION__ << " Sun-Moon angle: " << sC << " rad, " << rad2deg(sC) << " deg\n"
////                      << sx << ' ' << sy << ' ' << sz << '\n'
////                      << mx0 << ' ' << my0 << ' ' << mz0 << '\n'
////                      << dot0 << '\n';
//            auto imagePath = rose()->getSharedImages();
//            imagePath.append("full_moon.png");
//            sdl::Surface moon{imagePath};
//            if (moon) {
//                for (int y = 0; y < moon->h; ++y) {
//                    // Convert y to pseudo-Latitude.
//                    auto mpLat = M_PI_2 - M_PI * (double) y / (double) moon->h;
//                    auto countX = 0;
//                    for (int x = 0; x < moon->w; ++x) {
//                        auto color = moon.color(x, y);
//                        if (color.a() > 0.)
//                            countX++;
//                    }
//                    if (countX == 0)
//                        continue;
//                    auto dLon = M_PI / (double) countX;
//                    double mpLon = -M_PI_2;
//                    for (int x = 0; x < moon->w; ++x) {
//                        auto color = moon.color(x, y);
//                        if (color.a() > 0.) {
//                            auto mx = sin(mpLon + M_PI_2);
//                            auto my = cos(mpLon + M_PI_2);
//                            auto mz = sin(0);
//
//                            auto dot = sx * mx + sy * my + sz * mz;
////                            std::cout << std::setw(4) << (int) (dot * 100);
//                            if (dot < 30.) {
//                                float mult = 0.95;
//                                if (dot < -10.)
//                                    mult = 0.5;
//                                else if (dot < 0.)
//                                    mult = 0.7;
//                                else if (dot < 10.)
//                                    mult = 0.9;
//                                color.r() *= mult;
//                                color.g() *= mult;
//                                color.b() *= mult;
//                                moon.setColor(x, y, color);
//                            }
//                            mpLon += dLon;
//                        } //else
////                            std::cout << "    ";
//                    }
////                    std::cout << '\n';
//                }
//                sdl::TextureData moonData{moon.toTexture(rose()->getRenderer())};
//                moonData.setBlendMOde(SDL_BLENDMODE_BLEND);
//                rose()->imageRepository().setImage(static_cast<ImageId>(set::AppImageId::Moon), std::move(moonData));
//                mCelestialIcons[1].imageId = static_cast<ImageId>(set::AppImageId::Moon);
//                mCelestialIcons[1].geo = GeoPosition{mLat, mLon};
//                setNeedsDrawing();
//            }
//        }
    }

    void MapProjection::setCelestialIcons() {
        auto[lat, lon] = subSolar();
        mCelestialIcons[0].imageId = static_cast<ImageId>(set::AppImageId::Sun);
        mCelestialIcons[0].geo = GeoPosition{lat, lon};

        if (mMoon) {
            DateTime predictTime{true};
            mMoon.predict(predictTime);
            auto[lat, lon] = mMoon.geo();
            mCelestialIcons[1].imageId = static_cast<ImageId>(set::AppImageId::Moon);
            mCelestialIcons[1].geo = GeoPosition{lat,lon};
        } else {
            mCelestialIcons[1].imageId = RoseImageInvalid;
        }
    }

    Position MapProjection::geoToMap(GeoPosition geo, ProjectionType projection, int splitPixel) {
        Position mapPos{};

        switch (projection) {
            case ProjectionType::StationAzmuthal: {
                double ca, B;
                solveSphere(geo.lon() - mQthRad.lon(), M_PI_2 - geo.lat(), sin(mQthRad.lat()),
                            cos(mQthRad.lat()), ca, B);
                if (ca > 0) {
                    auto a = acos(ca);
                    auto R0 = (double) mMapSize.width() / 4. - 1.;
                    auto R = a * (double) mMapSize.width() / (2. * M_PI);
                    R = std::min(R, R0);
                    auto dx = R * sin(B);
                    auto dy = R * cos(B);
                    mapPos = Position{mMapSize.width() / 4 + roundToInt(dx), mMapSize.height() / 2 - roundToInt(dy)};
                } else {
                    auto a = M_PI - acos(ca);
                    auto R0 = (double) mMapSize.width() / 4 - 1;
                    auto R = a * (double) mMapSize.width() / (2.f * (float) M_PI);
                    R = std::min(R, R0);
                    auto dx = -R * sin(B);
                    auto dy = R * cos(B);
                    mapPos = Position{3 * mMapSize.width() / 4 + roundToInt(dx), mMapSize.height() / 2 - roundToInt(dy)};
                }
            }
            break;
            case ProjectionType::Mercator:
                mapPos = Position{roundToInt(mMapSize.width() * (geo.lon() + M_PI) / (2. * M_PI)) % mMapSize.width(),
                            roundToInt(mMapSize.height() * (M_PI_2 - geo.lat()) / M_PI)};
                break;
            case ProjectionType::StationMercator: {
                mapPos = Position{roundToInt(mMapSize.width() * (geo.lon() + M_PI) / (2. * M_PI)) % mMapSize.width(),
                                roundToInt(mMapSize.height() * (M_PI_2 - geo.lat()) / M_PI)};
                mapPos.x() = (mapPos.x() + mMapSize.width() - splitPixel) % mMapSize.width();
                break;
            }
        }
        return mapPos;
    }

    std::tuple<bool, bool, double, double, double, DateTime, DateTime>
    MapProjection::findNextPass(const Satellite &satellite, const Observer &observer) {
        static constexpr long COARSE_DT = 90L;
        static constexpr long FINE_DT = (-2L);
        static constexpr double SAT_MIN_EL = 1.;


        DateTime t_now{}, set_time, rise_time;
        Satellite localSat{satellite};
        Observer localObs{observer};
        double max_elevation = 0., set_az, rise_az;

        t_now.userNow();

        double prevElevation{0};
        auto dt = COARSE_DT;
        DateTime t_srch = t_now + -FINE_DT;    // search time, start beyond any previous solution

        // init pel and make first step
        localSat.predict(t_srch);
        auto[tel, taz, trange, trate] = localSat.topo(observer);
        if (tel < SAT_MIN_EL)
            t_srch += dt;
        bool set_ok = false, rise_ok = false, ever_up = false, ever_down = false;

        // search up to a few days ahead for next rise and set times (for example for moon)
        while ((!set_ok || !rise_ok) && t_srch < t_now + 2.0F && (t_srch > t_now || tel > -1.)) {
            // find circumstances at time t_srch
            localSat.predict(t_srch);
            auto[tel, taz, trange, trate] = localSat.topo(observer);
            max_elevation = std::max(max_elevation, tel);

            // check for rising or setting events
            if (tel >= SAT_MIN_EL) {
                ever_up = true;
                if (prevElevation < SAT_MIN_EL) {
                    if (dt == FINE_DT) {
                        // found a refined set event (recall we are going backwards),
                        // record and resume forward time.
                        set_time = t_srch;
                        set_az = taz;
                        set_ok = true;
                        dt = COARSE_DT;
                        prevElevation = tel;
                    } else if (!rise_ok) {
                        // found a coarse rise event, go back slower looking for better set
                        dt = FINE_DT;
                        prevElevation = tel;
                    }
                }
            } else {
                ever_down = true;
                if (prevElevation > SAT_MIN_EL) {
                    if (dt == FINE_DT) {
                        // found a refined rise event (recall we are going backwards).
                        // record and resume forward time but skip if set is within COARSE_DT because we
                        // would jump over it and find the NEXT set.
                        DateTime check_set = t_srch + COARSE_DT;
                        localSat.predict(check_set);
                        auto[check_tel, check_taz, check_trange, check_trate] = localSat.topo(observer);
                        if (check_tel >= SAT_MIN_EL) {
                            rise_time = t_srch;
                            rise_az = taz;
                            rise_ok = true;
                        }
                        // regardless, resume forward search
                        dt = COARSE_DT;
                        prevElevation = tel;
                    } else if (!set_ok) {
                        // found a coarse set event, go back slower looking for better rise
                        dt = FINE_DT;
                        prevElevation = tel;
                    }
                }
            }
            t_srch += dt;
            prevElevation = tel;
        }
        return std::make_tuple(rise_ok, set_ok, rise_az, set_az, max_elevation, rise_time, set_time);
    }

    void MapProjection::updateEphemerisFile() {
        DateTime now{true};

        std::vector<TrackedSatellite> tracked{mSatelliteList};
        tracked.erase(std::remove_if(tracked.begin(), tracked.end(), [&now](const TrackedSatellite &i) {
            auto[riseOk, setOk, riseTime, setTime] = i.dataStub.getPassData();
            return setTime < now;
        }), tracked.end());

        if (tracked.size() < 5) {
            std::vector<set::AppImageId> imagePool = {set::AppImageId::DotRed, set::AppImageId::DotGreen,
                                                      set::AppImageId::DotBlue, set::AppImageId::DotYellow,
                                                      set::AppImageId::DotPurple, set::AppImageId::DotAqua};

            for (const auto &satellite : tracked) {
                imagePool.erase(std::find(imagePool.begin(), imagePool.end(), static_cast<set::AppImageId>(satellite.dataStub.imageId)));
            }

            using SatelliteListType = std::pair<SatelliteDataStub,Satellite>;
            std::vector<SatelliteListType> satelliteList;
            Ephemeris ephemeris{mEphemerisFilePath[static_cast<size_t>(mEphemerisFile)]};
            for (const auto &esv : ephemeris) {
                if (esv.first != "Moon") {
                    Satellite satellite{esv.second};
                    auto[rise_ok, set_ok, rise_az, set_az, max_elevation, rise_time, set_time] =
                    findNextPass(satellite, mObserver);
                    if (set_ok && max_elevation > mMinimumElevation) {
                        SatelliteDataStub dataStub;
                        dataStub.name = satellite.getName();
                        dataStub.setPassData(rise_ok, set_ok, rise_time, set_time);
                        satelliteList.emplace_back(dataStub,satellite);
                    }
                }
            }

            std::sort(satelliteList.begin(), satelliteList.end(),[](SatelliteListType &s0, SatelliteListType &s1){
                return s0.first < s1.first;
            });

            if (tracked.empty()) {
                while (tracked.size() < 5 && !satelliteList.empty() && !imagePool.empty()) {
                    auto satellitePair = satelliteList.front();
                    satellitePair.first.imageId = static_cast<ImageId>(imagePool.front());
                    tracked.push_back(TrackedSatellite{satellitePair.first, satellitePair.second});
                    imagePool.erase(imagePool.begin());
                    satelliteList.erase(satelliteList.begin());
                }
            } else {
                while (tracked.size() < 5 && !satelliteList.empty() && !imagePool.empty()) {
                    if (std::find_if(tracked.begin(), tracked.end(), [&satelliteList](const TrackedSatellite &ts) {
                        return ts.satellite.getName() == satelliteList.begin()->second.getName();
                    }) == tracked.end()) {
                        auto satellitePair = satelliteList.front();
                        satellitePair.first.imageId = static_cast<ImageId>(imagePool.front());
                        tracked.push_back(TrackedSatellite{satellitePair.first, satellitePair.second});
                        imagePool.erase(imagePool.begin());
                    }
                    satelliteList.erase(satelliteList.begin());
                }
            }

            std::lock_guard<std::mutex> lockGuard{mSatListMutex};
            mSatelliteList = tracked;
        }

//        auto timer = time(nullptr);
//        for (const auto &satellite : tracked) {
//            std::cout << static_cast<size_t>(satellite.imageId) << ' ' << satellite.satellite.getName()
//                      << ' ' << satellite.satellite.passTimeString(timer) << '\n';
//        }
//        std::cout << '\n';
    }

    void MapProjection::drawOrbitalPath(sdl::Renderer &renderer, TrackedSatellite &satellite, Position mapPos,
                                        int splitPixel) {
        auto period = satellite.satellite.period();
        auto step = period / 40.;
        auto useStep = step;
        DateTime now{true};

        std::vector<Position> mapPoints{};
        auto index = now;
        while (index < (now + (period + step))) {
            satellite.satellite.predict(index);
            GeoPosition geo{satellite.satellite.geo()};
            mapPoints.emplace_back(geoToMap(geo, mProjection, splitPixel));
            index += step;
        }

        std::array<std::vector<Position>::iterator,2> partition{mapPoints.end(), mapPoints.end()};
        size_t idx = 0;
        switch (mProjection) {
            case ProjectionType::Mercator:
            case ProjectionType::StationMercator:
                for (auto it = mapPoints.begin() + 1; it != mapPoints.end(); ++it) {
                    if (abs((it-1)->x() - it->x()) > mMapSize.width()/2)
                        partition[idx++] = it;
                }
                break;
            case ProjectionType::StationAzmuthal:
                auto antipode = mapPoints.front().x() < mMapSize.width()/2;
                for (auto it = mapPoints.begin(); it != mapPoints.end(); ++it) {
                    auto a = it->x() < mMapSize.width()/2;
                    if (a != antipode) {
                        antipode = a;
                        partition[idx++] = it;
                    }
                }
                break;
        }

        auto mapPoint0 = mapPoints.front();
        std::for_each(mapPoints.begin() + 1, idx ? partition[0] : mapPoints.end(), [&](Position &position) {
            if (mapPoint0 != position)
                mDrawingContext->renderLine(renderer, mapPoint0 + mapPos, position + mapPos);
            mapPoint0 = position;
        });

        if (idx > 0 && partition[0] != mapPoints.end()) {
            mapPoint0 = (*partition[0]);
            std::for_each(partition[0] + 1, partition[1], [&](Position &position) {
                if (mapPoint0 != position)
                    mDrawingContext->renderLine(renderer, mapPoint0 + mapPos, position + mapPos);
                mapPoint0 = position;
            });

            if (idx > 1 && partition[1] != mapPoints.end()) {
                mapPoint0 = (*partition[1]);
                std::for_each(partition[1] + 1, mapPoints.end(), [&](Position &position) {
                    if (mapPoint0 != position)
                        mDrawingContext->renderLine(renderer, mapPoint0 + mapPos, position + mapPos);
                    mapPoint0 = position;
                });
            }
        }
    }

    void MapProjection::drawFootprint(sdl::Renderer &renderer, TrackedSatellite &satellite, Position mapPos,
                                      int splitPixel) {
        using ComputeType = double;
        static constexpr int StepDeg = 5;
        static constexpr size_t LineSegments = 360 / StepDeg;
        static constexpr ComputeType BearingStep = 2. * M_PI / LineSegments;

        // Get the information needed to draw the foot print.
        DateTime now{true};
        satellite.satellite.predict(now);
        GeoPosition geo{satellite.satellite.geo()};
        auto d = satellite.satellite.viewingRadius(0.);
//        geo.lat() = -M_PI_2 - d / 2.;

        // Distance from the nearest pole.
        auto d0 = d;
        auto d1 = d;

        switch (mProjection) {
            case ProjectionType::Mercator:
            case ProjectionType::StationMercator:
                if (geo.lat() > 0)
                    d0 = range(geo, GeoPosition{M_PI_2, geo.lon()});
                else
                    d0 = range(geo, GeoPosition{-M_PI_2, geo.lon()});
                break;
            case ProjectionType::StationAzmuthal:
                d0 = range(geo, mQthRad);
                d1 = range(geo, mAntipode);
                break;
        }

        ComputeType firstBearing = 0.;
        auto lastBearing = firstBearing + 2. * M_PI;
        std::vector<Position> mapPoints{};
        while (firstBearing < lastBearing) {
            auto p = geoToMap(projected(geo, d, firstBearing), mProjection, splitPixel);
            if (p.x() < 0)
                p.x() += mMapSize.width();
            if (p.x() > mMapSize.width())
                p.x() -= mMapSize.width();
            mapPoints.emplace_back(p);
            firstBearing += BearingStep;
        }

        auto partition = mapPoints.end();
        switch (mProjection) {
            case ProjectionType::Mercator:
            case ProjectionType::StationMercator:
                if (d0 < d) {
                    std::sort(mapPoints.begin(), mapPoints.end(), [](Position &p0, Position &p1) {
                        return p0.x() < p1.x();
                    });

                    mapPoints.front().x() = 0;
                    mapPoints.back().x() = mMapSize.width() - 1;
                }
                break;
            case ProjectionType::StationAzmuthal:
                if ((d0 + d > M_PI_2 && d0 < M_PI_2) || (d1 + d > M_PI_2 && d1 < M_PI_2)) {
                    partition = std::stable_partition(mapPoints.begin(), mapPoints.end(),[&](Position &position){
                        return position.x() > mMapSize.width()/2;
                    });
                }
                break;
        }

        auto mapPoint0 = mapPoints.front();
        std::for_each(mapPoints.begin() + 1, partition, [&](Position &position) {
            if (mapPoint0 != position)
                mDrawingContext->renderLine(renderer, mapPoint0 + mapPos, position + mapPos);
            mapPoint0 = position;
        });

        if (partition != mapPoints.end()) {
            mapPoint0 = (*partition);
            std::for_each(partition+1, mapPoints.end(), [&](Position &position){
                mDrawingContext->renderLine(renderer, mapPoint0 + mapPos, position + mapPos);
                mapPoint0 = position;
            });
        }
    }

    void
    MapProjection::drawAntiAliasedLine(sdl::Renderer &renderer, Position start, Position end) {
        auto plot = [&](double x, double y, double b) {
            auto c = color::RGBA{ 1.f, 0.f, 0.f, (float)std::clamp(b, .3, 1.)};
            renderer.drawPoint(Position(roundToInt(x), roundToInt(y)), c);
        };
        auto ipart = [&](double x) { return floor(x); };
        auto fpart = [&](double x) { return x - floor(x); };
        auto rfpart = [&](double x) { return 1. - fpart(x); };
        double x0 = start.x(), y0 = start.y(), x1 = end.x(), y1 = end.y();

        auto steep = abs(y1-y0) > abs(x1-x0);

        if (steep) {
            std::swap(x0, y0);
            std::swap(x1, y1);
        }

        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        auto dx = x1 - x0;
        auto dy = y1 - y0;
        auto gradient = dx == 0.0 ? 1. : dy / dx;

        // Handle first endpoint
        auto xEnd = round(x0);
        auto yEnd = y0 + gradient * (xEnd - x0);
        auto xGap = rfpart(x0 + 0.5);
        auto xPxl1 = xEnd; // this will be used in the main loop
        auto yPxl1 = ipart(yEnd);

        if (steep) {
            plot(yPxl1, xPxl1, rfpart(yEnd) * xGap);
            plot(yPxl1+1, xPxl1, fpart(yEnd) * xGap);
        } else {
            plot(xPxl1, yPxl1, rfpart(yEnd) * xGap);
            plot(xPxl1+1, yPxl1, rfpart(yEnd) * xGap);
        }

        auto intery = yEnd + gradient; // first y-intersection for the main loop

        // Handle second endpoint.

        xEnd = round(x1);
        yEnd = y1 + gradient * (xEnd - x1);
        xGap = fpart(x1 + 0.5);
        auto xPxl2 = xEnd; // this will be used in the main loop
        auto yPxl2 = ipart(yEnd);
        if (steep) {
            plot(yPxl2, xPxl2, rfpart(yEnd) * xGap);
            plot(yPxl2+1, xPxl2, fpart(yEnd) * xGap);
        } else {
            plot(xPxl2, yPxl2, rfpart(yEnd) * xGap);
            plot(xPxl2+1, yPxl2, fpart(yEnd) * xGap);
        }

        // main loop

        if (steep) {
            for (double x = xPxl1 + 1; x < xPxl2; x += 1.) {
                plot(ipart(intery), x, rfpart(intery));
                plot(ipart(intery)+1, x, fpart(intery));
                intery += gradient;
            }
        } else {
            for (double x = xPxl1 + 1; x < xPxl2; x += 1.) {
                plot(x, ipart(intery), rfpart(intery));
                plot(x+1, ipart(intery), fpart(intery));
                intery += gradient;
            }
        }
    }

    std::string SatelliteDataStub::passTimeString(time_t relative) const {
        auto[riseOk, setOk, riseDateTime, setDateTime] = getPassData();

        auto mkTimeStr = [](std::ostream &s, time_t t, time_t relative) {
            static constexpr size_t bufferLength = 64;
            static constexpr char fmtMinSec[] = "%M:%S";
            static constexpr char fmtHourMin[] = "%Hh%M";
            static constexpr char fmtDayHourMin[] = "%jd%Hh%M";
            static constexpr char fmtDate[] = "%F";

            time_t timer = t - relative;
            auto lt = localtime(&timer);
            timer += lt->tm_gmtoff;
            auto tm = gmtime(&timer);
            char buffer[bufferLength];
            char *fmt = const_cast<char *>(fmtMinSec);
            if (timer >= 172800)
                fmt = const_cast<char *>(fmtDate);
            if (timer >= 86400)
                fmt = const_cast<char *>(fmtDayHourMin);
            else if (timer >= 3600)
                fmt = const_cast<char *>(fmtHourMin);
            auto length = strftime(buffer, bufferLength, fmt, tm);
            s << buffer;
        };

        DateTime now{true};
        if (riseOk && riseDateTime > now) {
            std::stringstream strm{};
            auto rise = riseDateTime.mktime();
            mkTimeStr(strm, rise, relative);
            strm << " - ";
            if (setOk) {
                mkTimeStr(strm, setDateTime.mktime(), relative ? rise : 0);
            }
            return strm.str();
        } else if (setOk && setDateTime > now) {
            std::stringstream strm{};
            mkTimeStr(strm, setDateTime.mktime(), relative);
            return strm.str();
        }

        return "Has Set.";
    }
}
