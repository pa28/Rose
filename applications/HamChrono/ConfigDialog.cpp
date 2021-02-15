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
            std::cout << __PRETTY_FUNCTION__ << '\n';
            switch (button.second) {
                case DialogClose:
                    if (std::find_if(mTextFields.begin(), mTextFields.end(), [](const std::shared_ptr<TextField>& it){
                        return it->isModified();
                    }) != mTextFields.end()) {
                        std::cout << "Modified values\n";
                        rose()->createPopup<UnsavedDialog>() << mActionButtonSlot;
                        rose()->needsLayout();
                    } else {
                        rose()->needsDrawing(true);
                        rose()->removeWindow(getWidget<Window>());
                    }
                    break;
                case DialogOk:
                    rose()->needsDrawing(true);
                    rose()->removeWindow(getWidget<Window>());
                    break;
                case DialogCancel:
                    break;
            }
        });

        auto column = mMessageRow << wdg<Column>();
        auto row1 = column << wdg<Row>();

        qthConfigure(row1);

        auto keyboard = std::make_shared<QUERTY>();
        column << wdg<Keyboard>(keyboard);
        requestFocus();
        setActionButtons(mActionButtons);
        setButtonSlot(mActionButtonSlot, false);
    }

    void ConfigDialog::qthConfigure(shared_ptr <Row> &parent) {
        auto callsign = rose()->settings()->getValue("CALLSIGN",std::string{});
        auto qth = rose()->settings()->getValue<GeoPosition>("QTH_Location");

        std::string latTxt, lonTxt;
        if (qth) {
            latTxt = util::fmtNumber(qth->lat(), 6);
            lonTxt = util::fmtNumber(qth->lon(), 7);
        }

        parent << wdg<Frame>(6) << BorderStyle::Notch << CornerStyle::Round
               << wdg<Column>()
               << wdg<Label>("QTH") << Manip::Parent
               << wdg<TextField>(Id{set::CALLSIGN}, 6) >> mTextFields[0]
               << BorderStyle::Notch << CornerStyle::Round << FontSize{20}
               << ToUpperCase{} << mCallRegex
               << Manip::Parent
               << wdg<TextField>(Id{set::QTH_Loc_Lat}, 6) >> mTextFields[1]
               << BorderStyle::Notch << CornerStyle::Round << FontSize{20} << mFloatRegex
               << Manip::Parent
               << wdg<TextField>(Id{set::QTH_Loc_Lon}, 6) >> mTextFields[2]
                           << BorderStyle::Notch << CornerStyle::Round << FontSize{20} << mFloatRegex
                           << Manip::Parent;
    }
}
