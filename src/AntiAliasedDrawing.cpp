/**
 * @file AntiAliasedDrawing.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-22
 */

#include "AntiAliasedDrawing.h"

namespace rose {

    AntiAliasedDrawing::AntiAliasedDrawing(gm::Context &context, DrawingType drawingType) {
        mDrawingType = drawingType;
    }

    void AntiAliasedDrawing::setWidthColor(gm::Context &context, int width, color::RGBA rgba, Size& widgetSize) {
        mColor = rgba;
        mWidth = width;
        mWidgetSize = widgetSize;

        switch (mDrawingType) {
            case SimpleLine:
                break;
#if 0
            case SimpleRectangle: {
                mTexture = gm::Texture{context, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                       std::max(widgetSize.w, widgetSize.h) + 2, mWidth + 2};
                gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                auto transparent = rgba;
                transparent.a() = 0.;
                context.fillRect(Rectangle{0, 0, mTexture.getSize().w, mTexture.getSize().h}, transparent);
                context.fillRect(Rectangle{0, 1, mTexture.getSize().w, mTexture.getSize().h - 2},
                                 mColor);
            }
                break;
            case AntiAliased: {
                mTexture = gm::Texture{context, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                       NubWidth, NubHeight};
                gm::RenderTargetGuard renderTargetGuard{context, mTexture};
                mTexture.setBlendMode(SDL_BLENDMODE_BLEND);
                auto transparent = rgba;
                transparent.a() = 0.;
                context.fillRect(Rectangle{0, 0, NubWidth, NubHeight}, transparent);
                context.fillRect(Rectangle{0, (NubHeight - NubColorSize) / 2, NubWidth, (NubHeight - NubColorSize) / 2},
                                 mColor);
            }
                break;
#endif
        }
    }

    void AntiAliasedDrawing::drawLine(gm::Context &context, Position<float> p0, Position<float> p1, int interiorWidth) {
        auto plot = [&context,this](const Position<int>& p, float alpha) {
            return context.drawPoint(p, mColor.withAlpha(alpha));
        };

        auto stuff0 = [&interiorWidth]() {
            if (interiorWidth < 2)
                return 1;
            return interiorWidth / 2 + 1;
        };

        auto stuff1 = [&interiorWidth]() {
            if (interiorWidth < 2)
                return 1;
            auto r = interiorWidth / 2;
            auto q = interiorWidth % 1;
            r = r + (interiorWidth % 2 ? 2 : 1);
            return r;
        };

        auto ipart = [](float x) {
            return std::floor(x);
        };

        auto fpart = [](float x) {
            return x - std::floor(x);
        };

        auto rfpart = [&](float x) {
            return 1.f - fpart(x);
        };

        gm::DrawColorGuard drawColorGuard{context, mColor};

        auto steep = std::abs(p1.y - p0.y) > std::abs(p1.x - p0.x);

        auto stuffPixels = [&](Position<int> p0, Position<int> p1) -> void {
            if (steep) {
                for (p0.x = p0.x + 1; p0.x < p1.x; ++p0.x) {
                    plot(p0, 1.f);
                }
            } else {
                for (p0.y = p0.y + 1; p0.y < p1.y; ++p0.y)
                    plot(p0, 1.f);
            }
        };

        if (steep) {
            std::swap(p0.x, p0.y);
            std::swap(p1.x, p1.y);
        }

        if (p0.x > p1.x) {
            std::swap(p0.x, p1.x);
            std::swap(p0.y, p1.y);
        }

        Position<float> d{p1.x - p0.x, p1.y - p0.y};

        auto gradient = 1.f;
        if (d.x != 0.f)
            gradient = d.y / d.x;

        // handle first endpoint
        auto xend = std::round(p0.x);
        auto yend = p0.y + gradient * (xend - p0.x);
        auto xgap = rfpart(p0.x  + 0.5f);
        auto xpxl1 = xend;
        auto ypxl1 = ipart(yend);

        if (steep) {
            Position<int> position0{static_cast<int>(ypxl1-stuff0()), static_cast<int>(xpxl1)};
            Position<int> position1{static_cast<int>(ypxl1+stuff1()), static_cast<int>(xpxl1)};
            stuffPixels(position0,position1);
            plot(position0, rfpart(yend) * xgap);
            plot(position1, fpart(yend) * xgap);
        } else {
            Position<int> position0{static_cast<int>(xpxl1), static_cast<int>(ypxl1-stuff0())};
            Position<int> position1{static_cast<int>(xpxl1), static_cast<int>(ypxl1+stuff1())};
            stuffPixels(position0,position1);
            plot(position0, rfpart(yend) * xgap);
            plot(position1, fpart(yend) * xgap);
        }

        auto intery = yend + gradient;

        // handle second endpoint
        xend = std::round(p1.x);
        yend = p1.y + gradient * (xend - p1.x);
        xgap = fpart(p1.x + 0.5f);
        auto xpxl2 = xend;
        auto ypxl2 = ipart(yend);

        if (steep) {
            Position<int> position0{static_cast<int>(ypxl2-stuff0()), static_cast<int>(xpxl2)};
            Position<int> position1{static_cast<int>(ypxl2+stuff1()), static_cast<int>(xpxl2)};
            plot(position0, rfpart(yend) * xgap);
            plot(position1, fpart(yend) * xgap);
            stuffPixels(position0,position1);
        } else {
            Position<int> position0{static_cast<int>(xpxl2), static_cast<int>(ypxl2-stuff0())};
            Position<int> position1{static_cast<int>(xpxl2), static_cast<int>(ypxl2+stuff1())};
            plot(position0, rfpart(yend) * xgap);
            plot(position1, fpart(yend) * xgap);
            stuffPixels(position0,position1);
        }

        // main loop
        if (steep) {
            for (auto x = xpxl1 + 1; x < xpxl2; ++x) {
                Position<int> position0{ipart(intery)-stuff0(), x};
                Position<int> position1{ipart(intery)+stuff1(), x};
                plot(position0, rfpart(intery));
                plot(position1, fpart(intery));
                stuffPixels(position0,position1);
                intery += gradient;
            }
        } else {
            for (auto x = xpxl1 + 1; x < xpxl2; ++x) {
                Position<int> position0{x, ipart(intery)-stuff0()};
                Position<int> position1{x, ipart(intery)+stuff1()};
                plot(position0, rfpart(intery));
                plot(position1, fpart(intery));
                stuffPixels(position0,position1);
                intery += gradient;
            }
        }
    }
}
