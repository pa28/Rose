/**
 * @file Button.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#include "Application.h"
#include "Button.h"

#include <utility>
#include "Settings.h"
#include "Theme.h"

namespace rose {

    ButtonFrame::ButtonFrame(ButtonType buttonType) noexcept : Frame() {
        if (buttonType != ButtonType::Label) {
            mButtonSemantics = std::make_unique<ButtonSemantics>(*this);
            mButtonSemantics->setButtonType(buttonType);
        }
        mPadding = Padding{Theme::getTheme().ButtonPadding};
    }


    TextButton::TextButton(ButtonType buttonType) noexcept : ButtonFrame(buttonType), Text() {
        mPointSize = Theme::getTheme().ButtonPointSize;
        mFontName = Theme::getTheme().BoldFont;

        mLayoutManager = std::make_unique<TextButtonLayoutManager>(*this);

        if (mButtonSemantics) {
            mButtonSemantics->setButtonDisplayCallback([&](ButtonDisplayState buttonDisplayState) {
                buttonDisplayStateChange(buttonDisplayState);
                getApplication().redrawBackground();
            });

//            mButtonSemantics->setButtonStateChangeCallback([&](ButtonStateChange buttonStateChange) {
//                switch (buttonStateChange) {
//                    case ButtonStateChange::Pushed:
//                        std::cout << "Button state: Pushed\n";
//                        break;
//                    case ButtonStateChange::Off:
//                        std::cout << "Button state: Off\n";
//                        break;
//                    case ButtonStateChange::On:
//                        std::cout << "Button state: On\n";
//                        break;
//                }
//            });
        }
    }

    TextButton::TextButton(const std::string &text, ButtonType buttonType) : TextButton(buttonType) {
        mText = text;
    }

    TextButton::TextButton(const Id &id, ButtonType buttonType)
            : TextButton(buttonType) {
        mId = id;
        Settings &settings{Settings::getSettings()};
        mText = settings.getValue(id.idString, std::string{id.idString});
    }

    Rectangle TextButton::layout(gm::Context &context, const Rectangle &screenRect) {
        return Frame::layout(context, screenRect);
    }

    void TextButton::draw(gm::Context &context, const Position &containerPosition) {
        Frame::draw(context, containerPosition);

        if (!mTexture) {
            createTextureBlended(context);
        }

        auto drawPosition = drawPadding(containerPosition) + mPos + mFramePadding.position() + Position{mFrameWidth};

        if (mTexture) {
            Rectangle dst{drawPosition, mTextSize};

            if (mCentreHorizontal)
                dst.x += (mScreenRect.w - mTexture.getSize().w) / 2 - mPadding.l - mFrameWidth;
            if (mCentreVertical)
                dst.y += (mScreenRect.h - mTexture.getSize().h) / 2 - mPadding.t - mFrameWidth;

            context.renderCopy(mTexture, dst);
        }
    }

    Rectangle TextButton::layoutContent(gm::Context &context, const Rectangle &screenRect) {
        createTextureBlended(context);
        return Rectangle{Position::Zero, mTextSize};
    }

    TextButtonLayoutManager::TextButtonLayoutManager(TextButton &textButton) : LayoutManager(), mTextButton(textButton) {
        mMaxContent = 0;
    }

    Rectangle
    TextButtonLayoutManager::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr,
                                           LayoutManager::Itr) {
        return mTextButton.layoutContent(context, screenRect);
    }

    ImageButton::ImageButton(ButtonType buttonType) noexcept : ButtonFrame(buttonType), Image() {
        mRequestedSize = Theme::getTheme().ImageLabelSize;

        mLayoutManager = std::make_unique<ImageButtonLayoutManager>(*this);

        if (mButtonSemantics) {
            mButtonSemantics->setButtonDisplayCallback([&](ButtonDisplayState buttonDisplayState) {
                buttonDisplayStateChange(buttonDisplayState);
                getApplication().redrawBackground();
            });

            mButtonSemantics->setButtonStateChangeCallback([&](ButtonStateChange buttonStateChange) {
                switch (buttonStateChange) {
                    case ButtonStateChange::Pushed:
                        std::cout << "Button state: Pushed\n";
                        break;
                    case ButtonStateChange::Off:
                        std::cout << "Button state: Off\n";
                        break;
                    case ButtonStateChange::On:
                        std::cout << "Button state: On\n";
                        break;
                }
            });
        }
    }

    ImageButton::ImageButton(ImageId imageId, ButtonType buttonType) noexcept: ImageButton(buttonType) {
        mImageId = imageId;
    }

    Rectangle ImageButton::layout(gm::Context &context, const Rectangle &screenRect) {
        return Frame::layout(context, screenRect);
    }

    void ImageButton::draw(gm::Context &context, const Position &containerPosition) {
        Frame::draw(context, containerPosition);

        auto drawPosition = drawPadding(containerPosition) + mPos + mFramePadding.position() + Position{mFrameWidth};

        if (mImageId != ImageId::NoImage) {
            ImageStore& imageStore{ImageStore::getStore()};
            Rectangle src{Position::Zero, imageStore.size(mImageId)};
            Rectangle dst{drawPosition, imageStore.size(mImageId)};

            if (mCentreHorizontal)
                dst.x += (mScreenRect.w - dst.w) / 2 - mPadding.l - mFrameWidth;
            if (mCentreVertical)
                dst.y += (mScreenRect.h - dst.h) / 2 - mPadding.t - mFrameWidth;

            if (mRenderFlip.mFlip == SDL_FLIP_NONE)
                imageStore.renderCopy(context, mImageId, dst);
            else
                imageStore.renderCopyEx(context, mImageId, src, dst, 0., mRenderFlip);
        }
    }

    Rectangle ImageButton::layoutContent(gm::Context &context, const Rectangle &screenRect) {
        mSize = ImageStore::getStore().size(mImageId);
        if (mPreferredSize)
            mSize = mPreferredSize;
        mPos = mPreferredPos;

        if (mSize < mRequestedSize) {
            auto space = mRequestedSize - mSize;
            mPadding.l = space.w / 2;
            mPadding.r = space.w - mPadding.l;
            mPadding.t = space.h / 2;
            mPadding.b = space.h - mPadding.t;
        }

        return Rectangle{mPos, mSize};
    }

    void ImageButton::setImage(ImageId imageId) {
        mImageId = imageId;
        getApplication().redrawBackground();
    }

    ImageButtonLayoutManager::ImageButtonLayoutManager(ImageButton &imageButton) : mImageButton(imageButton) {
        mMaxContent = 0;
    }

    Rectangle
    ImageButtonLayoutManager::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                            LayoutManager::Itr last) {
        return mImageButton.layoutContent(context, screenRect);
    }
}
