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

        std::for_each(first, last, [&context, &screenRect, &oneIsVisible, &size](auto &obj){
            if (auto visual = std::dynamic_pointer_cast<Visual>(obj); visual) {
                if (visual->isVisible()) {
                    if (oneIsVisible)
                        visual->setVisible(false);
                    oneIsVisible = true;
                }
                auto rect = visual->layout(context, screenRect);
                size.w = std::max(size.w, rect.w);
                size.h = std::max(size.h, rect.h);
            }
        });

        Rectangle layoutRect{Position::Zero, size};

        std::for_each(first, last, [&layoutRect](auto &obj){
            if (auto visual = std::dynamic_pointer_cast<Visual>(obj); visual)
                visual->setScreenRectangle(layoutRect);
        });

        return layoutRect;
    }

    Rectangle
    PlacementLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                   LayoutManager::Itr last) {
        if (first != last) {
            if (auto visual = std::dynamic_pointer_cast<Visual>(*first); visual) {
                auto contRect = visual->layout(context, screenRect);
                if (auto attachView = visual->getHintMap<LayoutHint::AttachmentHint>(); attachView) {
                    for (auto &hint : attachView.value()) {
                        switch (static_cast<LayoutHint::Attachment>(hint.first)) {
                            case LayoutHint::Attachment::None:
                                std::cerr << __PRETTY_FUNCTION__ << " Primary object has LayoutHint::None.\n";
                                break;
                            case LayoutHint::Attachment::TopLeft:
                                contRect = Position::Zero;
                                break;
                            case LayoutHint::Attachment::TopRight:
                                contRect = Position{screenRect.w - contRect.w, 0};
                                break;
                            case LayoutHint::Attachment::BottomLeft:
                                contRect = Position{0, screenRect.h - contRect.h};
                                break;
                            case LayoutHint::Attachment::BottomRight:
                                contRect = Position{screenRect.w - contRect.w, screenRect.h - contRect.h};
                                break;
                            default:
                                std::cerr << __PRETTY_FUNCTION__ << " Attachment not allowed for Primary object.\n";
                                break;
                        }
                    }
                }
                visual->setScreenRectangle(contRect);
            }

            for (auto itr = first + 1; itr != last; itr++) {
                if (auto visual = std::dynamic_pointer_cast<Visual>(*itr); visual) {
                    auto contRect = visual->layout(context, screenRect);
                    std::shared_ptr<Visual> ref{};
                    if (auto attachView = visual->getHintMap<LayoutHint::AttachmentHint>(); attachView) {
//                        std::sort(getLayoutHints(visual).begin(), getLayoutHints(visual).end());
                        for (auto &hint : attachView.value()) {
                            auto refIndex = hint.second;
                            auto n = last - first;
                            if (refIndex != LayoutHint::RefIndexNone && refIndex < (last - first)) {
                                ref = std::dynamic_pointer_cast<Visual>(*(first + refIndex));
                            }
                            switch (static_cast<LayoutHint::Attachment>(hint.first)) {
                                case LayoutHint::Attachment::None:
                                    std::cerr << __PRETTY_FUNCTION__ << " Primary object has LayoutHint::None.\n";
                                    break;
                                case LayoutHint::Attachment::TopLeft:
                                    contRect = Position::Zero;
                                    break;
                                case LayoutHint::Attachment::TopRight:
                                    contRect = Position{screenRect.w - contRect.w, 0};
                                    break;
                                case LayoutHint::Attachment::BottomLeft:
                                    contRect = Position{0, screenRect.h - contRect.h};
                                    break;
                                case LayoutHint::Attachment::BottomRight:
                                    contRect = Position{screenRect.w - contRect.w, screenRect.h - contRect.h};
                                    break;
                                case LayoutHint::Attachment::Top:
                                    contRect.y = 0;
                                    break;
                                case LayoutHint::Attachment::Left:
                                    contRect.x = 0;
                                    break;
                                case LayoutHint::Attachment::Bottom:
                                    contRect.h = screenRect.h - contRect.y;
                                    break;
                                case LayoutHint::Attachment::Right:
                                    contRect.w = screenRect.w - contRect.x;
                                    break;
                                case LayoutHint::Attachment::TopTo:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.y = rect.y + rect.h;
                                    }
                                    break;
                                case LayoutHint::Attachment::LeftTo:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.x = rect.x + rect.w;
                                    }
                                    break;
                                case LayoutHint::Attachment::BottomTo:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.h = rect.y - contRect.y;
                                    }
                                    break;
                                case LayoutHint::Attachment::RightTo:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.w = rect.x - contRect.x;
                                    }
                                    break;
                                case LayoutHint::Attachment::TopWith:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.y = rect.y;
                                    }
                                    break;
                                case LayoutHint::Attachment::LeftWith:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.x = rect.x;
                                    }
                                    break;
                                case LayoutHint::Attachment::BottomWith:
                                    if (ref) {
                                        auto rect = getScreenRectangle(ref);
                                        contRect.h = rect.h - (rect.y - contRect.y);
                                    }
                                case LayoutHint::Attachment::RightWith:
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
        }
        return screenRect;
    }
}
