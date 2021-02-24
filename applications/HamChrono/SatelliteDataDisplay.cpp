/**
 * @file SatelliteDataDisplay.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-23
 */

#include "Label.h"
#include "Manipulators.h"
#include "SatelliteDataDisplay.h"

namespace rose {

    void SatelliteDataDisplay::initializeComposite() {
        Frame::initializeComposite();

        time_t now = time(nullptr);
        getWidget<Frame>() << wdg<Column>()
                           << wdg<Label>(mName, mImageId) << Manip::Parent
                           << wdg<Label>(mDataStub ? mDataStub->passTimeString(now) : "");
    }

    Rectangle SatelliteDataDisplay::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Frame::widgetLayout(renderer, available, layoutStage);
    }

    void SatelliteDataDisplay::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Frame::draw(renderer, parentRect);
    }

    void SatelliteDataDisplay::setData(ImageId imageId, const string &name, const SatelliteDataStub &dataStub) {
        mImageId = static_cast<RoseImageId>(imageId);
        mName = name;
        mDataStub = dataStub;

        auto column = getSingleChild<Column>();
        if (column) {
            if (column->size() == 2) {
                auto label = column->at(0)->as<Label>();
                if (label) {
                    label->setText(mName);
                    label->setBadge(mImageId);
                }

                label = column->at(1)->as<Label>();
                if (label) {
                    time_t now = time(nullptr);
                    label->setText(mDataStub->passTimeString(now));
                }
            }
        }
    }
}
