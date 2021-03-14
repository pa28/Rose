/**
 * @file Layout.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-14
 */

#include "Layout.h"

namespace rose {

    Rectangle StackLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                         LayoutManager::Itr last) {
        Size size;
        auto itr = first;
        bool oneIsVisible{false};

        while (itr != last) {
            auto visual = std::dynamic_pointer_cast<Visual>(*itr);
            if (visual->isVisible()) {
                if (oneIsVisible)
                    visual->setVisible(false);
                oneIsVisible = true;
            }
            auto rect = visual->layout(context, screenRect);
            size.w = std::max(size.w, rect.w);
            size.h = std::max(size.h, rect.h);
            itr++;
        }

        Rectangle layoutRect{Position::Zero, size};

        while (first != last) {
            auto visual = std::dynamic_pointer_cast<Visual>(*itr);
            visual->setScreenRectangle(layoutRect);
        }

        return layoutRect;
    }

    Rectangle LinearLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                          LayoutManager::Itr last) {
        Rectangle layoutRect{};
        Position layoutPos{};

        while (first != last) {
            auto visual = std::dynamic_pointer_cast<Visual>(*first);
            if (visual->isVisible()) {
                auto contentRect = visual->layout(context, screenRect);
                visual->setScreenRectangle(Rectangle{layoutPos, contentRect.size()});
                layoutPos.primary(mOrientation) += contentRect.size().primary(mOrientation);
                layoutRect.sizePri(mOrientation) += contentRect.sizePri(mOrientation);
                layoutRect.sizeSec(mOrientation) = std::max(layoutRect.sizeSec(mOrientation),contentRect.sizePri(mOrientation));
            }
            first++;
        }

        return layoutRect;
    }
}
