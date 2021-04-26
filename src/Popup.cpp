/**
 * @file Popup.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-24
 */

#include "Popup.h"
#include "Frame.h"
#include "TestWidget.h"
#include "Manager.h"
#include "Application.h"
#include "Visual.h"
#include "Keyboard.h"

namespace rose {

    void PopupWindow::addedToContainer() {
        std::cout << __PRETTY_FUNCTION__ << getIdPath() << '\n';
        getNode<PopupWindow>()
                << wdg<Frame>() << Theme::getTheme().BevelFrame << CornerStyle::Square
                    << wdg<Column>() << Padding(2)
                        << wdg<TextLabel>(Id{"PopupWindow"}) << PointSize(20) << endw
                        << wdg<Frame>() << Padding(0)
                            << wdg<Frame>(true)
                                    << Theme::getTheme().BevelFrame
                                    << CornerStyle::Square
                                    << Padding(2)
                                    >> mContentFrame
                                << endw
                            << endw
                        << wdg<TextButton>("Close", [&](ButtonStateChange buttonStateChange){
                            if (buttonStateChange == ButtonStateChange::Pushed) {
                                mRemovePopup = true;
                                getScreen()->getApplication().redrawBackground();
                            }
                        }) << Theme::getTheme().SemiBevelFrame;

        mContentFrame << wdg<Keyboard>();

    }
}
