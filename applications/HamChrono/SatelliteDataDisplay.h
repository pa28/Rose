/**
 * @file SatelliteDataDisplay.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-23
 */

#pragma once

#include "Button.h"
#include "Container.h"
#include "MapProjection.h"

namespace rose {

    /**
     * @class SatelliteDataDisplay
     * @brief Display satellite orbital data.
     */
    class SatelliteDataDisplay : public ButtonFrame {
    protected:
        RoseImageId mImageId{RoseImageInvalid};
        std::string mName{};
        std::optional<SatelliteMetaData> mMetaData{};

    public:
        ~SatelliteDataDisplay() override = default;

        SatelliteDataDisplay() = delete;

        SatelliteDataDisplay(SatelliteDataDisplay &&) = delete;

        SatelliteDataDisplay(const SatelliteDataDisplay &) = delete;

        SatelliteDataDisplay &operator=(SatelliteDataDisplay &&) = delete;

        SatelliteDataDisplay &operator=(const SatelliteDataDisplay &) = delete;

        explicit SatelliteDataDisplay(int padding) : ButtonFrame(padding) {}

        /**
         * @brief Set the display data.
         * @param imageId The ImageId of the satellite display icon.
         * @param name The Satellite name.
         * @param dataStub The last pas data stub.
         */
        SatelliteDataDisplay(ImageId imageId, const std::string &name, const SatelliteMetaData &dataStub);

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /// See Widget::widgetLayout()
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /// See Widget::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Set the display data.
         * @param imageId The ImageId of the satellite display icon.
         * @param name The Satellite name.
         * @param satellite The last pas data stub.
         */
        void setData(const TrackedSatellite &satellite);

        void timeUpdate(time_t timer);

        const std::string& getName() const { return mName; }
    };

    /**
     * @class SatelliteDataSet
     * @brief A class to manage a number of SatelliteDataDisplay objects.
     */
    class SatelliteDataSet : public Frame {
    protected:
        /// Receive settings updates from the settings database.
        std::shared_ptr<Slot<std::string>> mSettingsUpdateRx{};

    public:
        ~SatelliteDataSet() override = default;

        SatelliteDataSet(SatelliteDataSet &&) = delete;

        SatelliteDataSet(const SatelliteDataSet &) = delete;

        SatelliteDataSet &operator=(SatelliteDataSet &&) = delete;

        SatelliteDataSet &operator=(const SatelliteDataSet &) = delete;

        /**
         * @brief Constructor see: Widget constructor.
         * @param parent
         */
        SatelliteDataSet() : Frame(3) {
            mClassName = "SatelliteDataSet";
        }

        /// See Widget::initializeComposite()
        void initializeComposite() override;

        /// Slot to receive tracked satellite data.
        std::shared_ptr<Slot<MapProjection::SignalType>> trackedSatelliteRx{};

        /// Slot to receive timing pulses to update the display.
        std::shared_ptr<Slot<int>> secondRx{};

        /// The RadioBehavior object.
        RadioBehavior radioBehavior{true};
    };
}

