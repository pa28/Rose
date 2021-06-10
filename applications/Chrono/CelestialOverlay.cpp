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

    CelestialOverlay::CelestialOverlay(std::shared_ptr<TimerTick> timerTick, path &xdgDataPath) : MapProjection(
            std::move(timerTick), xdgDataPath) {

    }

    void CelestialOverlay::draw(gm::Context &context, const Position &containerPosition) {
        MapProjection::draw(context, containerPosition);

        Rectangle widgetRect{containerPosition + mPos, mSize};

        if (mDisplayCelestialObjects) {
            int splitPixel = util::roundToInt((double) widgetRect.w * ((mQth.lon) / 360.));
            if (splitPixel < 0)
                splitPixel += widgetRect.w;
            for(auto& celestial : CelestialOverlayFileName) {
                switch (celestial.mapOverLayImage) {
                    case MapOverLayImage::Sun:
                        drawMapItem(mMapOverlayId[static_cast<std::size_t>(celestial.mapOverLayImage)],
                                    context, widgetRect, mSubSolar, mProjection, splitPixel);
                        break;
                    case MapOverLayImage::Moon:
                        drawMapItem(mMapOverlayId[static_cast<std::size_t>(celestial.mapOverLayImage)],
                                    context, widgetRect, mSubLunar, mProjection, splitPixel);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    Rectangle CelestialOverlay::layout(gm::Context &context, const Rectangle &screenRect) {
        return MapProjection::layout(context, screenRect);
    }

    void CelestialOverlay::addedToContainer() {
        MapProjection::addedToContainer();

        loadMapCelestialObjectImages(mXdgDataPath, getApplication().context());

        mSatelliteObservation = SatelliteObservation{Observer{mQth.lat, mQth.lon, 0.}};
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
        mCelestialUpdateTimer->receiver = [&](int minutes){
            if ((minutes % 2) == 0 && !mMapProjectionsInvalid && !mForegroundBackgroundFuture.valid()) {
                setCelestialObservations();
            }
        };

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
