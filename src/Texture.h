/**
 * @file Texture.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-20
 */

#pragma once

#include <filesystem>
#include <memory>
#include <SDL.h>
#include "Font.h"
#include "ScreenMetrics.h"
#include "Surface.h"


namespace rose::sdl {

    class Renderer;

    /**
     * @brief A functor to destroy an SDL_Texture in a std::unique_ptr (rose::sdl::Texture)
     */
    class TextureDestroy {
    public:
        /**
         * @brief Call the SDL API to destroy an SDL_Texture.
         * @param sdlTexture A pointer to the SDL_Texture to destroy.
         */
        void operator()(SDL_Texture *sdlTexture) {
            SDL_DestroyTexture(sdlTexture);
        }
    };

    /**
     * @class Texture
     * @brief An encapsulation of the SDL_Texture structure.
     */
    class Texture : public std::unique_ptr<SDL_Texture, TextureDestroy> {
    public:
        Texture() = default;

        ~Texture() = default;

        Texture(const Texture &) = delete;

        Texture(Texture &&) = default;

        Texture &operator=(const Texture &) = delete;

        Texture &operator=(Texture &&texture) = default;

        explicit Texture(SDL_Texture *texture) : Texture() { reset(texture); }

        /**
         * @breif Constructor
         * @param renderer The Renderer to use.
         * @param format The pixel format from SDL_PixelFormatEnum.
         * @param access The Texture access from SDL_TextureAccess.
         * @param width The width of the texture.
         * @param height The height of the texture.
         */
        Texture(Renderer &renderer, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height);

        int setBlendMOde(SDL_BlendMode blendMode) {
            return SDL_SetTextureBlendMode(get(), blendMode);
        }

        [[nodiscard]] std::tuple<int, int> getSize() const noexcept {
            int w, h;
            SDL_QueryTexture(get(), nullptr, nullptr, &w, &h);
            return std::make_tuple(w, h);
        }
    };

    /**
     * @class TextureData
     * @brief A Texture with associated meta data.
     */
    class TextureData : public Texture {
    public:
        /**
         * @brief Default constructor.
         * @details Constructs an empty, expired, dirty objects.
         */
        TextureData() = default;

        TextureData(const TextureData &) = delete; ///< Deleted copy constructor.
        TextureData &operator=(const TextureData &) = delete; ///< Deleted const copy constructor.

        /**
         * @breif Constructor
         * @param renderer The Renderer to use.
         * @param format The pixel format from SDL_PixelFormatEnum.
         * @param access The Texture access from SDL_TextureAccess.
         * @param width The width of the texture.
         * @param height The height of the texture.
         */
        TextureData(Renderer &renderer, SDL_PixelFormatEnum format, SDL_TextureAccess access, int width, int height)
                : Texture(renderer, format, access, width, height) {
            setMetaData();
        }

        /**
         * @brief Move constructor
         * @details The Texture meta data is set from the new texture.
         * @param texture the texture to move into the TextureData object
         */
        explicit TextureData(Texture &&texture) noexcept: TextureData() {
            (*this) = texture.release();
            setMetaData();
        }

        /**
         * @brief Construct a new TextureData from an SDL_Texture pointer.
         * @details The SDL_Texture pointer is used to reset the std::unique_ptr and the
         * texture meta data is set from the texture.
         * @param texture SDL_Texture*
         */
        explicit TextureData(SDL_Texture *texture) noexcept: Texture(texture) {
            setMetaData();
        }

        /**
         * @brief Move constructor.
         * @param other the object to move.
         */
        TextureData(TextureData &&other) noexcept = default;

        /**
         * @brief Move assignment.
         * @param other the object to assign.
         * @return A reference to this object.
         */
        TextureData &operator=(TextureData &&other) noexcept = default;

        /**
         * @brief Assign an SDL_Texture pointer to this object.
         * @details The Texture meta data is set from the new texture.
         * @param texture the pointer to the texture.
         * @return A reference to this object.
         */
        TextureData &operator=(SDL_Texture *texture) {
            reset(texture);
            setMetaData();
            return *this;
        }

        /**
         * @brief Determine if the texture has expired.
         * @return true if mLife is in the past.
         */
        [[nodiscard]] bool expired() const noexcept {
            return operator bool() && mLife < std::chrono::system_clock::now();
        }

