/**
 * @file GraphicsModel.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 * @brief Abstraction of the graphics model.
 */

#pragma once

#include "Configuration.h"

#include <SDL.h>

#include <memory>
#include <functional>
#include <Utilities.h>
#include "Visual.h"
#include "Color.h"
#include "Texture.h"
#include "CommonSignals.h"


namespace rose {
    class Application;
}

namespace rose::gm {

    /**
     * @brief A functor to destroy an SDL_Window in a std::unique_ptr (rose::sdl::Window).
     */
    class SdlWindowDestroy {
    public:
        /**
         * @brief Call the SDL API to destroy an SDL_Window.
         * @param sdlWindow A pointer to the SDL_Window to destroy.
         */
        void operator()(SDL_Window *sdlWindow) {
            SDL_DestroyWindow(sdlWindow);
        }
    };

    using SdlWindow = std::unique_ptr<SDL_Window, SdlWindowDestroy>;   //!< An SDL_Window unique pointer

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
     * @struct RenderFlip
     * @brief A Widget manipulator to indicate if and how rendering a texture should be flipped.
     * @details The structure supports flipping on the Horizontal, Vertical or both axis.
     */
    struct RenderFlip {
        SDL_RendererFlip mFlip;

        /// Default constructor -- No flipping
        constexpr RenderFlip() noexcept: mFlip(SDL_FLIP_NONE) {}

        /// Constructor -- user specified flipping.
        constexpr explicit RenderFlip(SDL_RendererFlip flip) noexcept: mFlip(flip) {}
    };

    /**
     * @classs Context
     * @brief An abstraction of graphics rendering context.
     */
    class Context {
        friend class RenderTargetGuard;

    protected:
        /**
         * @brief A functor to destroy an SDL_Renderer
         */
        class RendererDestroy {
        public:
            /**
             * @brief Call the SDL API to destroy the renderer.
             * @param sdlRenderer The SDL_Renderer
             */
            void operator()(SDL_Renderer *sdlRenderer) {
                SDL_DestroyRenderer(sdlRenderer);
            }
        };

        using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDestroy>; ///< An SDL_Renderer unique pointer
        RendererPtr mRenderer{};    ///< The Renderer.

        SDL_Texture *mCurrentRenderTarget{nullptr};

    public:

        Context() = default;

        Context(const Context &context) = delete;

        Context(Context &&context) noexcept = default;

        Context &operator=(const Context &context) = delete;

        Context &operator=(Context &&context) = default;

        explicit Context(SdlWindow &window, int index, Uint32 flags) : Context()
        { mRenderer.reset(SDL_CreateRenderer(window.get(), index, flags)); }

        /// Test for a valid Context
        explicit operator bool() const noexcept { return mRenderer.operator bool(); }

        /// Get an opaque pointer for API calls.
        [[nodiscard]] auto get() const { return mRenderer.get(); }

        /// Set the draw blend mode.
        void setDrawBlendMode(SDL_BlendMode blendMode) {
            SDL_SetRenderDrawBlendMode(mRenderer.get(), blendMode);
        }

        /**
         * @brief Copy source Texture to destination Texture and set the BlendMode on the destination Texture.
         * @details The function uses RenderTargetGuard to temporarily set the render Target to the destination,
         * calls SDL_RenderCopy to copy the texture, and sets the BlendMode on the destination texture to
         * SDL_BLENDMODE_BLEND.
         * @param source
         * @param destination
         */
        void copyFullTexture(Texture &source, Texture &destination);

        /// Prepare for the start of a rendering iteration.
        int renderClear() { return SDL_RenderClear(mRenderer.get()); }

        /// Complete a rendering iteration.
        void renderPresent() { SDL_RenderPresent(mRenderer.get()); }

        /**
         * @brief Copy a Texture to the current render target using the size of the Texture and the size of the
         * target.
         * @param texture The texture to copy.
         * @return The return status of the API call.
         */
        int renderCopy(const Texture &texture);

        /**
         * @brief Copy a Texture to the current render target using the size of the Texture and specified
         * destination rectangle.
         * @param texture The source Texture.
         * @param dst The destination Rectangle.
         * @return The return status of the API call.
         */
        int renderCopy(const Texture &texture, Rectangle dst);

        /**
         * @brief Copy part of the Texture specified by the source Rectangle to the render target location
         * specified by the destination Rectangle.
         * @param texture The Texture.
         * @param src The source Rectangle.
         * @param dst The destination Rectangle.
         * @return The return status of API call.
         */
        int renderCopy(const Texture &texture, Rectangle src, Rectangle dst);

