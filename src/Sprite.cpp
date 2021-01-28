//
// Created by richard on 2020-10-30.
//

#include "Sprite.h"
#include "Entypo.h"
#include "Rose.h"
#include "Widget.h"

namespace rose {

    Sprite::Sprite(std::shared_ptr<Widget> parent) : Widget(parent) {
        mTicks = SDL_GetTicks();
    }

    void Sprite::draw(sdl::Renderer &renderer, RectangleInt parentRect) {
        auto scale = rose().imageRepository().image(mImageId).getScale();
        auto iconSize = rose().imageRepository().image(mImageId).getSize();
        if (scale != ImageRepository::InvalidImageId) {
            if (!mCurrentPosition)
                mCurrentPosition = Position<float>{(float) mPos.x(), (float) mPos.y()};

            auto dTicks = SDL_GetTicks() - mTicks;
            auto screenPos = getScreenPosition();

            if ((float) dTicks < mVectorTicks) {
                auto vectorPart = ((float) dTicks / mVectorTicks);
                mCurrentPosition += mCurrentVector * vectorPart;
                mPos.x() = roundToInt(mCurrentPosition.x() * (float) scale);
                mPos.y() = roundToInt(mCurrentPosition.y() * (float) scale);
                mCurrentVector -= mCurrentVector * vectorPart;
                mVectorTicks -= mVectorTicks * vectorPart;
            }

            SDL_Rect src;
            src.x = (iconSize.width() / scale) * (scale - 1 - (mPos.x() % scale));
            src.y = (iconSize.height() / scale) * (scale - 1 - (mPos.y() % scale));
            src.w = iconSize.width() / scale;
            src.h = iconSize.height() / scale;

            SDL_Rect dst;
            dst.x = mPos.x() / scale;
            dst.y = mPos.y() / scale;
            dst.w = iconSize.width() / scale;
            dst.h = iconSize.height() / scale;

            rose().imageRepository().renderCopy(renderer, mImageId, src, dst);
//        SDL_RenderCopy(renderer.get(), mTexture.get(), &src, &dst);

            mTicks += dTicks;
        }
    }
}