        /**
         * @brief Determine if the texture is dirty.
         * @return return the internal dirty flag.
         */
        [[nodiscard]] bool dirty() const noexcept { return mDirty; }

        /**
         * @brief Set the texture dirty status.
         * @param dirty the value to copy into the internal dirty flag.
         */
        void setDirty(bool dirty) { mDirty = dirty; }

        /**
         * @brief Get the path member.
         * @return a view of the path member variable.
         */
        std::string_view getPath() { return mPath; }

        /**
         * @brief Get the path member from a const TextureData object.
         * @return a const view of the path member varialbe.
         */
        [[nodiscard]] std::string_view getPath() const { return mPath; }

        /**
         * @brief Assign a new value to the path member.
         * @tparam StringType any type which may be assigned to a std::string
         * @param path the new value.
         */
        template<typename StringType>
        void setPath(const StringType path) { mPath = path; }

        /**
         * @brief Get the URI member.
         * @return a view of the URI member variable.
         */
        std::string_view getURI() { return mURI; }

        /**
         * @brief Get the URI member from a const TextureData object.
         * @return a const view of the URI member variable.
         */
        [[nodiscard]] std::string_view getURI() const { return mURI; }

        /**
         * @brief Assign a new value to the URI member.
         * @tparam StringType any type which may be assigned to a std::string
         * @param URI the new value.
         */
        template<typename StringType>
        void setURI(const StringType URI) { mURI = URI; }

        /**
         * @brief Get the Name member.
         * @return a view of the Name member variable.
         */
        std::string_view getName() { return mName; }

        /**
         * @brief Get the Name member from a const TextureData object.
         * @return a const view of the Name member variable.
         */
        [[nodiscard]] std::string_view getName() const { return mName; }

        /**
         * @brief Assign a new value to the Name member.
         * @tparam StringType any type which may be assigned to a std::string
         * @param Name the new value.
         */
        template<typename StringType>
        void setName(const StringType Name) { mName = Name; }

        /**
         * @brief Set the time the texture was loaded
         * @param loaded the time point of loading.
         */
        void setLoadedTime(const std::chrono::time_point<std::chrono::system_clock> loaded) { mLoaded = loaded; }

        /**
         * @brief Set the scale value associated with the texture.
         * @param scale the scale factor
         */
        void setScale(int scale) { mScale = scale; }

        /**
         * @brief Get the scale value associate with the texture.
         * @return the scale factor
         */
        [[nodiscard]] int getScale() const { return mScale; }

        /**
         * @brief Get the icon size.
         * @return a SizeInt.
         */
        [[nodiscard]] Size getSize() const { return mSize; }

        /**
         * @brief Populate an SDL_Rect with size of the texture with a Zero position.
         * @param rect A reference to an SDL_Rect.
         */
        void getRectangle(SDL_Rect &rect) const {
            rect.x = rect.y = 0;
            rect.w = mSize.width();
            rect.h = mSize.height();
        }

        /**
         * @brief Return a Rectangle with the size of the texture and Zero position.
         * @return The Rectangle.
         */
        [[nodiscard]] Rectangle getRectangle() const {
            return Rectangle{0, 0, mSize.width(), mSize.height()};
        }

        /**
         * @brief Set the size of the Texture.
         * @details Not recommended to be used by application. The size is set when a Texture is assigned to
         * the object.
         * @param size The Texture SizeInt.
         */
        void setSize(Size size) { mSize = size; }

    protected:
        /**
         * @brief Set the class meta data from the texture.
         */
        void setMetaData() {
            int w, h;
            SDL_QueryTexture(get(), &mFormat, &mAccess, &w, &h);
            mSize = Size{w, h};
            mDirty = false;
        }

        Size mSize;      ///< The size of the texture.
        int mScale{
                1};      ///< Store a scale value for use by library or application programs. The Sprite class uses this field.
        uint32_t mFormat{}; ///< The Format of the texture.
        int mAccess{};      ///< The Access mode of the texture.
        std::string mPath;  ///< The path name to the source of the texture on the local file system.
        std::string mURI;   ///< The URI of the source of the texture.

