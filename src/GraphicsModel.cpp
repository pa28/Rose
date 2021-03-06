/**
 * @file GraphicsModel.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include <iostream>
#include "GraphicsModel.h"
#include "Utilities.h"

#if GRAPHICS_MODEL_SDL2

#include <SDL.h>
#include <SDL_ttf.h>

#endif

namespace rose::gm {

#if GRAPHICS_MODEL_SDL2
    /**
     * @class Fps
     * @brief Throttle the application to limit the number of frames per second the scene is refreshed.
     */
    class Fps {
    public:
        /**
         * @brief Constructor
         * @param tickInterval The number of SDL ticks desired between frames.
         */
        explicit Fps(int tickInterval = 30)
                : m_tickInterval(tickInterval), m_nextTime(SDL_GetTicks() + tickInterval) {
        }

        /**
         * @brief Wait until the next frame interval.
         */
        void next() {
            SDL_Delay(getTicksToNextFrame());

            m_nextTime += m_tickInterval;
        }

    private:
        const int m_tickInterval;       ///< The number of SDL ticks per frame.
        Uint32 m_nextTime;              ///< The time of the next frame start.

        /**
         * Compute the number of ticks to wait until the next frame time.
         * @return The number of ticks to wait
         */
        [[nodiscard]] Uint32 getTicksToNextFrame() const {
            Uint32 now = SDL_GetTicks();

            return (m_nextTime <= now) ? 0 : m_nextTime - now;
        }
    };
#endif //GRAPHICS_MODEL_SDL2

    Texture Context::createTexture(Size size) {
        Texture texture{*this, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                        size.w, size.h};
        return std::move(texture);
    }

    int Context::renderCopy(const Texture &texture, Rectangle dst) {
        SDL_Rect dstRect{dst.x, dst.y, dst.w, dst.h};

        return SDL_RenderCopy(mRenderer.get(), texture.get(), nullptr, &dstRect);
    }

    int Context::renderCopy(const Texture &texture, Rectangle src, Rectangle dst) {
        SDL_Rect srcRect{src.x, src.y, src.w, src.h};
        SDL_Rect dstRect{dst.x, dst.y, src.w, src.h};
        return SDL_RenderCopy(mRenderer.get(), texture.get(), &srcRect, &dstRect);
    }

    int Context::renderCopy(const Texture &texture) {
        return SDL_RenderCopy(mRenderer.get(), texture.get(), nullptr, nullptr);
    }

    void Context::copyFullTexture(Texture &source, Texture &destination) {
        RenderTargetGuard renderTargetGuard(*this, destination);
        renderCopy(source);
        destination.setBlendMOde(SDL_BLENDMODE_BLEND);
    }

    int Context::renderCopyEx(Texture &texture, Rectangle src, Rectangle dst, double angle, RenderFlip renderFlip,
                               std::optional<Position> point) const {
        SDL_Rect srcRect{src.x, src.y, src.w, src.h};
        SDL_Rect dstRect{dst.x, dst.y, dst.w, dst.h};
        if (point) {
            SDL_Point sdlPoint;
            sdlPoint.x = point->x;
            sdlPoint.y = point->y;
            return SDL_RenderCopyEx(get(), texture.get(), &srcRect, &dstRect, angle, &sdlPoint, renderFlip.mFlip);
        } else {
            return SDL_RenderCopyEx(get(), texture.get(), &srcRect, &dstRect, angle, nullptr, renderFlip.mFlip);
        }
    }


    Texture::Texture(Context &context, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height)
            : Texture() {
        reset(SDL_CreateTexture(context.get(), format, access, width, height));
//        if (!operator bool()) {
//            throw RoseRuntimeError(
//                    util::StringCompositor("SDL_CreateTexture: (", width, 'x', height, ") -- ", SDL_GetError()));
//        }
    }

    Texture::Texture(Context &context, Size size) : Texture() {
        reset(SDL_CreateTexture(context.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.w,
                                size.h));
//        if (!operator bool())
//            throw RoseRuntimeError(
//                    util::StringCompositor("SDL_CreateTexture: ", size.w, 'x', size.h, ") -- ",
//                                           SDL_GetError()));
    }

    RenderTargetGuard::RenderTargetGuard(Context &context, Texture &texture) : mContext(context) {
        mLastTexture = context.mCurrentRenderTarget;
        context.mCurrentRenderTarget = texture.get();
        status = SDL_SetRenderTarget(context.get(), context.mCurrentRenderTarget);
    }

    int RenderTargetGuard::setRenderTarget(Texture &texture) {
        mContext.mCurrentRenderTarget = texture.get();
        return SDL_SetRenderTarget(mContext.get(), mContext.mCurrentRenderTarget);
    }

    RenderTargetGuard::~RenderTargetGuard() noexcept(false) {
        mContext.mCurrentRenderTarget = mLastTexture;
        status = SDL_SetRenderTarget(mContext.get(), mContext.mCurrentRenderTarget);
    }

    bool GraphicsModel::initialize(std::string &title, Size initialSize) {
#if GRAPHICS_MODEL_SDL2
        SDL_RendererInfo info;

        // Set image scaling quality to the highest value available
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);   // Initialize SDL2
        TTF_Init();

        atexit(SDL_Quit);

        SDL_Window *window;        // Declare a pointer to an SDL_Window

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // Create an application window with the following settings:
        mSdlWindow.reset(SDL_CreateWindow(
                (title.empty() ? "SDL2 window" : std::string{title}.c_str()),         //    const char* title
                SDL_WINDOWPOS_UNDEFINED,  //    int x: initial x position
                SDL_WINDOWPOS_UNDEFINED,  //    int y: initial y position
                initialSize.w,            //    int w: width, in pixels
                initialSize.h,            //    int h: height, in pixels
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE           //    Uint32 flags: window options, see docs
        ));

        if (mSdlWindow) {
            mContext = Context{mSdlWindow, -1, SDL_RENDERER_ACCELERATED
                                                      | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC};

            if (mContext) {
                mContext.setDrawBlendMode(SDL_BLENDMODE_BLEND);
            } else {
                ErrorCode = RoseErrorCode::SDL_RENDERER_CREATE;
                std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << '\n';
                return false;
            }
        } else {
            std::cerr << "Could not create SDL_Window: " << SDL_GetError() << '\n';
            ErrorCode = RoseErrorCode::SDL_WINDOW_CREATE;
            return false;
        }
