/**
 * @file CelestialOverlay.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-06-10
 * @brief Add observable celestial objects to MapProjection.
 * @details The CelestialOverlay is designed to display icons representing the positions of the Sun and Moon
 * on the Earth overlaid on a MapProjection. The CelestialOverlay can only be added to a MapProjection and
 * communicates with the MapProjection to match display with the MapOverlay.
 */

#pragma once

#include "MapProjection.h"

namespace rose {

    /**
     * @class CelestialOverlay
     * @brief Add observable celestial objects to MapProjection.
     * @details The CelestialOverlay is designed to display icons representing the positions of the Sun and Moon
     * on the Earth overlaid on a MapProjection. The CelestialOverlay can only be added to a MapProjection and
     * communicates with the MapProjection to match display with the MapOverlay.
     */
    class CelestialOverlay : public Widget {

    protected:
        static constexpr std::array<OverlayImageSpec,static_cast<size_t>(MapOverLayImage::Count)> CelestialOverlayFileName
                {{
                         { MapOverLayImage::Sun, "35px-SunBg.png" },
                         { MapOverLayImage::Moon, "moon22.png"},
                 }};

        std::array<ImageId,CelestialOverlayFileName.size()> mMapOverlayId{};

        /// Slot to receive celestial update time signals on.
        TickProtocol::slot_type mCelestialUpdateTimer{};

        /// If true celestial objects (Sun, Moon) will be displayed.
        bool mDisplayCelestialObjects{true};

        /// The last calculated celestial observations.
        SatelliteObservation mCelestialObservations;

        /// The geographic sub-solar position.
        GeoPosition mSubSolar{};

        /// The geographic sub-lunar position.
        GeoPosition mSubLunar{};

        /// The last calculated observations.
        SatelliteObservation mSatelliteObservation;

        /// Path to the XDG application data directory.
        std::filesystem::path mXdgDataPath;

        /// Source of timing information.
        std::shared_ptr<TimerTick> mTimerTick{};

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
        void draw(gm::Context &context, const Position<int>& containerPosition) override;

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

        void setCelestialObservations() {
            auto[latS, lonS] = MapProjection::subSolar();
            mSubSolar = GeoPosition{latS, lonS, true};

            if (mCelestialObservations.empty()) {
                mCelestialObservations = SatelliteObservation{mSatelliteObservation.observer(), "Moon"};
                mCelestialObservations.predict(DateTime{true});
            }

            if (!mCelestialObservations.empty()) {
                mCelestialObservations.predict(DateTime{true});
                auto[lat, lon] = mCelestialObservations.front().geo();
                mSubLunar = GeoPosition{lat, lon, true};
            }
        }

        static void throwContainerError() {
            throw ContainerTypeError("Expected MapProjection as container for CelestialOverlay");
        }
    };
}

