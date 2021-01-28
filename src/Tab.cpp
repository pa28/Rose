/**
 * @file Tab.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 2.0
 * @date 2021-01-10
 */

#include "Constants.h"
#include "Tab.h"
#include "Types.h"
#include "Manipulators.h"

namespace rose {

    Rectangle Tab::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto layout = Column::initialLayout(renderer, available);
        return layout;
    }

    void Tab::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        Column::draw(renderer, parentRect);
    }

    void Tab::initializeComposite() {
        mClassName = "Tab";

        auto tab = getWidget<Tab>();
        mHdr = tab->add<TabHeader>();
        mFrame = tab->add<Frame>() << BorderStyle::Notch;
        mBody = mFrame->add<TabBody>();

        rxState = std::make_shared<Slot<ButtonSignalType>>();
        rxState->setCallback([=](uint32_t, ButtonSignalType buttonSignalType) {
            if (buttonSignalType.first) {
                for (auto child = mHdr->begin(); child != mHdr->end(); ++child) {
                    auto tabButton = (*child)->as<Button>();
                    if (tabButton && tabButton->getSelectState())
                        activeTab(child - mHdr->begin());
                }
            }
        });

        rxPushed = std::make_shared<Slot<ButtonSignalType>>();
        rxPushed->setCallback([=](uint32_t, ButtonSignalType buttonSignalType) {
            for (auto &child : *mHdr) {
                auto tabButton = child->as<Button>();
                if (tabButton)
                    tabButton->setSelectState(ButtonSetState::ButtonOff);
            }
        });
    }

    void Tab::addChild(const shared_ptr<Widget> &widget) {
        auto isFirst = mBody->empty();
        if (auto tabPage = widget->as<TabPage>(); tabPage) {
            addTabButton(tabPage->tabLabel());
            mBody->addChild(widget);
            widget->setVisible(isFirst);
        } else
            throw RoseLogicError("Only objects derived from rose::TabPage can be added to a Tab Widget.");
    }

    void Tab::addTabButton(const string &label) {
        auto active = mHdr->size() == mActiveTab;
        auto button = mHdr << wdg<Button>(label, ButtonType::TabButton, 20)
                           << CornerStyle::Round << BorderStyle::TabTop;
        button->setSelectState(active ? ButtonSetState::ButtonOn : ButtonSetState::ButtonOff);

        button->txState.connect(rxState);
        button->txPushed.connect(rxPushed);
    }

    void Tab::activeTab(size_t tabIdx) {
        if (tabIdx < mBody->size()) {
            for (auto &page : *mBody) {
                page->setVisible(false);
            }
            auto page = mBody->begin() + tabIdx;
            (*page)->setVisible(true);
            setNeedsDrawing();
        }
    }

    void TabBody::initializeComposite() {
        Container::initializeComposite();
    }

    void TabBody::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        auto containerAvailable = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);
        for (auto &child : mChildren) {
            if (child->mVisible) {
                child->draw(renderer, containerAvailable);
            }
        }
    }

    Rectangle TabBody::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto frame = parent<Frame>();
        auto frameWidth = frame->frameWidth();
        auto frameAvailable = clampAvailableArea(available, mPos, mSize);
        frameAvailable.width() -= frameWidth * 2;
        frameAvailable.height() -= frameWidth * 2;

        int maxWidth = 0;
        int maxHeight = 0;

        Rectangle layout{};
        for (auto &child : mChildren) {
            LayoutHints& childHints{child->layoutHints()};
            layout = child->initialLayout(renderer, frameAvailable);
//            if (!layout.getPosition())
//                layout = Position::Zero;
            childHints.mAssignedRect = layout;

            childHints.mAssignedRect->x() = frameWidth;
            childHints.mAssignedRect->y() = frameWidth;
            layout.width() += frameWidth * 2;
            layout.height() += frameWidth * 2;

            maxWidth = std::max(maxWidth, layout.width());
            maxHeight = std::max(maxHeight, layout.height());
        }

        layout = Position::Zero;

        for (auto &child : mChildren) {
            child->mLayoutHints.mAssignedRect = layout;
        }

        return layout;
    }

    bool TabBody::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->mouseButtonEvent(mousePos, button, down, modifiers);
        return false;
    }

    bool
    TabBody::mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->mouseMotionEvent(cursorPosition, rel, button, modifiers);
        return false;
    }

    bool TabBody::mouseDragEvent(const Position &p, const Position &rel, int button, int modifiers) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->mouseDragEvent(p, rel, button, modifiers);
        return false;
    }

    bool TabBody::mouseEnterEvent(const Position &p, bool enter) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->mouseEnterEvent(p, enter);
        return false;
    }

    bool TabBody::scrollEvent(const Position &p, double x, double y) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->scrollEvent(p, x, y);
        return false;
    }

    bool TabBody::focusEvent(bool focused) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return Widget::focusEvent(focused);
        return false;
    }

    bool TabBody::keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->keyboardEvent(key, scancode, action, modifiers);
        return false;
    }

    bool TabBody::keyboardCharacterEvent(unsigned int codepoint) {
        if (auto tabPage = activeChild<Widget>(); tabPage)
            return tabPage->keyboardCharacterEvent(codepoint);
        return false;
    }

    std::shared_ptr<Widget> TabBody::findWidget(const Position &pos) {
        if (auto tabPage = activeChild<Container>(); tabPage)
            return tabPage->findWidget(pos);
        return nullptr;
    }

    std::shared_ptr<Widget> TabBody::findWidget(const Id &id) {
        if (auto tabPage = activeChild<Container>(); tabPage)
            return tabPage->findWidget(id);
        return nullptr;
    }
}
