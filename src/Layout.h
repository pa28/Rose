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
        ~StackLayout() override = default;

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) override;

    };

    class PlacementLayout : public LayoutManager {
    protected:

    public:
        ~PlacementLayout() override = default;

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };
}

