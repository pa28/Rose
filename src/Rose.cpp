//
// Created by richard on 2020-10-20.
//

#include <exception>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <SDL_ttf.h>
#include "Entypo.h"
#include "Font.h"
#include "Math.h"
#include "PopupWindow.h"
#include "Rose.h"
#include "ScreenMetrics.h"
#include "Signals.h"
#include "Surface.h"
#include "Texture.h"
#include "Widget.h"

namespace rose {
    using namespace util;

    uint32_t SignalSerialNumber::sNextSerialNumber = 1;

    /**
     * @brief Generate an XDG Base Directory.
     * @param name The XDG environment variable name.
     * @param home The path to the home directory.
     * @param defaultPath The default XDG path relative to the home directory.
     * @param app The application name.
     * @return
     */
    std::filesystem::path getenv_path(const char *name, const std::filesystem::path &home,
                                      const char *defaultPath,
                                      const std::string &app) {
        char *value = getenv(name);
        if (value)
            return std::filesystem::path{value};

        std::filesystem::path app_path = home;
        app_path.append(defaultPath);
        app_path.append(app);
        return app_path;
    }

    Rose::~Rose() {
    }

    Rose::Rose(Size screenSize, int argc, char **argv, const std::string_view title) : mCmdLineParser(argc, argv) {
        mWidth = screenSize.width();
        mHeight = screenSize.height();

        mHomeDirectory = std::string{getenv("HOME")};
        auto pid = getpid();

        std::filesystem::path procExec{"/proc"};
        procExec.append(to_string(pid)).append("exe");

        if (std::filesystem::is_symlink(procExec)) {
            auto appName = std::filesystem::read_symlink(procExec).filename().string();

            mDataHome = getenv_path("XDG_DATA_HOME", mHomeDirectory, ".local/share", appName);
            mConfigHome = getenv_path("XDG_CONFIG_HOME", mHomeDirectory, ".config", appName);
            mCacheHome = getenv_path("XDG_CACHE_HOME", mHomeDirectory, ".cache", appName);
        } else {
            std::cerr << StringCompositor('"', procExec, '"', " is not a symbolic link to application.\n");
            mErrorCode = RoseErrorCode::XDG_PATH_FAIL;
            return;
        }

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

        if (mCmdLineParser.cmdOptionExists("-width"))
            mWidth = (int) strtol(mCmdLineParser.getCmdOption("-width").c_str(), nullptr, 10);

        if (mCmdLineParser.cmdOptionExists("-height"))
            mHeight = (int) strtol(mCmdLineParser.getCmdOption("-width").c_str(), nullptr, 10);

        // Create an application window with the following settings:
        mSdlWindow.reset(SDL_CreateWindow(
                (title.empty() ? "An SDL2 window" : std::string{title}.c_str()),         //    const char* title
                SDL_WINDOWPOS_UNDEFINED,  //    int x: initial x position
                SDL_WINDOWPOS_UNDEFINED,  //    int y: initial y position
                mWidth,                      //    int w: width, in pixels
                mHeight,                      //    int h: height, in pixels
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                SDL_WINDOW_ALLOW_HIGHDPI        //    Uint32 flags: window options, see docs
        ));

        if (mSdlWindow) {
            mRenderer = sdl::Renderer{mSdlWindow, -1, SDL_RENDERER_ACCELERATED
                                      | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC};

            if (mRenderer) {
                mRenderer.setDrawBlendMode(SDL_BLENDMODE_BLEND);
            } else {
                mErrorCode = RoseErrorCode::SDL_RENDERER_CREATE;
                std::cerr << "Could not create renderer: " << SDL_GetError() << '\n';
            }
        } else {
            std::cerr << "Could not create window: " << SDL_GetError() << '\n';
            mErrorCode = RoseErrorCode::SDL_WINDOW_CREATE;
        }
    }

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

