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

    void GridOverlay::draw(gm::Context &context, const Position &containerPosition) {
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
                    Rectangle textureRect{Position{}, widgetRect.size()};
                    mTexture = gm::Texture{context, widgetRect.size()};
                    mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                    gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                    gm::DrawColorGuard drawColorGuard{context, color::RGBA::TransparentBlack};
                    context.renderClear();

                    AntiAliasedDrawing antiAliasedDrawing{context, AntiAliasedDrawing::SimpleRectangle};
                    antiAliasedDrawing.setWidthColor(context, 3, color::RGBA{0.4f, 1.f, 0.4f, 1.0f});

                    if (mDrawLatLon) {
                        for (auto lat = 15; lat <= 75; lat += 15) {
                            mapProjection->drawLatitude(context, antiAliasedDrawing, static_cast<double>(lat),
                                                        textureRect);
                            mapProjection->drawLatitude(context, antiAliasedDrawing, static_cast<double>(-lat),
                                                        textureRect);
                        }

                        for (auto lon = 15; lon <= 180; lon += 15) {
                            mapProjection->drawLongitude(context, antiAliasedDrawing, static_cast<double>(lon), 75.,
                                                         textureRect);
                            mapProjection->drawLongitude(context, antiAliasedDrawing, static_cast<double>(-lon), 75.,
                                                         textureRect);
                        }
                    }

                    if (mDrawEquator)
                        mapProjection->drawLatitude(context, antiAliasedDrawing, EquatorLatitude, textureRect);

                    if (mDrawPrimeMeridian)
                        mapProjection->drawLongitude(context, antiAliasedDrawing, PrimeMeridian, 90., textureRect);

                    if (mDrawInternationalDateLine) {
                        antiAliasedDrawing.setColor(context, color::RGBA{1.f, 0.f, 0.f, 1.0f});
                        mapProjection->drawMapLine(context, antiAliasedDrawing, textureRect,
                                                   InternationalDateLine.begin(),
                                                   InternationalDateLine.end());
                    }

                    if (mDrawTropics) {
                        antiAliasedDrawing.setColor(context, color::RGBA{1.f, 1.f, 0.f, 1.0f});
                        mapProjection->drawLatitude(context, antiAliasedDrawing, TropicLatitude, textureRect);
                        mapProjection->drawLatitude(context, antiAliasedDrawing, -TropicLatitude, textureRect);
                        mapProjection->drawLatitude(context, antiAliasedDrawing, ArcticCircle, textureRect);
                        mapProjection->drawLatitude(context, antiAliasedDrawing, -ArcticCircle, textureRect);
                    }
                }
            } else {
                throwContainerError();
            }

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

        mTimerTick->minuteSignal.connect(mGridUpdateTimer);
    }
}
