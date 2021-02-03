/** @file Slider.cpp
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2020-12-30
  */

#include "Manipulators.h"
#include "Slider.h"

namespace rose {

    Slider::Slider(float value) : LinearScale() {
        setValue(value, false);
        mSupportsDrag = true;
        mAcceptsFocus = true;
        mSupportsScrollWheel = true;
    }

    Slider::Slider(ImageId imageId) : LinearScale(imageId) {
        mSupportsDrag = true;
        mAcceptsFocus = true;
        mSupportsScrollWheel = true;
    }

    Slider::Slider(float lowerBound, float upperBound, float value, ImageId imageId)
            : LinearScale(lowerBound, upperBound, value, imageId) {
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
        return true;
    }

    bool
    Slider::mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) {
        return Container::mouseMotionEvent(cursorPosition, rel, button, modifiers);
    }

    bool Slider::mouseDragEvent(const Position &mousePos, const Position &rel, int button, int modifiers) {
        if (mChildren.empty())
            return false;

        auto relMousePos = mousePos - mLayoutHints.mAssignedRect->getPosition();
        auto sliderSize = interiorRectangle().getSize() - getPadding()->padSize();
        relMousePos = relMousePos - layoutHints().mAssignedRect->getPosition();

        if (auto thumb = getChild<ImageView>(); thumb) {
            auto thumbSize = thumb->getSize();
            relMousePos = relMousePos - thumb->layoutHints().mAssignedRect->getPosition();
            thumb->mouseEnterEvent(relMousePos, false);

            int clampMax;
            float value = 0.f;
            switch (mOrientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    clampMax = sliderSize.width() - thumbSize->width();
                    mSliderOffset = std::clamp<int>(mSliderOffset + rel.x(), 0, clampMax);
                    break;
                case Orientation::Vertical:
                    clampMax = sliderSize.height() - thumbSize->height();
                    mSliderOffset = std::clamp<int>(mSliderOffset + rel.y(), 0, clampMax);
                    break;
                case Orientation::Both:
                    break;
            }

            value = (float)mSliderOffset / (float)clampMax;
            setValue(value, true);
            setNeedsDrawing();
        }

        return true;
    }

    bool Slider::scrollEvent(const Position &p, int32_t x, int32_t y) {
        if (mChildren.empty())
            return false;

        auto sliderSize = interiorRectangle().getSize() - getPadding()->padSize();
        if (auto thumb = getChild<ImageView>(); thumb) {
            auto thumbSize = thumb->getSize();

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
            switch (mOrientation) {
                case Orientation::Unset:
                case Orientation::Horizontal:
                    clampMax = sliderSize.width() - thumbSize->width();
                    mSliderOffset = std::clamp<int>(mSliderOffset + y * multiplier, 0, clampMax);
                    break;
                case Orientation::Vertical:
                    clampMax = sliderSize.height() - thumbSize->height();
                    mSliderOffset = std::clamp<int>(mSliderOffset + y * multiplier, 0, clampMax);
                    break;
                case Orientation::Both:
                    break;
            }

            auto value = (float)mSliderOffset / (float)clampMax;

            setValue(value, true);
            setNeedsDrawing();
        }

        return true;
    }

    bool Slider::keyboardEvent(int key, int scancode, int action, int modifiers) {
        return Container::keyboardEvent(key, scancode, action, modifiers);
    }

    bool Slider::keyboardCharacterEvent(unsigned int codepoint) {
        return Container::keyboardCharacterEvent(codepoint);
    }

}
