/**
 * @file Guage.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-14
 */

#include "Border.h"
#include "Gauge.h"
#include "Manipulators.h"
#include "Math.h"

namespace rose {

    Gauge::Gauge(ImageId imageId) : mIndicator(imageId), mValueLimits(defaultLimits), mValueHues(defaultHues) {
    }

    Gauge::Gauge(const GaugeValueLimits &gaugeValueLimits, const GaugeValueHues &gaugeValueHuess, ImageId imageId)
        : Gauge(imageId) {
        mValueLimits = gaugeValueLimits;
        mValueHues = gaugeValueHuess;
    }

    Gauge::Gauge(GaugeValueLimits &&gaugeValueLimits, GaugeValueHues &&gaugeValueHues, ImageId imageId)
        : Gauge(imageId) {
        mValueLimits = gaugeValueLimits;
        mValueHues = gaugeValueHues;
    }

    void Gauge::initializeComposite() {
        Frame::initializeComposite();
        mLayoutHints.mElastic = true;
        mLayoutHints.mShrinkable = true;

        auto interior = getWidget<Frame>() << BorderStyle::Notch << CornerStyle::Round
                                           << wdg<Border>(4) << Elastic(true)
                                           << wdg<GaugeInterior>() << mOrientation << Elastic(true);

        valueRx = std::make_shared<Slot<float>>();
        valueRx->setCallback([=](uint32_t, float value) {
            setValue(value);
        });
    }

    Rectangle Gauge::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        return Frame::initialLayout(renderer, available);
    }

    void Gauge::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Frame::draw(renderer, parentRect);
    }

    void Gauge::setValue(float value) {
        mValue = value;
    }

    void Gauge::setOrientation(Orientation orientation) {
        Widget::setOrientation(orientation);
    }

    Rectangle Gauge::GaugeInterior::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto gageRect = clampAvailableArea(available, mPos, mSize);
        auto layout = gageRect;
        switch (mOrientation) {
            case Orientation::Unset:
            case Orientation::Horizontal:
                layout.height() = 15;
                layout.width() = gageRect.width();
                break;
            case Orientation::Vertical:
                layout.width() = 15;
                layout.height() = gageRect.height();
                break;
        }
        layout = Position::Zero;
        return layout;
    }

    void Gauge::GaugeInterior::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        auto gaugeRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);
        auto gauge = parent()->parent<Gauge>();
        if (!mTextureValid || !mTexture) {
            auto valueBands = computeValueBands(gauge->mValueLimits, gaugeRect, mOrientation);
            sdl::Surface surface{gaugeRect.width(), gaugeRect.height()};
            if (surface) {
                auto baseHSLA = rose()->theme().mBaseColorHSLA;
                baseHSLA.saturation() = 0.7f;
                baseHSLA.value() = 0.8f;
                for (auto idx = GaugeIndex::LowerBound; idx < GaugeIndex::UpperBound; ++idx) {
                    auto hue = baseHSLA;
                    hue.hue() = gauge->mValueHues[indexValue(idx)];
                    auto color = color::RGBA{hue};
                    auto next = idx + 1;
                    Rectangle fill{};
                    switch (mOrientation) {
                        case Orientation::Unset:
                        case Orientation::Horizontal:
                            fill.x() = valueBands[indexValue(idx)];
                            fill.width() = valueBands[indexValue(next)] - valueBands[indexValue(idx)];
                            fill.y() = 0;
                            fill.height() = gaugeRect.height();
                            break;
                        case Orientation::Vertical:
                            fill.y() = gaugeRect.height() - valueBands[indexValue(next)];
                            fill.height() = valueBands[indexValue(next)] - valueBands[indexValue(idx)];
                            fill.x() = 0;
                            fill.width() = gaugeRect.width();
                            break;
                    }
                    surface.fillRectangle(fill, color);
                }
                mTextureValid = surface.textureFromSurface(renderer, mTexture);
            } else {
                throw RoseRuntimeError(StringCompositor("Creation of Surface failed. ", __FILE__, ':', __LINE__));
            }
        }
        renderer.renderCopy(mTexture, gaugeRect);
    }

    array<int, 6> Gauge::GaugeInterior::computeValueBands(GaugeValueLimits gaugeValueLimits, Rectangle rect,
                                                          Orientation orientation) {
        std::array<int, 6> results{};

        float minBound = gaugeValueLimits[indexValue(GaugeIndex::LowerBound)];
        float maxBound = gaugeValueLimits[indexValue(GaugeIndex::UpperBound)];
        float range = maxBound - minBound;
        float size = 0.f;
        switch (orientation) {
            case Orientation::Unset:
            case Orientation::Horizontal:
                size = (float) rect.width();
                break;
            case Orientation::Vertical:
                size = (float) rect.height();
                break;
        }

        std::transform(gaugeValueLimits.begin(), gaugeValueLimits.end(), results.begin(),
                       [=](float limit) -> int {
                           return util::roundToInt(size * (limit / range));
                       });

        return results;
    }
}
