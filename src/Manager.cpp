/**
 * @file Manager.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-24
 */

#include "Manager.h"

namespace rose {

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

    std::optional<int> getHintValue(const Visual::ValueMap &map, LayoutHint::GridLayoutHint hint) {
        if (auto value = map.find(static_cast<int>(hint)); value != map.end()) {
            return value->second;
        }
        return std::nullopt;
    }

    Rectangle GridLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) {
        Size maxSize{};
        for (auto index = first; index != last; ++index) {
            auto visual = std::dynamic_pointer_cast<Visual>(*index);
            if (visual->isVisible()) {
                auto contentRect = visual->layout(context, screenRect);
                maxSize.w = std::max(maxSize.w, contentRect.w);
                maxSize.h = std::max(maxSize.h, contentRect.h);
            }
        }

        Position pos{};
        Rectangle layoutRect{};
        for (auto index = first; index != last; ++index) {
            auto visual = std::dynamic_pointer_cast<Visual>(*index);
            if (visual->isVisible()) {
                if (pos.primary(mOrientation) == 0)
                    layoutRect.sizeSec(mOrientation) +=
                            maxSize.secondary(mOrientation) + mInternalSpacing.secondary(mOrientation);

                auto hintMap = visual->getHintMap<LayoutHint::GridHint>();

                Rectangle contentRect{pos, maxSize};

                if (hintMap) {
                    if (auto axisSize = getHintValue(hintMap.value(), LayoutHint::GridLayoutHint::AxisSize); axisSize)
                        contentRect.sizePri(mOrientation) = axisSize.value() * contentRect.sizePri(mOrientation) / 100;
                    if (auto axisOffset = getHintValue(hintMap.value(), LayoutHint::GridLayoutHint::AxisOffset); axisOffset)
                        contentRect.posPri(mOrientation) += axisOffset.value() * contentRect.sizePri(mOrientation) / 100;
                }

                pos.primary(mOrientation) += contentRect.sizePri(mOrientation);
                layoutRect.sizePri(mOrientation) = std::max(layoutRect.sizePri(mOrientation),
                                                            pos.primary(mOrientation));
                pos.primary(mOrientation) += mInternalSpacing.primary(mOrientation);

                visual->setScreenRectangle(contentRect);

                if (hintMap) {
                    if (auto endAxis = getHintValue(hintMap.value(), LayoutHint::GridLayoutHint::EndStride); endAxis) {
                        pos.primary(mOrientation) = 0;
                        pos.secondary(mOrientation) +=
                                maxSize.secondary(mOrientation) + mInternalSpacing.secondary(mOrientation);
                    }
                } else {
                    if (mStride > 0 && pos.primary(mOrientation) >= mStride * maxSize.primary(mOrientation)) {
                        pos.primary(mOrientation) = 0;
                        pos.secondary(mOrientation) +=
                                maxSize.secondary(mOrientation) + mInternalSpacing.secondary(mOrientation);
                    }
                }
            }
        }

        std::cout << __PRETTY_FUNCTION__ << layoutRect << '\n';
        return layoutRect;
    }
}
