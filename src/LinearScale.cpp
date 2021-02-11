/**
 * @file LinearScale.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-16
 */

#include "LinearScale.h"
#include "Manipulators.h"

namespace rose {
    LinearScale::LinearScale(LinearScaleIndicator scaleIndicator) : Frame(), mLinearScaleInd(scaleIndicator) {
    }

    LinearScale::LinearScale(float lowerBound, float upperBound, float value, LinearScaleIndicator scaleIndicator) : LinearScale(scaleIndicator) {
        mLowerBound0 = lowerBound;
        mUpperBound0 = upperBound;
        mValue0 = value;
    }

    Rectangle LinearScale::getIndicatorRectangle(ImageId imageId) {
        Rectangle rectangle{};
        if (imageId != RoseImageId::RoseImageInvalid) {
            rectangle = rose()->imageRepository(imageId).getRectangle();
            if (mOrientation == Orientation::Vertical) {
                if (imageId == RoseImageId::IconLeftDir || imageId == RoseImageId::IconRightDir) {
                    auto hExtra = rectangle.height() - rectangle.width();
                    rectangle.y() = hExtra / 2;
                    rectangle.height() = rectangle.width();
                }
            } else if (mOrientation == Orientation::Horizontal) {
                if (imageId == RoseImageId::IconUpDir || imageId == RoseImageId::IconDownDir) {
                    auto hExtra = rectangle.height() - rectangle.width();
                    rectangle.y() = hExtra / 2;
                    rectangle.height() = rectangle.width();
                }
            }
        }
        return rectangle;
    }

    void LinearScale::initializeComposite() {
        Frame::initializeComposite();

        mGradient = Gradient::GreenYellowRed;

        setBorder(BorderStyle::Notch);
        setCornerStyle(CornerStyle::Round);
        setPadding(4);
        if (mOrientation == Orientation::Unset || mOrientation == Orientation::Both)
            mOrientation = Orientation::Horizontal;
        mLayoutHints.mElastic = Elastic{mOrientation};
        mLayoutHints.mShrinkable = false;

        valueRx = std::make_shared<Slot<SignalType>>();
        valueRx->setCallback([=](uint32_t signalSN, SignalType signalType) {
            setValue(signalType.first, false);
            if (signalSN != mSignalSerialNumber) {
                valueTx.transmit(signalSN, signalType);
            }
        });

        rxScaledValue0 = std::make_shared<Slot<ScaledSignal>>();
        rxScaledValue0->setCallback([=](uint32_t signalSN, ScaledSignal signal){
            mLowerBound0 = signal[1];
            mUpperBound0 = signal[2];
            mValue0 = std::clamp(signal[0], mLowerBound0, mUpperBound0);
//            std::cout << "Rx Temp: " << mValue0 << ',' << mLowerBound0 << ',' << mUpperBound0 << '\n';
            setNeedsDrawing();
        });

        rxScaledValue1 = std::make_shared<Slot<ScaledSignal>>();
        rxScaledValue1->setCallback([=](uint32_t signalSN, ScaledSignal signal){
            mLowerBound1 = signal[1];
            mUpperBound1 = signal[2];
            mValue1 = std::clamp(signal[0], mLowerBound1, mUpperBound1);
//            std::cout << "Rx Proc: " << mValue1 << ',' << mLowerBound1 << ',' << mUpperBound1 << '\n';
            setNeedsDrawing();
        });
    }

    void LinearScale::setValue(float value, bool transmit) {
        mValue0 = std::clamp(value, mLowerBound0, mUpperBound0);

        if (transmit) {
            valueTx.transmit(mSignalSerialNumber.serialNumber(),
                             SignalType{value, mSignalToken});
        }
    }

    Rectangle LinearScale::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto frameAvailable = clampAvailableArea(available, mPos, mSize);
        frameAvailable = mLayoutHints.layoutBegin(frameAvailable);

        switch (mLinearScaleInd) {
            case LinearScaleIndicator::RoundThumb:
                mImageId0 = RoseImageId::BevelOutRoundCorners;
                mImageId1 = RoseImageId::CenterRoundBaseColor;
                mImageRect0 = rose()->imageRepository(mImageId0).getRectangle();
                mImageRect1 = rose()->imageRepository(mImageId1).getRectangle();
                break;
            case LinearScaleIndicator::SquareThumb:
                mImageId0 = RoseImageId::BevelOutSquareCorners;
                mImageId1 = RoseImageId::CenterSquareBaseColor;
                mImageRect0 = getIndicatorRectangle(mImageId0);
                mImageRect1 = getIndicatorRectangle(mImageId1);
                break;
            case LinearScaleIndicator::SingleChannel:
                if (mOrientation == Orientation::Horizontal) {
                    mImageId0 = RoseImageId::ScaleNeedleUp;
                    mImageId0 = RoseImageId::RoseImageInvalid;
                    mImageRect0 = getIndicatorRectangle(mImageId0);
                    mImageRect1 = Rectangle{};
                    break;
                } else if (mOrientation == Orientation::Vertical) {
                    mImageId0 = RoseImageId::IconRightDir;
                    mImageId1 = RoseImageId::RoseImageInvalid;
                    mImageRect0 = getIndicatorRectangle(mImageId0);
                    mImageRect1 = Rectangle{};
                    break;
                }
                break;
            case LinearScaleIndicator::DualChannel:
                if (mOrientation == Orientation::Horizontal) {
                    mImageId0 = RoseImageId::ScaleNeedleUp;
                    mImageId1 = RoseImageId::ScaleNeedleDown;
                    mImageRect0 = getIndicatorRectangle(mImageId0);
                    mImageRect1 = getIndicatorRectangle(mImageId1);
                    break;
                } else if (mOrientation == Orientation::Vertical) {
                    mImageId0 = RoseImageId::IconRightDir;
                    mImageId1 = RoseImageId::IconLeftDir;
                    mImageRect0 = getIndicatorRectangle(mImageId0);
                    mImageRect1 = getIndicatorRectangle(mImageId1);
                    break;
                }
                break;
        }

