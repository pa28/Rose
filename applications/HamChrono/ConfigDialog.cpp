/**
 * @file ConfigDialog.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-14
 */

#include "ConfigDialog.h"
#include "HamChrono.h"
#include "Keyboard.h"
#include "Manipulators.h"
#include "TextField.h"

namespace rose {

    void ConfigDialog::initializeComposite() {
        Dialog::initializeComposite();
        mModal = Modality::Modal;
        mCallRegex = std::make_shared<std::regex>(std::string{CallPattern}.c_str());
        mFloatRegex = std::make_shared<std::regex>(std::string{FloatPattern}.c_str());

        mActionButtonSlot = std::make_shared<Slot<Button::SignalType>>();
        mActionButtonSlot->setCallback([&](uint32_t, Button::SignalType button){
        });

        auto column = mMessageRow << wdg<Column>();
        auto row1 = column << wdg<Row>();

        qthConfigure(row1);

        auto keyboard = std::make_shared<QUERTY>();
        column << wdg<Keyboard>(keyboard);
        requestFocus();
        setActionButtons(mActionButtons);
        setButtonSlot(mActionButtonSlot);
    }

    void ConfigDialog::qthConfigure(shared_ptr <Row> &parent) {
        auto callsign = rose()->settings()->getValue("CALLSIGN",std::string{});
        auto qth = rose()->settings()->getValue<GeoPosition>("QTH_Location");

        std::string latTxt, lonTxt;
        if (qth) {
            latTxt = util::fmtReal(qth->lat(), 6);
            lonTxt = util::fmtReal(qth->lon(), 7);
        }

        parent << wdg<Frame>(6) << BorderStyle::Notch << CornerStyle::Round
                   << wdg<Column>()
                       << wdg<Label>("QTH") << Manip::Parent
                       << wdg<TextField>(10, callsign, "", "Call", 6) >> mCallSign
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20}
                           << ToUpperCase{} << mCallRegex
                           << Manip::Parent
                       << wdg<TextField>(8, latTxt, "Deg", "Lat", 6) >> mLatitude
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20} << mFloatRegex
                           << Manip::Parent
                       << wdg<TextField>(9, lonTxt, "Deg", "Lon", 6) >> mLongitude
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20} << mFloatRegex
                           << Manip::Parent;
    }
}
