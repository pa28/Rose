/**
 * @file Button.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#include "Button.h"

namespace rose {

    ButtonFrame::ButtonFrame() : Frame(), mButtonSemantics(*this) {
        mButtonSemantics.setButtonDisplayCallback([&](ButtonDisplayState buttonDisplayState){
            std::cout << __PRETTY_FUNCTION__ << '\n';
        });

        mButtonSemantics.setButtonStateChangeCallback([&](ButtonStateChange buttonStateChange){
            std::cout << __PRETTY_FUNCTION__ << '\n';
        });
    }
}