        /**
         * @brief A name this image may be known by.
         * @details Use of this field is at the discretion of the user application.
         */
        std::string mName;
        bool mDirty{true};  ///< A flag indicating the image needs to be regenerated or reloaded.

        std::chrono::time_point<std::chrono::system_clock> mLoaded{}, //!< The time the texture was generated or loaded.
        mLife{}; //!< The time when the texture should be regenerated or reloaded.
    };

    /**
     * @brief Create texture data from a file system path.
     * @param renderer The Renderer to use.
     * @param filePath The std::filesystem::path pointing to the file to load.
     * @return The TextureData object
     */
    TextureData CreateTextureFromFile(Renderer &renderer, std::filesystem::path &filePath);

    /**
     * @brief  Create texture data from a file system path.
     * @tparam String A type of a string like value.
     * @param renderer The Renderer to use.
     * @param dirPath The std::filesystem::path pointing to the director to load the file from.
     * @param fileName The file name in a string like value.
     * @return
     */
    template<typename String>
    TextureData CreateTextureFromFile(Renderer &renderer, std::filesystem::path &dirPath, String fileName) {
        auto filePath = dirPath;
        filePath.append(fileName);
        return CreateTextureFromFile(renderer, filePath);
    }

    class GradientTexture;

    /**
     * @class GradientTexture
     * @brief A gradient rendering kernel.
     * @details This generates a small texture, which when rendered into a larger area is interpolated by
     * the graphics hardware to a gradient across the destination.
     * See: https://stackoverflow.com/questions/20348616/how-to-create-a-color-gradient-in-sdl
     */
    class GradientTexture : public Texture {
    public:
        GradientTexture() = default;

        ~GradientTexture() = default;

        GradientTexture(const GradientTexture &) = delete;

        GradientTexture(GradientTexture &&) = default;

        GradientTexture &operator=(const GradientTexture &) = delete;

        GradientTexture &operator=(GradientTexture &&) = delete;

        GradientTexture(Renderer &renderer, const color::RGBA &topLeft, const color::RGBA &topRight,
                        const color::RGBA &bottomLeft, const color::RGBA &bottomRight);

        GradientTexture(Renderer &renderer, const color::RGBA &start, const color::RGBA &end,
                        Orientation orientation = Orientation::Horizontal);

        void setColors(const color::RGBA &topLeft, const color::RGBA &topRight, const color::RGBA &bottomLeft,
                       const color::RGBA &bottomRight);

        void setColors(const color::RGBA &start, const color::RGBA &end,
                       Orientation orientation = Orientation::Horizontal);
    };

    /**
     * @class GradientScale
     * @brief A type of texture that if stretch across a destination Rectangle in rendering, will create a
     * gradient coloring.
     */
    class GradientScale : public Texture {
    protected:

    public:
        GradientScale() = default;

        ~GradientScale() = default;

        GradientScale(const GradientScale &) = delete;

        GradientScale(GradientScale &&) = default;

        GradientScale &operator=(const GradientScale &) = delete;

        GradientScale &operator=(GradientScale &&) = delete;
    };

    /**
     * @brief A functor to destroy an SDL_PixelFormat in a std::unique_ptr (rose::sdl::PixelFormat)
     */
    class PixelFormatDestroy {
    public:
        /**
         * @brief Call the SDL API to free an SDL_PixelFormat.
         * @param sdlPixelFormat A pointer to the SDL_PixelFormat to free.
         */
        void operator()(SDL_PixelFormat *sdlPixelFormat) {
            SDL_FreeFormat(sdlPixelFormat);
        }
    };

    /**
     * @class PixelFormat
     * @brief An encapsulation of an SDL_PixelFormat.
     */
    class PixelFormat : public std::unique_ptr<SDL_PixelFormat, PixelFormatDestroy> {
    public:
        PixelFormat() = default;

        /**
         * @brief Constructor
         * @param sdlPixelFormat An existing SDL_PixelFormat structure.
         * @details The SDL_PixelFormat structure becomes owned by the PixelFormat class and will be freed
         * when the PixelFormat goes out of scop.
         */
        explicit PixelFormat(SDL_PixelFormat *sdlPixelFormat)
                : std::unique_ptr<SDL_PixelFormat, PixelFormatDestroy>(sdlPixelFormat) {}