    /**
     * @brief The Rose version of the SDL event loop.
     */
    void Rose::eventLoop() {
        try {
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

                    if (mTranslateFingerEvents) {
                        // Translate finger events to equivalent mouse events
                        if (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP) {
                            SDL_Event mbe;

                            // Translate finger events to mouse evnets.
                            mbe.type = (e.type == SDL_FINGERDOWN) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
                            mbe.button.timestamp = e.tfinger.timestamp;
                            mbe.button.windowID = SDL_GetWindowID(mSdlWindow.get());
                            mbe.button.which = SDL_TOUCH_MOUSEID;
                            mbe.button.button = SDL_BUTTON_LEFT;
                            mbe.button.state = (e.type == SDL_FINGERDOWN) ? SDL_PRESSED : SDL_RELEASED;
                            mbe.button.clicks = 1;
                            mbe.button.x = (Sint32) (e.tfinger.x * (float) mWidth);
                            mbe.button.y = (Sint32) (e.tfinger.y * (float) mHeight);
                            SDL_WarpMouseGlobal(mbe.button.x, mbe.button.y);
                            SDL_PushEvent(&mbe);
                        } else if (e.type == SDL_FINGERMOTION) {
                            SDL_WarpMouseGlobal((Sint32) (e.tfinger.x * (float) mWidth),
                                                (Sint32) (e.tfinger.y * (float) mHeight));
                        }
                    }
                    onEvent(e);
                }

                SDL_SetRenderDrawColor(mRenderer.get(), 0x0, 0x0, 0x0, 0xff);
                SDL_RenderClear(mRenderer.get());

                drawAll();

                fps.next();
            }
        }

        catch (const std::runtime_error &e) {
            throw e;
        }
    }

    void Rose::drawAll() {
        if (mRedoLayout) {
            initialLayout(mRenderer);
            mRedoLayout = false;
        }

        if (mNeedsDrawing) {
            sdl::ClipRectangleGuard clipGuard(mRenderer, Rectangle{0, 0, mWidth, mHeight});
            mRenderer.renderClear();

            for (auto &window : mWindowList)
                // Invalid size or position values indicate the Widget has control over them.
                window->draw(mRenderer,Rectangle{0, 0, mWidth, mHeight});
            // Render the rect to the screen
            mRenderer.renderPresent();
            mNeedsDrawing = false;
        }
    }

    bool Rose::onEvent(SDL_Event &e) {
        switch (e.type) {
            case SDL_MOUSEWHEEL: {
                if (!mProcessEvents)
                    return false;
                return scrollCallbackEvent(e.wheel.x, e.wheel.y);
            }
                break;

            case SDL_MOUSEMOTION: {
                if (!mProcessEvents)
                    return false;
                return cursorPosCallbackEvent(e.motion.x, e.motion.y);
            }
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                if (!mProcessEvents)
                    return false;

                SDL_Keymod mods = SDL_GetModState();
                return mouseButtonCallbackEvent(e.button.button, e.button.type, mods);
            }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                if (!mProcessEvents)
                    return false;

                SDL_Keymod mods = SDL_GetModState();
                return keyCallbackEvent(e.key.keysym.sym, e.key.keysym.scancode, e.key.state, mods);
            }
                break;

            case SDL_TEXTINPUT: {
                if (!mProcessEvents)
                    return false;
                return charCallbackEvent(e.text.text[0]);
            }
                break;
        }
        return false;
    }

    void Rose::clearFocus() {
        // Clear the old focus path.
        for (const auto &w: mFocusPath) {
            if (!w.lock()->getFocused())
                continue;
            w.lock()->focusEvent(false);
        }
        mFocusPath.clear();
    }

    void Rose::updateFocus(const std::shared_ptr<Widget> &focusWidget) {
        clearFocus();
        // Climb up the tree gathering a path
        // To a widget without a parent is the "screen"
        // The one below is the "window"
        auto widget = focusWidget;
        while (widget->parent()) {
            mFocusPath.push_back(widget);
            widget = widget->parent()->getWidget();
        }
        mFocusPath.push_back(widget);

        for (auto &child : mFocusPath)
            child.lock()->setFocused(true);

        if (widget)
            moveToFront(widget);
    }

    bool Rose::scrollCallbackEvent(double x, double y) {
        mLastInteraction = SDL_GetTicks();
        try {
            if (!mFocusPath.empty()) {
                auto window = mFocusPath.back().lock()->as<rose::Window>();
                if (window && window->getModal()) {
                    if (!window->contains(mMousePos))
                        return false;
                }
            }
            for (auto &window : ReverseContainerView(mWindowList)) {
                if (auto widget = window->findWidget(mMousePos); widget) {
                    auto container = widget->as<Container>();
                    if (!container)
                        container = widget->parent();

                    while (container && !container->supportsDrag())
                        container = container->parent();

                    if (container)
                        return container->scrollEvent(mMousePos, x, y);
                    return false;
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what()
                      << std::endl;
            abort();
        }

        return false;
    }

    bool Rose::cursorPosCallbackEvent(double x, double y) {
        Position cursorPos(roundToInt(x), roundToInt(y));
        bool ret = false;
        mLastInteraction = SDL_GetTicks();
        try {
            if (mDragActive && !mDragWidget.expired()) {
                auto sDragWidget = mDragWidget.lock();
//                auto dragParent = sDragWidget->parent();
//                if (dragParent)
//                    ret = dragParent->mouseDragEvent(cursorPos,cursorPos - mMousePos, mMouseState, mModifiers);
//                else
                    ret = sDragWidget->mouseDragEvent(cursorPos, cursorPos - mMousePos, mMouseState, mModifiers);
            }

            if (!ret)
                for (auto &window : ReverseContainerView(mWindowList)) {
                    if (window->layoutHints().mAssignedRect->contains(cursorPos)) {
                        auto relCursorPos = cursorPos - window->layoutHints().mAssignedRect->getPosition();
                        ret = window->mouseMotionEvent(cursorPos, cursorPos - mMousePos, mMouseState, mModifiers);
                        if (ret)
                            break;
                    }
                }

            mMousePos = cursorPos;
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }

        return ret;
    }

    std::shared_ptr<Widget> Rose::findWidget(const Position &pos) {
        std::shared_ptr<Widget> widget{};

        for (auto &window : ReverseContainerView(mWindowList)) {
            if (window->layoutHints().mAssignedRect) {
                if (window->layoutHints().mAssignedRect->contains(pos)) {
                    return window->findWidget(pos - window->layoutHints().mAssignedRect->getPosition());
                }
            } else if (window->getSize() && window->getPos()) {
                if (pos >= window->getPos() && (pos - window->getPos().value()) <= window->getSize())
                    return window->findWidget(pos - window->getPos().value());
            } else {
                throw std::runtime_error("Window location not well formed.");
            }

            if (window->getModal() == Modal)
                break;
        }
        return nullptr;
    }

    void Rose::removeWindow(std::shared_ptr<Window> window) {
        if (!mWindowList.empty() && mWindowList.size() > 1) {
            // Only clear focus if this window has it.
            if (!empty(mFocusPath)) {
                if (window == mFocusPath.back().lock())
                    clearFocus();
            }
            mWindowList.erase(std::find(mWindowList.begin(), mWindowList.end(), window));
            needsDrawing(true);
        }
    }

    bool Rose::mouseButtonCallbackEvent(int button, int action, int modifiers) {
        mModifiers = modifiers;
        mLastInteraction = SDL_GetTicks();
        try {
            if (!mFocusPath.empty()) {
                if (auto window = mFocusPath.back().lock()->as<rose::Window>(); window) {
                    auto containsMouse = window->contains(mMousePos);
                    switch (window->getModal()) {
                        case NotModal:
                            break;
                        case Ephemeral:
                            if (!containsMouse) {
                                clearFocus();
                                removeWindow(window);
                                needsDrawing(true);
                                return true;
                            }
                            break;
                        case Modal:
                            if (!containsMouse)
                                return false;
                    }
                } else
                    throw std::logic_error("Widget on Focus Path not a Window.");
            }

            if (action == SDL_MOUSEBUTTONDOWN)
                mMouseState |= 1u << (uint32_t) button;
            else
                mMouseState &= ~(1u << (uint32_t) button);

            auto dropWidget = findWidget(mMousePos);
            if (mDragActive && action == SDL_MOUSEBUTTONUP && !mDragWidget.expired() && dropWidget != mDragWidget.lock()) {
                auto sDragWidget = mDragWidget.lock();
//                auto dragParent = sDragWidget->parent();
//                if (!dragParent)
//                    dragParent = sDragWidget->as<Container>();
                sDragWidget->mouseButtonEvent(
                        mMousePos - sDragWidget->getScreenPosition(), button,
                        false, mModifiers);
            }

            if (action == SDL_MOUSEBUTTONDOWN && button == SDL_BUTTON_LEFT) {
                mDragWidget = findWidget(mMousePos);
                mDragActive = true;
                if (mDragWidget.lock().get() == mWindowList.front().get()) {
                    mDragWidget.reset();
                    mDragActive = false;
                }
                if (!mDragActive)
                    clearFocus();
            } else {
                mDragActive = false;
                mDragWidget.reset();
            }

            for (auto &window : ReverseContainerView(mWindowList)) {
                if (auto assignedRect = window->layoutHints().mAssignedRect; assignedRect && assignedRect->contains(mMousePos)) {
                    auto relMousePos = mMousePos - assignedRect->getPosition();
                    if (window->mouseButtonEvent(relMousePos, button, action == SDL_MOUSEBUTTONDOWN, mModifiers))
                        return true;
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }

        return false;
    }

    bool Rose::keyCallbackEvent(int key, int scancode, int action, int mods) {
        mLastInteraction = SDL_GetTicks();
        try {
            for (auto &window : ReverseContainerView(mWindowList)) {
                if (window->keyboardEvent(key, scancode, action, mods))
                    return true;
            }
            return false;
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Rose::charCallbackEvent(unsigned int codepoint) {
        mLastInteraction = SDL_GetTicks();
        try {
            for (auto &window : ReverseContainerView(mWindowList)) {
                if (window->keyboardCharacterEvent(codepoint))
                    return true;
            }
            return false;
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in event handler: " << e.what()
                      << std::endl;
            abort();
        }
    }

    void Rose::moveToFront(std::shared_ptr<Widget> widget) {
        if (mWindowList.empty() || mWindowList.size() == 1)
            return;
        std::sort(mWindowList.begin(), mWindowList.end(),
                  [&widget](auto w0, auto w1) {
                      if (w0 != nullptr && w1 != nullptr) {
                          if (w0->getVisible() && w1->getVisible()) {
                              if (w0->getModal() == w1->getModal()) {
                                  return w1 == widget;
                              } else {
                                  return w1->getModal() != NotModal;
                              }
                          } else {
                              return w1->getVisible();
                          }
                      } else {
                          return w1 != nullptr;
                      }
                  });
    }

    std::array<char, 8> Rose::utf8(int c) {
        auto uc = (uint) c;
        std::array<char, 8> seq{};
        int n = 0;
        if (uc < 0x80) n = 1;
        else if (uc < 0x800) n = 2;
        else if (uc < 0x10000) n = 3;
        else if (uc < 0x200000) n = 4;
        else if (uc < 0x4000000) n = 5;
        else if (uc <= 0x7fffffff) n = 6;
        seq[n] = '\0';
        switch (n) {
            case 6:
                seq[5] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x4000000u;
            case 5:
                seq[4] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x200000u;
            case 4:
                seq[3] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x10000u;
            case 3:
                seq[2] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0x800u;
            case 2:
                seq[1] = 0x80u | (uc & 0x3fu);
                uc = uc >> 6u;
                uc |= 0xc0u;
            case 1:
                seq[0] = uc;
            default:
                break;
        }
        return seq;
    }

    void Rose::createIcon(int iconCode, int iconSize, const color::RGBA &iconColor, sdl::TextureData &textureData) {
        auto icon = utf8(iconCode);
        getTexAndRectUtf8(mRenderer, icon.data(), mTheme.mIconFontName, iconSize,
                          iconColor, textureData);
    }

    void Rose::createStandardIcons() {
        static constexpr std::array<RoseImageId, 7> iconList{IconCancel, IconHelp,
                                                             IconInfo, IconCheck,
                                                             IconAlert, IconDown,
                                                             IconRight};

        for (auto &iconId : iconList) {
            int iconCode;
            switch (iconId) {
                case IconCancel:
                    iconCode = ENTYPO_ICON_CANCEL_CIRCLED;
                    break;
                case IconHelp:
                    iconCode = ENTYPO_ICON_CIRCLED_HELP;
                    break;
                case IconInfo:
                    iconCode = ENTYPO_ICON_INFO;
                    break;
                case IconCheck:
                    iconCode = ENTYPO_ICON_CHECK;
                    break;
                case IconAlert:
                    iconCode = ENTYPO_ICON_ALERT;
                    break;
                case IconDown:
                    iconCode = ENTYPO_ICON_DOWN_DIR;
                    break;
                case IconRight:
                    iconCode = ENTYPO_ICON_RIGHT_DIR;
                    break;
                default:
                    throw std::runtime_error("Unhandled icon id.");
            }
            sdl::TextureData textureData;
            createIcon(iconCode, mTheme.mIconFontSize, mTheme.mTextColour, textureData);
            mImageRepository.setImage(iconId, std::move(textureData));

            auto font = mFontCache.getFont(mTheme.mIconFontName, mTheme.mIconFontSize);
            mIconFontMetrics.fontAscent = TTF_FontAscent(font.value().get());
            mIconFontMetrics.fontDescent = TTF_FontDescent(font.value().get());
            mIconFontMetrics.fontHeight = TTF_FontHeight(font.value().get());
            mIconFontMetrics.fontLineSkip = TTF_FontLineSkip(font.value().get());
        }
    }

    void
    Rose::createScaledIcon(int iconCode, const std::string &fontName, int iconSize, int scale, const color::RGBA &iconColor,
                           sdl::TextureData &texture) {
        auto icon = utf8(iconCode);
        auto surface = getSurfaceUtf8(icon.data(), fontName, iconSize * scale, iconColor);

        sdl::Surface generate{surface.get()->w, surface.get()->h, 32, rmask, gmask, bmask, amask};

        int xStride = generate.get()->w / scale;
        int yStride = generate.get()->h / scale;

        auto color = iconColor;

        for (int y = 0; y < generate->h; ++y) {
            for (int x = 0; x < generate->w; ++x) {
                uint32_t sum = 0;
                uint32_t count = 0;
                for (int y0 = 0; y0 < scale && y + y0 < generate->h; ++y0) {
                    for (int x0 = 0; x0 < scale && x + x0 < generate->w; ++x0) {
                        if (x + x0 < generate->w && y + y0 < generate->h) {
                            sum += (surface.pixel(x + x0, y + y0) & amask) >> ashift;
                            ++count;
                        }
                    }
                }
                int xw = xStride * (x % scale) + x / scale;
                int yw = yStride * (y % scale) + y / scale;
                color.a() = (((float) sum / (float) count) / 255.f);
                if (xw < generate->w && yw < generate->h)
                    generate.pixel(xw, yw) = sdl::mapRGBA(surface->format, color);
            }
        }

        texture.setScale(scale);
        texture.setSize(Size{surface->w, surface->h});
        texture = surface.toTexture(mRenderer).release();
    }

    sdl::Surface Rose::getSurfaceUtf8(const char *text, const std::string &fontName, size_t ptSize, color::RGBA textColor) {
        auto font = mFontCache.getFont(fontName, ptSize);

        if (!font)
            return sdl::Surface{};

        return sdl::Surface{TTF_RenderUTF8_Blended(font.value().get(), text, textColor.toSdlColor())};
    }

    void Rose::getTexAndRectUtf8(sdl::Renderer &renderer, const char *text, const std::string &fontName, size_t ptsize,
                                 color::RGBA textColor, sdl::TextureData &textureData) {
        auto font = mFontCache.getFont(fontName, ptsize);

        if (!font)
            throw (std::runtime_error(StringCompositor("Can not find font '", fontName, "'.")));

        sdl::Surface surface;
        surface.reset(TTF_RenderUTF8_Blended(font.value().get(), text, textColor.toSdlColor()));

        if (surface) {
            textureData = surface.toTexture(renderer).release();
            textureData.setSize(Size{surface->w, surface->h});
        }
    }

    void Rose::copyFullTexture(sdl::Renderer &renderer, sdl::Texture &texture, sdl::TextureData &textureData) {
        sdl::RenderTargetGuard renderTargetGuard(renderer, textureData);
        renderer.renderCopy(texture);
        textureData.setBlendMOde(SDL_BLENDMODE_BLEND);
    }

    sdl::Surface Rose::createBlankSurface(int size) {
        sdl::Surface surface{size, size, 32, rmask, gmask, bmask, amask};
        surface.setBlendMode(SDL_BLENDMODE_BLEND);
        surface.fillRectangle(color::RGBA::TransparentBlack);
        return std::move(surface);
    }

    void
    Rose::createSquareCorners(sdl::Renderer &renderer, int radius, int borderWidth, rose::color::RGBA top, rose::color::RGBA bot,
                              rose::color::RGBA left, rose::color::RGBA right) {
        sdl::Surface surfaceBevelOut, surfaceNotchIn, surfaceBevelIn, surfaceNotchOut;
        int scale = 1;

        int rS = radius * scale;        // The radius at scale
        int bS = borderWidth * scale;   // The border width at scale
        int size = radius * scale * 2;  // The size of the side of a square to accommodate rS

        surfaceBevelOut = createBlankSurface(size);
        surfaceNotchIn = createBlankSurface(size);
        surfaceBevelIn = createBlankSurface(size);
        surfaceNotchOut = createBlankSurface(size);

        for (int d = 0; d < borderWidth; ++d) {
            color::RGBA color, invColor;
            auto outside = d < borderWidth / 2;

            // For each line in the frame
            for (int line = 0; line < 4; ++line) {
                Rectangle rect;
                switch (line) {
                    case 0: // Top
                        color = top;
                        invColor = bot;
                        rect.x() = d;
                        rect.y() = d;
                        rect.width() = size - d*2 - 1;
                        rect.height() = 1;
                        break;
                    case 1: // Bottom
                        color = bot;
                        invColor = top;
                        rect.x() = d;
                        rect.y() = size - d - 1;
                        rect.width() = size - d*2 - 1;
                        rect.height() = 1;
                        break;
                    case 2: // Left
                        color = left;
                        invColor = right;
                        rect.x() = d;
                        rect.y() = d;
                        rect.width() = 1;
                        rect.height() = size - d*2 - 1;;
                        break;
                    case 3: // Right
                        color = right;
                        invColor = left;
                        rect.x() = size - d - 1;
                        rect.y() = d;
                        rect.width() = 1;
                        rect.height() = size - d*2 - 1;;
                        break;
                    default:
                        break;
                }
                if (line < 4) {
                    auto notchColor = outside ? invColor : color;
                    auto invNotchColor = outside ? color : invColor;
                    surfaceBevelOut.fillRectangle(rect, color);
                    surfaceBevelIn.fillRectangle(rect, invColor);
                    surfaceNotchIn.fillRectangle(rect, notchColor);
                    surfaceNotchOut.fillRectangle(rect, invNotchColor);
                }
            }
        }

        sdl::Texture texture;

        texture = surfaceBevelOut.toTexture(renderer);
        sdl::TextureData bevelOutCornerTrim{ renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                             radius * 2, radius * 2};
        copyFullTexture(renderer, texture, bevelOutCornerTrim);
        mImageRepository.setImage(BevelOutSquareCorners, std::move(bevelOutCornerTrim));

        texture = surfaceBevelIn.toTexture(renderer);
        sdl::TextureData bevelInCornerTrim{ renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                            radius * 2, radius * 2};
        copyFullTexture(renderer, texture, bevelInCornerTrim);
        mImageRepository.setImage(BevelInSquareCorners, std::move(bevelInCornerTrim));

        texture = surfaceNotchOut.toTexture(renderer);
        sdl::TextureData notchOutCornerTrim{ renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                             radius * 2, radius * 2};
        copyFullTexture(renderer, texture, notchOutCornerTrim);
        mImageRepository.setImage(NotchOutSquareCorners, std::move(notchOutCornerTrim));

        texture = surfaceNotchIn.toTexture(renderer);
        sdl::TextureData notchInCornerTrim{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                           radius * 2, radius * 2};
        copyFullTexture(renderer, texture, notchInCornerTrim);
        mImageRepository.setImage(NotchInSquareCorners, std::move(notchInCornerTrim));
    }

    void Rose::createRoundCorners(sdl::Renderer &renderer, int scale, int radius, int borderWidth, color::RGBA top,
                                  color::RGBA bot,
                                  color::RGBA left, color::RGBA right) {

        sdl::Surface surfaceBevelOut, surfaceNotchIn, surfaceBevelIn, surfaceNotchOut;

        int rS = radius * scale;        // The radius at scale
        int bS = borderWidth * scale;   // The border width at scale
        int size = radius * scale * 2;  // The size of the side of a square to accommodate rS

        surfaceBevelOut = createBlankSurface(size);
        surfaceNotchIn = createBlankSurface(size);
        surfaceBevelIn = createBlankSurface(size);
        surfaceNotchOut = createBlankSurface(size);

        auto pixelFormat = surfaceBevelOut->format;

        auto r0 = rS * rS;                      // the maximum radius squared of the border corner
        auto rn = (rS - bS / 2) * (rS - bS / 2);    // the notch switch over radius squared
        auto r1 = (rS - bS) * (rS - bS);        // the minimum radius squared

        for (int x = 0; x < size; ++x) {
            for (int y = 0; y < size; ++y) {
                int xr = rS - x;                // x component of the current radius
                int yr = rS - y;                // y component of the current radius
                int r2 = xr * xr + yr * yr;

                // Outside the max radius is transparent
                if (r2 > r0) {
                    auto pixel = color::set_a_value(surfaceBevelOut.pixel(x, y), 0);
                    surfaceBevelOut.pixel(x, y) = pixel;
                    surfaceBevelIn.pixel(x, y) = pixel;
                    surfaceNotchOut.pixel(x, y) = pixel;
                    surfaceNotchIn.pixel(x, y) = pixel;
                } else if (r2 > r1) {
                    float s = (float) (yr * yr) / (float) r2;
                    float c = (float) (xr * xr) / (float) r2;
                    auto tl = sdl::mapRGBA(pixelFormat, top * s + left * c);
                    auto tr = sdl::mapRGBA(pixelFormat, top * s + right * c);
                    auto bl = sdl::mapRGBA(pixelFormat, bot * s + left * c);
                    auto br = sdl::mapRGBA(pixelFormat, bot * s + right * c);
                    if (xr > 0)
                        if (yr > 0) {
                            surfaceBevelOut.pixel(x, y) = tl;
                            surfaceBevelIn.pixel(x, y) = br;
                            if (r2 > rn) {
                                surfaceNotchIn.pixel(x, y) = br;
                                surfaceNotchOut.pixel(x, y) = tl;
                            } else {
                                surfaceNotchIn.pixel(x, y) = tl;
                                surfaceNotchOut.pixel(x, y) = br;
                            }
                        } else {
                            surfaceBevelOut.pixel(x, y) = bl;
                            surfaceBevelIn.pixel(x, y) = tr;
                            if (r2 > rn) {
                                surfaceNotchIn.pixel(x, y) = tr;
                                surfaceNotchOut.pixel(x, y) = bl;
                            } else {
                                surfaceNotchIn.pixel(x, y) = bl;
                                surfaceNotchOut.pixel(x, y) = tr;
                            }
                        }
                    else if (yr > 0) {
                        surfaceBevelOut.pixel(x, y) = tr;
                        surfaceBevelIn.pixel(x, y) = bl;
                        if (r2 > rn) {
                            surfaceNotchIn.pixel(x, y) = bl;
                            surfaceNotchOut.pixel(x, y) = tr;
                        } else {
                            surfaceNotchIn.pixel(x, y) = tr;
                            surfaceNotchOut.pixel(x, y) = bl;
                        }
                    } else {
                        surfaceBevelOut.pixel(x, y) = br;
                        surfaceBevelIn.pixel(x, y) = tl;
                        if (r2 > rn) {
                            surfaceNotchIn.pixel(x, y) = tl;
                            surfaceNotchOut.pixel(x, y) = br;
                        } else {
                            surfaceNotchIn.pixel(x, y) = br;
                            surfaceNotchOut.pixel(x, y) = tl;
                        }
                    }
                    // inside the min radius is transparent
                } else {
                    auto pixel = color::set_a_value(surfaceBevelOut.pixel(x, y), 0);
                    surfaceBevelOut.pixel(x, y) = pixel;
                    surfaceBevelIn.pixel(x, y) = pixel;
                    surfaceNotchOut.pixel(x, y) = pixel;
                    surfaceNotchIn.pixel(x, y) = pixel;
                }
            }
        }

        sdl::Texture texture;

        texture = surfaceBevelOut.toTexture(renderer);
        sdl::TextureData bevelOutCornerTrim{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                            radius * 2, radius * 2};
        copyFullTexture(renderer, texture, bevelOutCornerTrim);
        mImageRepository.setImage(BevelOutRoundCorners, std::move(bevelOutCornerTrim));

        texture = surfaceBevelIn.toTexture(renderer);
        sdl::TextureData bevelInCornerTrim{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                           radius * 2, radius * 2};
        copyFullTexture(renderer, texture, bevelInCornerTrim);
        mImageRepository.setImage(BevelInRoundCorners, std::move(bevelInCornerTrim));

        texture = surfaceNotchOut.toTexture(renderer);
        sdl::TextureData notchOutCornerTrim{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                            radius * 2, radius * 2};
        copyFullTexture(renderer, texture, notchOutCornerTrim);
        mImageRepository.setImage(NotchOutRoundCorners, std::move(notchOutCornerTrim));

        texture = surfaceNotchIn.toTexture(renderer);
        sdl::TextureData notchInCornerTrim{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                           radius * 2, radius * 2};
        copyFullTexture(renderer, texture, notchInCornerTrim);
        mImageRepository.setImage(NotchInRoundCorners, std::move(notchInCornerTrim));

    }

    void Rose::createCenters(sdl::Renderer &renderer, int scale, int radius) {
        static constexpr std::array<RoseImageId, 8> centerList{CenterRoundBaseColor, CenterRoundInvertColor,
                                                               CenterRoundRedColor, CenterRoundGreenColor,
                                                               CenterRoundBlueColor, CenterRoundYellowColor,
                                                               CenterSquareBaseColor, CenterSquareInvertColor};

        int rS = radius * scale;        // The radius at scale
        auto size = 2 * rS;
        auto r0 = rS * rS;              // the maximum radius squared of the border corner

        for (auto id : centerList) {
            color::RGBA color;
            switch (id) {
                case CenterRoundBaseColor:
                case CenterSquareBaseColor:
                    color = mTheme.mBaseColor;
                    break;
                case CenterRoundInvertColor:
                case CenterSquareInvertColor:
                    color = mTheme.mInvertColor;
                    break;
                case CenterRoundRedColor:
                    color = color::RGBA{mTheme.mBaseColorHSLA.withHue(0).withMinSaturation(0.6)};
                    break;
                case CenterRoundGreenColor:
                    color = color::RGBA{mTheme.mBaseColorHSLA.withHue(120).withMinSaturation(0.6)};
                    break;
                case CenterRoundBlueColor:
                    color = color::RGBA{mTheme.mBaseColorHSLA.withHue(240).withMinSaturation(0.6)};
                    break;
                case CenterRoundYellowColor:
                    color = color::RGBA{mTheme.mBaseColorHSLA.withHue(60).withMinSaturation(0.6)};
                    break;
                default:
                    throw (RoseLogicError("Center id not handled."));
            }

            sdl::Texture texture;

            sdl::Surface surface{size, size,32, rmask, gmask, bmask, amask};
            surface.setBlendMode(SDL_BLENDMODE_BLEND);
            surface.fillRectangle(color);

            switch (id) {
                case CenterSquareBaseColor:
                case CenterSquareInvertColor:
                    surface.fillRectangle(color);
                    break;
                case CenterRoundBaseColor:
                case CenterRoundInvertColor:
                case CenterRoundRedColor:
                case CenterRoundGreenColor:
                case CenterRoundYellowColor:
                case CenterRoundBlueColor:
                    for (int x = 0; x < size; ++x) {
                        for (int y = 0; y < size; ++y) {
                            int xr = rS - x;                // x component of the current radius
                            int yr = rS - y;                // y component of the current radius
                            int r2 = xr * xr + yr * yr;

                            if (r2 > r0) {
                                surface.pixel(x, y) = color::set_a_value(surface.pixel(x, y), 0);
                            }
                        }
                    }
                    break;
                default:
                    throw (RoseLogicError("Center id not handled."));
            }

            texture = surface.toTexture(renderer);
            sdl::TextureData center{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                      radius * 2, radius * 2};
            sdl::RenderTargetGuard renderTargetGuard(renderer, center);
            renderer.renderCopy(texture);
            center.setBlendMOde(SDL_BLENDMODE_BLEND);
            mImageRepository.setImage(id, std::move(center));
        }
    }

    shared_ptr<Window> Rose::createPopup() {
        auto window = std::make_shared<PopupWindow>(shared_from_this());
        window->initializeComposite();
        mWindowList.emplace_back(window);
        return window;
    }
}
