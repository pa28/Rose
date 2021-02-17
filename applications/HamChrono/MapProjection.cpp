/**
 * @file MapProjections.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#include "MapProjection.h"

#include <filesystem>
#include <utility>
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
            if (!mFutureSun.valid())
                mFutureSun = std::async(std::launch::async, &MapProjection::setForegroundBackground, this);
        });

        mMapCache->itemFetched.connect(mapFileRx);

        settingsUpdateRx = std::make_shared<Slot<std::string>>();
        settingsUpdateRx->setCallback([&](uint32_t, const std::string& name){
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
            }
        });

        rose()->settings()->dataChangeTx.connect(settingsUpdateRx);

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

        drawMapItems(mCelestialIcons.begin(), mCelestialIcons.end(), renderer,
                     widgetRect, mProjection == ProjectionType::StationAzmuthal, splitPixel);
    }

    void MapProjection::drawMapItem(const MapIcon &mapItem, sdl::Renderer &renderer, Rectangle mapRectangle, bool azimuthal,
                                    int splitPixel) {
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
        } else if (h > 0 && w == 0) {
            dst.y() = mapRectangle.y() - h;
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
            dst.y() += mapRectangle.height();
            rose()->imageRepository().renderCopy(renderer, mapItem.imageId, dst);
        } else if (h > 0 && w > 0) {
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
                            if (cosDeltaSigma > GrayLineCos) {
                                fract_day = 1.0 - pow(cosDeltaSigma / GrayLineCos, GrayLinePow);
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

    void MapProjection::setCelestialIcons() {
        auto [lat,lon] = subSolar();
        mCelestialIcons[0].imageId = static_cast<ImageId>(set::AppImageId::Sun);
        mCelestialIcons[0].geo = GeoPosition{lat,lon};
        mCelestialIcons[1].imageId = static_cast<ImageId>(set::AppImageId::Moon);
        // ToDo: implement ephemeris and moon phase.
        mCelestialIcons[1].geo = antipode(mCelestialIcons[0].geo);
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
}
