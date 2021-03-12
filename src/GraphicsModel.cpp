/**
 * @file GraphicsModel.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include <iostream>
#include "Font.h"
#include "GraphicsModel.h"
#include "Texture.h"
#include "Utilities.h"
#include "Settings.h"

#include <SDL.h>
#include <SDL_ttf.h>

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
        auto texture = CreateTexture(*this, size);
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
        TextureSetBlendMode(destination, SDL_BLENDMODE_BLEND);
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

    int Context::setDrawColor(color::RGBA color) {
        auto c = color.toSdlColor();
        return SDL_SetRenderDrawColor(get(), c.r, c.g, c.b, c.a);
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

    bool GraphicsModel::initialize(const std::string &title, Size initialSize, Position initialPosition,
                                   uint32_t extraFlags) {
#if GRAPHICS_MODEL_SDL2
        std::cout << __PRETTY_FUNCTION__ << '\n';

        Settings &settings{Settings::getSettings()};
        SDL_RendererInfo info;

        // Set image scaling quality to the highest value available
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);   // Initialize SDL2
        TTF_Init();

        atexit(SDL_Quit);

        std::cout << "    Number of screens: " << SDL_GetNumVideoDisplays() << '\n';
        for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {
            SDL_Rect displayBounds{0,0,0,0};

            if (SDL_GetDisplayBounds(i, &displayBounds)) {
                mDisplayBounds.emplace_back(Rectangle{});
            } else {
                mDisplayBounds.emplace_back(Rectangle{displayBounds.x, displayBounds.y, displayBounds.w, displayBounds.h});
            }
        }

        SDL_Window *window;        // Declare a pointer to an SDL_Window
        uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

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
                initialPosition.x,        //    int x: initial x position
                initialPosition.y,        //    int y: initial y position
                initialSize.w,            //    int w: width, in pixels
                initialSize.h,            //    int h: height, in pixels
                flags | extraFlags
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
    void GraphicsModel::eventLoop(std::shared_ptr<Screen> &screen) {
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
                if (eventCallback)
                    eventCallback(e);
            }

            drawAll(screen);

            fps.next();
        }
#endif
    }

    void GraphicsModel::drawAll(std::shared_ptr<Screen> &screen) {
        if (!mBackground || (TextureGetSize(mBackground) != screenRectangle().size())) {
            mBackground = CreateTexture(mContext, screenRectangle().size());
            mRedrawBackground = true;
        }

        if (mRedrawBackground) {
            RenderTargetGuard renderTargetGuard{mContext, mBackground};

            mContext.setDrawColor(color::RGBA::TransparentBlack);
            mContext.renderClear();

            for (auto &content : *screen) {
                if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
                    window->draw(mContext, Position::Zero);
                }
            }

            mContext.renderPresent();
        }

        if (mAnimation || mRedrawBackground) {
            mContext.setDrawColor(color::RGBA::TransparentBlack);
            mContext.renderClear();
            mContext.renderCopy(mBackground);
            if (mAnimation) {
                // Render animated widgets.
            }
            mContext.renderPresent();
        }

        mRedrawBackground = false;
    }

//    std::optional<FocusTree> GraphicsModel::focusTree(Screen &screen, Position mousePosition) {
//        auto it = screen.
//        for (auto &content : ReverseContainerView(screen)) {
//            if (auto window = std::dynamic_pointer_cast<Window>(content); window) {
//                auto windowRect = window->getScreenRectangle(Position::Zero);
//                if (windowRect.contains(mousePosition)) {
//                    return window->focusTree(mousePosition);
//                } else if (window->isModal())
//                    return std::nullopt;
//            }
//        }
//        return std::nullopt;
//    }

    DrawColorGuard::DrawColorGuard(Context &context, SDL_Color color) : mContext(context) {
        mStatus = 0;
        if (int status = SDL_GetRenderDrawColor( mContext.get(), &mOldColor.r, &mOldColor.g,
                                                 &mOldColor.b, &mOldColor.a); status == 0 ) {
            mStatus = SDL_SetRenderDrawColor( mContext.get(), color.r, color.g, color.b, color.a);
        } else {
            mStatus = status;
        }
    }

    DrawColorGuard::DrawColorGuard(Context &context, color::RGBA color) : DrawColorGuard(context,
                                                                                           color.toSdlColor()) {}

    int DrawColorGuard::setDrawColor(SDL_Color color) {
        return SDL_SetRenderDrawColor(mContext.get(), color.r, color.g, color.b, color.a);
    }

    color::RGBA getRGBA(SDL_PixelFormat *format, uint32_t pixel) {
        uint8_t r, g, b, a;
        SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
        return color::RGBA{(uint)r, (uint)g, (uint)b, (uint)a};
    }

    uint32_t mapRGBA(SDL_PixelFormat *format, const color::RGBA &color) {
        auto c = color.toSdlColor();
        return SDL_MapRGBA(format, c.r, c.g, c.b, c.a);
    }
}