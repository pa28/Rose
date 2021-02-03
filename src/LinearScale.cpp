/**
 * @file LinearScale.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-16
 */

#include "LinearScale.h"
#include "Manipulators.h"

namespace rose {
    LinearScale::LinearScale(rose::ImageId imageId) : Frame() {
        mImageId = imageId;
        switch (mImageId) {
            case RoseImageId::RoseImageInvalid:
                break;
            case RoseImageId::BevelInRoundCorners:
            case RoseImageId::BevelOutRoundCorners:
            case RoseImageId::NotchInRoundCorners:
            case RoseImageId::NotchOutRoundCorners:
                mCenterId = RoseImageId::CenterRoundBaseColor;
                break;
            case RoseImageId::BevelInSquareCorners:
            case RoseImageId::BevelOutSquareCorners:
            case RoseImageId::NotchInSquareCorners:
            case RoseImageId::NotchOutSquareCorners:
                mCenterId = RoseImageId::CenterSquareBaseColor;
                break;
        }
    }

    LinearScale::LinearScale(float lowerBound, float upperBound, float value, ImageId imageId) : LinearScale(imageId) {
        mLowerBound = lowerBound;
        mUpperBound = upperBound;
        mValue = value;
    }

    void LinearScale::initializeComposite() {
        Frame::initializeComposite();

        mGradient = Gradient::GreenYellowRed;

        setBorder(BorderStyle::Notch);
        setCornerStyle(CornerStyle::Round);
        mPadding = 4;
        if (mOrientation == Orientation::Unset)
            mOrientation = Orientation::Horizontal;
        mLayoutHints.mElastic = Elastic{mOrientation};
        mLayoutHints.mShrinkable = false;

        mIndicator = getWidget<Frame>() << wdg<LinearScaleImage>(mImageId);
        if (!mImageId)
            mIndicator << Size{20, 20};

        valueRx = std::make_shared<Slot<SignalType>>();
        valueRx->setCallback([=](uint32_t signalSN, SignalType signalType) {
            setValue(signalType.first, false);
            if (signalSN != mSignalSerialNumber) {
                valueTx.transmit(signalSN, signalType);
            }
        });
    }

    void LinearScale::setValue(float value, bool transmit) {
        mValue = std::clamp(value, mLowerBound, mUpperBound);

        if (transmit) {
            valueTx.transmit(mSignalSerialNumber.serialNumber(),
                             SignalType{value, mSignalToken});
        }
    }

    Rectangle LinearScale::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Frame::widgetLayout(renderer, available, 0);
    }

    void LinearScale::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            drawFrameOnly(renderer, widgetRect);
            drawBorder(renderer, widgetRect);

            for (auto &child : mChildren) {
                child->draw(renderer, widgetRect);
            }
        }
    }

    /*
     * Local LinearScaleBorder methods.
     */

    void LinearScale::initializeBorderComposite() {
    }

    Rectangle
    LinearScale::initialBorderLayout(sdl::Renderer &renderer, Rectangle available, Rectangle borderLayout) {
        return borderLayout;
    }

    void LinearScale::drawBorder(sdl::Renderer &renderer, Rectangle available) {
        auto interior = Rectangle{available.getPosition(), interiorRectangle().getSize()};
        auto padding = getPadding();
        if (padding) {
            interior.x() += mFrameWidth + padding->left();
            interior.y() += mFrameWidth + padding->top();
            interior.width() -= padding->width();
            interior.height() -= padding->height();
        }
        if (mImageId) {
            auto size = rose()->imageRepository(mImageId).getSize();
            switch (mOrientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    interior.x() += size.width() / 2;
                    interior.width() -= size.width();
                    break;
                case Orientation::Vertical:
                    interior.y() += size.height() / 2;
                    interior.height() -= size.height();
                    break;
            }
        }
        auto scaleSize = interiorRectangle().getSize() - (padding ? padding->padSize() : Size::Zero);
        if (mGradient != Gradient::None) {
            drawGradientBackground(renderer, mGradient, interior, mOrientation);
        }
        auto imageView = front()->as<ImageView>();
        if (imageView) {
            if (mImageId) {
                auto thumbSize = imageView->getSize();
                auto length = mOrientation == Orientation::Vertical ?
                              scaleSize.height() - thumbSize->height() :
                              scaleSize.width() - thumbSize->width();
                auto offset = (float) length * (mValue - mLowerBound) / (mUpperBound - mLowerBound);
                auto intOffset = roundToInt(offset);
                switch (mOrientation) {
                    case Orientation::Unset:
                    case Orientation::Horizontal:
                        imageView->layoutHints().mAssignedRect->x() = padding->left() + intOffset;
                        break;
                    case Orientation::Vertical:
                        imageView->layoutHints().mAssignedRect->y() = padding->top() + length - intOffset;
                        break;
                }
            }
        }
    }

    /*
     * Local LinearScale Image methods.
     */

    void LinearScale::initializeImageComposite() {
    }

    Rectangle
    LinearScale::initialImageLayout(sdl::Renderer &renderer, Rectangle available, Rectangle imageLayout) {
        return imageLayout;
    }

    void LinearScale::drawImage(sdl::Renderer &renderer, Rectangle available) {
        auto scaleSize = interiorRectangle().getSize() - getPadding()->padSize();
        auto padding = getPadding();
        auto imageView = front()->as<ImageView>();
        if (imageView) {
            auto imageRect = imageView->layoutHints().mAssignedRect;
            if (mImageId) {
                Rectangle dst{available.x() + imageRect->x(), available.y() + imageRect->y(),
                              imageRect->width(), imageRect->height()};
                if (mCenterId)
                    rose()->imageRepository().renderCopy(renderer, mCenterId, dst);
                rose()->imageRepository().renderCopy(renderer, mImageId, dst);
            }
        }
    }

#if 0
    /*
     * LinearScaleBorder
     */

    void LinearScale::LinearScaleBorder::initializeComposite() {
        Border::initializeComposite();
        parent<LinearScale>()->initializeBorderComposite();
    }

    Rectangle
    LinearScale::LinearScaleBorder::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto layout = Border::widgetLayout(renderer, available, 0);
        return parent<LinearScale>()->initialBorderLayout(renderer, available, layout);
    }

    void LinearScale::LinearScaleBorder::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        parent<LinearScale>()->drawBorder(renderer, parentRect);
        Border::draw(renderer, parentRect);
    }
#endif

    /*
     * LinearScaleImage
     */

    void LinearScale::LinearScaleImage::initializeComposite() {
        ImageView::initializeComposite();
        parent<LinearScale>()->initializeImageComposite();
    }

    Rectangle
    LinearScale::LinearScaleImage::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto linearScale = parent()->parent<LinearScale>();
        auto imageRect = clampAvailableArea(available, mPos, mSize);

        if (mImageId) {
            imageRect = ImageView::widgetLayout(renderer, available, 0);
        }

        return parent<LinearScale>()->initialImageLayout(renderer, available, imageRect);
    }

    void LinearScale::LinearScaleImage::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        parent<LinearScale>()->drawImage(renderer, parentRect);
    }
}
