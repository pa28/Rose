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
        ButtonFrame::initializeComposite();

        mButtonType = ButtonType::ToggleButton;

        time_t now = time(nullptr);
        getWidget<ButtonFrame>() << wdg<Column>()
                           << wdg<Label>(mName, mImageId)
                                   << FontSize{18}
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

    SatelliteDataDisplay::SatelliteDataDisplay(ImageId imageId, const string &name, const SatelliteMetaData &dataStub)
            : ButtonFrame(0) {
        mImageId = static_cast<RoseImageId>(imageId);
        mName = name;
        mMetaData = dataStub;
        mButtonType = ButtonType::ToggleButton;
    }

    void SatelliteDataSet::initializeComposite() {
        Frame::initializeComposite();

        if (rose()->hasSettings()) {
            mSettingsUpdateRx = std::make_shared<Slot<std::string>>();
            mSettingsUpdateRx->setCallback([&](uint32_t, const std::string &name) {
                if (!name.empty()) {
                    if (name == set::SatelliteMode) {
                        setVisible(rose()->settings()->getValue(name, 0) != 0);
                    }
                }
            });
            rose()->settings()->dataChangeTx.connect(mSettingsUpdateRx);
        }

        trackedSatelliteRx = std::make_shared<Slot<MapProjection::SignalType>>();
        trackedSatelliteRx->setCallback([&](uint32_t, MapProjection::SignalType satellites) {
            auto radioState = radioBehavior.getState();
            std::string satelliteName{};
            if (auto column = getSingleChild<Column>(); column) {
                if (radioState.second < column->size()) {
                    satelliteName = column->at(radioState.second)->as<SatelliteDataDisplay>()->getName();
                } else {
                    radioState.first = RadioBehavior::State::None;
                    radioState.second = 0;
                }

                auto timer = time(nullptr);
                auto dataDisplay = column->begin();
                for (const auto &satellite : satellites) {
                    if (dataDisplay == column->end())
                        break;
                    if (auto disp = (*dataDisplay)->as<SatelliteDataDisplay>(); disp) {
                        disp->setData(satellite);
                    }

                    if (!satelliteName.empty() && satellite.satellite.getName() == satelliteName) {
                        radioState.first = RadioBehavior::State::Set;
                        radioState.second = dataDisplay - column->begin();
                    }
                    (*dataDisplay)->setVisible(true);
                    ++dataDisplay;
                }

                while (dataDisplay != column->end()) {
                    (*dataDisplay)->setVisible(false);
                    ++dataDisplay;
                }
            }
            radioBehavior.setState(radioState);
        });

        secondRx = std::make_shared<Slot<int>>();
        secondRx->setCallback([&](uint32_t, int second) {
            if (auto column = getSingleChild<Column>(); column) {
                auto timer = time(nullptr);
                for (auto &child : *column) {
                    if (auto dataDisplay = child->as<SatelliteDataDisplay>(); dataDisplay)
                        dataDisplay->timeUpdate(timer);
                }
            }
        });

        auto column = getWidget<SatelliteDataSet>()
                << BorderStyle::Notch
                << Elastic(Orientation::Horizontal)
                << wdg<Column>();

        for (auto i = 0; i < 6; ++i) {
            auto display = column << wdg<SatelliteDataDisplay>(0)
                   << Elastic{Orientation::Horizontal};
            auto frame = display->as<ButtonFrame>();
            radioBehavior.emplace_back(frame);
        }
    }
}
