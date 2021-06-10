/**
 * @file CelestialOverlay.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-06-10
 * @brief Add observable celestial objects to MapProjection.
 */

#pragma once

#include "MapProjection.h"

namespace rose {

    /**
     * @class CelestialOverlay
     * @brief Add observable celestial objects to MapProjection.
     */
    class CelestialOverlay : public MapProjection {

    protected:
        static constexpr std::array<OverlayImageSpec,static_cast<size_t>(MapOverLayImage::Count)> CelestialOverlayFileName
                {{
                         { MapOverLayImage::Sun, "35px-SunBg.png" },
                         { MapOverLayImage::Moon, "moon22.png"},
                 }};

        std::array<ImageId,CelestialOverlayFileName.size()> mMapOverlayId{};

        /// If true celestial objects (Sun, Moon) will be displayed.
        bool mDisplayCelestialObjects{true};

    public:
        CelestialOverlay() = delete;

        explicit CelestialOverlay(std::shared_ptr<TimerTick> timerTick, std::filesystem::path& xdgDataPath);

        ~CelestialOverlay() override = default;

        CelestialOverlay(const CelestialOverlay &) = delete;

        CelestialOverlay(CelestialOverlay &&) = delete;

        CelestialOverlay &operator=(const CelestialOverlay &) = delete;

        CelestialOverlay &operator=(CelestialOverlay &&) = delete;

        static constexpr std::string_view id = "CelestialOverlay";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /// Draw the CelestialOverlay
        void draw(gm::Context &context, const Position &containerPosition) override;

        /// Layout the CelestialOverlay
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /**
         * @brief Called when added to a container.
         * @details Connects the map slot receiver for notification when maps are loaded, then calls
         * cacheCurrentMaps().
         */
        void addedToContainer() override;

        /// Load overlay images into the ImageStore.
        void loadMapCelestialObjectImages(const std::filesystem::path &xdgResourcePath, gm::Context &context);
    };
}

