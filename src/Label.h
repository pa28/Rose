/** @file Label.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-15
  *
  * Another significant redesign to update the coding standards to C++17,
  * reduce the amount of bare pointer handling (especially in user code),
  * and focus on the RaspberryPi environment.
  *
  * License terms for the changes as well as the base nanogui-sdl code are
  * contained int the LICENSE.txt file.
  *
  * A significant redesign of this code was contributed by Christian Schueller.
  *
  * Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
  * Adaptation for SDL by Dalerank <dalerankn8@gmail.com>
  *
  * All rights reserved. Use of this source code is governed by a
  * BSD-style license that can be found in the LICENSE.txt file.
  *
  * @brief The Label Class.
  */

#pragma once

#include <utility>

#include "Color.h"
#include "Rose.h"

namespace rose {

    /**
     * @class Label
     * @brief A Widget to display short text with an optional badge which may be on the left or right.
     * @details Badges are taken from the Rose ImageRepository. The application may build standard icon
     * badges by calling Ross::createStandardIcons() at the appropriate time during start up. Identifiers
     * for these standard icons come from rose::RoseImageId. See Label::mBadge.
     */
    class Label : public Widget {
    protected:
        std::string mText{};                    ///< The text displayed on the label
        ImageId mBadge{RoseImageInvalid};       ///< The badge displayed, RoseImageInvalid == none
        ImageId mBadgeCenter{RoseImageInvalid}; ///< The image for the center of a border badge.
        bool mBadgeRight{};                     ///< True if the badge is to be right of the text
        int mFontSize{};                        ///< The text font point size
        std::string mFontName{};                ///< The text font name
        color::RGBA mTextColor{};               ///< The text colour
        Size mTextSize{};                    ///< The rendered size of the text
        FontMetrics mFontMetrics{};             ///< The metrics of the text font
        FontMetrics mIconFontMetrics{};         ///< The metrics of the icon font
        std::optional<FontPointer> mFont{};     ///< The text font

        sdl::Texture mTexture{};                ///< The text texture
        bool mTextureDirty{true};               ///< True when the texture does not match the text
        bool mBadgeDirty{true};                 ///< True when the badge metrics do not match the badge selection

        int mLabelBadgeSpace{};                 ///< Space between the badge and text in pixels.

        /**
         * @brief Fetch the font when needed.
         */
        void fetchFont();

        /**
         * @brief Create the text texture.
         * @param renderer the rendere to use
         */
        void createTexture(sdl::Renderer &renderer);

        /**
         * @brief Create a composite texture of (possibly empty) text and badge
         * @param renderer
         */
        void compositeBadge(sdl::Renderer &renderer);

    public:
        Label();
        ~Label() override = default;
        Label(Label &&) = delete;
        Label(const Label &) = delete;
        Label& operator=(Label &&) = delete;
        Label& operator=(const Label &) = delete;

        /**
         * @brief Create a Label widget with text and an optional badge
         * @param text the text label.
         * @param badge the badge image id.
         */
        explicit Label(const std::string &text, RoseImageId badge = RoseImageId::RoseImageInvalid);

        /**
         * @brief Create a Label widget with text and an optional badge
         * @param text the text label.
         * @param badge the badge image id.
         */
        explicit Label(const std::string_view text, RoseImageId badge = RoseImageId::RoseImageInvalid)
                : Label(std::string{text}, badge) {}

        /**
         * @brief Create a Label widget with text and an optional badge
         * @param text the text label.
         * @param badge the badge image id.
         */
        explicit Label(const char *text, RoseImageId badge = RoseImageId::RoseImageInvalid)
                : Label(std::string{text}, badge) {}

        /**
         * @brief Determine the desired size of the label with text and badge.
         * @param renderer the Renderer to use if needed.
         * @param available The Rectangle available for layout.
         * @return Rectangle The layout Rectangle.
         */
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /**
         * @brief Draw the Label
         * @param renderer the renderer used to draw.
         * @param parentRect The layout size computed for this widget
         * @param parentPosition The layout position computed for this widget
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        void setLabelHorizontalAlignment(LabelHorizontalAlignment alignment) { mLayoutHints.mLabelHorAlign = alignment; }

        /**
         * @brief Set the horizontal alignment of the text within the label.
         * @param alignment The kind of horizontal alignment.
         */
        void setLabelVerticalAlignment(LabelVerticalAlignment alignment) { mLayoutHints.mLabelVerAlign = alignment; }

        /**
         * @brief Set the text font size.
         * @details Setting the font size invalidates the text texture and the font.
         * @param fontSize
         */
        void setFontSize(int fontSize) {
            mFontSize = fontSize;
            mTextureDirty = true;
            mFont = nullopt;
            setNeedsDrawing();
        }

        /**
         * @brief Get the text font size.
         * @return the font point size (pixels).
         */
        int getFontSize() const { return mFontSize; }

        /**
         * @brief Set the text of the label
         * @param text The text
         */
        void setText(const std::string &text) {
            mText = text;
            mTextureDirty = true;
            setNeedsDrawing();
        }

        std::shared_ptr<Slot<std::string>> mTextRx{};   ///< A slot to receive Label text updates.

        /**
         * @brief Get the text of the label
         * @return the text.
         */
        const std::string &getText() const { return mText; }

