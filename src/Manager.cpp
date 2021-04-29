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
        std::vector<Size> maxSizeList(mStride);
        Size maxSize{};
        if (mStride) {
            int sizeIdx = 0;
            for (auto index = first; index != last; ++index) {
                auto visual = std::dynamic_pointer_cast<Visual>(*index);
                if (visual->isVisible()) {
                    auto contentRect = visual->layout(context, screenRect);
                    maxSizeList[sizeIdx].w = std::max(maxSizeList[sizeIdx].w, contentRect.w);
                    maxSizeList[sizeIdx].h = std::max(maxSizeList[sizeIdx].h, contentRect.h);
                }
                ++sizeIdx;
                if (sizeIdx >= mStride)
                    sizeIdx = 0;
            }
        } else {
            for (auto index = first; index != last; ++index) {
                auto visual = std::dynamic_pointer_cast<Visual>(*index);
                if (visual->isVisible()) {
                    auto contentRect = visual->layout(context, screenRect);
                    maxSize.w = std::max(maxSize.w, contentRect.w);
                    maxSize.h = std::max(maxSize.h, contentRect.h);
                }
            }
        }

        Position pos{};
        Rectangle layoutRect{};
        if (mStride) {
            auto sizeIdx = maxSizeList.begin();
            for (auto index = first; index != last; ++index) {
                auto visual = std::dynamic_pointer_cast<Visual>(*index);
                if (visual->isVisible()) {
                    if (sizeIdx == maxSizeList.begin())
                        layoutRect.sizeSec(mOrientation) +=
                               (*sizeIdx).secondary(mOrientation) + mInternalSpacing.secondary(mOrientation);

                    Rectangle contentRect{pos, (*sizeIdx)};

                    auto advance = contentRect.sizePri(mOrientation) + mInternalSpacing.primary(mOrientation);

                    pos.primary(mOrientation) += advance;

                    layoutRect.sizePri(mOrientation) = std::max(layoutRect.sizePri(mOrientation),
                                                                pos.primary(mOrientation));

                    visual->setScreenRectangle(contentRect);

                    ++sizeIdx;
                    if (sizeIdx == maxSizeList.end()) {
                        pos.primary(mOrientation) = 0;
                        pos.secondary(mOrientation) +=
                                maxSizeList.back().secondary(mOrientation) + mInternalSpacing.secondary(mOrientation);

                        sizeIdx = maxSizeList.begin();
                    }
                }
            }
        } else {
            for (auto index = first; index != last; ++index) {
                auto visual = std::dynamic_pointer_cast<Visual>(*index);
                if (visual->isVisible()) {
                    if (pos.primary(mOrientation) == 0)
                        layoutRect.sizeSec(mOrientation) +=
                                maxSize.secondary(mOrientation) + mInternalSpacing.secondary(mOrientation);

                    auto hintMap = visual->getHintMap<LayoutHint::GridHint>();

                    Rectangle contentRect{pos, maxSize};

                    auto advance = contentRect.sizePri(mOrientation) + mInternalSpacing.primary(mOrientation);

                    if (hintMap) {
                        if (auto axisSize = getHintValue(hintMap.value(),
                                                         LayoutHint::GridLayoutHint::AxisSize); axisSize) {
                            advance = axisSize.value() * advance / 100;
                            contentRect.sizePri(mOrientation) = advance - mInternalSpacing.primary(mOrientation);
                        }
                        if (auto axisOffset = getHintValue(hintMap.value(),
                                                           LayoutHint::GridLayoutHint::AxisOffset); axisOffset) {
                            contentRect.posPri(mOrientation) +=
                                    axisOffset.value() * contentRect.sizePri(mOrientation) / 100;
                            advance += axisOffset.value() * contentRect.sizePri(mOrientation) / 100;
                        }
                    }
                    pos.primary(mOrientation) += advance;

                    layoutRect.sizePri(mOrientation) = std::max(layoutRect.sizePri(mOrientation),
                                                                pos.primary(mOrientation));

                    visual->setScreenRectangle(contentRect);

                    if (hintMap) {
                        if (auto endAxis = getHintValue(hintMap.value(),
                                                        LayoutHint::GridLayoutHint::EndStride); endAxis) {
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
        }

        std::cout << __PRETTY_FUNCTION__ << layoutRect << '\n';
        return layoutRect;
    }
}
