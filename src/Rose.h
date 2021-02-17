/** @file Rose.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-20
  * @brief The base class for a Rose application.
  */

/**
 * @mainpage
 *
 * @section elements Elements of the UI
 *
 * These are the elements of the user interface.
 *
 * @subpage widget_hierarchy "Widget Hierarchy: Who's Who in the Zoo.
 *
 * @section signaling Methods of Intra-Application Communications
 *
 * Communications between the objects objects of an application.
 *
 * @subpage signals_slots "Signals and Slots: Communicating between objects.
 */

#pragma once

#include <array>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <atomic>
#include <SDL.h>
#include "Constants.h"
#include "Font.h"
#include "ImageRepository.h"
#include "Container.h"
#include "Settings.h"
#include "Surface.h"
#include "Texture.h"
#include "EventSemantics.h"

/**
 * @namespace rose
 * @brief Rose library elements.
 */
namespace rose {

    using namespace util;

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
     * @brief Rose object error codes.
     */
    enum class RoseErrorCode {
        OK,                         ///< No error
        ROSE_EXCEPTION,             ///< Exception thrown and caught in main().
        SDL_WINDOW_CREATE,          ///< Error while creating the SDL_Window
        SDL_RENDERER_CREATE,        ///< Error while creating the SDL_Renderer
        XDG_PATH_FAIL,              ///< Insufficient information to generate standard XDG Base Directories.
    };

    /**
     * @enum FoundWidgetType
     * @brief The type of widget found.
     */
    enum class FoundWidgetType {
        RegularWidget,              ///< A normal Widget
        PopupWindow,                ///< A popup that should be dismissed if a gesture lands outside.
        ModalWindow,                ///< A modal window, gestures outside should be ignored.
    };

    template<class A, typename ... Args>
    std::shared_ptr<A> RoseFactory(Args ... args) {
        return std::make_shared<A>(args...);
    }

    /**
     * @class Rose
     * @brief The base class for application classes using the UI library.
     */
    class Rose : public std::enable_shared_from_this<Rose> {
    protected:

        /**
         * @brief Draw the UI scene.
         * @details If the mNeedsDrawing flag has been set by one of the scene Widgets, then the entire scene
         * is re-drawn. Otherwise nothing is drawn during the frame period. This provides significant savings
         * in system resources if there are no frame by frame changes in the scene.
         */
        void drawAll();

        sdl::Window mSdlWindow;                 ///< The SDL library window SDL_Window wrapped in a std::unique_ptr

        std::vector<std::shared_ptr<Window>> mWindowList{};   ///< The list of Rose::Window objects in the scene.

//        /**
//         * @brief Handle events from the SDL library.
//         * @param e the SDL_Event
//         * @return true if the event was handled by a UI element
//         */
//        bool onEvent(SDL_Event &e);

        /// Handle or propagate scroll callback events
        bool scrollCallbackEvent(double x, double y);

        /// Handle or propagate cursor position callback events
        bool cursorPosCallbackEvent(double x, double y);

//        /// Handle or propagate mouse button callback events
//        bool mouseButtonCallbackEvent(int button, int action, int modifiers);

//        /// Handle or propagate keyboard callback events
//        bool keyCallbackEvent(int key, int scancode, int action, int mods);

//        /// Handle or propagate character callback events
//        bool charCallbackEvent(unsigned int codepoint);

        /// Move a Window to the front of the scene
        void moveToFront(std::shared_ptr<Widget> widget);

        ImageRepository mImageRepository{};         ///< A repository for storing widely used textures

