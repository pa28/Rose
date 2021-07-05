/**
 * @file GridOverlay.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-06-21
 * @brief Overlay grid lines on a MapProjection.
 */

#pragma once

#include "MapProjection.h"

namespace rose {

    /**
     * @class GridOverlay.
     * @brief Draw grid lines to annotate the MapProjection.
     */
    class GridOverlay : public Widget {
    protected:
        /// True if there are elements to display.
        bool mGridOverlayObjects{true};

        bool mDrawLatLon{false};
        bool mDrawPrimeMeridian{false};
        bool mDrawInternationalDateLine{false};
        bool mDrawEquator{false};
        bool mDrawTropics{false};

        /// The map projection type.
        MapProjectionType mProjection{};

        /// Slot to receive celestial update time signals on.
        TickProtocol::slot_type mGridUpdateTimer{};

        /// Source of timing information.
        std::shared_ptr<TimerTick> mTimerTick{};

        gm::Texture mTexture{};

    public:
        GridOverlay() = delete;

        explicit GridOverlay(std::shared_ptr<TimerTick> timerTick);

        ~GridOverlay() override = default;

        GridOverlay(const GridOverlay &) = delete;

        GridOverlay(GridOverlay &&) = delete;

        GridOverlay &operator=(const GridOverlay &) = delete;

        GridOverlay &operator=(GridOverlay &&) = delete;

        static constexpr std::string_view id = "GridOverlay";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /// Draw the GridOverlay
        void draw(gm::Context &context, const Position &containerPosition) override;

        /// Layout the GridOverlay
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /**
         * @brief Called when added to a container.
         * @details Connects the map slot receiver for notification when maps are loaded, then calls
         * cacheCurrentMaps().
         */
        void addedToContainer() override;

        static void throwContainerError() {
            throw ContainerTypeError("Expected MapProjection as container for GridOverlay");
        }
    };
}

