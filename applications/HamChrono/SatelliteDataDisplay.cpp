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
                           << wdg<Label>(mName, mImageId)
                                   << FontSize{20}
                                   << Manip::Parent
                           << wdg<Label>(mMetaData ? mMetaData->passTimeString(now) : "")
                                   << FontSize{15};
    }

    Rectangle SatelliteDataDisplay::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        return Frame::widgetLayout(renderer, available, layoutStage);
    }

    void SatelliteDataDisplay::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Frame::draw(renderer, parentRect);
    }

    void SatelliteDataDisplay::setData(const TrackedSatellite &satellite) {
        mImageId = static_cast<RoseImageId>(satellite.metaData.imageId);
        mName = satellite.satellite.getName();
        mMetaData = satellite.metaData;

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
                    label->setText(mMetaData->passTimeString(now));
                }
            }
        }
    }

    void SatelliteDataDisplay::timeUpdate(time_t timer) {
        if (auto column = getSingleChild<Column>(); column) {
            if (column->size() >= 2) {
                if (auto label = column->at(1)->as<Label>(); label) {
                    label->setText(mMetaData->passTimeString(timer));
                }
            }
        }
    }

    void SatelliteDataSet::initializeComposite() {
        Column::initializeComposite();
        trackedSatelliteRx = std::make_shared<Slot<MapProjection::SignalType>>();
        trackedSatelliteRx->setCallback([&](uint32_t, MapProjection::SignalType satellites) {
            auto timer = time(nullptr);
            auto dataDisplay = mChildren.begin();
            for (const auto &satellite : satellites) {
                if (dataDisplay == mChildren.end())
                    break;
                if (auto disp = (*dataDisplay)->as<SatelliteDataDisplay>(); disp) {
                    disp->setData(satellite);
                }
                ++dataDisplay;
            }
        });

        secondRx = std::make_shared<Slot<int>>();
        secondRx->setCallback([&](uint32_t, int second){
            auto timer = time(nullptr);
            for (auto &child : mChildren) {
                if (auto dataDisplay = child->as<SatelliteDataDisplay>(); dataDisplay)
                    dataDisplay->timeUpdate(timer);
            }
        });

        for (auto i = 0; i < 5; ++i) {
            getWidget<Column>() << wdg<SatelliteDataDisplay>();
        }
    }
}
