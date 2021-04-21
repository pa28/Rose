/**
 * @file Application.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#pragma once

#include <utility>

#include "Visual.h"
#include "GraphicsModel.h"

namespace rose {

    /**
     * @class EventSemantics
     * @brief A class to process event semantics.
     */
    class EventSemantics {
    public:
        /**
         * @enum WindowEventType
         * @brief The type of window event.
         */
        enum WindowEventType : int {
            Shown, Hidden, Exposed, SizeChanged, Minimized, Maximized, FullScreen, Restored, Enter, Leave, Focus, UnFocus,
            Close, Moved, Resized,
        };

        using WindowStateChangeCallback = std::function<void(Application&, WindowEventType)>;
        using WindowPositionChangeCallback = std::function<void(Application&, WindowEventType, Position)>;
        using WindowSizeChangeCallback = std::function<void(Application&, WindowEventType, Size)>;
        using KeyboardEventCallback = std::function<bool(Application&, const SDL_KeyboardEvent&)>;
        using MouseMotionEventCallback = std::function<bool(Application&, const SDL_MouseMotionEvent&)>;
        using MouseButtonEventCallback = std::function<bool(Application&, const SDL_MouseButtonEvent&)>;
        using MouseWheelEventCallback = std::function<bool(Application&, const SDL_MouseWheelEvent&)>;
        using FingerTouchEventCallback = std::function<bool(Application&, const SDL_TouchFingerEvent&)>;

    protected:
        /// The Application the EventSemantics are processed for.
        Application& mApplication;
        WindowSizeChangeCallback windowSizeChangeCallback;
        WindowPositionChangeCallback windowPositionChangeCallback;
        WindowStateChangeCallback windowStateChangeCallback;
        KeyboardEventCallback keyboardEventCallback;
        MouseMotionEventCallback mouseMotionEventCallback;
        MouseButtonEventCallback mouseButtonEventCallback;
        MouseWheelEventCallback mouseWheelEventCallback;
        FingerTouchEventCallback fingerTouchEventCallback;

        void windowStateChange(WindowEventType type) {
            if (windowStateChangeCallback)
                windowStateChangeCallback(mApplication, type);
        }

        void windowSizeChange(WindowEventType type, Size size) {
            if (windowSizeChangeCallback)
                windowSizeChangeCallback(mApplication, type, size);
        }

        void windowPositionChange(WindowEventType type, Position position) {
            if (windowPositionChangeCallback)
                windowPositionChangeCallback(mApplication, type, position);
        }

    public:
        EventSemantics() = delete;

        explicit EventSemantics(Application& application) : mApplication(application) {}

        void onEvent(SDL_Event &e);

        void windowEvent(SDL_WindowEvent &e);

        void keyboardEvent(SDL_KeyboardEvent &e);

        void mouseMotionEvent(SDL_MouseMotionEvent &e);

        void mouseButtonEvent(SDL_MouseButtonEvent &e);

        void mouseWheelEvent(SDL_MouseWheelEvent &e);

        void fingerTouchEvent(SDL_TouchFingerEvent &e);

        void setWindowStateChangeCallback(WindowStateChangeCallback callback) {
            windowStateChangeCallback = std::move(callback);
        }

        void setWindowPositionChangeCallback(WindowPositionChangeCallback callback) {
            windowPositionChangeCallback = std::move(callback);
        }

        void setWindowSizeChangeCallback(WindowSizeChangeCallback callback) {
            windowSizeChangeCallback = std::move(callback);
        }

        void setKeyboardEventCallback(KeyboardEventCallback callback) {
            keyboardEventCallback = std::move(callback);
        }

        void setMouseMotionEventCallback(MouseMotionEventCallback callback) {
            mouseMotionEventCallback = std::move(callback);
        }

        void setMouseButtonEventCallback(MouseButtonEventCallback callback) {
            mouseButtonEventCallback = std::move(callback);
        }

        void setMouseWheelEventCallback(MouseWheelEventCallback callback) {
            mouseWheelEventCallback = std::move(callback);
        }

        void setFingerTouchEventCallback(FingerTouchEventCallback callback) {
            fingerTouchEventCallback = std::move(callback);
        }
    };

    /**
     * @class InputParser
     * @brief Parse command line arguments.
     */
    class InputParser {
    public:
        /**
         * @brief Constructor
         * @param argc The number of command line arguments passed to the application
         * @param argv The array of command line arguments.
         */
        InputParser(int &argc, char **argv) {
            for (int i = 1; i < argc; ++i)
                this->tokens.emplace_back(argv[i]);
        }

        /// @author iain
        [[nodiscard]] const std::string &getCmdOption(const std::string_view &option) const {
            std::vector<std::string>::const_iterator itr;
            itr = std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
                return *itr;
            }
            static const std::string empty_string;
            return empty_string;
        }

        /// @author iain
        [[nodiscard]] bool cmdOptionExists(const std::string_view &option) const {
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }

    private:
        std::vector<std::string> tokens;
    };

    /**
     * @class Application
     * @brief
     */
    class Application {
    protected:

        std::shared_ptr<Screen> mScreen{};
        EventSemantics mEventSemantics;
        gm::GraphicsModel mGraphicsModel{};
        EventSemantics::WindowEventType mAppState{EventSemantics::Restored};
        InputParser mInputParser;

        std::shared_ptr<Widget> mPointerWidget{};
        std::shared_ptr<Widget> mClickFocusWidget{};
        std::shared_ptr<Widget> mDragFocusWidget{};
        std::shared_ptr<Widget> mScrollFocusWidget{};
        std::shared_ptr<Widget> mKeyFocusWidget{};

        std::map<SDL_Keycode, std::pair<uint32_t,std::weak_ptr<Widget>>> mKeyboardShortcuts{};

        bool mMouseButtonPressed{false};
        uint mMouseButtonId{0};
        Position mMousePosition{};

    public:
        Application() = delete;

        Application(int argc, char **argv);

        virtual void initialize(const std::string &title, Size defaultSize);

        virtual void windowStateChange(EventSemantics::WindowEventType type);

        virtual void windowSizeChange(EventSemantics::WindowEventType type, Size size);

        virtual void windowPositionChange(EventSemantics::WindowEventType type, Position position);

        virtual bool keyboardEventCallback(const SDL_KeyboardEvent& keyboardEvent);

        virtual bool mouseMotionEventCallback(const SDL_MouseMotionEvent& mouseMotionEvent);

        virtual bool mouseButtonEventCallback(const SDL_MouseButtonEvent& mouseButtonEvent);

        virtual bool mouseWheelEventCallback(const SDL_MouseWheelEvent& mouseWheelEvent);

        virtual bool fingerTouchEventCallback(const SDL_TouchFingerEvent& fingerTouchEvent);

        gm::Context& context() { return mGraphicsModel.context(); }

        std::shared_ptr<Screen>& screen() { return mScreen; }

        void layout();

        std::shared_ptr<Widget> pointerWidget(Position position);

        [[nodiscard]] Padding windowBorders() const noexcept {
            return mGraphicsModel.windowBorders();
        }

        gm::SdlWindow& getSdlWindow() {
            return mGraphicsModel.getSdlWindow();
        }

        void capturePointerWidget(std::shared_ptr<Widget> widget) {
            if (mPointerWidget)
                mPointerWidget->leaveEvent();
            mPointerWidget = std::move(widget);
        }

        void captureScrollWheelWidget(std::shared_ptr<Widget> widget) {
            if (mPointerWidget)
                mPointerWidget->leaveEvent();
            mPointerWidget = std::move(widget);
        }

        void
        registerKeyboardShortcut(SDL_Keycode keycode, const std::shared_ptr<Widget> &widget, uint32_t shortcutCode) {
            std::cout << __PRETTY_FUNCTION__ << " keycode: " << keycode << '\n';
            mKeyboardShortcuts[keycode] = std::make_pair(shortcutCode, widget);
        }

        void redrawBackground() {
            mGraphicsModel.redrawBackground();
        }

        virtual void run();
    };
}

