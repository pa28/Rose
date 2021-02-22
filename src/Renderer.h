/**
 * @file Renderer.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#pragma once

#include <memory>
#include <stack>
#include <SDL.h>
#include "Color.h"
#include "ScreenMetrics.h"
#include "Utilities.h"
#include "Types.h"

/**
 * @namespace rose::sdl
 * @brief Class and functions that are used to access the SDL2 api.
 */
namespace rose::sdl {

    class Texture;

    class GradientTexture;

    /**
     * @struct RenderFlip
     * @brief A Widget manipulator to indicate if and how rendering a texture should be flipped.
     * @details The structure supports flipping on the Horizontal, Vertical or both axis.
     */
    struct RenderFlip {
        SDL_RendererFlip mFlip;

        /// Default constructor -- No flipping
        constexpr RenderFlip() noexcept : mFlip(SDL_FLIP_NONE) {}

        /// Constructor -- user specified flipping.
        constexpr explicit RenderFlip(SDL_RendererFlip flip) noexcept : mFlip(flip) {}
    };

    /**
     * @brief A functor to destroy an SDL_Window in a std::unique_ptr (rose::sdl::Window).
     */
    class WindowDestroy {
    public:
        /**
         * @brief Call the SDL API to destroy an SDL_Window.
         * @param sdlWindow A pointer to the SDL_Window to destroy.
         */
        void operator()(SDL_Window *sdlWindow) {
            SDL_DestroyWindow(sdlWindow);
        }
    };

    using Window = std::unique_ptr<SDL_Window, WindowDestroy>;   //!< An SDL_Window unique pointer

    /**
     * @class Renderer
     * @brief Written as a workaround for an issue in the SDL2 Library.
     * @details https://stackoverflow.com/questions/50415099/sdl-setrendertarget-doesnt-set-the-tartget
     */
    class Renderer {
    protected:
        friend class RenderTargetGuard;

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

        std::stack<SDL_Texture *> mTargetTextureStack{};     ///< The stack of render targets

        /**
         * @brief Pop a render target off the stack. If there are none, set the default render target.
         * @details The top of the stack is the current render target, unless the default is current in which
         * case the stack will be empty. If the stack has 1 or 0 render targets on it, the renderer is set
         * to the default target.
         * @return The return status of SDL_SetRenderTarget
         */
        [[nodiscard]] int popRenderTarget() {
            if (mTargetTextureStack.empty())
                return SDL_SetRenderTarget(mRenderer.get(), nullptr);
            else {
                mTargetTextureStack.pop();
                if (mTargetTextureStack.empty())
                    return SDL_SetRenderTarget(mRenderer.get(), nullptr);
                else
                    return SDL_SetRenderTarget(mRenderer.get(), mTargetTextureStack.top());
            }
        }

        /**
         * @brief Set a new render target, and push it onto the stack.
         * @param texture The new render target.
         * @return The return status of SDL_SetRenderTarget
         */
        [[nodiscard]] int pushRenderTarget(sdl::Texture &texture);

        /**
         * @brief Set the render target to the default, and push it onto the stack.
         * @return The return status of SDL_SetRenderTarget
         */
        [[nodiscard]] int pushRenderTarget() {
            mTargetTextureStack.push(nullptr);
            return SDL_SetRenderTarget(mRenderer.get(), nullptr);
        }

    public:
        /**
         * Construct an empty renderer.
         */
        Renderer() = default;

        Renderer(const Renderer &renderer) = delete; ///< Deleted copy constructor.

        /**
         * Construct a renderer associated with a Window.
         * @param window The associated Window.
         * @param index The index argument to SDL_CreateRenderer.
         * @param flags The flags argument to SDL_CreateRenderer.
         */
        Renderer(Window &window, int index, Uint32 flags);

        /**
         * @brief Move constructor defaulted.
         * @param renderer The Renderer to move.
         */
        Renderer(Renderer &&renderer) noexcept = default;

        Renderer &operator=(const Renderer &renderer) = delete; ///< Deleted copy assignment operator.

        /**
         * @brief Move assignment operator.
         * @param renderer The renderer to assign, this becomes empty after the assignment.
         * @return A reference to this renderer.
         */
        Renderer &operator=(Renderer &&renderer) = default;

        /**
         * @brief Test the renderer.
         * @return false if the renderer is empty, true if it is valid.
         */
        explicit operator bool() const noexcept { return mRenderer.operator bool(); }

