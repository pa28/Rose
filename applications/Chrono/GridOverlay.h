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
        enum class GridType {
            Equator,            ///< The Equator
            PrimeMeridian,      ///< The Prime Meridian
            IntDateLine,        ///< The International Date Line.
            Tropics,            ///< The Tropics of Cancer and Capricorn, Arctic and Antarctic Circles
            LatLon,             ///< All remaining Latitudes and Longitudes at 15 degree interval.
        };

        struct GridData {
            GridType gridType{GridType::Equator};
            bool draw{false};
            int lineWidth{1};
            color::RGBA color{color::RGBA::OpaqueBlack};
        };

        std::array<GridData, 5> mGridData{{
                                                  {GridType::LatLon, false, 1,
                                                          color::RGBA{0.5f, 0.5f, 0.5f, 1.0f}},
                                                  {GridType::Equator, true, 1,
                                                          color::RGBA{0.4f, 1.f, 0.4f, 1.0f}},
                                                  {GridType::PrimeMeridian, true, 1,
                                                          color::RGBA{0.4f, 1.f, 0.4f, 1.0f}},
                                                  {GridType::Tropics, true, 1,
                                                          color::RGBA{1.f, 1.f, 0.f, 1.0f}},
                                                  {GridType::IntDateLine, true, 1,
                                                          color::RGBA{1.f, 0.f, 0.f, 1.0f}},
                                          }};

        /// True if there are elements to display.
        bool mGridOverlayObjects{true};

        /// The map projection type.
        MapProjectionType mProjection{};

        /// Slot to receive celestial update time signals on.
        TickProtocol::slot_type mGridUpdateTimer{};

        /// Source of timing information.
        std::shared_ptr<TimerTick> mTimerTick{};

        /// The Texture used to render all grid lines.
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
        void draw(gm::Context &context, const Position<int>& containerPosition) override;

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