        std::atomic_bool mRunEventLoop{true};       ///< The event loop (application) runs while this is true
        std::atomic_bool mNeedsDrawing{true};       ///< When true the scene is draw during the frame period
        std::atomic_bool mRedoLayout{false};        ///< When true redo the layout before drawing.
        bool mTranslateFingerEvents{false};         ///< When true finger events are translated to mouse events
        bool mProcessEvents{true};                  ///< When true events are processed
        bool mDragActive{false};                    ///< True when the use is dragging an object
        std::weak_ptr<Widget> mDragWidget;          ///< The Widget being dragged.
        uint32_t mMouseState{},                     ///< The mouse state
        mModifiers{};                               ///< Keyboard modifiers active
        RoseErrorCode mErrorCode{RoseErrorCode::OK};    ///< Application exit code.
        std::filesystem::path mHomeDirectory;       ///< The user's home directory path
        std::filesystem::path mDataHome;            ///< The user's XDG Data Home path
        std::filesystem::path mConfigHome;          ///< The user's XDG Config Home path
        std::filesystem::path mCacheHome;           ///< The user's XDG Cache Home path
        int mWidth,                                 ///< The width of the SDL_Window
        mHeight;                                    ///< The height of the SDL_Window
        sdl::Renderer mRenderer;                    ///< The Renderer used for SDL operations
        FontCache mFontCache{};                     ///< A cache of fonts in use by name and point size
        std::vector<std::weak_ptr<Widget>> mFocusPath{};        ///< The path to the widget in focus.
        double mLastInteraction{};                  ///< Time snce the last interaction with the user.
        Position mMousePos;                         ///< The current mouse position in the scene
        Theme mTheme{};                             ///< The Theme object in use.
        FontMetrics mIconFontMetrics{};             ///< Font metrics for the icons in the Image Repository
        InputParser mCmdLineParser;                 ///< The results of command line parsing
        std::unique_ptr<Settings> mSettings{};      ///< The application settings database, set by derived app class
        EventSemantics mEventSemantics;             ///< Mouse semantics processor.

        /**
         * @brief Copy texture to textureData in a way the preserves the transparency encoded into texture and
         * sets the blend mode.
         * @param renderer The renderer to use.
         * @param texture The source texture.
         * @param textureData The destination TextureData.
         */
        static void copyFullTexture(sdl::Renderer &renderer, sdl::Texture &texture, sdl::TextureData &textureData);

        /**
         * @brief Create a square Surface size x size and fill it with transparent black.
         * @param size The number of pixels on a side.
         * @return the created Surface.
         */
        static sdl::Surface createBlankSurface(int size);

        /**
         * @brief Generate the round corner Textures.
         * @details If rounded corners are used in the application this function must be called after the Theme
         * parameters have been determined, or changed, to create the round corner images.
         * @param renderer
         * @param scale The scale used for anti-aliasing the radius curve.
         * @param radius The radius of the generated corners
         * @param borderWidth The width of the border
         * @param top a shadow color for the top.
         * @param bot a shadow color for the bottom.
         * @param left a shadow color for the left.
         * @param right a shadow color for the right.
         */
        void
        createRoundCorners(sdl::Renderer &renderer, int scale, int radius, int borderWidth, rose::color::RGBA top, rose::color::RGBA bot,
                           rose::color::RGBA left, rose::color::RGBA right);

        /**
         * @brief Generate the square corner Textures.
         * @details If square corners are used in the application this function must be called after the Theme
         * parameters have been determined, or changed, to create the square corner images.
         * @param renderer
         * @param radius The radius of the generated corners, should be the same as used for round corners
         * @param borderWidth The width of the border
         * @param top a shadow color for the top.
         * @param bot a shadow color for the bottom.
         * @param left a shadow color for the left.
         * @param right a shadow color for the right.
         */
        void
        createSquareCorners(sdl::Renderer &renderer, int radius, int borderWidth, rose::color::RGBA top, rose::color::RGBA bot,
                            rose::color::RGBA left, rose::color::RGBA right);

        /**
         * @brief Create centers to use with corner sheets as badges or icons.
         * @details If rounded corners are used in the application as badges this function must be called after the Theme
         * parameters have been determined, or changed to create the center images. Scale and radius must match those
         * used in createRoundCorners.
         * @param renderer the renderer
         * @param scale the anti aliasing scale
         * @param radius the radius
         */
        void createCenters(sdl::Renderer &renderer, int scale, int radius);

