/** @file PopupWindow.cpp
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-31
  */

#include "Border.h"
#include "Manipulators.h"
#include "PopupWindow.h"

namespace rose {

    Popup::Popup(const shared_ptr <Rose>& parent) : Window(parent) {
        mClassName = "Popup";
        mSupportsDrag = false;
        mModal = Ephemeral;
        mMinimumSize = Size::Zero;
    }

    Popup::Popup(const shared_ptr <Rose>& parent, const Position &position) : Window(parent, position, Size{}){
        mClassName = "Popup";
        mSupportsDrag = false;
        mModal = Ephemeral;
        mMinimumSize = Size::Zero;
    }

    Popup::Popup(const shared_ptr <Rose>& parent, const Position &pos, const Size &minimumSize) : Window(parent, pos, Size{}) {
        mClassName = "Popup";
        mSupportsDrag = false;
        mModal = Ephemeral;
        mMinimumSize = minimumSize;
    }

    void Popup::initializeComposite() {
        auto sRose = rose();
        mFrame = Container::add<Frame>();

        mFrame->setBorder(BorderStyle::None);

        mColumn = mFrame << wdg<Column>();
        mColumn->setMinimumWidth(mMinimumSize.width());

        mDismissButtonRx = std::make_shared<Slot<Button::SignalType>>();
        mDismissButtonRx->setCallback([=](uint32_t, Button::SignalType signalType){
            sRose->needsDrawing(true);
            sRose->removeWindow(getWidget<Window>());
        });

        sRose->needsLayout();
    }

    Rectangle Popup::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto windowRect = clampAvailableArea(available, mPos, mSize);
        auto layout = mFrame->widgetLayout(renderer, windowRect, 0);
        mFrame->layoutHints().mAssignedRect = layout;
        layout = windowRect.getPosition();
        return layout;
    }

    void Popup::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

        for (auto &child : mChildren) {
            child->draw(renderer, widgetRect);
        }
    }

    template<class C>
    void Popup::setActionButtons(C actionButtonList) {
        std::for_each(actionButtonList.begin(), actionButtonList.end(),
                      [this](const DialogActionButton &dialogActionButton){
            createActionButton(dialogActionButton);
        });
    }

    PopupWindow::PopupWindow(shared_ptr <Rose> parent) : Popup(parent) {
        mClassName = "PopupWindow";
        mSupportsDrag = true;
        mModal = Modal;
    }

    PopupWindow::PopupWindow(shared_ptr <Rose> parent, const Position &position) : Popup(parent, position) {
        mClassName = "PopupWindow";
        mSupportsDrag = true;
        mModal = Modal;
    }

    PopupWindow::PopupWindow(shared_ptr <Rose> parent, const Position &pos, const Size &size) : Popup(parent, pos, size) {
        mClassName = "PopupWindow";
        mSupportsDrag = true;
        mModal = Modal;
    }

    void PopupWindow::initializeComposite() {
        auto sRose = rose();
        Popup::initializeComposite();

        mFrame->setBorder(BorderStyle::Bevel);

        mTitle = mColumn << wdg<Label>(mWindowTitle)
                << FontSize{sRose->theme().mTitleFontSize}
                << LabelHorizontalAlignment::Center;
    }

    bool PopupWindow::mouseDragEvent(const Position &mousePos, const Position &rel, int button, int modifiers) {
        if (mChildren.empty())
            return false;

        auto sRose = rose();

        mChildren.front()->mouseEnterEvent(mousePos - mLayoutHints.mAssignedRect->getPosition(), false);
        auto rect = mLayoutHints.mAssignedRect;
        int clampX = sRose->width() - rect->width();
        int clampY = sRose->height() - rect->height();
        mLayoutHints.mAssignedRect->x() = std::clamp<int>(rect->x() + rel.x(), 0, clampX);
        mLayoutHints.mAssignedRect->y() = std::clamp<int>(rect->y() + rel.y(), 0, clampY);
        setNeedsDrawing();
        return true;
    }

    Rectangle PopupWindow::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto layout = Popup::widgetLayout(renderer, available, 0);
        layout = Position{(available.width()-layout.width())/2, (available.height()-layout.height())/2};
        return layout;
    }

    Dialog::Dialog(shared_ptr <Rose> parent) : PopupWindow(parent) {

    }

    Dialog::Dialog(shared_ptr <Rose> parent, const Position &position) : PopupWindow(parent, position) {

    }

    Dialog::Dialog(shared_ptr <Rose> parent, const Position &pos, const Size &size) : PopupWindow(parent, pos, size) {

    }

    void Dialog::initializeComposite() {
        PopupWindow::initializeComposite();
        mModal = Modality::Modal;

        mFrame << BorderStyle::BevelIn;
        mFrame->setInvert(true);
        auto frame = mColumn << wdg<Frame>() << BorderStyle::BevelIn;
        auto border = frame << wdg<Border>(4);
        auto column = border << wdg<Column>() << InternalSpace{4};
        mMessageRow = column << wdg<Row>();
        mButtonRow = column << wdg<Row>();
        mButtonRow << InternalSpace{10};
        requestFocus();
    }

    void Dialog::createActionButton(const DialogActionButton &dialogActionButton) {
        ImageId badge;
        WidgetText label;

        switch (dialogActionButton.mActionButton) {
            case ActionButtonOk:
                badge = RoseImageId::IconCheck;
                label = ActionButtonLabel[0];
                break;
            case ActionButtonCancel:
                badge = RoseImageId::IconCancel;
                label = ActionButtonLabel[1];
                break;
        }

        auto button = mButtonRow << wdg<Button>() << label << badge << dialogActionButton.mSignalToken;
//        if (mActionButtonRx)
//            button->txPushed.connect(mActionButtonRx);
//
//        if (mDismissButtonRx)
//            button->txPushed.connect(mDismissButtonRx);
    }

    void Dialog::setButtonSlot(shared_ptr<Slot<Button::SignalType>> &buttonSlot) {
        mActionButtonRx = buttonSlot;
        std::for_each(mButtonRow->begin(), mButtonRow->end(),
                      [this](auto widget){
            if (auto button = widget->template as<Button>(); button) {
                button->txPushed.disconnectAll();
                button->txPushed.connect(mActionButtonRx);
                button->txPushed.connect(mDismissButtonRx);
            }
        });
    }

    ExitDialog::ExitDialog(shared_ptr <Rose> parent) : Dialog(parent) {
        mWindowTitle = mExitTitle;
    }

    ExitDialog::ExitDialog(shared_ptr <Rose> parent, const Position &position) : Dialog(parent, position) {
        mWindowTitle = mExitTitle;
    }

    ExitDialog::ExitDialog(shared_ptr <Rose> parent, const Position &pos, const Size &size) : Dialog(parent, pos, size) {
        mWindowTitle = mExitTitle;
    }

    void ExitDialog::initializeComposite() {
        Dialog::initializeComposite();
        mMessageRow << wdg<Label>(mExitMessage);
        setActionButtons(mActionButtons);
    }
}
