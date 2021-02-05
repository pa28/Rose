/**
 * @file Font.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @date 2020-10-21
 * @version 1.0
 * @brief True Type Fonts and supporting types and functions.
 */
/*
    Another significant redesign to update the coding standards to C++17,
    reduce the amount of bare pointer handling (especially in user code),
    and focus on the RaspberryPi environment.
    
    License terms for the changes as well as the base nanogui-sdl code are
    contained int the LICENSE.txt file.
    
    A significant redesign of this code was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <filesystem>
#include <optional>
#include <memory>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <map>
#include "Theme.h"
#include "Utilities.h"

namespace rose {

    /**
     * @class FontDestroy
     * @brief A functor to destroy a TTF_Font
     */
    class FontDestroy {
    public:
        /**
         * @brief Destroy a TTF_Font pointer
         * @param ttfFont the Font pointer
         */
        void operator()(TTF_Font *ttfFont) {
            TTF_CloseFont(ttfFont);
        }
    };

    using FontPointer = std::shared_ptr<TTF_Font>;                  ///< Type for TTF smart pointer
    using FontCacheKey = std::pair<std::string, int>;               ///< Type for TTF cache key
    using FontCacheStore = std::map<FontCacheKey, FontPointer>;     ///< Type for TTF cache store

    /**
     * @brief Get the size of a UTF8 string.
     * @param fontPointer The font to use.
     * @param text The string
     * @return A std::tuple<int,int> with the width and height of the text.
     */
    inline auto textSizeUTF8(FontPointer &fontPointer, const std::string &text) {
        int w, h;
        TTF_SizeUTF8(fontPointer.get(), text.c_str(), &w, &h);
        return std::make_tuple(w, h);
    }

    /**
     * @class FontCache
     * @brief Cache storage for requested fonts.
     */
    class FontCache {
    public:
        ~FontCache() = default;

        FontCache() = default;

        /**
         * @brief Locate a font file.
         * @tparam StringType the type of fontName
         * @param path The path to start search from
         * @param fontName the Font name.
         * @return a std::optional<std::filesystem::path> of the font file.
         */
        template<typename StringType>
        std::optional<std::filesystem::path> locateFont(std::filesystem::path &path, StringType fontName) {
            for (auto &p : std::filesystem::recursive_directory_iterator(path)) {
                if (p.path().stem() == fontName && p.is_regular_file()) {
                    return p.path();
                }
            }
            return std::nullopt;
        }

        /**
         * @brief Find a font name in the font name cache
         * @tparam StringType the type of fontName
         * @param fontName the Font name
         * @return a std::optional<std::filesystem::path> of the font file for the Font.
         */
        template<typename StringType>
        std::optional<std::filesystem::path> getFontPath(StringType fontName) {
            if (auto found = mFontPathMap.find(fontName); found != mFontPathMap.end())
                return found->second;
            std::filesystem::path rootPath{Theme::dFontRootPath};
            auto fontPath = locateFont(rootPath, fontName);
            if (fontPath) {
                mFontPathMap[fontName] = fontPath.value();
//                std::cout << __PRETTY_FUNCTION__ << ' ' << fontPath.value() << '\n';
                return fontPath;
            }
            return std::nullopt;
        }

        /**
         * @brief Get a FontPointer to a named Font of a specific point size.
         * @tparam StringType the type of the Font name passed in.
         * @param fontName The requested font name.
         * @param ptSize The requested point size
         * @return a std::optional<FontPointer>
         */
        template<typename StringType>
        std::optional<FontPointer> getFont(StringType fontName, int ptSize) {
            if (auto found = mFontCache.find(FontCacheKey{fontName, ptSize}); found != mFontCache.end()) {
                return found->second;
            }

            if (auto fontPath = getFontPath(fontName); fontPath) {
                FontPointer fontPointer{TTF_OpenFont(fontPath.value().c_str(), ptSize), FontDestroy{}};
                auto font = mFontCache.emplace(FontCacheKey{fontName, ptSize}, fontPointer);
                if (font.second) {
                    return font.first->second;
                }
            }

            return std::nullopt;
        }

    protected:
        std::map<std::string, std::filesystem::path> mFontPathMap;  ///< The font file path cache

        std::map<FontCacheKey, FontPointer> mFontCache;             ///< The font cache
    };

    inline std::tuple<int, int, int, int, int> getGlyphMetrics(FontPointer &font, char glyph) {
        int minx{}, maxx{}, miny{}, maxy{}, advance{};
        if (font)
            TTF_GlyphMetrics(font.get(), glyph, &minx, &maxx, &miny, &maxy, &advance);
        return std::make_tuple(minx, maxx, miny, maxy, advance);
    }

    /**
     * @brief Get the font metrics of the current font.
     * @details See <a href="https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_29.html#SEC29">TTF_FontHeight</a>
     * et al.
     * @return a std::tuple with font height, font ascent, font descent, and font line skip.
     */
    inline auto getFontMetrics(FontPointer &font) {
        util::FontMetrics fontMetrics{};
        fontMetrics.fontHeight = TTF_FontHeight(font.get());
        fontMetrics.fontAscent = TTF_FontAscent(font.get());
        fontMetrics.fontDescent = TTF_FontDescent(font.get());
        fontMetrics.fontLineSkip = TTF_FontLineSkip(font.get());
        return fontMetrics;
    }

    /**
     * @brief Fetch a font.
     * @details If the font at the size requested is in the cache, the cached value is returned. If the font
     * can't be found the default font "FreeSans" is tried. If that can't be found and exception is thrown.
     * @param fontCache The FontCache.
     * @param fontName The name of the font.
     * @param fontSize The point size of the font in pixels.
     * @return The font.
     */
    inline auto fetchFont(FontCache &fontCache, const std::string &fontName, int fontSize) {
        auto font = fontCache.getFont(fontName, fontSize);
        if (!font) {
            font = fontCache.getFont("FreeSans", fontSize);
            if (!font)
                throw std::runtime_error(util::StringCompositor("Neither font", fontName, " nor default font 'FreeSans' found: ",
                                                          FILE_LOC));
        }
        return font;
    }

}