        /**
         * @brief Common initialization procedures step one.
         * @details This initializes XDG configuration file paths for user data, configuration and settings.
         */
        void initializeStepOne();

        void initializeStepTwo(string_view title);

    public:

        /**
         * @enum IconColor
         * @brief Standards color selections for creating Icon images in batches.
         */
        enum class IconColor : size_t {
            TextColor, BaseColor,
            Black, Red, ThemeRed, Yellow, ThemeYellow, Green, ThemeGreen, White,
        };

        /**
         * @struct IconItem
         * @brief Provide the data required to create icons in batches.
         */
        struct IconItem {
            ImageId imageId;
            int entypoCode;
            IconColor color;
        };

        Rose() = default;
        virtual ~Rose();
        Rose(Rose &&) = delete;
        Rose(const Rose &) = delete;
        Rose& operator=(Rose &&) = delete;
        Rose& operator=(const Rose &) = delete;

        /**
         * @brief Sets the needs drawing flag.
         * @param draw true if the scene graph should be drawn during this frame.
         */
        void needsDrawing(bool draw) { mNeedsDrawing = draw; }

        /**
         * @brief Force a layout of the scene before the next draw.
         */
        void needsLayout() { mRedoLayout = true; }

        /**
         * @brief The Rose object constructor
         * @param screenSize The size of the SDL_Screen in use.
         * @param argc from the arguments to main()
         * @param argv from the arguments to main()
         * @param title The window title if the application is running under a desktop.
         */
        Rose(Size screenSize, int argc, char **argv, std::string_view title = "");

        /**
         * @brief Rose object constructor.
         * @details The screen size is taken from a settings database opened at $HOME/.config/rose_settings.db. If there
         * are no screen size values held in the database the screen is set to 800x480. The command line is then
         * searched for arguments of the form -WIDTHxHEIGHT. If found the screen size is set to the indicated value.
         * what ever value is finally arrived at is written back to the settings database and will be used as
         * the default on next run if no appropriate argument is found.</p>
         * Supported values are 800x480, 1600x960, 2400x1440, and 3200x1920.
         * @param argc from the arguments to main()
         * @param argv from the arguments to main()
         * @param title The window title if the application is running under a desktop.
         */
        Rose(int argc, char **argv, std::string_view title = "");

        /**
         * @brief Access the SDL_Window (screen) width.
         * @return the width
         */
        [[nodiscard]] int width() const { return mWidth; }

        /**
         * @brief Access the SDL_Window (screen) height.
         * @return the height
         */
        [[nodiscard]] int height() const { return mHeight; }

        /**
         * @brief Get the screen size.
         * @return Size with the screen size.
         */
        [[nodiscard]] Size size() const { return Size{mWidth, mHeight}; }

        /**
         * @brief Determine if there has been an error
         * @return ture if the Rose error code is not RoseErrorCode::OK
         */
        explicit operator bool() const { return mErrorCode == RoseErrorCode::OK; }

        /**
         * @brief Access the current Rose error state.
         * @return the current error code.
         */
        [[nodiscard]] RoseErrorCode getErrorCode() const { return mErrorCode; }

        /**
         * @brief Perform the initial layout of the application scene
         * @param renderer The renderer to use if needed.
         */
        void initialLayout(sdl::Renderer &renderer) {
            for (auto &window : mWindowList) {
                Rectangle screenAvailable{Position::Zero, size()};
                auto layout = window->widgetLayout(renderer, screenAvailable, 0);
                window->layoutHints().mAssignedRect = layout;
            }
        }

        /**
         * @brief Run the event loop.
         * @details This function will not return unless mRunEventLoop is set to false or an uncaught exception
         * is thrown. The life time of the event loop usually determines the lifetime of the application.
         */
        virtual void eventLoop();