        /**
         * @brief The the underlying SDL_Renderer* for use with the SDL2 API.
         * @return An SDL_Renderer*
         */
        [[nodiscard]] auto get() const { return mRenderer.get(); }

        /**
         * @brief Set the SDL_BlendMode on the renderer.
         * @param blendMode
         */
        void setDrawBlendMode(SDL_BlendMode blendMode) {
            SDL_SetRenderDrawBlendMode(mRenderer.get(), blendMode);
        }

        /**
         * @brief Create a Texture with the given size.
         * @param size The Texture size.
         * @return a new Texture object.
         */
        Texture createTexture(Size size);

        /**
         * @brief Copy source Texture to destination Texture and set the BlendMode on the destination Texture.
         * @details The function uses RenderTargetGuard to temporarily set the render Target to the destination,
         * calls SDL_RenderCopy to copy the texture, and sets the BlendMode on the destination texture to
         * SDL_BLENDMODE_BLEND.
         * @param source
         * @param destination
         */
        void copyFullTexture(sdl::Texture &source, sdl::Texture &destination);

        /**
         * @brief Calls SDL_RenderClear on the renderer.
         * @return The return status from SDL_RenderClear.
         */
        int renderClear() { return SDL_RenderClear(mRenderer.get()); }

        /**
         * @brief Calls SDL_RenderPresent on the renderer.
         */
        void renderPresent() { SDL_RenderPresent(mRenderer.get()); }

        /**
         * @brief Calls SDL_RenderCopy to copy the source texture to the current render target.
         * @details SDL_RenderCopy is called with nullptr for srcrect and dstrect.
         * @param texture The texture to copy.
         * @return The return status of SDL_RenderCopy.
         */
        int renderCopy(const Texture &texture);

        /**
         * @brief Calls SDL_RenderCopy to copy the source texture to the current target at a destination Rectangle.
         * @param texture The sdl::Texture
         * @param dst The destination RectangleInt
         * @return The return status of SDL_RenderCopy.
         */
        int renderCopy(const Texture &texture, Rectangle dst);

        /**
         * @brief Calls SDL_RenderCopy to copy the source texture from a source Rectangle
         * to the current target at a destination Rectangle.
         * @param texture The Texture.
         * @param src The source Rectangle.
         * @param dst The destination Rectangle.
         * @return The return status of SDL_RenderCopy.
         */
        int renderCopy(const Texture &texture, Rectangle src, Rectangle dst);

        /**
         * @brief Render copy a GradientTexture kernel across the destination Rectangle.
         * @param texture The GradientTexture.
         * @param dst The destination Rectangle.
         * @return The return status of SDL_RenderCopy.
         */
        int renderCopy(GradientTexture &texture, Rectangle dst);

        /**
         * @brief Render with extras
         * @param src Source Rectangle
         * @param dst Destination Rectangle
         * @param angle Rotation Angle
         * @param renderFlip Flip
         * @param point Rotation Position if specified.
         * @return Stastus code returned by SDL_RenderCopyEx()
         */
        int renderCopyEx(Texture &texture, Rectangle src, Rectangle dst, double angle, RenderFlip renderFlip,
                         std::optional<Position> point = std::nullopt);

        /**
         * @brief Calls SDL_RenderFillRect after setting the RenderDrawColor to color.
         * @details The existing RenderDrawColor is saved and restored.
         * @param rectangle The rectangle to fill.
         * @param color The fill color.
         * @return The return status of SDL_RenderFillRect.
         */
        int fillRect(Rectangle rectangle, color::RGBA color);

        /**
         * @brief Render a line
         * @param x0 Start point X
         * @param y0 Start point Y
         * @param x1 End point X
         * @param y1 End point Y
         * @return The SDL2 API return status code.
         */
        int drawLine(int x0, int y0, int x1, int y1);

        /**
         * @brief Render lines.
         * @details The container contains a list of Position objects.
         * @tparam C The container type.
         * @param pointsContainer The points container.
         * @return The SDL2 API return status code.
         */
        template<class C>
        int drawLines(C pointsContainer) {
            auto *points = new SDL_Point[pointsContainer.size()];
            auto first = pointsContainer.begin();
            auto last = pointsContainer.end();
            for (size_t idx = 0; idx < pointsContainer.size() && first != last; ++idx) {
                points[idx].x = first->x();
                points[idx].y = first->y();
                ++first;
            }
            auto status = SDL_RenderDrawLines(get(), points, pointsContainer.size());
            delete [] points;
            return status;
        }
    };

