/** @file Slider.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-30
  */

#pragma once

#include "Border.h"
#include "Frame.h"
#include "ImageView.h"
#include "LinearScale.h"

namespace rose {

    /**
     * @class Slider
     * @brief An analog input Widget.
     */

    class Slider : public LinearScale {
    protected:
        int mSliderOffset{0};           ///< The offset of the thumb from the position of lower bound.
        uint32_t mLastScrollTick{0};    ///< The last interaction tick.

    public:
        ~Slider() override = default;
        Slider(const Slider &) = delete;
        Slider& operator=(const Slider &) = delete;
        Slider(Slider &&) = delete;
        Slider& operator=(Slider &&) = delete;

        explicit Slider(float value);

        /**
         * @brief Constructor
         * @param imageId The ImageId for the indicator which is omitted if imageId == RoseImageInvalid.
         */
        explicit Slider(ImageId imageId = BevelOutRoundCorners);

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /**
         * @brief Constructor
         * @param lowerBound The lower bound of the scale.
         * @param upperBound The upper bound of the scale.
         * @param value The initial value of the scale.
         * @param imageId The ImageId for the indicator which is omitted if imageId == RoseImageInvalid.
         */
        Slider(float lowerBound, float upperBound, float value, ImageId imageId = RoseImageInvalid);

        /// Handle a mouse enter event.
        bool mouseEnterEvent(const Position &p, bool enter) override;

        /// Handle a mouse button event (default implementation: propagate to children)
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

        /// Handle a mouse motion event (default implementation: propagate to children)
        bool mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse drag event (default implementation: do nothing)
        bool mouseDragEvent(const Position &p, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse scroll event (default implementation: propagate to children)
        bool scrollEvent(const Position &p, double x, double y) override;

        /// Handle a keyboard event (default implementation: do nothing)
        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

        /// Handle text input (UTF-32 format) (default implementation: do nothing)
        bool keyboardCharacterEvent(unsigned int codepoint) override;
    };
}