        /**
         * @brief Set the text of the label and return a std::shared_ptr<Label>
         * @param text The new text.
         * @return std::shared_ptr<Label> to this
         */
        std::shared_ptr<Label> withText(const std::string &text) {
            setText(text);
            return getWidget<Label>();
        }

        /**
         * @brief Set the text font size and return a std::shared_ptr to this Label.
         * @details See setFontSize(int fontSize)
         * @param fontSize
         * @return a std::shared_ptr to this Label.
         */
        std::shared_ptr<Label> withFontSize(int fontSize) {
            setFontSize(fontSize);
            return getWidget<Label>();
        }

        /**
         * @brief Set the font name.
         * @param fontName
         */
        void setFontName(const std::string &fontName) {
            mFontName = fontName;
            mTextureDirty = true;
            mFont = nullopt;
            setNeedsDrawing();
        }

        /**
         * @brief Get the font name.
         * @return the font name
         */
        const std::string &getFontName() const { return mFontName; }

        /**
         * @brief Set the ImageId of the Badge.
         * @param imageId An ImageId or RoseImageId.
         */
        void setBadge(ImageId imageId) noexcept { mBadge = imageId; mBadgeDirty = true; }

        /**
         * @brief Get the ImageId of the Badge.
         * @return The ImageId which may be a RoseImageId.
         */
        ImageId getBadge() const noexcept { return mBadge; }

        /**
         * @brief Set whether or not the Badge is displayed on the right.
         * @param badgeRight True to display the Badge on the right, false for left.
         */
        void setBadgeRight(bool badgeRight) noexcept { mBadgeRight = badgeRight; mTextureDirty = true; }

        /**
         * @brief Set the font name and return a std::shared_ptr<Label>
         * @param fontName
         * @return std::shared_ptr<Label>
         */
        std::shared_ptr<Label> withFontName(const std::string &fontName) {
            setFontName(fontName);
            return getWidget<Label>();
        }

        /**
         * @brief Get the glyph metrics using the current font.
         * @details See <a href="https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_38.html">TTF_GlyphMetrics</a>
         * @param glyph the glyph to examin.
         * @return a std::tuple with minx, maxx, miny, maxy, and advance
         */
        std::tuple<int, int, int, int, int> getGlyphMetrics(char glyph);

        /**
         * @brief Get the font metrics of the current font.
         * @details See <a href="https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_29.html#SEC29">TTF_FontHeight</a>
         * et al.
         * @return a std::tuple with font height, font ascent, font descent, and font line skip.
         */
        std::tuple<int, int, int, int> getFontMetrics();

        /**
         * @brief See Widget::initializeComposite
         */
        void initializeComposite() override;
    };

    /**
     * @brief Type specifier to pass a Font size to a Widget through a manipulator.
     */
    using FontSize = int;

    /**
     * @struct FontName
     * @brief A structure to pass a Font name to a Widget through a manipulator.
     */
    struct FontName {
        std::string fontName{};
        FontName() : fontName() {}
        explicit FontName(const char *string) : fontName(string) {}
        explicit FontName(const std::string_view stringView) : fontName(stringView) {}
        explicit FontName(std::string string) : fontName(std::move(string)) {}
    };
}

/**
 * @addtogroup WidgetManip
 * @{
 */
/**
 * @brief A widget manipulator for setting the text on a widget.
 * @tparam WidgetClass The type of Widget
 * @param widget The Widget
 * @param text The text
 * @return The Widget
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const std::string &text) {
    widget->setText(text);
    return widget;
}

/**
 * @brief A widget manipulator for setting the text on a widget.
 * @tparam WidgetClass The type of Widget
 * @param widget The Widget
 * @param text The text
 * @return The Widget
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const std::string_view text) {
    widget->setText(std::string{text});
    return widget;
}

/**
 * @brief A widget manipulator for setting the text on a widget.
 * @tparam WidgetClass The type of Widget
 * @param widget The Widget
 * @param text The text
 * @return The Widget
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, const char *text) {
    widget->setText(std::string{text});
    return widget;
}

/**
 * @brief A widget manipulator to set horizontal alignment.
 * @tparam WidgetClass The type of the widget
 * @param widget The widget
 * @param alignment The kind of alingment
 * @return The widget
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass>
operator<<(std::shared_ptr<WidgetClass> widget, rose::LabelHorizontalAlignment alignment) {
    widget->setLabelHorizontalAlignment(alignment);
    return widget;
}

/**
 * @brief A Widget manipulator to set horizontal alignment.
 * @tparam WidgetClass The type of the widget
 * @param widget The widget
 * @param alignment The kind of alingment
 * @return The widget
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::LabelVerticalAlignment alignment) {
    widget->setLabelVerticalAlignment(alignment);
    return widget;
}

/**
 * @brief A Label manipulator to set the FontSize.
 * @tparam WidgetClass The type of Widget to manipulate.
 * @param widget The Widget to manipulate.
 * @param fontSize The font size in points (pixels).
 * @return The manipulated Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::FontSize fontSize) {
    widget->setFontSize(fontSize);
    return widget;
}

/**
 * @brief A Label manipulator to set the FontName
 * @tparam WidgetClass The type of Widget to manipulate.
 * @param widget The Widget to manipulate.
 * @param fontName Then name of the True Type font to use.
 * @return The manipulated Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::FontName fontName) {
    widget->setFontName(fontName.fontName);
    return widget;
}

/** @} */