        /**
         * @brief Render with extensions.
         * @param src Source Rectangle.
         * @param dst Destination Rectangle.
         * @param angle Rotation Angle.
         * @param renderFlip Flip
         * @param point Rotation Position if specified.
         * @return Status code returned by SDL_RenderCopyEx()
         */
        int renderCopyEx(Texture &texture, Rectangle src, Rectangle dst, double angle, RenderFlip renderFlip,
                         std::optional<Position<int>> point = std::nullopt) const;

        /**
         * @brief Set the drawing color used for drawing Rectangles, lines and clearing.
         * @param color The new drawing Color.
         * @return Status code returned by the API.
         */
        int setDrawColor(color::RGBA color);

        /**
         * @brief Set the drawing color used for drawing Rectangles, lines and clearing.
         * @param color The new drawing Color.
         * @return Status code returned by the API.
         */
        int setDrawColor(color::HSVA color) {
            return setDrawColor(color.toRGBA());
        }

        /**
         * @brief Render a filled Rectangle.
         * @param rect The Rectangle
         * @param color The fill color.
         * @return The status return from the SDL API.
         */
        int fillRect(Rectangle rect, color::RGBA color);
    };

    /**
     * @class RenderTargetGuardException
     * @brief Thrown by RenderTargetGuard on errors.
     */
    class RenderTargetGuardException : public std::runtime_error {
    public:
        /**
         * @brief Create a RenderTargetGuardException.
         * @param what_arg An indication of what went wrong.
         */
        explicit RenderTargetGuardException(const std::string &what_arg) : std::runtime_error(what_arg) {}

        explicit RenderTargetGuardException(const char *what_arg) : std::runtime_error(what_arg) {}
    };

    /**
     * @class DrawColorGuardException
     * @brief Thrown by DrawColorGuard on errors.
     */
    class DrawColorGuardException : public std::runtime_error {
    public:
        explicit DrawColorGuardException(const std::string &what_arg) : std::runtime_error(what_arg) {}

        explicit DrawColorGuardException(const char *what_arg) : std::runtime_error(what_arg) {}
    };

    /**
     * @class RenderTargetGuard
     * @brief Store the current render target replacing it with a new render target. When the object is
     * destroyed (by going out of scope) the old render target is restored.
     */
    class RenderTargetGuard {
    protected:
        Context &mContext;                      ///< The Context being guarded
        bool popped{false};                     ///< True if the target guard has already been popped off the stack.
        SDL_Texture *mLastTexture{nullptr};     ///< Save the current render target here.
        int status{0};                          ///< The return status from the last SDL API call.

    public:
        RenderTargetGuard() = delete;   ///< Deleted default constructor.
        RenderTargetGuard(const RenderTargetGuard &) = delete;   ///< Deleted copy constructor.

        /**
         * @brief Set the old render target back on the renderer when destroyed.
         */
        ~RenderTargetGuard() noexcept(false);

        /**
         * @brief Test the status of the RenderTargetGuard.
         * @details The status is good (true is returned) if the status value returned from the last
         * operation on the Context object returned 0.
         * @return True if the last operation succeeded.
         */
        explicit operator bool() const noexcept { return status == 0; }

        /**
         * @brief Constructor
         * @param renderer The renderer which render target will be managed.
         * @param texture The texture which will become the new render target.
         */
        RenderTargetGuard(Context &context, Texture &texture);

        /**
         * @brief Set a new render target without pushing it on the stack.
         * @details This may be used when a number of render target changes are needed in a context block.
         * A RenderTargetGuard is created, calls to setRenderTarget are used to manipulate the render target.
         * When the RenderTargetGuard goes out of scope the original render target is restored.
         * @param texture
         * @return
         */
        int setRenderTarget(Texture &texture);
    };

    /**
     * @class DrawColorGuard
     * @brief Store the current draw color replacing it with a new draw color. When the object is
     * destroyed (by going out of scope) the old draw color is set.
     */
    class DrawColorGuard {
    protected:
        Context &mContext;      ///< The renderer to which the draw colors are set.
        SDL_Color mOldColor{};  ///< The old draw color
        int mStatus;            ///< The status of the last SDL operation.

    public:
        DrawColorGuard() = delete;  ///< Deleted default constructor.
        DrawColorGuard(const DrawColorGuard &) = delete; ///< Deleted copy constructor.
        DrawColorGuard(DrawColorGuard &&) = default;

        DrawColorGuard &operator=(const DrawColorGuard &) = delete;

