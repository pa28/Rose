//
// Created by richard on 2021-01-03.
//

#include "Border.h"
#include "Menu.h"

#include <utility>

namespace rose {

    PopupMenu::PopupMenu(const shared_ptr <Rose>& rose, Position pos, const MenuDataList &menuDataList,
                         std::shared_ptr<Slot<Button::SignalType>> appButtonRx)
        : Popup(rose, pos), mMenuDataList(menuDataList), mAppButtonRx(std::move(appButtonRx)) {
        mMinimumSize = Size::Zero;
    }

    PopupMenu::PopupMenu(const shared_ptr<Rose>& rose, Position pos, Size minimumSize, const MenuDataList &menuDataList,
                         shared_ptr<Slot<Button::SignalType>> appButtonRx)
         : Popup(rose, pos, minimumSize), mMenuDataList(menuDataList), mAppButtonRx(std::move(appButtonRx)) {
    }

    void PopupMenu::initializeComposite() {
        Popup::initializeComposite();

        mModal = Modality::Ephemeral;
        for (const auto &item : mMenuDataList) {
            auto button = wdg<Button>(std::string{item.mLabelText});
            addChild(button);
            // ToDo: Identify why a signal token is interpreted as a font size in this case
            // button << item.mSignalToken;
            button->setSignalToken(item.mSignalToken);
            button->txPushed.connect(mAppButtonRx);
            button->txPushed.connect(mDismissButtonRx);
            button->layoutHints().mElastic = Elastic{Orientation::Both};
        }
        requestFocus();
    }

    bool PopupMenu::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        for (auto &child : mChildren) {
            if (child->mouseButtonEvent(mousePos, button, down, modifiers))
                return true;
        }
        return false;
    }

    CascadeButton::CascadeButton() : Button(), mCascadeButtonType(CascadeButtonType::CascadeDown) {

    }

    CascadeButton::CascadeButton(const string &labelString) : Button(labelString), mCascadeButtonType(CascadeButtonType::CascadeDown) {

    }

    CascadeButton::CascadeButton(const Id &id) : Button(id), mCascadeButtonType(CascadeButtonType::CascadeDown) {

    }

    CascadeButton::CascadeButton(const string &labelString, CascadeButtonType type, int fontSize)
        : Button(labelString, ButtonType::MenuCascade, fontSize), mCascadeButtonType(CascadeButtonType::CascadeDown) {

    }

    CascadeButton::CascadeButton(const Id &id, CascadeButtonType type, int fontSize)
        : Button(id, ButtonType::MenuCascade, fontSize), mCascadeButtonType(CascadeButtonType::CascadeDown) {

    }

    void CascadeButton::initializeComposite() {
        Button::initializeComposite();
        if (auto label = getSingleChild<Label>(); label) {
            label->setBadgeRight(true);
            if (mCascadeButtonType == CascadeButtonType::CascadeDown)
                label->setBadge(RoseImageId::IconDownDir);
            else
                label->setBadge(RoseImageId::IconRightDir);
        } else
            throw std::logic_error("First child of CascadeButton is not a Label");

        mCascadeButtonRx = std::make_shared<Slot<Button::SignalType>>();
        mCascadeButtonRx->setCallback([=](uint32_t, Button::SignalType signalType){
            if (!mMenuData.empty()) {
                auto menuPosition = getScreenPosition();
                menuPosition.y() += mLayoutHints.mAssignedRect->height() - mLayoutHints.mFrameWidth;
                menuPosition.x() -= mLayoutHints.mFrameWidth;
                auto minimumSize = mLayoutHints.mAssignedRect->getSize();
                minimumSize.height() = 0;
                mPopupMenu = rose()->createPopup<PopupMenu>(menuPosition, minimumSize,
                                                            mMenuData, mAppButtonSlot);
            }
        });
        Button::txPushed.connect(mCascadeButtonRx);
    }

    void CascadeButton::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Frame::draw(renderer, parentRect);
    }
}
