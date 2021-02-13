/**
 * @file MapProjections.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-13
 */

#include "MapProjection.h"
#include "Math.h"

namespace rose {

    MapProjection::MapProjection(std::optional<ImageId> day, std::optional<ImageId> night, GeoPosition qth, Size mapSize) {
        mDayMapImage = day ? day.value() : RoseImageInvalid;
        mNightMapImage = night ? night.value() : RoseImageInvalid;
        mQth = qth;
        mMapSize = mapSize;
    }

    void MapProjection::initializeComposite() {
    }

    Rectangle MapProjection::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Rectangle{0, 0, mMapSize.width(), mMapSize.height()};
    }

    void MapProjection::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (!mGeoChron) {
            if (mDayMapImage != RoseImageInvalid && mNightMapImage != RoseImageInvalid)
                if (!rose()->imageRepository().isValidImage(renderer, mDayMapImage) ||
                    !rose()->imageRepository().isValidImage(renderer, mNightMapImage))
                    return;
        }

        Rectangle widgetRect{};
        widgetRect = parentRect.getPosition() + mLayoutHints.mAssignedRect->getPosition();
        widgetRect = mLayoutHints.mAssignedRect->getSize();

        switch (mProjection) {
            case ProjectionType::Mercator:
                rose()->imageRepository().renderCopy(renderer, mDayMapImage, widgetRect);
                break;
            case ProjectionType::StationMercator: {
                auto lon = mQth.lon();
                int splitPixel = util::roundToInt((double)mMapSize.width() * ((lon) / 360.));
                if (splitPixel < 0)
                    splitPixel += mMapSize.width();

                Rectangle src{splitPixel, 0, mMapSize.width()-splitPixel, mMapSize.height()};
                Rectangle dst{widgetRect};
                dst.width() = src.width();
                dst.height() = src.height();
                rose()->imageRepository().renderCopy(renderer, mDayMapImage, src, dst);

                src.x() = 0;
                dst.x() += src.width();
                src.width() = splitPixel;
                dst.width() = splitPixel;
                rose()->imageRepository().renderCopy(renderer, mDayMapImage, src, dst);
            }
                break;
            case ProjectionType::StationAzmuthal:
                break;
        }
    }
}
