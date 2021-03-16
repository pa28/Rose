/**
 * @file Application.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-09
 */

#pragma once

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

    protected:
        Application& mApplication;
        WindowSizeChangeCallback windowSizeChangeCallback;
        WindowPositionChangeCallback windowPositionChangeCallback;
        WindowStateChangeCallback windowStateChangeCallback;

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

    public:
        Application() = delete;
        ~Application() = default;

        Application(int argc, char **argv);

        void initialize(const std::string &title, Size defaultSize);

        virtual void windowStateChange(EventSemantics::WindowEventType type);

        virtual void windowSizeChange(EventSemantics::WindowEventType type, Size size);

        virtual void windowPositionChange(EventSemantics::WindowEventType type, Position position);

        gm::Context& context() { return mGraphicsModel.context(); }

        std::shared_ptr<Screen>& screen() { return mScreen; }

        void layout();

        [[nodiscard]] Padding windowBorders() const noexcept {
            return mGraphicsModel.windowBorders();
        }

        gm::SdlWindow& getSdlWindow() {
            return mGraphicsModel.getSdlWindow();
        }

        virtual void run();
    };
}

