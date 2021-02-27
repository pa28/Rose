/**
 * @file SatelliteDataDisplay.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-23
 */

#pragma once

#include "Frame.h"
#include "Container.h"
#include "MapProjection.h"

namespace rose {

    /**
     * @class SatelliteDataDisplay
     * @brief Display satellite orbital data.
     */
    class SatelliteDataDisplay : public Frame {
    protected:
        RoseImageId mImageId{RoseImageInvalid};
        std::string mName{};
        std::optional<SatelliteMetaData> mDataStub{};

    public:
        ~SatelliteDataDisplay() override = default;

        SatelliteDataDisplay() = default;

        SatelliteDataDisplay(SatelliteDataDisplay &&) = delete;

        SatelliteDataDisplay(const SatelliteDataDisplay &) = delete;

        SatelliteDataDisplay &operator=(SatelliteDataDisplay &&) = delete;

        SatelliteDataDisplay &operator=(const SatelliteDataDisplay &) = delete;

        /**
         * @brief Set the display data.
         * @param imageId The ImageId of the satellite display icon.
         * @param name The Satellite name.
         * @param dataStub The last pas data stub.
         */
        SatelliteDataDisplay(ImageId imageId, const std::string &name, const SatelliteMetaData &dataStub)
                : SatelliteDataDisplay() {
            mImageId = static_cast<RoseImageId>(imageId);
            mName = name;
            mDataStub = dataStub;
        }

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
         * @param dataStub The last pas data stub.
         */
        void setData(ImageId imageId, const std::string &name, const SatelliteMetaData &dataStub);
    };

    /**
     * @class SatelliteDataSet
     * @brief A class to manage a number of SatelliteDataDisplay objects.
     */
    class SatelliteDataSet : public Column {
    protected:

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
        SatelliteDataSet() : Column() {
            mClassName = "SatelliteDataSet";
        }
    };
}

