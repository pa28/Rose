/**
 * @file MapProjection.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-27
 */

#include "Application.h"
#include "MapProjection.h"

#include <utility>
#include "GraphicsModel.h"
#include "Texture.h"
#include "Settings.h"
#include "Math.h"

namespace rose {

    MapProjection::MapProjection(std::shared_ptr<TimerTick> timerTick, std::filesystem::path& xdgDataPath) {
        mXdgDataPath = xdgDataPath;
        mTimerTick = std::move(timerTick);

        Environment &environment{Environment::getEnvironment()};
        mMapCache = std::make_unique<WebCache>("https://www.clearskyinstitute.com/ham/HamClock/maps/",
                                               Environment::getEnvironment().cacheHome(),
                                               "Maps", std::chrono::hours{24 * 30});

        mKeyboardShortcutCallback = [&](uint32_t shortcutCode, bool pressed, uint repeat) {
            MapProjectionType projectionType = mProjection;
            MapDepiction mapDepiction = mMapDepiction;
            switch (static_cast<ShortCutCode>(shortcutCode)) {
                case MercatorProjection:
                    projectionType = MapProjectionType::StationMercator;
                    break;
                case AzimuthalProjection:
                    projectionType = MapProjectionType::StationAzimuthal;
                    break;
                case TerrainMap:
                    mapDepiction = MapDepiction::Terrain;
                    break;
                case CountryMap:
                    mapDepiction = MapDepiction::Countries;
                    break;
            }

            if (projectionType != mProjection) {
                mProjection = projectionType;
                Settings::getSettings().setValue(set::ChronoMapProjection, static_cast<int>(mProjection));
                getApplication().redrawBackground();
            }

            if (mapDepiction != mMapDepiction) {
                mMapDepiction = mapDepiction;
                Settings::getSettings().setValue(set::ChronoMapDepiction, static_cast<int>(mMapDepiction));
                cacheCurrentMaps();
            }
        };
    }

    void MapProjection::cacheCurrentMaps() {
        mMapProjectionsInvalid = true;

        std::array<std::tuple<MapDepiction,MapSize,MapIllumination>,2> maps{
                std::make_tuple(mMapDepiction, mMapSize, MapIllumination::Day),
                std::make_tuple(mMapDepiction, mMapSize, MapIllumination::Night),
        };

        for (auto &map : maps) {
            auto [depiction,size,illumination] = map;
            auto mapDepictionId = MapImageId(depiction,size,illumination);
            auto name = MapFileName(depiction,size,illumination);
            mMapCache->setCacheItem(mapDepictionId, name);
            mMapCache->fetchItem(mapDepictionId);
        }
    }

    void MapProjection::addedToContainer() {
        std::cout << __PRETTY_FUNCTION__ << ' ' << getIdPath() << '\n';
        Node::addedToContainer();

        Settings &settings{Settings::getSettings()};
        mProjection = static_cast<MapProjectionType>(
                settings.getValue(set::ChronoMapProjection, static_cast<int>(MapProjectionType::StationMercator)));

        mMapDepiction = static_cast<MapDepiction>(
                settings.getValue(set::ChronoMapDepiction, static_cast<int>(MapDepiction::Terrain)));

        mMapSlot = WebCacheProtocol::createSlot();
        mMapSlot->receiver = [&](uint32_t key, long status) {
            std::cout << __PRETTY_FUNCTION__ << ' ' << key << ' ' << status << '\n';
            getApplication().redrawBackground();
        };
        mMapCache->cacheLoaded.connect(mMapSlot);

        mCelestialTimer = TickProtocol::createSlot();
        mCelestialTimer->receiver = [&](int minutes){
            if ((minutes % 3) == 0 && !mMapProjectionsInvalid && !mForegroundBackgroundFuture.valid()) {
                mForegroundBackgroundFuture = std::async(std::launch::async, &MapProjection::setForegroundBackground, this);
            }
        };

        mTimerTick->minuteSignal.connect(mCelestialTimer);

        cacheCurrentMaps();

        loadMapObjectImages(mXdgDataPath, getApplication().context());
    }