        /**
         * @brief Access the Font cache
         * @return a reference to the Font cache pointer
         */
        FontCache &fontCache() { return mFontCache; }

        /**
         * @brief Clear focus from the focus Widget
         */
        void clearFocus();

        /**
         * @brief Remove a Window from the scene.
         * @param window The Window to remove.
         */
        void removeWindow(std::shared_ptr<Window> window);

        /**
         * @brief Update the focus to point to widget
         * @param widget the Widget to focus
         */
        void updateFocus(const std::shared_ptr<Widget> &widget);

        /**
         * Access the renderer
         * @return a reference to the renderer pointer
         */
        sdl::Renderer &getRenderer() { return mRenderer; }

        /**
         * @brief Convert a UTF8 character to an array for rendering
         * @param c the character code point
         * @return the equivalent array
         */
        static std::array<char, 8> utf8(int c);

        /**
         * @brief Create a new default Window
         * @return A WindowPtr to the window.
         */
        std::shared_ptr<Window> createWindow() {
            auto window = std::make_shared<Window>(shared_from_this());
            mWindowList.push_back(window);
            return window;
        }

        /**
         * @brief Find a Widget given an Id
         * @param id The Widget Id
         * @return std::shared_ptr<Widget> which is empty if no widget is found.
         */
        std::shared_ptr<Widget> findWidget(const Id id) {
            for (auto &window : mWindowList) {
                if (window->getId() == id)
                    return window;
                if (auto widget = window->findWidget(id); widget)
                    return widget;
            }
            return nullptr;
        }

        /**
         * @brief Create a new Window
         * @details The Window is provided a reference to the Rose object and is placed in the list of windows.
         * @tparam Args The type of arguments in the parameter pack.
         * @param args The arguments in the parameter pack.
         * @return A WindowPtr to the window.
         */
        template<typename ... Args>
        std::shared_ptr<Window> createWindow(Args &&... args) {
            auto window = std::make_shared<Window>(*this, std::forward<Args>(args)...);
            mWindowList.push_back(window);
            return window;
        }

        /**
         * @brief Create a new popup Window
         * @return A WindowPtr to the popup window.
         */
        std::shared_ptr<Window> createPopup();

        /**
         * @brief Create a popup Window
         * @details The Window is provided a reference to the Rose object and is placed in the list of windows.
         * @tparam Args The type of arguments in the parameter pack.
         * @param args The arguments in the parameter pack.
         * @return A WindowPtr to the window.
         */
        template<class PopupClass, typename ... Args>
        std::shared_ptr<PopupClass> createPopup(Args &&... args) {
            auto window = std::make_shared<PopupClass>(shared_from_this(), std::forward<Args>(args)...);
            window->initializeComposite();
            mWindowList.emplace_back(window);
            return window;
        }

        /**
         * @brief Set the drag widget
         * @param widgetPtr
         */
        void setDragWidget(std::shared_ptr<Widget> widgetPtr) { mDragWidget = widgetPtr; }

        /**
         * @brief Find the widget which uniquely contains the position.
         * @param position The position the widget must contain.
         * @return a std::shared_ptr with the widget, or empty.
         */
        std::tuple<FoundWidgetType,std::shared_ptr<Widget>> findWidget(const Position &position);

        /**
         * @brief Create a texture from a UTF8 code point of a font.
         * @details This is usually used with an icon font to generate icons or badges
         * @param renderer The Renderer
         * @param text The code point
         * @param fontName The Font name
         * @param ptsize The Font point size
         * @param textColor The text rendering color.
         * @param textureData The TextureData object to render to.
         */
        void getTexAndRectUtf8(sdl::Renderer &renderer, const char *text, const std::string &fontName, size_t ptsize,
                               color::RGBA textColor, sdl::TextureData &textureData);

