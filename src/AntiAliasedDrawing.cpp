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
}