        switch (mOrientation) {
            case Orientation::Horizontal:
                frameAvailable.height() = std::max(mImageRect0.height(), mImageRect1.height()) * 3 / 2;
                frameAvailable.width() = frameAvailable.height();
                break;
            case Orientation::Vertical:
                frameAvailable.width() = std::max(mImageRect0.width(), mImageRect1.width()) * 3 / 2;
                frameAvailable.height() = frameAvailable.width();
                break;
        }

        frameAvailable = mLayoutHints.layoutEnd(frameAvailable);

        return frameAvailable;
    }

    void LinearScale::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            drawFrameOnly(renderer, widgetRect);
            drawBorder(renderer, widgetRect);
//
//            if (auto child = getSingleChild(); child) {
//                child->draw(renderer, widgetRect);
//            }
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
        auto interior = interiorArea();
        interior = mLayoutHints.relativePositionShift(mLayoutHints.layoutBegin(available));

        if (mImageId0) {
            switch (mOrientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    interior.x() += mImageRect0.width() / 2;
                    interior.width() -= mImageRect0.width();
                    break;
                case Orientation::Vertical:
                    interior.y() += mImageRect0.height() / 2;
                    interior.height() -= mImageRect0.height();
                    break;
                case Orientation::Both:
                    break;
            }
        }

        auto scaleSize = interior.getSize() - mLayoutHints.mPadding.padSize();
        if (mGradient != Gradient::None) {
            drawGradientBackground(renderer, mGradient, interior, mOrientation);
        }

        if (mOrientation == Orientation::Horizontal) {
            if (mImageId0 != RoseImageInvalid) {
                Rectangle dst{interior};
                dst.width() = mImageRect0.width();
                dst.height() = mImageRect0.height();
                dst.x() += roundToInt((mValue0 - mLowerBound0) / (mUpperBound0 - mLowerBound0) * (float)interior.width()) - mImageRect0.width()/2;
                dst.y() = interior.y() + interior.height() - mImageRect0.height();
                rose()->imageRepository().renderCopy(renderer, mImageId0, mImageRect0, dst);
            }

            if (mImageId1 != RoseImageInvalid) {
                Rectangle dst{interior};
                dst.width() = mImageRect0.width();
                dst.height() = mImageRect0.height();
                dst.x() += roundToInt((mValue1 - mLowerBound1) / (mUpperBound1 - mLowerBound1) * (float)interior.width()) - mImageRect0.width()/2;
                rose()->imageRepository().renderCopy(renderer, mImageId1, mImageRect1, dst);
            }
        } else if (mOrientation == Orientation::Vertical) {
            if (mImageId0 != RoseImageInvalid) {
                Rectangle dst{interior};
                dst.width() = mImageRect0.width();
                dst.height() = mImageRect0.height();
                dst.y() += 20;
                dst.x() = interior.x() + interior.width() - mImageRect0.width();
                rose()->imageRepository().renderCopy(renderer, mImageId0, mImageRect0, dst);
            }

            if (mImageId1 != RoseImageInvalid) {
                Rectangle dst{interior};
                dst.width() = mImageRect0.width();
                dst.height() = mImageRect0.height();
                dst.y() += 40;
                rose()->imageRepository().renderCopy(renderer, mImageId1, mImageRect1, dst);
            }
        }

//        if (auto imageView = getSingleChild<ImageView>()) {
//            if (mImageId0) {
//                auto thumbSize = imageView->getSize();
//                auto length = mOrientation == Orientation::Vertical ?
//                              scaleSize.height() - thumbSize->height() :
//                              scaleSize.width() - thumbSize->width();
//                auto offset = (float) length * (mValue - mLowerBound) / (mUpperBound - mLowerBound);
//                auto intOffset = roundToInt(offset);
//                switch (mOrientation) {
//                    case Orientation::Unset:
//                    case Orientation::Horizontal:
//                        imageView->layoutHints().mAssignedRect->x() = mLayoutHints.mPadding.left() + intOffset;
//                        break;
//                    case Orientation::Vertical:
//                        imageView->layoutHints().mAssignedRect->y() = mLayoutHints.mPadding.top() + length - intOffset;
//                        break;
//                    case Orientation::Both:
//                        break;
//                }
//            }
//        }
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
        auto scaleSize = interiorArea().getSize() - mLayoutHints.mPadding.padSize();
        auto padding = getPadding();
        auto imageView = front()->as<ImageView>();
        if (imageView) {
            auto imageRect = imageView->layoutHints().mAssignedRect;
            if (mImageId0) {
                Rectangle dst{available.x() + imageRect->x(), available.y() + imageRect->y(),
                              imageRect->width(), imageRect->height()};
                if (mImageId1)
                    rose()->imageRepository().renderCopy(renderer, mImageId1, dst);
                rose()->imageRepository().renderCopy(renderer, mImageId0, dst);
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