    void MapProjection::draw(gm::Context &context, const Position &containerPosition) {
        std::array<uint32_t,2> mapId{MapImageId(mMapDepiction, mMapSize, MapIllumination::Day),
                                     MapImageId(mMapDepiction, mMapSize, MapIllumination::Night)};

        std::array<std::optional<std::filesystem::path>,2> mapPath{mMapCache->localItemExists(mapId[0]),
                                                    mMapCache->localItemExists(mapId[1])};

        if (mMapProjectionsInvalid) {
            if (!mComputeAzimuthalMapsFuture.valid()) {
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

                    mComputeAzimuthalMapsFuture = std::async(std::launch::async,
                                                             &MapProjection::computeAzimuthalMaps, this);
                }
            } else {
                std::chrono::milliseconds span{100};
                if (auto futureStatus = mComputeAzimuthalMapsFuture.wait_for(span); futureStatus ==
                                                                                    std::future_status::ready) {

                    if (mComputeAzimuthalMapsFuture.get()) {
                        mForegroundBackgroundFuture = std::async(std::launch::async,
                                                                 &MapProjection::setForegroundBackground, this);
                        mMapProjectionsInvalid = false;
                    } else
                        std::cout << __PRETTY_FUNCTION__ << " computeAzimuthalMaps returned false.\n";
                }
            }
        }

