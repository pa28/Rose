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
        bool isFirst = true;

        while (first != last) {
            auto visual = std::dynamic_pointer_cast<Visual>(*first);
            if (visual->isVisible()) {
                auto contentRect = visual->layout(context, screenRect);
                if (isFirst) {
                    isFirst = false;
                } else {
                    layoutPos.primary(mOrientation) += mInternalSpacing;
                    layoutRect.sizePri(mOrientation) += mInternalSpacing;
                }
                visual->setScreenRectangle(Rectangle{layoutPos, contentRect.size()});
                layoutPos.primary(mOrientation) += contentRect.size().primary(mOrientation);
                layoutRect.sizePri(mOrientation) += contentRect.sizePri(mOrientation);
                layoutRect.sizeSec(mOrientation) = std::max(layoutRect.sizeSec(mOrientation),
                                                            contentRect.sizeSec(mOrientation));
            }
            first++;
        }

        return layoutRect;
    }

    Rectangle
    PlacementLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                   LayoutManager::Itr last) {
        if (first != last) {
            if (auto visual = std::dynamic_pointer_cast<Visual>(*first); visual) {
                auto contRect = visual->layout(context, screenRect);
                for (auto &hint : getLayoutHints(visual)) {
                    switch (hint.attachment()) {
                        case LayoutHint::None:
                            std::cerr << __PRETTY_FUNCTION__ << " Primary object has LayoutHint::None.\n";
                            break;
                        case LayoutHint::TopLeft:
                            contRect = Position::Zero;
                            break;
                        case LayoutHint::TopRight:
                            contRect = Position{screenRect.w - contRect.w, 0};
                            break;
                        case LayoutHint::BottomLeft:
                            contRect = Position{0, screenRect.h - contRect.h};
                            break;
                        case LayoutHint::BottomRight:
                            contRect = Position{screenRect.w - contRect.w, screenRect.h - contRect.h};
                            break;
                        default:
                            std::cerr << __PRETTY_FUNCTION__ << " Attachment not allowed for Primary object.\n";
                            break;
                    }
                }
                visual->setScreenRectangle(contRect);
            }

            for (auto itr = first + 1; itr != last; itr++) {
                if (auto visual = std::dynamic_pointer_cast<Visual>(*itr); visual) {
                    auto contRect = visual->layout(context, screenRect);
                    std::shared_ptr<Visual> ref{};
                    std::sort(getLayoutHints(visual).begin(), getLayoutHints(visual).end());
                    for (auto &hint : getLayoutHints(visual)) {
                        auto refIndex = hint.refIndex();
                        auto n = last - first;
                        if (refIndex != LayoutHint::RefIndexNone && refIndex < (last - first)) {
                            ref = std::dynamic_pointer_cast<Visual>(*(first + refIndex));
                        }
                        switch (hint.attachment()) {
                            case LayoutHint::None:
                                std::cerr << __PRETTY_FUNCTION__ << " Primary object has LayoutHint::None.\n";
                                break;
                            case LayoutHint::TopLeft:
                                contRect = Position::Zero;
                                break;
                            case LayoutHint::TopRight:
                                contRect = Position{screenRect.w - contRect.w, 0};
                                break;
                            case LayoutHint::BottomLeft:
                                contRect = Position{0, screenRect.h - contRect.h};
                                break;
                            case LayoutHint::BottomRight:
                                contRect = Position{screenRect.w - contRect.w, screenRect.h - contRect.h};
                                break;
                            case LayoutHint::Top:
                                contRect.y = 0;
                                break;
                            case LayoutHint::Left:
                                contRect.x = 0;
                                break;
                            case LayoutHint::Bottom:
                                contRect.h = screenRect.h - contRect.y;
                                break;
                            case LayoutHint::Right:
                                contRect.w = screenRect.w - contRect.x;
                                break;
                            case LayoutHint::TopTo:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.y = rect.y + rect.h;
                                }
                                break;
                            case LayoutHint::LeftTo:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.x = rect.x + rect.w;
                                }
                                break;
                            case LayoutHint::BottomTo:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.h = rect.y - contRect.y;
                                }
                                break;
                            case LayoutHint::RightTo:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.w = rect.x - contRect.x;
                                }
                                break;
                            case LayoutHint::TopWith:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.y = rect.y;
                                }
                                break;
                            case LayoutHint::LeftWith:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.x = rect.x;
                                }
                                break;
                            case LayoutHint::BottomWith:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.h = rect.h - (rect.y - contRect.y);
                                }
                            case LayoutHint::RightWith:
                                if (ref) {
                                    auto rect = getScreenRectangle(ref);
                                    contRect.w = rect.w - (rect.x - contRect.x);
                                }
                        }
                    }
                    visual->setScreenRectangle(contRect);
                }
            }
        }
        return screenRect;
    }
}
