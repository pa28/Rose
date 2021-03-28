/**
 * @file MapProjection.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-27
 */

#include "Application.h"
#include "MapProjection.h"
#include "GraphicsModel.h"
#include "Math.h"

namespace rose {

    MapProjection::MapProjection() {
        Environment &environment{Environment::getEnvironment()};
        mMapCache = std::make_unique<WebCache>("https://www.clearskyinstitute.com/ham/HamClock/maps/",
                                               Environment::getEnvironment().cacheHome(),
                                               "Maps", std::chrono::hours{24 * 30});
    }

    void MapProjection::cacheCurrentMaps() {
        std::array<std::tuple<MapDepiction,MapSize,MapIllumination>,2> maps{
                std::make_tuple(mMapDepiction, mMapSize, MapIllumination::Day),
                std::make_tuple(mMapDepiction, mMapSize, MapIllumination::Night),
        };

        for (auto &map : maps) {
            auto [depiction,size,illumination] = map;
            auto id = MapImageId(depiction,size,illumination);
            auto name = MapFileName(depiction,size,illumination);
            mMapCache->setCacheItem(id, name);
            mMapCache->fetchItem(id);
        }
    }

    void MapProjection::addedToContainer() {
        Node::addedToContainer();
        mMapSlot = WebCacheProtocol::createSlot();
        mMapSlot->receiver = [&](uint32_t key, long status) {
            std::cout << __PRETTY_FUNCTION__ << ' ' << key << ' ' << status << '\n';
            getApplication().redrawBackground();
        };
        mMapCache->cacheLoaded.connect(mMapSlot);

        cacheCurrentMaps();
    }

    void MapProjection::draw(gm::Context &context, const Position &containerPosition) {

        std::array<uint32_t,2> mapId{MapImageId(mMapDepiction, mMapSize, MapIllumination::Day),
                                     MapImageId(mMapDepiction, mMapSize, MapIllumination::Night)};

        std::array<std::optional<std::filesystem::path>,2> mapPath{mMapCache->localItemExists(mapId[0]),
                                                    mMapCache->localItemExists(mapId[1])};

        if (mapPath[0] && mapPath[1]) {
            for (size_t i = 0; i < mapPath.size(); i++) {
                gm::Surface bmp{mapPath[i].value()};
                if (i) {
                    Size bmpSize{bmp->w, bmp->h};
                    if (bmpSize != mMapImgSize)
                        throw std::runtime_error("Image size mismatch.");
                } else {
                    mMapImgSize = Size{bmp->w, bmp->h};
                }
                mMapSurface[i] = gm::Surface{bmp->w, bmp->h};
                mMapSurface[i].blitSurface(bmp);
                mAzSurface[i] = gm::Surface{bmp->w, bmp->h};
            }

            computeAzimuthalMaps();
            mNewSurfaces = setForegroundBackground();
        }

        if (mNewSurfaces) {
            mNewSurfaces = false;
            for (size_t i = 0; i < mMercatorTemp.size(); ++i) {
                mMercator[i] = mMercatorTemp[i].toTexture(context);
                mMercator[i].setBlendMode(SDL_BLENDMODE_BLEND);
                mMercatorTemp[i].reset(nullptr);
            }

            for (size_t i = 0; i < mAzimuthalTemp.size(); ++i) {
                mAzimuthal[i] = mAzimuthalTemp[i].toTexture(context);
                mAzimuthal[i].setBlendMode(SDL_BLENDMODE_BLEND);
                mAzimuthalTemp[i].reset(nullptr);
            }
        }

        if (!mMercator[0] || !mAzimuthal[0]) {
            return;
        }

        Rectangle widgetRect{containerPosition + mPos, mSize};
        int splitPixel = 0;
        switch (mProjection) {
            case MapProjectionType::Mercator:
                context.renderCopy(mMercator[1], widgetRect);
                context.renderCopy(mMercator[0], widgetRect);
                break;
            case MapProjectionType::StationMercator: {
                auto lon = mQth.lon;
                splitPixel = util::roundToInt((double) mMapImgSize.w * ((lon) / 360.));
                if (splitPixel < 0)
                    splitPixel += mMapImgSize.w;

                Rectangle src{splitPixel, 0, mMapImgSize.w - splitPixel, mMapImgSize.h};
                Rectangle dst{widgetRect};
                dst.w = src.w;
                dst.h = src.h;
                context.renderCopy(mMercator[1], src, dst);
                context.renderCopy(mMercator[0], src, dst);

                src.x = 0;
                dst.x += src.w;
                src.w = splitPixel;
                dst.w = splitPixel;
                context.renderCopy(mMercator[1], src, dst);
                context.renderCopy(mMercator[0], src, dst);
            }
                break;
            case MapProjectionType::StationAzimuthal:
                context.renderCopy(mAzimuthal[1], widgetRect);
                context.renderCopy(mAzimuthal[0], widgetRect);
                break;
        }
    }

