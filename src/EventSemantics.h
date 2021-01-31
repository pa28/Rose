/**
 * @file EventSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#pragma once

#include <SDL.h>
#include <memory>
#include <optional>
#include <vector>
#include "Widget.h"

namespace rose {

    class Rose;

/**
 * @class EventSemantics
 * @brief Encapsulation of event semantics
 */
    class EventSemantics {
    protected:
        bool mClickTransaction{};
        uint mButtonState{};

        Rose &mRose;
        std::optional<SDL_Event> mEventQue{};
        std::vector<std::weak_ptr<Widget>> mFocusTrail{};       ///< The current focus trail.
        std::shared_ptr<Widget> mTextFocus{};                   ///< Widget that has text focus.
        std::shared_ptr<Widget> mDragFocus{};                   ///< Widget that has drag focus.
        std::shared_ptr<Widget> mScrollFocus{};                 ///< Widget that has scroll wheel focus.

        Position mMousePosition{};                              ///< The last noted position of the mouse.

        /**
         * @brief Find the Widget at a Position and set it as the focus widget.
         * @param focusPos The focus Position.
         */
        std::shared_ptr<Widget> identifyFocusWidget(Position focusPos);

        /**
         * @brief Set a Widget as the focus Widget and build the focus trail.
         * @param widget The new Focus Widget.
         */
        void setFocusWidget(const std::shared_ptr<Widget>& widget);

        /**
         * @brief Identify the Drag focus Widget.
         * @details If a Drag focus Widget is already identified it will be used. If not, and a focus trail has
         * been identified it will be searched for a Widget that supports drag events. If all fails, the focus
         * Position will be used to build a new focus trail and search it for a Widget that supports drag.
         * @param focusPos The focus Position.
         * @return A std::shared_ptr<Widget> to drag focus Widget if found, otherwise empty.
         */
        std::shared_ptr<Widget> identifyDragFocusWidget(Position focusPos);

        /**
         * @brief Identify the Text focus Widget.
         * @details If a Text focus Widget is already identified it will be used. If not, and a focus trail has
         * been identified it will be search for a Widget that supports drag events. If all fails the events will
         * be sent to the stray event recipient which is usually the font window.
         * @return A std::shared_ptr<Widget> to text focus Widget if found, otherwise the stray event Widget.
         */
        std::shared_ptr<Widget> identifyTextFocusWidget();

        /**
         * @brief Identify the scroll focus Widget.
         * @param focusPos The position of the event to determining where focus should go.
         * @return A std::shared_ptr<Widget> to the scroll focus Widget or empty if none.
         */
        std::shared_ptr<Widget> identifyScrollFocusWidget(Position focusPos);

        /**
         * @brief Convert floating point finger coordinates to a position.
         * @param x the X coordinate
         * @param y the Y coordinate
         * @return A Position.
         */
        Position convertFingerCoordinates(float x, float y);

    public:
        EventSemantics() = delete;
        ~EventSemantics() = default;
        EventSemantics(EventSemantics &&) = delete;
        EventSemantics(const EventSemantics &) = delete;
        EventSemantics& operator=(EventSemantics &&) = delete;
        EventSemantics& operator=(const EventSemantics &) = delete;

        explicit EventSemantics(Rose &rose);

        void onEvent(SDL_Event &event);

        void processEvent(SDL_Event &event);

        void flushFifo();

        /**
         * @brief Mouse Wheel event
         * @param timestamp timestamp of the event
         * @param windowId the window with mouse focus, if any
         * @param which the mouse instance id, or SDL_TOUCH_MOUSEID; see Remarks for details
         * @param x the amount scrolled horizontally, positive to the right and negative to the left
         * @param y the amount scrolled vertically, positive away from the user and negative towards the user
         * @param direction SDL_MOUSEWHEEL_NORMAL or SDL_MOUSEWHEEL_FLIPPED
         * @return true if event processed.
         */
        void mouseWheel(uint32_t timestamp, uint32_t windowId, uint32_t which, int32_t x, int32_t y, uint32_t direction);

        /**
         * @brief Mouse motion event.
         * @param event the full event.
         * @param state the state of the buttons.
         * @param x X coordinate, relative to window
         * @param y Y coordinate, relative to window
         * @param relX relative motion in the X direction
         * @param relY relative motion in the Y direction
         */
        void mouseMotion(SDL_Event &event, uint32_t state, int32_t x, int32_t y, int32_t relX, int32_t relY);

        /**
         * @brief Mouse button event
         * @param event The full event.
         * @param button The button identifier.
         * @param state The state of the button (pressed or released).
         * @param clicks 1 for single-click, 2 for double-click, etc.
         * @param x X coordinate, relative to window
         * @param y Y coordinate, relative to window
         */
        void mouseButton(SDL_Event &event, uint button, uint state, uint clicks, int32_t x, int32_t y);

        /**
         * @brief Finger motion event.
         * @param event The full event.
         * @param touchId the touch device id
         * @param fingerId the finger id
         * @param x the x-axis location of the touch event, normalized [0...1]
         * @param y the y-axis location of the touch event, normalized [0...1]
         * @param dx the distance moved in the x-axis, normalized [-1...1]
         * @param dy the distance moved in the y-axis, normalized [-1...1]
         * @param pressure the quantity of pressure applied, normalized [0...1]
         */
        void fingerMotion(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                          float dx, float dy, float pressure);

        /**
         * @brief Finger Down event.
         * @param event The full event.
         * @param touchId the touch device id
         * @param fingerId the finger id
         * @param x the x-axis location of the touch event, normalized [0...1]
         * @param y the y-axis location of the touch event, normalized [0...1]
         * @param dx the distance moved in the x-axis, normalized [-1...1]
         * @param dy the distance moved in the y-axis, normalized [-1...1]
         * @param pressure the quantity of pressure applied, normalized [0...1]
         */
        void fingerDown(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                        float dx, float dy, float pressure);

        /**
         * @brief Finger up event.
         * @param event The full event.
         * @param touchId the touch device id
         * @param fingerId the finger id
         * @param x the x-axis location of the touch event, normalized [0...1]
         * @param y the y-axis location of the touch event, normalized [0...1]
         * @param dx the distance moved in the x-axis, normalized [-1...1]
         * @param dy the distance moved in the y-axis, normalized [-1...1]
         * @param pressure the quantity of pressure applied, normalized [0...1]
         */
        void fingerUp(SDL_Event &event, SDL_TouchID touchId, SDL_TouchID fingerId, float x, float y,
                      float dx, float dy, float pressure);

        /**
         * @brief Multi-Gesture event
         * @param event The full event.
         * @param dTheta the amount that the fingers rotated during this motion
         * @param dDist the amount that the fingers pinched during this motion
         * @param x the normalized center of gesture
         * @param y the normalized center of gesture
         * @param nFingers the number of fingers used in the gesture
         */
        void multiGesture(SDL_Event &event, float dTheta, float dDist, float x, float y, uint16_t nFingers);

        /**
         * @brief Key events
         * @param event The full event.
         * @param state The state of the key; SDL_PRESSED or SDL_RELEASED.
         * @param repeat Non-zero if this is a key repeat.
         * @param keysym The SDL_Keysym representing the key that was pressed or released.
         */
        void keyEvent(SDL_Event &event, uint state, uint repeat, SDL_Keysym keysym);

        /**
         * @brief Text input events
         * @param event The full event.
         * @param text The input text in UTF-8 encoding.
         */
        void textInputEvent(SDL_Event &event, const std::string &text);
    };
}