        /**
         * @brief Set the old clip rectangle back on the renderer when destroyed.
         */
        ~DrawColorGuard() noexcept(false) {
            if (SDL_SetRenderDrawColor(mContext.get(), mOldColor.r, mOldColor.g, mOldColor.b, mOldColor.a)) {
                throw DrawColorGuardException(StringCompositor("Call to SDL_XxxRenderDrawColor failed:",
                                                                     SDL_GetError()));
            }
        }

        /**
         * @brief Construct DrawColorGuard with and SDL_Color.
         * @param renderer The renderer to set the color on.
         * @param color The SDL_Color.
         */
        DrawColorGuard(Context &context, SDL_Color color);

        /**
         * @brief Construct DrawColorGuard with a rose::Color.
         * @param renderer The renderer to set the color on.
         * @param color The rose::Color.
         */
        DrawColorGuard(Context &context, color::RGBA color);

        /**
         * @brief Test the validity of the DrawColorGuard.
         * @return False if the last SDL operation return an error.
         */
        explicit operator bool() const noexcept { return mStatus == 0; }

        /**
         * @brief Set the draw Color on the renderer without pushing the old color on the stack.
         * @param color The SDL_Color to set.
         * @return The SDL2 API return status code.
         */
        int setDrawColor(SDL_Color color);

        /**
         * @brief Set the draw Color on the renderer without pushing the old color on the stack.
         * @param color The Color to set.
         * @return The SDL2 API return status code.
         */
        int setDrawColor(color::RGBA color) {
            return setDrawColor(color.toSdlColor());
        }
    };

    /**
     * @class ClipRectangleGuard
     * @brief Store the current clip rectangle replacing it with a new clip rectangle. When the object is
     * destroyed (by going out of scope) the old clip rectangle is set.
     */
    class ClipRectangleGuard {
    protected:
        Context &mContext;    ///< The renderer to which the clip rectangles are set.
        SDL_Rect mOldClip{};    ///< The old clip rectangle
        int mStatus{0};

    public:
        ClipRectangleGuard() = delete;  ///< Deleted default constructor
        ClipRectangleGuard(const ClipRectangleGuard &) = delete;     ///< Deleted copy constructor

        explicit operator bool () const { return mStatus == 0; }

        /**
         * @brief Set the old clip rectangle back on the renderer when destroyed.
         */
        ~ClipRectangleGuard() {
            if (mOldClip.w == 0 && mOldClip.y == 0)
                mStatus = SDL_RenderSetClipRect(mContext.get(), nullptr);
            else
                mStatus = SDL_RenderSetClipRect(mContext.get(), &mOldClip);
        }

        /**
         * @brief Speculative constructor.
         * @details This constructor saves the current clip rectangle to be restored later, but does not
         * set a new clip rectangle. A new clip rectangle may be set using the assignment operators.
         * @param context The renderer to guard the clip rectangle of.
         */
        explicit ClipRectangleGuard(Context &context) : mContext(context) {
            SDL_RenderGetClipRect(mContext.get(), &mOldClip);
        }

        /**
         * @brief Constructor. Store the current clip rectangle and set the new one.
         * @param context The renderer to set the clip rectangles on.
         * @param clip The new clip rectangle.
         */
        ClipRectangleGuard(Context &context, const SDL_Rect &clip) : mContext(context) {
            SDL_RenderGetClipRect(mContext.get(), &mOldClip);
            mStatus = SDL_RenderSetClipRect(mContext.get(), &clip);
        }

        /**
         * @brief Constructor. Store the current clip rectangle and set the new one.
         * @param renderer The renderer to set the clip rectangles on.
         * @param x X co-ordinate of the new clip rectangle.
         * @param y Y co-ordinate of the new clip rectangle.
         * @param w Width of the new clip rectangle.
         * @param h Height of the new clip rectangle.
         */
        ClipRectangleGuard(Context &context, int x, int y, int w, int h)
                : ClipRectangleGuard(context, SDL_Rect{x, y, w, h}) {}

        /**
         * @brief Conditional constructor.
         * @details This constructor saves the current clip rectangle to be restored later. If clip
         * is valid it is converted to an SDL_Rect and set as the new clip rectangle
         * @param renderer The renderer to set the clip rectangles on.
         * @param clip A, possibly invalid, RectangleInt.
         */
        ClipRectangleGuard(Context &context, const Rectangle &clip) : mContext(context) {
            SDL_RenderGetClipRect(mContext.get(), &mOldClip);
            SDL_Rect rect{clip.x, clip.y, clip.w, clip.h};
            mStatus = SDL_RenderSetClipRect(mContext.get(), &rect);
        }

