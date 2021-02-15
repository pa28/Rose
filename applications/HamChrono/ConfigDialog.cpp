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
        PopupWindow::initializeComposite();
        mModal = Modality::Modal;
        mCallRegex = std::make_shared<std::regex>(std::string{CallPattern}.c_str());
        mFloatRegex = std::make_shared<std::regex>(std::string{FloatPattern}.c_str());

        mFrame << BorderStyle::BevelIn;
        mFrame->setInvert(true);
        auto frame = mColumn << wdg<Frame>(4) << BorderStyle::BevelIn;
        auto column = frame << wdg<Column>() << InternalSpace{4};
        std::shared_ptr<Container> row1 = column << wdg<Row>();

        qthConfigure(row1);

        auto keyboard = std::make_shared<QUERTY>();
        column << wdg<Keyboard>(keyboard);
        requestFocus();

    }

    void ConfigDialog::qthConfigure(shared_ptr <Container> &parent) {
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
                       << wdg<TextField>(10, callsign, "", "Call", 6)
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20}
                           << ToUpperCase{} << mCallRegex
                           << Manip::Parent
                       << wdg<TextField>(8, latTxt, "Deg", "Lat", 6)
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20} << mFloatRegex
                           << Manip::Parent
                       << wdg<TextField>(9, lonTxt, "Deg", "Lon", 6)
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20} << mFloatRegex
                           << Manip::Parent;
    }
}
