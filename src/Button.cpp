/**
 * @file Button.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#include "Button.h"

namespace rose {

    ButtonFrame::ButtonFrame() noexcept : Frame(), mButtonSemantics(*this) {
        mButtonSemantics.setButtonDisplayCallback([&](ButtonDisplayState buttonDisplayState){
            std::cout << __PRETTY_FUNCTION__ << ' ' << (int)buttonDisplayState << '\n';
        });

        mButtonSemantics.setButtonStateChangeCallback([&](ButtonStateChange buttonStateChange){
            std::cout << __PRETTY_FUNCTION__ << ' ' << (int)buttonStateChange << "\n\n";
        });
    }

    ButtonFrame::ButtonFrame(int padding) noexcept: ButtonFrame() {
        mPadding = Padding{padding};
    }


}