#endif
        return true;
    }

    /**
 * @brief The Rose version of the SDL event loop.
 */
    void GraphicsModel::eventLoop() {
#if GRAPHICS_MODEL_SDL2
        SDL_Event e;
        Fps fps;

        while (mRunEventLoop) {
            //Handle events on queue
            while (SDL_PollEvent(&e) != 0) {
                //User requests quit
                if (e.type == SDL_QUIT) {
                    mRunEventLoop = false;
                    continue;
                }
                mEventSemantics.onEvent(e);
            }

            drawAll();

            fps.next();
        }
#endif
    }

    void GraphicsModel::drawAll() {
#if GRAPHICS_MODEL_SDL2
        // ToDo: Change once colour model is implemented.
        SDL_SetRenderDrawColor(mContext.get(), 0x0, 0x0, 0x0, 0xff);
#endif
        mContext.renderClear();
        mContext.renderPresent();
    }

    GraphicsModel::GraphicsModel() : mEventSemantics(*this) {
        mEventSemantics.setWindowPositionChangeCallback(&GraphicsModel::windowPositionChange);

        mEventSemantics.setWindowSizeChangeCallback(&GraphicsModel::windowSizeChange);

        mEventSemantics.setWindowStateChangeCallback(&GraphicsModel::windowStateChange);
    }

    void GraphicsModel::windowStateChange(EventSemantics::WindowEventType type) {
        std::cout << __PRETTY_FUNCTION__ << '\n';
    }

    void GraphicsModel::windowSizeChange(EventSemantics::WindowEventType type, Size size) {
        std::cout << __PRETTY_FUNCTION__ << size << '\n';
    }

    void GraphicsModel::windowPositionChange(EventSemantics::WindowEventType type, Position position) {
        std::cout << __PRETTY_FUNCTION__ << position << '\n';
    }

    std::optional<FocusTree> GraphicsModel::focusTree(Position mousePosition) {
        for (auto &content : ReverseContainerView(mScreen)) {
            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                auto windowRect = window->getScreenRectangle(Position::Zero);
                if (windowRect.contains(mousePosition)) {
                    return window->focusTree(mousePosition);
                } else if (window->isModal())
                    return std::nullopt;
            }
        }
        return std::nullopt;
    }