        /**
         * @brief Create a minimal texture from an icon character.
         * @details A minimal texture has the extra transparent pixles on the top, bottom, left and right stripped
         * away.
         * @param renderer The Renderer
         * @param text The code point
         * @param fontName The Font name
         * @param ptsize The Font point size
         * @param iconColor The icon rendering color.
         * @return The TextureData object to render to.
         */
        sdl::TextureData getMinimalIcon(sdl::Renderer &renderer, const char *text, std::string &fontName,
                                        size_t ptsize, color::RGBA iconColor);

        /**
         * @brief Generate a surface from a UTF8 code point of a font.
         * @param text the code point
         * @param fontName the font name
         * @param ptSize the size of the font, determines the size of the texture
         * @param textColor the color to render
         * @param surface the surface generated
         */
        sdl::Surface getSurfaceUtf8(const char *text, const std::string &fontName, size_t ptSize, color::RGBA textColor);

        /**
         * @brief Create a texture of an icon
         * @param iconCode the font code point for the icon
         * @param iconSize the point size of the font generating the icon
         * @param iconColor the color of the generated icon
         * @param textureData return value for the generated texture.
         */
        void createIcon(int iconCode, int iconSize, const color::RGBA &iconColor, sdl::TextureData &textureData);

        /**
         * @brief Create textures in the Rose ImageRepository for standard icons.
         * @details If the standard icons are going to be used, the application must call this method after
         * the Theme values have been set.
         */
        void createStandardIcons();

        /**
         * @brief Create a scaled and anti-aliased icon from a UTF8 code point of a font.
         * @details The code point is rendered at iconSize * scale size. Then it is downscaled to a scale x scale
         * matrix of icons that are anti-aliased to be shifted varying amounts of 1/scale of a pixel from each
         * other. By sub sampling the rendering location and choosing one of the images from the matrix motion
         * can be considerable smoothed. See the Sprite class.
         * @param iconCode The icon code point.
         * @param fontName The Font name.
         * @param iconSize The Font point size.
         * @param scale The icon scaling factor.
         * @param iconColor The icon rendering color.
         * @param texture The Texture to render to.
         */
        void
        createScaledIcon(int iconCode, const std::string &fontName, int iconSize, int scale, const color::RGBA &iconColor,
                         sdl::TextureData &texture);

        /**
         * @brief Access the Rose ImageRepository.
         * @return a reference to the Rose ImageRepository.
         */
        ImageRepository &imageRepository() { return mImageRepository; }

        /**
         * @brief Access the Rose ImageRepository.
         * @return a reference to the Rose ImageRepository.
         */
        [[nodiscard]] const ImageRepository &imageRepository() const { return mImageRepository; }

        /**
         * @brief Get a specific TextureData object from the Rose ImageRepository.
         * @param imageId the identifier of the TextureData to get
         * @return a TextureData object pointing to the original copy of the image.
         */
        sdl::TextureData &imageRepository(ImageId imageId) {
            return mImageRepository.image(imageId);
        }

        /**
         * @brief Get a specific TextureData object from the Rose ImageRepository.
         * @param imageId the identifier of the TextureData to get
         * @return a TextureData object pointing to the original copy of the image.
         */
        [[nodiscard]] const sdl::TextureData &
        imageRepository(ImageId imageId) const { return mImageRepository.image(imageId); }

        /**
         * @brief Access the application Theme object
         * @return a reference to the Theme object.
         */
        Theme &theme() { return mTheme; }

        /**
         * @brief Determine if the settings database has been set.
         * @return True if set.
         */
        [[nodiscard]] bool hasSettings() const { return mSettings.operator bool(); }

        /**
         * @brief Access to the settings database.
         * @return A reference to the settings std::unique_ptr.
         */
        auto& settings() { return mSettings; }

        /**
         * @brief Access the application Theme object
         * @return a reference to the Theme object.
         */
        [[nodiscard]] const Theme &theme() const { return mTheme; }

        /**
         * @brief Get Icon Font metrics.
         * @return a reference to the FontMetrics object.
         */
        [[nodiscard]] const FontMetrics &iconFontMetrics() const { return mIconFontMetrics; }
    };
}
