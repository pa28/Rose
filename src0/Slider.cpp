/** @file Slider.cpp
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2020-12-30
  */

#include "Manipulators.h"
#include "Slider.h"

namespace rose {

    Slider::Slider(float value) : LinearScale(LinearScaleIndicator::RoundThumb) {
        setValue(value, true, false);
        mSupportsDrag = true;
        mAcceptsFocus = true;
        mSupportsScrollWheel = true;
    }

    Slider::Slider(ImageId imageId) : LinearScale(LinearScaleIndicator::RoundThumb) {
        mSupportsDrag = true;
        mAcceptsFocus = true;
        mSupportsScrollWheel = true;
    }

    Slider::Slider(float lowerBound, float upperBound, float value, ImageId imageId)
            : LinearScale(lowerBound, upperBound, value, LinearScaleIndicator::RoundThumb) {
        mSupportsDrag = true;
        mAcceptsFocus = true;
        mSupportsScrollWheel = true;
    }

    void Slider::initializeComposite() {
        LinearScale::initializeComposite();
        mSupportsDrag = true;
        mGradient = Gradient::GreenYellowRed;
    }

    bool Slider::mouseEnterEvent(const Position &p, bool enter) {
        return Container::mouseEnterEvent(p, enter);
    }

    bool Slider::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        if (!down) {
            if (mDrag)
                setValue(mValue, true, true);
            mDrag = false;
        }
        return true;
    }

    bool
    Slider::mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) {
        return Container::mouseMotionEvent(cursorPosition, rel, button, modifiers);
    }

    bool Slider::mouseDragEvent(const Position &mousePos, const Position &rel, int button, int modifiers) {
        auto relMousePos = mousePos - mLayoutHints.mAssignedRect->getPosition();
        auto sliderSize = interiorArea().getSize() - getPadding().padSize();
        relMousePos = relMousePos - layoutHints().mAssignedRect->getPosition();

        auto thumbSize = mImageRect1.getSize();

        int clampMax;
        clampMax = sliderSize.width() - thumbSize.width();
        mSliderOffset = std::clamp<int>(mSliderOffset + rel.x(), 0, clampMax);

        auto value = (float) mSliderOffset / (float) clampMax;
        setValue(value, false, true);
        setNeedsDrawing();

        mDrag = true;

        return true;
    }

    bool Slider::scrollEvent(const Position &p, int32_t x, int32_t y) {
        auto sliderSize = interiorArea().getSize() - getPadding().padSize();
        auto thumbSize = mImageRect1.getSize();

        auto ticks = SDL_GetTicks();
        int multiplier = 1;
        auto dTicks = ticks - mLastScrollTick;
        mLastScrollTick = ticks;

        if (dTicks > 250)
            multiplier = 1;
        else if (dTicks > 50)
            multiplier = 4;
        else if (dTicks > 10)
            multiplier = 8;
        else
            multiplier = 16;

        int clampMax = 0;
        clampMax = sliderSize.width() - thumbSize.width();
        mSliderOffset = std::clamp<int>(mSliderOffset + y * multiplier, 0, clampMax);

        auto value = (float) mSliderOffset / (float) clampMax;

        setValue(value, true, true);

        return true;
    }

//    bool Slider::keyboardEvent(int key, int scancode, int action, int modifiers) {
//        return Container::keyboardEvent(key, scancode, action, modifiers);
//    }

    bool Slider::keyboardCharacterEvent(unsigned int codepoint) {
        return Container::keyboardCharacterEvent(codepoint);
    }

    void Slider::setValue(float value, bool final, bool transmit) {
        setThumbOffset(value, 1.f);
        mValue = value;
        if (transmit) {
            valueTx.transmit(mSignalSerialNumber.serialNumber(), std::make_pair(final, value));
        }
    }

}
