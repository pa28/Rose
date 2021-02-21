/**
 * @file MapProjections.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#include "MapProjection.h"

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
            if (second % 5 == 1) {
                DateTime now{true};
                for (auto & satellite : mSatelliteList) {
                    satellite.predict(now);
                }
                setNeedsDrawing();
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

            if (mUpdateEphemeris)
                updateEphemerisFile();
            mUpdateEphemeris = false;
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
                setNeedsDrawing();
            }
        });

        mProjection = rose()->settings()->getValue(set::AzimuthalMode, 0) ? ProjectionType::StationAzmuthal :
                      ProjectionType::StationMercator;
        mCelestialMode = rose()->settings()->getValue(set::CelestialMode, 0) != 0;
        mSatelliteMode = rose()->settings()->getValue(set::SatelliteMode, 0) != 0;
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
                     widgetRect, mProjection == ProjectionType::StationAzmuthal, splitPixel);

        for (auto satellite = mSatelliteList.begin(); satellite != mSatelliteList.end(); ++satellite) {
            GeoPosition geo{satellite->geo()};
            auto iconIdx = static_cast<std::size_t>(set::AppImageId::DotRed) + (satellite-mSatelliteList.begin());
            MapIcon mapItem{static_cast<ImageId>(iconIdx), geo};
            drawMapItem(mapItem, renderer, widgetRect, mProjection == ProjectionType::StationAzmuthal, splitPixel);
        }

        if (mCelestialMode)
            drawMapItems(mCelestialIcons.begin(), mCelestialIcons.end(), renderer,
                     widgetRect, mProjection == ProjectionType::StationAzmuthal, splitPixel);
    }

    void MapProjection::drawMapItem(const MapIcon &mapItem, sdl::Renderer &renderer, Rectangle mapRectangle, bool azimuthal,
                                    int splitPixel) {
        if (mapItem.imageId == RoseImageInvalid)
            return;

        auto mapPos = geoToMap(mapItem.geo, azimuthal);
        auto iconSize = rose()->imageRepository(mapItem.imageId).getSize();
        mapPos.x() -= iconSize.width() / 2;
        mapPos.y() -= iconSize.height() / 2;

        sdl::ClipRectangleGuard clipRectangleGuard(renderer, mapRectangle);

        if (!azimuthal && splitPixel > 0) {
            mapPos.x() = (mapPos.x() + mMapSize.width() - splitPixel) % mMapSize.width();
        }

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
        } else if (h > 0 && w == 0 && !azimuthal) {
            dst.y() = mapRectangle.y() - h;
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            dst.y() += mapRectangle.height();
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
        } else if (h > 0 && w > 0 && !azimuthal) {
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
        auto lat = asin( sin(location.lat()) * cos(angDist) + cos(location.lat()) * sin(angDist) * cos(bearing));
        auto lon = location.lon() + atan2(sin(bearing) * sin(angDist) * cos(location.lat()), cos(angDist) - sin(location.lat()) * sin(lat));
        return GeoPosition{lat,lon};
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

    Position MapProjection::geoToMap(GeoPosition geo, bool azimuthal) {
        if (azimuthal) {
            double ca, B;
            solveSphere(geo.lon() - mQthRad.lon(), M_PI_2 - geo.lat(), sin(mQthRad.lat()),
                        cos(mQthRad.lat()), ca, B);
            if (ca > 0) {
                auto a = acos(ca);
                auto R0 = (double)mMapSize.width() / 4. - 1.;
                auto R = a * (double)mMapSize.width() / (2. * M_PI);
                R = std::min(R, R0);
                auto dx = R * sin(B);
                auto dy = R * cos(B);
                return Position{mMapSize.width() / 4 + roundToInt(dx), mMapSize.height() / 2 - roundToInt(dy)};
            } else {
                auto a = M_PI - acos(ca);
                auto R0 = (double) mMapSize.width() / 4 - 1;
                auto R = a * (double) mMapSize.width() / (2.f * (float)M_PI);
                R = std::min(R, R0);
                auto dx = -R * sin(B);
                auto dy = R * cos(B);
                return Position{3 * mMapSize.width() / 4 + roundToInt(dx), mMapSize.height() / 2 - roundToInt(dy)};
            }
        } else {
            return Position{roundToInt(mMapSize.width() * (geo.lon() + M_PI) / (2. * M_PI)) % mMapSize.width(),
                            roundToInt(mMapSize.height() * (M_PI_2 - geo.lat()) / M_PI)};
        }
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
        t_srch += dt;
        bool set_ok = false, rise_ok = false, ever_up = false, ever_down = false;

        // search up to a few days ahead for next rise and set times (for example for moon)
        while ((!set_ok || !rise_ok) && t_srch < t_now + 2.0F && t_srch > t_now) {
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
        Ephemeris ephemeris{mEphemerisFilePath[static_cast<size_t>(mEphemerisFile)]};
        DateTime now{true};
        std::vector<Satellite> satelliteList;
        for (const auto &esv : ephemeris) {
            if (esv.first != "Moon") {
                Satellite satellite{esv.second};
                auto [rise_ok, set_ok, rise_az, set_az, max_elevation, rise_time, set_time] =
                findNextPass(satellite, mObserver);
                if (rise_ok && set_ok && max_elevation > 30.) {
                    satellite.setPassData(rise_time, set_time);
                    satelliteList.emplace_back(satellite);
                }
            }
        }

        std::sort(satelliteList.begin(), satelliteList.end());
        if (satelliteList.size() > 5)
            satelliteList.erase(satelliteList.begin()+5,satelliteList.end());

        mSatelliteList = satelliteList;
    }
}