#if GRAPHICS_MODEL_SDL2

    void EventSemantics::onEvent(SDL_Event &e) {
        switch (e.type) {
            case SDL_WINDOWEVENT:
                windowEvent(e.window);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                keyboardEvent(e.key);
                break;
            case SDL_TEXTINPUT:
                std::cout << "TextInputEvent\n";
                break;
            case SDL_TEXTEDITING:
                std::cout << "TextEditingEvent\n";
                break;
            case SDL_MOUSEMOTION:
                mouseMotionEvent(e.motion);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseButtonEvent(e.button);
                break;
            case SDL_MOUSEWHEEL:
                mouseWheelEvent(e.wheel);
                break;
            case SDL_FINGERMOTION:
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
                break;
            case SDL_MULTIGESTURE:
                break;
            case SDL_KEYMAPCHANGED:
                break;
            default:
                std::cout << "Unhandled event: 0x" << std::hex << e.type << std::dec << '\n';
        }
    }

    void EventSemantics::windowEvent(SDL_WindowEvent &e) {
        switch (e.event) {
            case SDL_WINDOWEVENT_SHOWN:
                windowStateChange(Shown);
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                windowStateChange(Hidden);
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                windowStateChange(Exposed);
                break;
            case SDL_WINDOWEVENT_MOVED:
                windowPositionChange(Moved,Position{e.data1,e.data2});
                break;
            case SDL_WINDOWEVENT_RESIZED:
                windowSizeChange(Resized,Size{e.data1,e.data2});
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                windowStateChange(SizeChanged);
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                windowStateChange(Minimized);
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                windowStateChange(Maximized);
                break;
            case SDL_WINDOWEVENT_RESTORED:
                windowStateChange(Restored);
                break;
            case SDL_WINDOWEVENT_ENTER:
                windowStateChange(Enter);
                break;
            case SDL_WINDOWEVENT_LEAVE:
                windowStateChange(Leave);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                windowStateChange(Focus);
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                windowStateChange(UnFocus);
                break;
            case SDL_WINDOWEVENT_CLOSE:
                windowStateChange(Close);
                break;
            case SDL_WINDOWEVENT_TAKE_FOCUS:
            case SDL_WINDOWEVENT_HIT_TEST:
                break;
        }
    }

    void EventSemantics::keyboardEvent(SDL_KeyboardEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID
                  << ", state: " << (uint32_t) e.state << ", repeat: " << (uint32_t) e.repeat << '\n';
    }

    void EventSemantics::mouseMotionEvent(SDL_MouseMotionEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID << ", which: " << e.which
                  << ", state: " << (uint32_t) e.state
                  << ", pos: " << Position{e.x, e.y} << " rel: " << Position{e.xrel, e.yrel} << '\n';
    }

    void EventSemantics::mouseButtonEvent(SDL_MouseButtonEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID << ", which: " << e.which
                  << ", state: " << (uint32_t) e.state
                  << ", pos: " << Position{e.x, e.y} << '\n';
    }

    void EventSemantics::mouseWheelEvent(SDL_MouseWheelEvent &e) {
        std::cout << __PRETTY_FUNCTION__ << " Id: " << e.windowID << ", which: " << e.which
                  << ", direction: " << e.direction << ", pos: " << Position{e.x, e.y} << '\n';
    }

#endif
}