        /**
         * @brief Constructor
         * @param format The pixel format, a value from SDL_PixelFormatEnum.
         */
        explicit PixelFormat(SDL_PixelFormatEnum format) : PixelFormat(SDL_AllocFormat(format)) {}
    };

    /**
     * @brief Map a color::RGBA to a uint32_t.
     * @details This function will allocate a PixelFormat, call mapRGBA(PixelFormat,color::RGBA), then destroy a PixelFormat.
     * @param pixelFormat The SDL PixelFormat defined constant.
     * @param color The Colour to map.
     * @return The mapped Colour.
     */
    uint32_t mapRGBA(SDL_PixelFormatEnum pixelFormat, const color::RGBA &color);

    /**
     * @brief Map a color::RGBA to a uint32_t.
     * @param pixelFormat A reference to a PixelFormat.
     * @param color The color to map.
     * @return The mapped color.
     */
    uint32_t mapRGBA(PixelFormat &pixelFormat, const color::RGBA &color);

    /**
     * @brief Map a color::RGBA to a uint32_t.
     * @param format A pointer to an SDL_PixelFormat (from a Surface object for example).
     * @param color The color::RGBA to map.
     * @return The color::RGBA mapped to a uint32_t
     */
    uint32_t mapRGBA(SDL_PixelFormat *format, const color::RGBA &color);

    color::RGBA getRGBA(PixelFormat &pixelFormat, uint32_t pixel);

    color::RGBA getRGBA(SDL_PixelFormat *format, uint32_t pixel);

    /**
     * @brief Render text to a Texture.
     * @tparam Color The type of the color argument.
     * @param renderer The renderer to use.
     * @param font The font to use.
     * @param text The text to render
     * @param color The foreground Color.
     * @return The Texture.
     */
    template<typename Color>
    inline sdl::Texture
    renderTextureBlended(sdl::Renderer &renderer, FontPointer &font, const std::string &text, const Color &color) {
        static_assert(std::is_same_v<Color, ::rose::color::RGBA> ||
                      std::is_same_v<Color, ::rose::color::HSVA> ||
                      std::is_same_v<Color, SDL_Color>,
                      "Color type is not supported.");
        if constexpr (std::is_same_v<Color, ::rose::color::RGBA>) {
            auto fg = color.toSdlColor();
            return renderTextureBlended(renderer, font, text, fg);
        } else if constexpr (std::is_same_v<Color, ::rose::color::HSVA>) {
            auto fg = color::RGBA{color}.toSdlColor();
            return renderTextureBlended(renderer, font, text, fg);
        } else if constexpr (std::is_same_v<Color, SDL_Color>) {
            sdl::Surface surface{TTF_RenderText_Blended(font.get(), text.c_str(), color)};
            return sdl::Texture{SDL_CreateTextureFromSurface(renderer.get(), surface.get())};
        }
    }

    /**
     * @brief Render text to a Texture.
     * @tparam Color The type of the color argument.
     * @param renderer The renderer to use.
     * @param font The font to use.
     * @param text The text to render
     * @param color The foreground Color.
     * @return The Texture.
     */
    template<typename Color>
    sdl::Texture
    inline renderTextureBlendedUTF8(sdl::Renderer &renderer, FontPointer &font, const std::string &text,
                                    const Color &color) {
        static_assert(std::is_same_v<Color, ::rose::color::RGBA> ||
                      std::is_same_v<Color, ::rose::color::HSVA> ||
                      std::is_same_v<Color, SDL_Color>,
                      "Color type is not supported.");
        if constexpr (std::is_same_v<Color, ::rose::color::RGBA>) {
            auto fg = color.toSdlColor();
            return renderTextureBlendedUTF8(renderer, font, text, fg);
        } else if constexpr (std::is_same_v<Color, ::rose::color::HSVA>) {
            auto fg = color::RGBA{color}.toSdlColor();
            return renderTextureBlendedUTF8(renderer, font, text, fg);
        } else if constexpr (std::is_same_v<Color, SDL_Color>) {
            sdl::Surface surface{TTF_RenderUTF8_Blended(font.get(), text.c_str(), color)};
            return sdl::Texture{SDL_CreateTextureFromSurface(renderer.get(), surface.get())};
        }
    }

}

