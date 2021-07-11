/**
 * @file GridOverlay.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-06-21
 */

#include "GridOverlay.h"

namespace rose {

    GridOverlay::GridOverlay(std::shared_ptr<TimerTick> timerTick) : Widget() {
        mTimerTick = std::move(timerTick);
    }

    void GridOverlay::draw(gm::Context &context, const Position<int>& containerPosition) {
        Rectangle widgetRect{containerPosition + mPos, mSize};

        if (mGridOverlayObjects) {
            if (auto mapProjection = containerAs<MapProjection>(); mapProjection) {
                auto projection = mapProjection->getProjection();
                if (projection != mProjection) {
                    mProjection = projection;
                    if (mTexture)
                        mTexture.reset();
                }

                if (mTexture && mTexture.getSize() != widgetRect.size())
                    mTexture.reset();

                if (!mTexture) {
                    auto widgetSize = widgetRect.size();
                    Rectangle textureRect{Position<int>{}, widgetRect.size()};
                    mTexture = gm::Texture{context, widgetRect.size()};
                    mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                    gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                    gm::DrawColorGuard drawColorGuard{context, color::RGBA::TransparentBlack};
                    context.renderClear();

                    AntiAliasedDrawing antiAliasedDrawing{context, AntiAliasedDrawing::AntiAliased};

                    for (const auto &grid : mGridData) {
                        if (grid.draw) {
                            antiAliasedDrawing.setWidthColor(context, grid.lineWidth, grid.color, widgetSize);
                            switch (grid.gridType) {
                                case GridType::LatLon:
                                    for (auto lat = 15; lat <= 75; lat += 15) {
                                        mapProjection->drawLatitude(context, antiAliasedDrawing,
                                                                    static_cast<double>(lat),
                                                                    textureRect);
                                        mapProjection->drawLatitude(context, antiAliasedDrawing,
                                                                    static_cast<double>(-lat),
                                                                    textureRect);
                                    }

                                    for (auto lon = 15; lon <= 180; lon += 15) {
                                        mapProjection->drawLongitude(context, antiAliasedDrawing,
                                                                     static_cast<double>(lon), 75.,
                                                                     textureRect);
                                        mapProjection->drawLongitude(context, antiAliasedDrawing,
                                                                     static_cast<double>(-lon), 75.,
                                                                     textureRect);
                                    }
                                    break;
                                case GridType::Equator:
                                    mapProjection->drawLatitude(context, antiAliasedDrawing, EquatorLatitude, textureRect);
                                    break;
                                case GridType::PrimeMeridian:
                                    mapProjection->drawLongitude(context, antiAliasedDrawing, PrimeMeridian, 90., textureRect);
                                    break;
                                case GridType::IntDateLine:
                                    mapProjection->drawMapLine(context, antiAliasedDrawing, textureRect,
                                                               InternationalDateLine.begin(),
                                                               InternationalDateLine.end());
                                    break;
                                case GridType::Tropics:
                                    mapProjection->drawLatitude(context, antiAliasedDrawing, TropicLatitude, textureRect);
                                    mapProjection->drawLatitude(context, antiAliasedDrawing, -TropicLatitude, textureRect);
                                    mapProjection->drawLatitude(context, antiAliasedDrawing, ArcticCircle, textureRect);
                                    mapProjection->drawLatitude(context, antiAliasedDrawing, -ArcticCircle, textureRect);
                                    break;
                            }
                        }
                    }
                }
            } else {
                throwContainerError();
            }

            if (mTexture)
                context.renderCopy(mTexture, widgetRect);
        }
    }

    Rectangle GridOverlay::layout(gm::Context &context, const Rectangle &screenRect) {
        return screenRect;
    }

    void GridOverlay::addedToContainer() {
        if (auto mapProjection = containerAs<MapProjection>(); mapProjection) {
            mProjection = mapProjection->getProjection();
        } else {
            throwContainerError();
        }

//        mTimerTick->minuteSignal.connect(mGridUpdateTimer);
    }
}
