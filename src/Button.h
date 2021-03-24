/**
 * @file Button.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#pragma once

#include "Frame.h"
#include "PointerInteractions.h"
#include "Text.h"
#include "Image.h"

namespace rose {

    /**
     * @class ButtonFrame
     * @brief A frame that supports ButtonSemantics
     */
    class ButtonFrame : public Frame {
    protected:
        ButtonSemantics mButtonSemantics;

        ButtonDisplayCallback mButtonDisplayCallback{};
        ButtonStateChangeCallback mButtonStateChangeCallback{};

    public:
        ~ButtonFrame() override = default;

        ButtonFrame(const ButtonFrame&) = delete;

        ButtonFrame(ButtonFrame&&) = delete;

        ButtonFrame& operator=(const ButtonFrame&) = delete;

        ButtonFrame& operator=(ButtonFrame&&) = delete;

        explicit ButtonFrame() noexcept;

    };

    /**
     * @class TextButton
     * @brief
     */
    class TextButton : public ButtonFrame , public Text {
    protected:

    public:
        ~TextButton() override = default;

        TextButton(const TextButton&) = delete;

        TextButton(TextButton&&) = delete;

        TextButton& operator=(const TextButton&) = delete;

        TextButton& operator=(TextButton&&) = delete;

        TextButton() noexcept;

        explicit TextButton(const std::string& text, ButtonType buttonType = ButtonType::PushButton);

        explicit TextButton(const char* text, ButtonType buttonType = ButtonType::PushButton) : TextButton(std::string{text}, buttonType) {}

        explicit TextButton(const std::string_view& text, ButtonType buttonType = ButtonType::PushButton) : TextButton(std::string{text}, buttonType) {}

        explicit TextButton(const Id& id, ButtonType buttonType = ButtonType::PushButton);

        /**
         * @brief Layout the text button.
         * @param context The graphics Context to use.
         * @param screenRect The available screen Rectangle.
         * @return The requested screen Rectangle.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect);

        /**
         * @brief Draw the text button.
         * @details The text is rendered relative the the parent Container.
         * @param context The graphics context to use.
         * @param containerPosition The position of the parent Container.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;
    };

    class TextButtonLayoutManager : public LayoutManager {
    protected:
        TextButton& mTextButton;

    public:

        TextButtonLayoutManager() = delete;
        ~TextButtonLayoutManager() override = default;

        explicit TextButtonLayoutManager(TextButton& textButton);

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };

    class ImageButton : public ButtonFrame, public Image {
    protected:
        ImageId mImageId{ImageId::ThreeDots};

    public:
        ~ImageButton() override = default;

        ImageButton(const ImageButton&) = delete;

        ImageButton(ImageButton&&) = delete;

        ImageButton& operator=(const ImageButton&) = delete;

        ImageButton& operator=(ImageButton&&) = delete;

        ImageButton() noexcept;

        explicit ImageButton(ImageId imageId) noexcept;

    };
}