    Rectangle MapProjection::layout(gm::Context &context, const Rectangle &screenRect) {
        std::cout << __PRETTY_FUNCTION__ << screenRect << '\n';
        return screenRect;
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
        bool onAntipode = x > mapSize.w / 2;
        auto w2 = (mapSize.h / 2) * (mapSize.h / 2);
        auto dx = onAntipode ? x - (3 * mapSize.w) / 4 : x - mapSize.w / 4;
        auto dy = mapSize.h / 2 - y;
        auto r2 = dx * dx + dy * dy;    // radius squared

        if (r2 <= w2) {
            auto b = sqrt((double) r2 / (double) w2) * M_PI_2;    // Great circle distance.
            auto A = M_PI_2 - atan2((double) dy, (double) dx);       // Azimuth
            double ca, B;
            solveSphere(A, b, (onAntipode ? -siny : siny), cosy, ca, B);
            auto lat = (float) M_PI_2 - acos(ca);
            auto lon = location.lon + B + (onAntipode ? 6. : 5.) * (double) M_PI;
            lon = fmod(location.lon + B + (onAntipode ? 6. : 5.) * (double) M_PI, 2 * M_PI) - (double) M_PI;
            return std::make_tuple(true, lat, lon);
        }
        return std::make_tuple(false, 0., 0.);
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

#if 1

    bool MapProjection::computeAzimuthalMaps() {
        // Compute Azimuthal maps from the Mercator maps
        auto siny = sin(mQthRad.lat);
        auto cosy = cos(mQthRad.lat);
        for (int y = 0; y < mMapImgSize.h; y += 1) {
            for (int x = 0; x < mMapImgSize.w; x += 1) {
                if (mAbortFuture) {
                    mAbortFuture = false;
                    return false;
                }

                auto[valid, lat, lon] = xyToAzLatLong(x, y, mMapImgSize, mQthRad, siny, cosy);
                if (valid) {
                    GeoPosition position{lat, lon, true};
                    auto xx = std::min(mMapImgSize.w - 1,
                                       (int) round((double) mMapImgSize.w * ((lon + M_PI) / (2 * M_PI))));
                    auto yy = std::min(mMapImgSize.h - 1,
                                       (int) round((double) mMapImgSize.h * ((M_PI_2 - lat) / M_PI)));
                    mAzSurface[0].pixel(x, y) = gm::mapRGBA(mAzSurface[0]->format,
                                                            gm::getRGBA(mMapSurface[0]->format,
                                                                        mMapSurface[0].pixel(xx, yy)));
                    mAzSurface[1].pixel(x, y) = gm::mapRGBA(mAzSurface[1]->format,
                                                            gm::getRGBA(mMapSurface[1]->format,
                                                                        mMapSurface[1].pixel(xx, yy)));
                }
            }
        }
        return true;
    }

    bool MapProjection::setForegroundBackground() {
        for (size_t i = 0; i < mMercatorTemp.size(); ++i) {
            mMercatorTemp[i] = gm::Surface{mMapImgSize};
            mAzimuthalTemp[i] = gm::Surface{mMapImgSize};

            mMercatorTemp[i].setBlendMode(SDL_BLENDMODE_BLEND);
            mAzimuthalTemp[i].setBlendMode(SDL_BLENDMODE_BLEND);
            mMercatorTemp[i].blitSurface(mMapSurface[i]);
            mAzimuthalTemp[i].blitSurface(mAzSurface[i]);
        }

        auto[latS, lonS] = subSolar();

        auto siny = sin(mQthRad.lat);
        auto cosy = cos(mQthRad.lat);

        // Three loops for: Longitude, Latitude, and map form (Mercator, Azimuthal).
        // This lets us use the common calculations without repeating them, easier than
        // debugging two areas with the same computation.
        for (int x = 0; x < mMapImgSize.w; x += 1) {
            for (int y = 0; y < mMapImgSize.h; y += 1) {
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
                        auto tuple = xyToAzLatLong(x, y, mMapImgSize,
                                                   mQthRad, siny, cosy);
                        valid = std::get<0>(tuple);
                        latE = std::get<1>(tuple);
                        lonE = std::get<2>(tuple);
                    } else {
                        // The Mercator coordinates for the same map pixel
                        valid = true;
                        lonE = (float) ((float) x - (float) mMapImgSize.w / 2.f) * (float) M_PI /
                               (float) ((float) mMapImgSize.w / 2.);
                        latE = (float) ((float) mMapImgSize.h / 2.f - (float) y) * (float) M_PI_2 /
                               (float) ((float) mMapImgSize.h / 2.);
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
                                alpha = std::clamp((float) fract_day, 0.0313f, 1.f);
                            } else
                                alpha = 0.0313;  // Set the minimun alpha to keep some daytime colour on the night side
                        }
                    } else
                        alpha = 0;

                    // Set the alpha channel in the appropriate map.
                    if (az == 1) {
                        auto pixel = gm::getRGBA(mAzimuthalTemp[0]->format, mAzimuthalTemp[0].pixel(x, y));
                        pixel.a() = alpha;
                        mAzimuthalTemp[0].pixel(x, y) = gm::mapRGBA(mAzimuthalTemp[0]->format, pixel);
                    } else {
                        auto pixel = gm::getRGBA(mMercatorTemp[0]->format, mMercatorTemp[0].pixel(x, y));
                        pixel.a() = alpha;
                        mMercatorTemp[0].pixel(x, y) = gm::mapRGBA(mMercatorTemp[0]->format, pixel);
                    }
                }
            }
        }
        return true;
    }

#endif

}
