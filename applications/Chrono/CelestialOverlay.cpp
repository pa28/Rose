/**
 * @file CelestialOverlay.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-06-10
 */

#include "CelestialOverlay.h"

#include <utility>
#include "Application.h"

namespace rose {

    CelestialOverlay::CelestialOverlay(std::shared_ptr<TimerTick> timerTick, path &xdgDataPath) : Widget() {
        mXdgDataPath = xdgDataPath;
        mTimerTick = std::move(timerTick);
    }

    void CelestialOverlay::draw(gm::Context &context, const Position &containerPosition) {
        Rectangle widgetRect{containerPosition + mPos, mSize};

        if (mDisplayCelestialObjects) {
            if (auto mapProjection = containerAs<MapProjection>(); mapProjection) {
                int splitPixel = util::roundToInt((double) widgetRect.w * ((mapProjection->getQth().lon) / 360.));
                if (splitPixel < 0)
                    splitPixel += widgetRect.w;
                for (auto &celestial : CelestialOverlayFileName) {
                    switch (celestial.mapOverLayImage) {
                        case MapOverLayImage::Sun:
                            mapProjection->drawMapItem(
                                    mMapOverlayId[static_cast<std::size_t>(celestial.mapOverLayImage)],
                                    context, widgetRect, mSubSolar, mapProjection->getProjection(), splitPixel);
                            break;
                        case MapOverLayImage::Moon:
                            mapProjection->drawMapItem(
                                    mMapOverlayId[static_cast<std::size_t>(celestial.mapOverLayImage)],
                                    context, widgetRect, mSubLunar, mapProjection->getProjection(), splitPixel);
                            break;
                        default:
                            break;
                    }
                }
            } else {
                throwContainerError();
            }
        }
    }

    Rectangle CelestialOverlay::layout(gm::Context &context, const Rectangle &screenRect) {
        return screenRect;
    }

    void CelestialOverlay::addedToContainer() {
        if (auto mapProjection = containerAs<MapProjection>(); mapProjection) {
            loadMapCelestialObjectImages(mXdgDataPath, getApplication().context());

            mSatelliteObservation = SatelliteObservation{
                    Observer{mapProjection->getQth().lat, mapProjection->getQth().lon, 0.}};
            mSatelliteObservation.passPrediction(6, "ISS");

            mCelestialObservations = SatelliteObservation{mSatelliteObservation.observer(), "Moon"};
            mCelestialObservations.predict(DateTime{true});
            if (mCelestialObservations.empty()) {
                mDisplayCelestialObjects = false;
            } else {
                auto[lat, lon] = mCelestialObservations.front().geo();
                mSubLunar = GeoPosition{lat, lon, true};
            }

            mCelestialUpdateTimer = TickProtocol::createSlot();
            mCelestialUpdateTimer->receiver = [&](int minutes) {
                if ((minutes % 2) == 0) {
                    if (auto mapProjection = containerAs<MapProjection>(); mapProjection)
                        if (mapProjection->mapProjectionsValid())
                            setCelestialObservations();
                }
            };
        } else {
            throwContainerError();
        }

        mTimerTick->minuteSignal.connect(mCelestialUpdateTimer);
        setCelestialObservations();
    }

    void CelestialOverlay::loadMapCelestialObjectImages(const std::filesystem::path &xdgResourcePath, gm::Context &context) {
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
}