        if (mForegroundBackgroundFuture.valid()) {
            std::chrono::milliseconds span{100};
            if (auto futureStatus = mForegroundBackgroundFuture.wait_for(span); futureStatus ==
                                                                                std::future_status::ready) {

                if (mNewSurfaces = mForegroundBackgroundFuture.get(); !mNewSurfaces)
                    std::cout << __PRETTY_FUNCTION__ << " setForegroundBackground returned false.\n";
            }
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
                auto actualMapImgSize = mMercator[0].getSize();
                splitPixel = util::roundToInt((double) actualMapImgSize.w * ((lon) / 360.));
                if (splitPixel < 0)
                    splitPixel += actualMapImgSize.w;

                Rectangle src0{splitPixel, 0, actualMapImgSize.w - splitPixel, actualMapImgSize.h};
                Rectangle dst0{widgetRect};
                dst0.w = src0.w;
                dst0.h = src0.h;

                auto src1 = src0;
                auto dst1 = dst0;
                src1.x = 0;
                dst1.x += src1.w;
                src1.w = splitPixel;
                dst1.w = splitPixel;

                gm::Texture tempText{context, actualMapImgSize};
                if (tempText){
                    gm::RenderTargetGuard renderTargetGuard{context, tempText};

                    context.renderCopy(mMercator[1], src0, dst0);
                    context.renderCopy(mMercator[0], src0, dst0);

                    context.renderCopy(mMercator[1], src1, dst1);
                    context.renderCopy(mMercator[0], src1, dst1);
                } else {
                    std::cout << __PRETTY_FUNCTION__ << " Texture creation failed.\n";
                }
                context.renderCopy(tempText, widgetRect);
            }
                break;
            case MapProjectionType::StationAzimuthal:
                context.renderCopy(mAzimuthal[1], widgetRect);
                context.renderCopy(mAzimuthal[0], widgetRect);
                break;
        }

        if (mDisplayCelestialObjects) {
            for(auto& celestial : CelestialOverlayFileName) {
                switch (celestial.mapOverLayImage) {
                    case MapOverLayImage::Sun:
                        drawMapItem(mMapOverlayId[static_cast<std::size_t>(celestial.mapOverLayImage)],
                                    context, widgetRect, mSubSolar, mProjection, splitPixel);
                        break;
                    default:
                        drawMapItem(mMapOverlayId[static_cast<std::size_t>(celestial.mapOverLayImage)],
                                    context, widgetRect, mSubSolar, mProjection, splitPixel);
                        break;
                }
            }
        }
    }

    Rectangle MapProjection::layout(gm::Context &context, const Rectangle &screenRect) {
        if (MapImageSize(mMapSize) != screenRect.size()) {
            auto currentMapSize = mMapSize;
            for (int i = static_cast<int>(MapSize::Small); i < static_cast<int>(MapSize::Last); ++i) {
                if (MapImageSize(static_cast<MapSize>(i)) <= screenRect.size()) {
                    mMapSize = static_cast<MapSize>(i);
                } else {
                    break;
                }
            }
            if (currentMapSize != mMapSize)
                cacheCurrentMaps();
        }
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
     * @param sinY pre-computed sine of the latitude
     * @param cosY pre-computed cosine of the latitude
     * @return [valid, latitude, longitude ], valid if the pixel is on the Earth,
     * latitude -PI..+PI West to East, longitude +PI/2..-PI/2 North to South
     */
    std::tuple<bool, double, double>
    xyToAzLatLong(int x, int y, const Size &mapSize, const GeoPosition &location, double sinY, double cosY) {
        bool onAntipode = x > mapSize.w / 2;
        auto w2 = (mapSize.h / 2) * (mapSize.h / 2);
        auto dx = onAntipode ? x - (3 * mapSize.w) / 4 : x - mapSize.w / 4;
        auto dy = mapSize.h / 2 - y;
        auto r2 = dx * dx + dy * dy;    // radius squared

        if (r2 <= w2) {
            auto b = sqrt((double) r2 / (double) w2) * M_PI_2;    // Great circle distance.
            auto A = M_PI_2 - atan2((double) dy, (double) dx);       // Azimuth
            double ca, B;
            solveSphere(A, b, (onAntipode ? -sinY : sinY), cosY, ca, B);
            auto lat = (float) M_PI_2 - acos(ca);
            auto lon = location.lon + B + (onAntipode ? 6. : 5.) * (double) M_PI;
            lon = fmod(location.lon + B + (onAntipode ? 6. : 5.) * (double) M_PI, 2 * M_PI) - (double) M_PI;
            return std::make_tuple(true, lat, lon);
        }
        return std::make_tuple(false, 0., 0.);
    }

    /**
     * Compute the sub-solar geographic coordinates, used in plotting the solar illumination.
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
        double GMST = fmod(15 * (18.697374558 + 24.06570982441908 * D), 360.0);
        auto lng_d = fmod(RA - GMST + 36000.0 + 180.0, 360.0) - 180.0;
        auto lng = deg2rad(lng_d);

        return std::make_tuple(lat, lng);
    }

    bool MapProjection::computeAzimuthalMaps() {
        // Compute Azimuthal maps from the Mercator maps
        auto sinY = sin(mQthRad.lat);
        auto cosY = cos(mQthRad.lat);
        for (int y = 0; y < mMapImgSize.h; y += 1) {
            for (int x = 0; x < mMapImgSize.w; x += 1) {
                if (mAbortFuture) {
                    mAbortFuture = false;
                    return false;
                }

                auto[valid, lat, lon] = xyToAzLatLong(x, y, mMapImgSize, mQthRad, sinY, cosY);
                if (valid) {
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
        auto start = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        for (size_t i = 0; i < mMercatorTemp.size(); ++i) {
            mMercatorTemp[i] = gm::Surface{mMapImgSize};
            mAzimuthalTemp[i] = gm::Surface{mMapImgSize};

            mMercatorTemp[i].setBlendMode(SDL_BLENDMODE_BLEND);
            mAzimuthalTemp[i].setBlendMode(SDL_BLENDMODE_BLEND);
            mMercatorTemp[i].blitSurface(mMapSurface[i]);
            mAzimuthalTemp[i].blitSurface(mAzSurface[i]);
        }

        auto[latS, lonS] = subSolar();
        std::cout << __PRETTY_FUNCTION__ << "Sub-Solar: " << rad2deg(latS) << ", " << rad2deg(lonS) << '\n';
        mSubSolar = GeoPosition{latS, lonS, true};

        auto sinY = sin(mQthRad.lat);
        auto cosY = cos(mQthRad.lat);

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
                                                   mQthRad, sinY, cosY);
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
                        // Compute the amount of solar illumination and use it to compute the pixel alpha value
                        // GrayLineCos sets the interior angle between the sub-solar point and the location.
                        // GrayLinePower sets how fast it gets dark.
                        auto cosDeltaSigma = sin(latS) * sin(latE) + cos(latS) * cos(latE) * cos(abs(lonS - lonE));
                        double dayFraction;
                        if (cosDeltaSigma < 0) {
                            if (cosDeltaSigma > GrayLineCos[1]) {
                                dayFraction = 1.0 - pow(cosDeltaSigma / GrayLineCos[1], GrayLinePow);
                                alpha = std::clamp((float) dayFraction, 0.0313f, 1.f);
                            } else
                                alpha = 0.0313;  // Set the minimum alpha to keep some daytime colour on the night side
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
        auto stop = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        std::cout << __PRETTY_FUNCTION__ << " Duration: " << stop - start << '\n';
        getApplication().redrawBackground();
        return true;
    }

    void MapProjection::loadMapObjectImages(const std::filesystem::path &xdgResourcePath, gm::Context &context) {
        ImageStore &imageStore{ImageStore::getStore()};
        for (auto& overlay : CelestialOverlayFileName) {
            auto path = xdgResourcePath;
            path.append("images");
            path.append(overlay.fileName);
            auto imageId = imageStore.nextImageId();
            mMapOverlayId[static_cast<std::size_t>(overlay.mapOverLayImage)] = imageId;
            gm::Surface objectSurface(path);
            imageStore.setImage(imageId, std::move(objectSurface.toTexture(context)));
        }
    }

    void MapProjection::drawMapItem(const ImageId &mapItem, gm::Context& context, Rectangle mapRectangle,
                                    GeoPosition& geoPosition, MapProjectionType projection, int splitPixel) {
        if (mapItem == ImageId::NoImage)
            return;

        auto mapPos = geoToMap(geoPosition, projection, splitPixel);

        ImageStore& imageStore{ImageStore::getStore()};
        auto iconSize = imageStore.size(mapItem);
        mapPos.x -= iconSize.w / 2;
        mapPos.y -= iconSize.h / 2;

        gm::ClipRectangleGuard clipRectangleGuard(context, mapRectangle);

        mapPos = mapPos + mapRectangle.position();
        Rectangle dst{mapPos, iconSize};
        int h = 0;
        if (dst.y < mapRectangle.y) {
            // Top hang
            h = mapRectangle.y - dst.y;
        } else if (dst.y + dst.h > mapRectangle.y + mapRectangle.h) {
            // Bottom hang
            h = mapRectangle.y + mapRectangle.h - dst.y;
        }

        int w = 0;
        if (dst.x < mapRectangle.x) {
            w = mapRectangle.x - dst.x;
        } else if (dst.x + dst.w > mapRectangle.x + mapRectangle.w) {
            w = mapRectangle.x + mapRectangle.w - dst.x;
        }

        if (h == 0 && w == 0){
            imageStore.renderCopy(context, mapItem, dst);
            return;
        }

        if (h == 0 && w > 0) {
            // Left/Right hang - draw left side on right of map
            dst.x = mapRectangle.x - w;
            imageStore.renderCopy(context, mapItem, dst);
            dst.x += mapRectangle.w;
            imageStore.renderCopy(context, mapItem, dst);
        } else if (h > 0 && w == 0) {
            if (projection != MapProjectionType::StationAzimuthal || h < iconSize.h / 2) {
                imageStore.renderCopy(context, mapItem, dst);
            }
            if (projection != MapProjectionType::StationAzimuthal || h >= iconSize.h / 2) {
                imageStore.renderCopy(context, mapItem, dst);
            }
        } else if (h > 0 && w > 0 && projection != MapProjectionType::StationAzimuthal) {
            dst.x = mapRectangle.x - w;
            dst.y = mapRectangle.y - h;
            imageStore.renderCopy(context, mapItem, dst);
            dst.x += mapRectangle.w;
            imageStore.renderCopy(context, mapItem, dst);
            dst.y += mapRectangle.h;
            imageStore.renderCopy(context, mapItem, dst);
            dst.x -= mapRectangle.w;
            imageStore.renderCopy(context, mapItem, dst);
        }
    }

    Position MapProjection::geoToMap(GeoPosition geo, MapProjectionType projection, int splitPixel) const {
        Position mapPos{};

        switch (projection) {
            case MapProjectionType::StationAzimuthal: {
                double ca, B;
                solveSphere(geo.lon - mQthRad.lon, M_PI_2 - geo.lat, sin(mQthRad.lat),
                            cos(mQthRad.lat), ca, B);
                if (ca > 0) {
                    auto a = acos(ca);
                    auto R0 = (double) mMapImgSize.w / 4. - 1.;
                    auto R = a * (double) mMapImgSize.w / (2. * M_PI);
                    R = std::min(R, R0);
                    auto dx = R * sin(B);
                    auto dy = R * cos(B);
                    mapPos = Position{mMapImgSize.w / 4 + util::roundToInt(dx), mMapImgSize.h / 2 - util::roundToInt(dy)};
                } else {
                    auto a = M_PI - acos(ca);
                    auto R0 = (double) mMapImgSize.w / 4 - 1;
                    auto R = a * (double) mMapImgSize.w / (2.f * (float) M_PI);
                    R = std::min(R, R0);
                    auto dx = -R * sin(B);
                    auto dy = R * cos(B);
                    mapPos = Position{3 * mMapImgSize.w / 4 + util::roundToInt(dx), mMapImgSize.h / 2 - util::roundToInt(dy)};
                }
            }
                break;
            case MapProjectionType::Mercator:
                mapPos = Position{util::roundToInt(mMapImgSize.w * (geo.lon + M_PI) / (2. * M_PI)) % mMapImgSize.w,
                                  util::roundToInt(mMapImgSize.h * (M_PI_2 - geo.lat) / M_PI)};
                break;
            case MapProjectionType::StationMercator: {
                mapPos = Position{util::roundToInt(mMapImgSize.w * (geo.lon + M_PI) / (2. * M_PI)) % mMapImgSize.w,
                                  util::roundToInt(mMapImgSize.h * (M_PI_2 - geo.lat) / M_PI)};
                mapPos.x = (mapPos.x + mMapImgSize.w - splitPixel) % mMapImgSize.w;
                break;
            }
        }
        return mapPos;
    }
}
