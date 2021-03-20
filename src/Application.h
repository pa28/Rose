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
     * @brief
     */
    class EventSemantics {
    public:
        enum WindowEventType : int {
            Shown, Hidden, Exposed, SizeChanged, Minimized, Maximized, Restored, Enter, Leave, Focus, UnFocus,
            Close, Moved, Resized,
        };

        using WindowStateChangeCallback = std::function<void(Application&, WindowEventType)>;
        using WindowPositionChangeCallback = std::function<void(Application&, WindowEventType, Position)>;
        using WindowSizeChangeCallback = std::function<void(Application&, WindowEventType, Size)>;
        using KeyboardEventCallback = std::function<bool(Application&, const SDL_KeyboardEvent&)>;
        using MouseMotionEventCallback = std::function<bool(Application&, const SDL_MouseMotionEvent&)>;
        using MouseButtonEventCallback = std::function<bool(Application&, const SDL_MouseButtonEvent&)>;
        using MouseWheelEventCallback = std::function<bool(Application&, const SDL_MouseWheelEvent&)>;

    protected:
        Application& mApplication;
        WindowSizeChangeCallback windowSizeChangeCallback;
        WindowPositionChangeCallback windowPositionChangeCallback;
        WindowStateChangeCallback windowStateChangeCallback;
        KeyboardEventCallback keyboardEventCallback;
        MouseMotionEventCallback mouseMotionEventCallback;
        MouseButtonEventCallback mouseButtonEventCallback;
        MouseWheelEventCallback mouseWheelEventCallback;

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

        ~EventSemantics() = default;

        explicit EventSemantics(Application& application) : mApplication(application) {}

        void onEvent(SDL_Event &e);

        void windowEvent(SDL_WindowEvent &e);

        void keyboardEvent(SDL_KeyboardEvent &e);

        void mouseMotionEvent(SDL_MouseMotionEvent &e);

        void mouseButtonEvent(SDL_MouseButtonEvent &e);

        void mouseWheelEvent(SDL_MouseWheelEvent &e);

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

        bool mMouseButtonPressed{false};
        uint mMouseButtonId{0};
        Position mMousePosition{};

    public:
        Application() = delete;

        ~Application() = default;

        Application(int argc, char **argv);

        virtual void initialize(const std::string &title, Size defaultSize);

        virtual void windowStateChange(EventSemantics::WindowEventType type);

        virtual void windowSizeChange(EventSemantics::WindowEventType type, Size size);

        virtual void windowPositionChange(EventSemantics::WindowEventType type, Position position);

        virtual bool keyboardEventCallback(const SDL_KeyboardEvent& keyboardEvent);

        virtual bool mouseMotionEventCallback(const SDL_MouseMotionEvent& mouseMotionEvent);

        virtual bool mouseButtonEventCallback(const SDL_MouseButtonEvent& mouseButtonEvent);

        virtual bool mouseWheelEventCallback(const SDL_MouseWheelEvent& mouseWheelEvent);

        gm::Context& context() { return mGraphicsModel.context(); }

        std::shared_ptr<Screen>& screen() { return mScreen; }

        void layout();

        std::shared_ptr<Widget> focusWidget(SemanticGesture gesture, const Position &position);

        std::shared_ptr<Widget> pointerWidget(Position position);

        [[nodiscard]] Padding windowBorders() const noexcept {
            return mGraphicsModel.windowBorders();
        }

        gm::SdlWindow& getSdlWindow() {
            return mGraphicsModel.getSdlWindow();
        }

        /**
         * @brief Set the collection of focus widgets based on what is supported by a found widget.
         * @param widget The Widget which contains the pointer.
         * @param checkSupports The foci supported by the check.
         */
        void setFocusWidget(const std::shared_ptr<Widget>& widget, SemanticGesture checkSupports) {
            auto changeFocusWidget = [&widget, &checkSupports](std::shared_ptr<Widget>& focus, const SemanticGesture& gesture) {
                if (widget->supportedSemanticGestures().supports(gesture) && checkSupports.supports(gesture)) {
                    if (focus)
                        focus->clearFocus(gesture);
                    focus = widget;
                }
            };

            if (widget) {
                changeFocusWidget(mClickFocusWidget, SemanticGesture::Click);
//                if (widget->supportedSemanticGestures().supports(SemanticGesture::Click) &&
//                    checkSupports.supports(SemanticGesture::Click)) {
//                    if (mClickFocusWidget)
//                        mClickFocusWidget->clearFocus(SemanticGesture::Click);
//                    mClickFocusWidget = widget;
//                }

                changeFocusWidget(mDragFocusWidget, SemanticGesture::Drag);
//                if (widget->supportedSemanticGestures().supports(SemanticGesture::Drag) &&
//                    checkSupports.supports(SemanticGesture::Drag)) {
//                    if (mDragFocusWidget)
//                        mDragFocusWidget->clearFocus(SemanticGesture::Drag);
//                    mDragFocusWidget = widget;
//                }

                changeFocusWidget(mScrollFocusWidget, SemanticGesture::Scroll);
//                if (widget->supportedSemanticGestures().supports(SemanticGesture::Scroll) &&
//                    checkSupports.supports(SemanticGesture::Scroll)) {
//                    if (mScrollFocusWidget)
//                        mScrollFocusWidget->clearFocus(SemanticGesture::Scroll);
//                    mScrollFocusWidget = widget;
//                }

                changeFocusWidget(mKeyFocusWidget, SemanticGesture::Key);
//                if (widget->supportedSemanticGestures().supports(SemanticGesture::Key) &&
//                    checkSupports.supports(SemanticGesture::Key)) {
//                    if (mKeyFocusWidget)
//                        mScrollFocusWidget->clearFocus(SemanticGesture::Key);
//                    mKeyFocusWidget = widget;
//                }
            }
        }

        /**
         * @brief Clear the collection of focus widgets base on what is supported by a check that didn't find a Widget.
         * @param widget The widget that was returned by the check, should be nullptr.
         * @param checkSupports The foci supported by the check.
         */
        void clearFocusWidget(const std::shared_ptr<Widget>& widget,  SemanticGesture checkSupports) {
            if (widget && !widget->supportedSemanticGestures().supports(checkSupports)) {
                return;
            } else {
                if (checkSupports.supports(SemanticGesture::Click)) {
                    mClickFocusWidget->clearFocus(SemanticGesture::Click);
                    mClickFocusWidget.reset();
                }
                if (checkSupports.supports(SemanticGesture::Drag)) {
                    mDragFocusWidget->clearFocus(SemanticGesture::Drag);
                    mDragFocusWidget.reset();
                }
                if (checkSupports.supports(SemanticGesture::Scroll)) {
                    mScrollFocusWidget->clearFocus(SemanticGesture::Scroll);
                    mScrollFocusWidget.reset();
                }
                if (checkSupports.supports(SemanticGesture::Key)) {
                    mKeyFocusWidget->clearFocus(SemanticGesture::Key);
                    mKeyFocusWidget.reset();
                }
            }
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

        virtual void run();
    };
}

