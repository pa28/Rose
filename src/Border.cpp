/**
 * @file Border.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-07
 */

#include "Border.h"

namespace rose {

    Border::Border() : SingleChild() {
        mClassName = "Border";
    }

    Rectangle Border::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto borderAvailable = clampAvailableArea(available, mPos, mSize);
        if (mPadding) {
            borderAvailable.width() -= mPadding->width();
            borderAvailable.height() -= mPadding->height();
        }
        Rectangle layout{borderAvailable};
        for (auto &child : mChildren) {
            LayoutHints& childHints{child->layoutHints()};
            layout = child->initialLayout(renderer, borderAvailable);
//            if (!layout.getPosition())
//                layout = Position::Zero;
            childHints.mAssignedRect = layout;
            if (mPadding) {
                childHints.mAssignedRect.value().x() += mPadding->left();
                childHints.mAssignedRect.value().y() += mPadding->top();
                layout.width() += mPadding->width();
                layout.height() += mPadding->height();
            }
        }
        return layout;
    }

    void Border::draw(sdl::Renderer &renderer, Rectangle parentRect) {
//        assertLayout();
        if (mVisible) {
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            for (auto &child : mChildren) {
                child->draw(renderer, widgetRect);
            }
        }
    }

    void Border::initializeComposite() {
        if (!mPadding)
            mPadding = Padding::Zero;
    }
}