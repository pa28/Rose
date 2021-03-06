/**
 * @file EventSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-06
 */

#pragma once

#include <functional>
#include <utility>
#include "Configuration.h"
#include "Visual.h"

#if GRAPHICS_MODEL_SDL2
#include <SDL.h>
#endif

namespace rose {

/**
 * @class EventSemantics
 * @brief
 */
    class EventSemantics {
    public:
        enum WindowEventType {
            Shown, Hidden, Exposed, SizeChanged, Minimized, Maximized, Restored, Enter, Leave, Focus, UnFocus,
            Close, Moved, Resized,
        };

        using WindowStateChangeCallback = std::function<void(WindowEventType)>;
        using WindowPositionChangeCallback = std::function<void(WindowEventType,Position)>;
        using WindowSizeChangeCallback = std::function<void(WindowEventType,Size)>;

    protected:
        WindowSizeChangeCallback windowSizeChangeCallback;
        WindowPositionChangeCallback windowPositionChangeCallback;
        WindowStateChangeCallback windowStateChangeCallback;

        void windowStateChange(WindowEventType type) {
            if (windowStateChangeCallback)
                windowStateChangeCallback(type);
        }

        void windowSizeChange(WindowEventType type, Size size) {
            if (windowSizeChangeCallback)
                windowSizeChangeCallback(type,size);
        }

        void windowPositionChange(WindowEventType type, Position position) {
            if (windowPositionChangeCallback)
                windowPositionChangeCallback(type,position);
        }

    public:
        EventSemantics() = default;

        ~EventSemantics() = default;

#if GRAPHICS_MODEL_SDL2
        void onEvent(SDL_Event &e);

        void windowEvent(SDL_WindowEvent &e);

        void keyboardEvent(SDL_KeyboardEvent &e);

        void mouseMotionEvent(SDL_MouseMotionEvent &e);

        void mouseButtonEvent(SDL_MouseButtonEvent &e);

        void mouseWheelEvent(SDL_MouseWheelEvent &e);
#endif

        void setWindowStateChangeCallback(WindowStateChangeCallback callback) {
            windowStateChangeCallback = std::move(callback);
        }

        void setWindowPositionChangeCallback(WindowPositionChangeCallback callback) {
            windowPositionChangeCallback = std::move(callback);
        }

        void setWindowSizeChangeCallback(WindowSizeChangeCallback callback) {
            windowSizeChangeCallback = std::move(callback);
        }
    };
}

