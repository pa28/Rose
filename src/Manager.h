/**
 * @file Manager.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-24
 */

#pragma once

#include "Visual.h"
#include "Layout.h"

namespace rose {

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

    class GridLayout : public LayoutManager {
    protected:
        Orientation mOrientation{Orientation::Horizontal};
        Size mInternalSpacing{0};
        int mStride{0};

    public:
        GridLayout() = default;

        ~GridLayout() override = default;

        explicit GridLayout(Orientation orientation, int horizontalSpace = 0, int verticalSpace = 0, int stride = 0)
                : GridLayout() {
            mOrientation = orientation;
            mInternalSpacing.w = horizontalSpace;
            mInternalSpacing.h = verticalSpace;
            mStride = stride;
        }

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };

    class Row : public Manager {
    protected:

    public:
        Row() : Manager() {
            setLayoutManager(std::make_unique<LinearLayout>(Orientation::Horizontal));
        }

        static constexpr std::string_view id = "Row";
        std::string_view nodeId() const noexcept override {
            return id;
        }
    };

    class Column : public Manager {
    protected:

    public:
        Column() : Manager() {
            setLayoutManager(std::make_unique<LinearLayout>(Orientation::Vertical));
        }

        static constexpr std::string_view id = "Column";

        std::string_view nodeId() const noexcept override {
            return id;
        }
    };

    class Grid : public Manager {
    protected:

    public:
        Grid() : Manager() {
            setLayoutManager(std::make_unique<GridLayout>(Orientation::Horizontal, 2, 2, 0));
        }

        ~Grid() override = default;

        Grid(int stride) : Manager() {
            setLayoutManager(std::make_unique<GridLayout>(Orientation::Horizontal, 2, 2, stride));
        }

        static constexpr std::string_view id = "Grid";

        std::string_view nodeId() const noexcept override {
            return id;
        }

        void draw(gm::Context &context, const Position &containerPosition) override {
            Manager::draw(context, containerPosition);
        }
    };

    /**
     * @class Overlay
     * @brief A layout manager for to create overlays on the Container.
     * @details Each object managed by the Overlay is mapped into the same size and location as the container.
     */
    class Overlay : public LayoutManager {
    public:
        Overlay() = default;

        ~Overlay() override = default;

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };
}
