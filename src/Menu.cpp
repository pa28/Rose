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
            button << item.mSignalToken;
            button->txPushed.connect(mAppButtonRx);
            button->txPushed.connect(mDismissButtonRx);
            button->layoutHints().mElastic = true;
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
        mLayoutHints.mElastic = true;
        if (auto border = mChildren.front()->as<Border>(); border) {
            if (auto label = border->front()->as<Label>(); label) {
                label->setBadgeRight(true);
                if (mCascadeButtonType == CascadeButtonType::CascadeDown)
                    label->setBadge(RoseImageId::IconDown);
                else
                    label->setBadge(RoseImageId::IconRight);
            } else
                throw std::logic_error("First grandchild of CascadeButton is not a Label");
        } else
            throw std::logic_error("First child of CascadeButton is not a Border");

        mCascadeButtonRx = std::make_shared<Slot<Button::SignalType>>();
        mCascadeButtonRx->setCallback([=](uint32_t, Button::SignalType signalType){
            if (!mMenuData.empty()) {
                auto menuPosition = getScreenPosition();
                menuPosition.y() += mLayoutHints.mAssignedRect->height() - mFrameWidth;
                menuPosition.x() -= mFrameWidth;
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
