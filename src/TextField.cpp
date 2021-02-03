/**
 * @file TextField.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-03
 */

#include "TextField.h"

namespace rose {

    Rectangle TextField::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Frame::widgetLayout(renderer, available, layoutStage);
    }

    void TextField::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Frame::draw(renderer, parentRect);
    }

    void TextField::initializeComposite() {
        Frame::initializeComposite();
    }
}