    /**
     * @class RenderTargetGuardException
     * @brief Thrown by RenderTargetGuard on errors.
     */
    class RenderTargetGuardException : public RoseRuntimeError {
    public:
        /**
         * @brief Create a RenderTargetGuardException.
         * @param what_arg An indication of what went wrong.
         */
        explicit RenderTargetGuardException(const std::string &what_arg) : RoseRuntimeError(what_arg) {}
    };

    /**
     * @class RenderTargetGuard
     * @brief Store the current render target replacing it with a new render target. When the object is
     * destroyed (by going out of scope) the old render target is restored.
     */
    class RenderTargetGuard {
    protected:
        Renderer &mRenderer;    ///< The Renderer being guarded
        bool popped{false};     ///< True if the target guard has already been popped off the stack.
        int status{0};          ///< The return status from the last SDL API call.

    public:
        RenderTargetGuard() = delete;   ///< Deleted default constructor.
        RenderTargetGuard(const RenderTargetGuard &) = delete;   ///< Deleted copy constructor.

        /**
         * @brief Set the old render target back on the renderer when destroyed.
         */
        ~RenderTargetGuard() noexcept(false) {
            if (!popped) {
                status = mRenderer.popRenderTarget();
                if (status)
                    throw RenderTargetGuardException(util::StringCompositor("Call to SDL_SetRenderTarget failed:",
                                                                            SDL_GetError()));
            }
        }

        /**
         * @brief Test the status of the RenderTargetGuard.
         * @details The status is good (true is returned) if the status value returned from the last
         * operation on the Renderer object returned 0.
         * @return True if the last operation succeeded.
         */
        explicit operator bool() const noexcept { return status == 0; }

        /**
         * @brief Constructor
         * @param renderer The renderer which render target will be managed.
         * @param texture The texture which will become the new render target.
         */
        RenderTargetGuard(Renderer &renderer, Texture &texture) : mRenderer(renderer) {
            status = mRenderer.pushRenderTarget(texture);
        }

        /**
         * @brief Clear the render target so rendering will be sent to the screen backing buffer.
         */
        void clear() {
            status = mRenderer.popRenderTarget();
            popped = true;
        }

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
     * @class DrawColorGuardException
     * @brief Thrown by DrawColorGuard on errors.
     */
    class DrawColorGuardException : public RoseRuntimeError {
    public:
        /**
         * @brief Construct DrawColorGuardException
         * @param what_arg An indication of the error.
         */
        explicit DrawColorGuardException(const std::string &what_arg) : RoseRuntimeError(what_arg) {}
    };

    /**
     * @class DrawColorGuard
     * @brief Store the current draw color replacing it with a new draw color. When the object is
     * destroyed (by going out of scope) the old draw color is set.
     */
    class DrawColorGuard {
    protected:
        Renderer &mRenderer;    ///< The renderer to which the draw colors are set.
        SDL_Color mOldColor{};  ///< The old draw color
        int mStatus;             ///< The status of the last SDL operation.

    public:
        DrawColorGuard() = delete;  ///< Deleted default constructor.
        DrawColorGuard(const DrawColorGuard &) = delete; ///< Deleted copy constructor.
        DrawColorGuard(DrawColorGuard &&) = default;

        DrawColorGuard &operator=(const DrawColorGuard &) = delete;

        /**
         * @brief Set the old clip rectangle back on the renderer when destroyed.
         */
        ~DrawColorGuard() noexcept(false) {
            if (SDL_SetRenderDrawColor(mRenderer.get(), mOldColor.r, mOldColor.g, mOldColor.b, mOldColor.a)) {
                throw DrawColorGuardException(util::StringCompositor("Call to SDL_XxxRenderDrawColor failed:",
                                                                     SDL_GetError()));
            }
        }

        /**
         * @brief Construct DrawColorGuard with and SDL_Color.
         * @param renderer The renderer to set the color on.
         * @param color The SDL_Color.
         */
        DrawColorGuard(Renderer &renderer, SDL_Color color);