        /**
         * @brief Assign a new clip rectangle through the ClipRectangleGuard.
         * @param clip The new clip rectangle.
         * @return The ClipRectangleGuard.
         */
        ClipRectangleGuard &operator=(SDL_Rect &clip) {
            mStatus = SDL_RenderSetClipRect(mContext.get(), &clip);
            return *this;
        }

        /**
         * @brief Assign a new clip rectangle through the ClipRectangleGuard.
         * @details If clip is invalid no changes are made.
         * @param clip The new clip rectangle.
         * @return The ClipRectangleGuard.
         */
        ClipRectangleGuard &operator=(Rectangle &clip) {
            SDL_Rect rect{clip.x, clip.y, clip.w, clip.h};
            mStatus = SDL_RenderSetClipRect(mContext.get(), &rect);
            return *this;
        }

        ClipRectangleGuard &intersection(Rectangle &clip) {
            SDL_Rect current;
            SDL_RenderGetClipRect(mContext.get(), &current);
            if (SDL_RectEmpty(&current)) {
                operator=(clip);
            } else {
                mOldClip = current;
                Rectangle r{current.x, current.y, current.w, current.h};
                r = r.intersection(clip);
                current = SDL_Rect{r.x, r.y, r.w, r.h};
                mStatus = SDL_RenderSetClipRect(mContext.get(), &current);
            }
            return *this;
        }
    };

    class GraphicsModel {
    protected:
        SdlWindow mSdlWindow{};         ///< The SDL_Window which provides the application "Screen"

        Context mContext{};             ///< The graphics context used by the application graphics model.

        bool mRunEventLoop{true};       ///< Event loop continues while this is true.

        bool mRedrawBackground{true};   ///< When true the background Texture needs to be redrawn.

        Texture mBackground{};          ///< The background Texture.

        uint32_t mFrame{};              ///< The rendering frame.

        std::vector<Rectangle> mDisplayBounds{};

    public:
        RoseErrorCode ErrorCode{RoseErrorCode::OK};

        bool initialize(const std::string &title, Size initialSize, const Position<int>& initialPosition, uint32_t extraFlags);

        void eventLoop(std::shared_ptr<Screen> &screen);

        /**
         * @brief Draw the screen.
         * @details Screen drawing is accomplished in two steps. If/when the background needs to be redrawn
         * (mRedrawBackground is true) the background widgets (the majority of them) are rendered to the
         * mBackground Texture which is then rendered to the frame buffer.<p/>
         * If mRedrawBackground is false, but mAnimation is true mBackground is rendered to the screen, then
         * animated Widgets, those that change at sub-second rates, are rendered to the screen on top of the
         * background.<p/>
         * If neither mRedrawBackground nor mAnimation are true no rendering of the screen is required.
         * @param screen The Screen object to draw.
         */
        void drawAll(std::shared_ptr<Screen> &screen);

        std::function<void(SDL_Event)> eventCallback{};

        Context& context() { return mContext; }

        [[nodiscard]] int currentDisplayIndex() const { return SDL_GetWindowDisplayIndex(mSdlWindow.get()); }

        Rectangle displayBounds(int displayIndex = -1) {
            if (displayIndex < 0)
                displayIndex = currentDisplayIndex();

            if (displayIndex < 0 || displayIndex >= mDisplayBounds.size())
                displayIndex = 0;

            return mDisplayBounds.at(displayIndex);
        }

        Rectangle screenRectangle() {
            Rectangle screenRectangle{};
            SDL_GetWindowSize(mSdlWindow.get(), &screenRectangle.w, &screenRectangle.h);
            return screenRectangle;
        }

        void redrawBackground() { mRedrawBackground = true; }

        [[nodiscard]] Padding windowBorders() const noexcept {
            Padding p{};
            SDL_GetWindowBordersSize(mSdlWindow.get(), &p.t, &p.l, &p.b, &p.r);
            return p;
        }

        SdlWindow& getSdlWindow() {
            return mSdlWindow;
        }
    };

    /**
     * @brief Map a color::RGBA to a uint32_t.
     * @param format A pointer to an SDL_PixelFormat (from a Surface object for example).
     * @param color The color::RGBA to map.
     * @return The color::RGBA mapped to a uint32_t
     */
    uint32_t mapRGBA(SDL_PixelFormat *format, const color::RGBA &color);

    color::RGBA getRGBA(SDL_PixelFormat *format, uint32_t pixel);
}

