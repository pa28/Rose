/**
 * @file EventSemantics.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#include "EventSemantics.h"

#include <iostream>
#include "Math.h"
#include "Rose.h"

namespace rose {

    template<typename Arg, typename ... Args>
    std::ostream &print(std::ostream &strm, Arg &&arg, Args&& ... args) {
        strm << std::forward<Arg>(arg) << ':';
        ((strm << ' ' << std::forward<Args>(args)), ...);
        return strm;
    }

    EventSemantics::EventSemantics(Rose &rose) : mRose(rose) {}

    void EventSemantics::onEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                if (mEventQue)
                    processEvent(mEventQue.value());
                mEventQue.reset();
                processEvent(event);
                break;
            case SDL_MOUSEMOTION:
                mMousePosition.x() = event.motion.x;
                mMousePosition.y() = event.motion.y;
                if (event.motion.state) {
                    // Drag event
                    if (mEventQue) {
                        if (mEventQue->type == SDL_MOUSEBUTTONDOWN || mEventQue->type == SDL_MOUSEBUTTONUP)
                            mEventQue.reset();
                        else
                            processEvent(mEventQue.value());
                    }
                    processEvent(event);
                } else {
                    if (mEventQue) {
                        processEvent(mEventQue.value());
                        mEventQue.reset();
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_FINGERUP:
            case SDL_FINGERDOWN:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_FINGERMOTION:
                if (mEventQue) {
                    if (mEventQue->type != SDL_FINGERDOWN && mEventQue->type != SDL_FINGERUP)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_MULTIGESTURE:
                if (mEventQue) {
                    if (mEventQue->type != SDL_FINGERDOWN && mEventQue->type != SDL_FINGERUP && mEventQue->type != SDL_FINGERMOTION)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            case SDL_TEXTINPUT:
                if (mEventQue) {
                    if (mEventQue->type != SDL_KEYDOWN && mEventQue->type != SDL_KEYUP)
                        processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                mEventQue = event;
                break;
            default:
                if (mEventQue) {
                    processEvent(mEventQue.value());
                    mEventQue.reset();
                }
                processEvent(event);
        }
    }

    void EventSemantics::flushFifo() {
        if (mEventQue) {
            processEvent(mEventQue.value());
            mEventQue.reset();
        }
    }

    void EventSemantics::processEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_TEXTEDITING:
                std::cout << "TEXTEDITING " << event.edit.text << ' ' << event.edit.start << ' ' << event.edit.length << '\n';
                break;
            case SDL_MOUSEWHEEL:
                mouseWheel(event.wheel.timestamp, event.wheel.windowID, event.wheel.which,
                                  event.wheel.x, event.wheel.y, event.wheel.direction == SDL_MOUSEWHEEL_NORMAL);
                break;
            case SDL_MOUSEMOTION:
                mouseMotion(event, event.motion.state, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseButton(event, event.button.button, event.button.state, event.button.clicks, event.button.x, event.button.y);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                keyEvent(event, event.key.state, event.key.repeat, event.key.keysym);
                break;
            case SDL_TEXTINPUT:
                textInputEvent(event, std::string{event.text.text});
                break;
            case SDL_FINGERMOTION:
                fingerMotion(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
                break;
            case SDL_FINGERDOWN:
                fingerDown(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
                break;
            case SDL_FINGERUP:
                fingerUp(event, event.tfinger.touchId, event.tfinger.fingerId, event.tfinger.x, event.tfinger.y,
                                    event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
                break;
            case SDL_MULTIGESTURE:
                multiGesture(event, event.mgesture.dTheta, event.mgesture.dDist, event.mgesture.x, event.mgesture.y,
                                    event.mgesture.numFingers);
                break;
        }
    }

    void EventSemantics::mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y,
                                    uint32_t direction) {
//        print(std::cout, __FUNCTION__, timestamp, windowId, which, x, y, direction, mMousePosition, '\n');
        auto widget = identifyScrollFocusWidget(mMousePosition);
        if (direction == SDL_MOUSEWHEEL_FLIPPED) {
            x *= -1;
            y *= -1;
        }

        if (widget)
            widget->scrollEvent(mMousePosition, x, y);
    }

    void
    EventSemantics::mouseMotion(SDL_Event &event, uint32_t state, int32_t x, int32_t y, int32_t relX, int32_t relY) {
//        print(std::cout, __FUNCTION__, (state ? "Drag" : "Move"), state, x, y, relX, relY, '-', mClickTransaction, mTransactionPos,'\n');
        Position position{x,y};
        Position positionRel{relX, relY};
        auto modifiers = SDL_GetModState();
        if (state) {
            // Gesture is now a drag, cancel any click transactin in progress.
            if (mClickTransaction) {
                if ((position - mTransactionPos).abs() > 25) {
                    if (!mFocusTrail.empty()) {
                        auto weakPtr = mFocusTrail.front();
                        if (auto widget = mFocusTrail.front().lock(); widget && mClickTransaction) {
                            widget->clickTransactionCancel(position, state, false, modifiers);
                            mClickTransaction = false;
                        }
                    }
                } else {
                    return;
                }
            }

            auto widget = identifyDragFocusWidget(position);
            if (widget) {
                widget->mouseDragEvent(position, positionRel, state, modifiers);
            }
        }
    }

    void
    EventSemantics::mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y) {
//        print( std::cout, __FUNCTION__, button, state, clicks, x, y, '-', mClickTransaction, mTransactionPos, '\n');
        Position position{x,y};
        if (state == SDL_PRESSED) {
            mClickTransaction = true;
            mTransactionPos = position;
            mButtonState |= button;
            auto widget = identifyFocusWidget(position);
            if (widget)
                widget->mouseButtonEvent(position, button, true, SDL_GetModState());
        } else if (state == SDL_RELEASED) {
            mButtonState &= ~button;
            if (mClickTransaction && !mFocusTrail.empty()) {
                auto weakPtr = mFocusTrail.front();
                if (auto widget = mFocusTrail.front().lock(); widget && mClickTransaction) {
                    widget->mouseButtonEvent(position, button, false, SDL_GetModState());
                    mClickTransaction = false;
                    clearFocusWidget();
                }
            }
        }
    }

    void
    EventSemantics::fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                               float dy, float pressure) {
//        print(std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
        auto position = convertFingerCoordinates(x, y);
//        print(std::cout, __FUNCTION__, position, '\n');
        mClickTransaction = true;
        mTransactionPos = position;
        mButtonState = 1;
        auto widget = identifyFocusWidget(position);
        if (widget)
            widget->mouseButtonEvent(position, 1, true, SDL_GetModState());
    }

    void
    EventSemantics::fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y, float dx,
                             float dy, float pressure) {
//        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
        auto position = convertFingerCoordinates(x, y);
//        print(std::cout, __FUNCTION__, position, '\n');
        mClickTransaction = false;
        mButtonState = 0;
        auto widget = identifyFocusWidget(position);
        if (widget)
            widget->mouseButtonEvent(position, 1, false, SDL_GetModState());
    }

    void EventSemantics::fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                                      float dx, float dy, float pressure) {
//        print( std::cout, __FUNCTION__, event.tfinger.timestamp, touchId, fingerId, x, y, dx, dy, pressure, '\n');
        auto position = convertFingerCoordinates(x, y);
        auto positionRel = convertFingerCoordinates(dx, dy);
//        print(std::cout, __FUNCTION__, position, positionRel, 'n');
        auto modifiers = SDL_GetModState();
        // Gesture is now a drag, cancel any click transactin in progress.
        if (mClickTransaction) {
            if ((position - mTransactionPos).abs() > 25) {
                if (!mFocusTrail.empty()) {
                    auto weakPtr = mFocusTrail.front();
                    if (auto widget = mFocusTrail.front().lock(); widget && mClickTransaction) {
                        widget->clickTransactionCancel(position, 1, false, modifiers);
                        mClickTransaction = false;
                    }
                }
            } else {
                return;
            }
        }

        auto widget = identifyDragFocusWidget(position);
        if (widget) {
            widget->mouseDragEvent(position, positionRel, 1, modifiers);
        }
    }

    void
    EventSemantics::multiGesture(SDL_Event &event, float dTheta, float dDist, float x, float y, uint16_t nFingers) {
        print( std::cout, __FUNCTION__, event.tfinger.timestamp, dTheta, dDist, x, y, nFingers, '\n');
    }

    void EventSemantics::keyEvent(SDL_Event &event, uint state, uint repeat, SDL_Keysym keysym) {
        auto keyName = SDL_GetScancodeName(keysym.scancode);
        auto keyCode = SDL_GetKeyFromScancode(keysym.scancode);
        if (mTextFocus && state) {
            if (keyCode <= SDLK_DELETE) {
                mTextFocus->textInputEvent(std::string{static_cast<char>(static_cast<uint>(keyCode) & 0xFFu)});
            } else {
                if (auto used = mTextFocus->keyboardEvent(state, repeat, keysym); used)
                    return;
            }
        }
//        print(std::cout, __FUNCTION__, event.key.timestamp, state, repeat, keyName, '\n');
    }

    void EventSemantics::textInputEvent(SDL_Event &event, const string &text) {
//        print(std::cout, __FUNCTION__, text, '\n');
        if (mTextFocus)
            mTextFocus->textInputEvent(text);
    }

    std::shared_ptr<Widget> EventSemantics::identifyFocusWidget(Position focusPos) {
        auto [widgetType,focusWidget] = mRose.findWidget(focusPos);
        switch (widgetType) {
            case FoundWidgetType::RegularWidget:
//                std::cout << __PRETTY_FUNCTION__  << "RegularWidget\n";
                if (focusWidget) {
                    if (!mFocusTrail.empty()) {
                        if (!mFocusTrail.front().expired()) {
                            auto currentFocus = mFocusTrail.front().lock();
                            if (currentFocus == focusWidget)
                                return currentFocus;
                        }
                    }

                    auto keyboardFocus = focusWidget;

                    while (keyboardFocus && !keyboardFocus->supportsKeyboard())
                        keyboardFocus = keyboardFocus->parent();
                    if (keyboardFocus) {
                        if (mTextFocus && mTextFocus != keyboardFocus)
                            mTextFocus->keyboardFocusEvent(false);
                        mTextFocus = keyboardFocus;
                        if (mTextFocus)
                            mTextFocus->keyboardFocusEvent(true);
                    }

                    while (focusWidget && !focusWidget->acceptsFocus())
                        focusWidget = focusWidget->parent();
                    setFocusWidget(focusWidget);
                }
                break;
            case FoundWidgetType::PopupWindow:
//                std::cout << __PRETTY_FUNCTION__  << "PopupWindow\n";
                if (auto window = focusWidget->as<Window>(); window) {
                    mRose.removeWindow(window);
                    return identifyFocusWidget(focusPos);
                } else {
                    std::cout << "PopupWindow is not a Window\n";
                }
                break;
            case FoundWidgetType::ModalWindow:
//                std::cout << __PRETTY_FUNCTION__  << "ModalWindow\n";
                return nullptr;
                break;
        }
        return focusWidget;
    }

    void EventSemantics::clearFocusWidget() {
        mFocusTrail.clear();

        mDragFocus.reset();
        mScrollFocus.reset();
    }

    void EventSemantics::setFocusWidget(const std::shared_ptr<Widget>& widget) {
        if (widget) {
            clearFocusWidget();

            mFocusTrail.push_back(widget->weak_from_this());
            auto parent = widget->parent();
            while (parent) {
                mFocusTrail.push_back(parent->weak_from_this());
                parent = parent->parent();
            }
        }
    }

    std::shared_ptr<Widget> EventSemantics::identifyDragFocusWidget(Position focusPos) {
        if (mDragFocus) {
            return mDragFocus;
        }

        static auto findDragFocus = [&]() -> std::shared_ptr<Widget> {
            if (!mFocusTrail.empty()) {
                for (const auto& weakPtr : mFocusTrail) {
                    if (!weakPtr.expired()) {
                        auto widget = weakPtr.lock();
                        if (widget->supportsDrag()) {
                            mDragFocus = widget;
                            return mDragFocus;
                        }
                    }
                }
            }
            return nullptr;
        };

        auto widget = findDragFocus();
        if (!widget) {
            identifyFocusWidget(focusPos);
            widget = findDragFocus();
        }

        return widget;
    }

    std::shared_ptr<Widget> EventSemantics::identifyTextFocusWidget() {
        if (mTextFocus)
            return mTextFocus;

        if (!mFocusTrail.empty()) {
            for (const auto& weakPtr : mFocusTrail) {
                if (!weakPtr.expired()) {
                    auto widget = weakPtr.lock();
                    if (widget->supportsDrag()) {
                        mTextFocus = widget;
                        return mTextFocus;
                    }
                }
            }
        }

        return nullptr;
    }

    std::shared_ptr<Widget> EventSemantics::identifyScrollFocusWidget(Position focusPos) {
//        if (mScrollFocus && mScrollFocus->contains(focusPos))
//            return mScrollFocus;

        auto [widgetType,widget] = mRose.findWidget(focusPos);
        while (widget && !widget->supportsScrollWheel()) {
            widget = widget->parent();
        }

        mScrollFocus = widget;
        return widget;
    }

    Position EventSemantics::convertFingerCoordinates(float x, float y) {
        Position position{};
        position.x() = util::roundToInt((float)mRose.width() * x);
        position.y() = util::roundToInt((float)mRose.height() * y);
        return position;
    }
}
