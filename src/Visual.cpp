/**
 * @file Visual.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include "Visual.h"
#include "GraphicsModel.h"

namespace rose {

    SemanticGesture SemanticGesture::None{0x0};
    SemanticGesture SemanticGesture::Click{0x1};
    SemanticGesture SemanticGesture::Scroll{0x2};
    SemanticGesture SemanticGesture::Multi{0x4};

    void rose::Screen::draw(gm::Context &context, const Position &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->draw(context, mScreenRect.position());
            }
        }
    }

    Rectangle Screen::layout(gm::Context &context, const Rectangle &screenRect) {
        for (auto &content : (*this)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                window->layout(context, screenRect);
            }
        }
        return screenRect;
    }

    std::optional<FocusTree> Window::focusTree(Position mousePosition) {
        for (auto &content : *this) {
            if (auto visual = std::dynamic_pointer_cast<Visual>(content); visual) {
                auto rect = visual->getScreenRectangle(mPos);
                if (rect.contains(mousePosition)) {
                    FocusTree result{};
                    if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                        manager->focusTree(Position::Zero, mousePosition, result);
                    } else {
                        result.second = std::dynamic_pointer_cast<Widget>(content);
                        return result;
                    }
                }
            }
        }
        return std::nullopt;
    }

    void Window::draw(gm::Context &context, const Position &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->draw(context, mScreenRect.position());
            }
        }
    }

    Rectangle Window::layout(gm::Context &context, const Rectangle &screenRect) {
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->layout(context, screenRect);
            }
        }
        return screenRect;
    }

    void Manager::draw(gm::Context &context, const Position &containerPosition) {
        setScreenRectangle(containerPosition);
        for (auto &content : (*this)) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                manager->draw(context, mScreenRect.position());
            } else if (auto widget = std::dynamic_pointer_cast<Widget>(content); widget) {
                widget->draw(context, mScreenRect.position());
            }
        }
    }

    Rectangle Manager::layout(gm::Context &context, const Rectangle &screenRect) {
        Rectangle managerRect{screenRect};
        if (mPreferredPos != Position::Zero) {
            managerRect = mPreferredPos;
        }

        if (mPreferredSize != Size::Zero) {
            managerRect = mPreferredSize;
        }

        auto rect = mLayoutManager->layoutContent(context, managerRect, begin(), end());
        mPos = rect.position();
        mSize = rect.size();
        return rect;
    }

    void Manager::focusTree(const Position &containerPosition, const Position &mousePosition, FocusTree &result) {
        for (auto &content : *this) {
            if (auto visual = std::dynamic_pointer_cast<Visual>(content); visual) {
                auto rect = visual->getScreenRectangle(mPos);
                if (rect.contains(mousePosition)) {
                    if (auto manager = std::dynamic_pointer_cast<Manager>(content); manager) {
                        result.first.push_back(manager);
                        manager->focusTree(rect.position(), mousePosition, result);
                    } else {
                        result.second = std::dynamic_pointer_cast<Widget>(content);
                    }
                }
            }
        }
    }

    Manager::Manager() {
        mLayoutManager = std::make_unique<SimpleLayout>();
    }

    Rectangle
    SimpleLayout::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) {
        while (first != last) {
            if (auto manager = std::dynamic_pointer_cast<Manager>(*first); manager) {
                manager->layout(context, screenRect);
            } else if (auto widget = std::dynamic_pointer_cast<Widget>(*first); widget) {
                widget->layout(context, screenRect);
            }
            first++;
        }
        return screenRect;
    }
}
