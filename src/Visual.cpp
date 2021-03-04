/**
 * @file Visual.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include "Visual.h"

namespace rose {

    Position Position::Zero{};
    Size Size::Zero{};
    Rectangle Rectangle::Zero{};


    void rose::Screen::draw(const Position &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->draw(mScreenRect.position());
            }
        }
    }

    Rectangle Screen::layout(const Rectangle &screenRect) {
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->layout(screenRect);
            }
        }
        return screenRect;
    }

    void Window::draw(const Position &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->draw(mScreenRect.position());
            }
        }
    }

    Rectangle Window::layout(const Rectangle &screenRect) {
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->layout(screenRect);
            }
        }
        return screenRect;
    }

    void Manager::draw(const Position &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->draw(mScreenRect.position());
            } else if (auto widget = std::dynamic_pointer_cast<Widget>(content); widget) {
                widget->draw(mScreenRect.position());
            }
        }
    }

    Rectangle Manager::layout(const Rectangle &screenRect) {
        Rectangle managerRect{screenRect};
        if (mPreferredPos != Position::Zero) {
            managerRect = mPreferredPos;
        }

        if (mPreferredSize != Size::Zero) {
            managerRect = mPreferredSize;
        }

        auto rect = mLayoutManager->layoutContent(managerRect, begin(), end());
        mPos = rect.position();
        mSize = rect.size();
        return rect;
    }

    Rectangle
    SimpleLayout::layoutContent(const Rectangle &screenRect, LayoutManager::Itr first, LayoutManager::Itr last) {
        while (first != last) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(*first); manager) {
                manager->layout(screenRect);
            } else if (auto widget = std::dynamic_pointer_cast<Widget>(*first); widget) {
                widget->layout(screenRect);
            }
        }
        return screenRect;
    }
}