        /**
         * @brief Construct DrawColorGuard with a rose::Color.
         * @param renderer The renderer to set the color on.
         * @param color The rose::Color.
         */
        DrawColorGuard(Renderer &renderer, color::RGBA color);

        /**
         * @brief Test the validity of the DrawColorGuard.
         * @return False if the last SDL operation return an error.
         */
        explicit operator bool() const noexcept { return mStatus == 0; }

        /**
         * @brief Set the drow Color on the renderer without pushing the old color on the stack.
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
        Renderer &mRenderer;    ///< The renderer to which the clip rectangles are set.
        SDL_Rect mOldClip{};    ///< The old clip rectangle

    public:
        ClipRectangleGuard() = delete;  ///< Deleted default constructor
        ClipRectangleGuard(const ClipRectangleGuard &) = delete;     ///< Deleted copy constructor

        /**
         * @brief Set the old clip rectangle back on the renderer when destroyed.
         */
        ~ClipRectangleGuard() {
            SDL_RenderSetClipRect(mRenderer.get(), &mOldClip);
        }

        /**
         * @brief Speculative constructor.
         * @details This constructor saves the current clip rectangle to be restored later, but does not
         * set a new clip rectangle. A new clip rectangle may be set using the assignment operators.
         * @param renderer The renderer to guard the clip rectangle of.
         */
        explicit ClipRectangleGuard(Renderer &renderer) : mRenderer(renderer) {
            SDL_RenderGetClipRect(mRenderer.get(), &mOldClip);
        }

        /**
         * @brief Constructor. Store the current clip rectangle and set the new one.
         * @param renderer The renderer to set the clip rectangles on.
         * @param clip The new clip rectangle.
         */
        ClipRectangleGuard(Renderer &renderer, const SDL_Rect &clip) : mRenderer(renderer) {
            SDL_RenderGetClipRect(mRenderer.get(), &mOldClip);
            SDL_RenderSetClipRect(mRenderer.get(), &clip);
        }

        /**
         * @brief Constructor. Store the current clip rectangle and set the new one.
         * @param renderer The renderer to set the clip rectangles on.
         * @param x X co-ordinate of the new clip rectangle.
         * @param y Y co-ordinate of the new clip rectangle.
         * @param w Width of the new clip rectangle.
         * @param h Height of the new clip rectangle.
         */
        ClipRectangleGuard(Renderer &renderer, int x, int y, int w, int h)
                : ClipRectangleGuard(renderer, SDL_Rect{x, y, w, h}) {}

        /**
         * @brief Conditional constructor.
         * @details This constructor saves the current clip rectangle to be restored later. If clip
         * is valid it is converted to an SDL_Rect and set as the new clip rectangle
         * @param renderer The renderer to set the clip rectangles on.
         * @param clip A, possibly invalid, RectangleInt.
         */
        ClipRectangleGuard(Renderer &renderer, const Rectangle &clip) : mRenderer(renderer) {
            SDL_RenderGetClipRect(mRenderer.get(), &mOldClip);
            auto rect = clip.toSdlRect();
            SDL_RenderSetClipRect(mRenderer.get(), &rect);
        }

        /**
         * @brief Assign a new clip rectangle through the ClipRectangleGuard.
         * @param clip The new clip rectangle.
         * @return The ClipRectangleGuard.
         */
        ClipRectangleGuard &operator=(SDL_Rect &clip) {
            SDL_RenderSetClipRect(mRenderer.get(), &clip);
            return *this;
        }

        /**
         * @brief Assign a new clip rectangle through the ClipRectangleGuard.
         * @details If clip is invalid no changes are made.
         * @param clip The new clip rectangle.
         * @return The ClipRectangleGuard.
         */
        ClipRectangleGuard &operator=(Rectangle &clip) {
            auto rect = clip.toSdlRect();
            SDL_RenderSetClipRect(mRenderer.get(), &rect);
            return *this;
        }

        ClipRectangleGuard &intersection(Rectangle &clip) {
            SDL_Rect current;
            SDL_RenderGetClipRect(mRenderer.get(), &current);
            if (SDL_RectEmpty(&current)) {
                operator=(clip);
            } else {
                mOldClip = current;
                Rectangle r{current.x, current.y, current.w, current.h};
                current = r.intersection(clip).toSdlRect();
                SDL_RenderSetClipRect(mRenderer.get(), &current);
            }
            return *this;
        }
    };
}
