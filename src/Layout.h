/**
 * @file Layout.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-14
 */

#pragma once

#include "Visual.h"

namespace rose {

    /**
     * @class Layout
     * @brief
     */
    class StackLayout : public LayoutManager {
    protected:

    public:
        StackLayout() = default;

        ~StackLayout() override = default;

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) override;

    };

    class LinearLayout : public LayoutManager {
    protected:
        Orientation mOrientation{Orientation::Horizontal};
        int mInternalSpacing{0};

    public:
        LinearLayout() = default;

        ~LinearLayout() override = default;

        explicit LinearLayout(Orientation orientation, int internalSpace = 0) : LinearLayout() {
            mOrientation = orientation;
            mInternalSpacing = internalSpace;
        }

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };

    class PlacementLayout : public LayoutManager {
    protected:

    public:
        PlacementLayout() = default;

        ~PlacementLayout() override = default;

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };
}

