/**
 * @file Button.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#pragma once

#include <memory>
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
        std::unique_ptr<ButtonSemantics> mButtonSemantics{};

        ButtonDisplayCallback mButtonDisplayCallback{};
        ButtonStateChangeCallback mButtonStateChangeCallback{};

    public:
        ~ButtonFrame() override = default;

        ButtonFrame(const ButtonFrame&) = delete;

        ButtonFrame(ButtonFrame&&) = delete;

        ButtonFrame& operator=(const ButtonFrame&) = delete;

        ButtonFrame& operator=(ButtonFrame&&) = delete;

        explicit ButtonFrame(ButtonType buttonType) noexcept;

        static constexpr std::string_view id = "ButtonFrame";
        std::string_view nodeId() const noexcept override {
            return id;
        }
    };

    /**
     * @class TextButton
     * @brief
     */
    class TextButton : public ButtonFrame , public Text {
    protected:
        friend class TextButtonLayoutManager;

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect);

    public:
        ~TextButton() override = default;

        TextButton(const TextButton&) = delete;

        TextButton(TextButton&&) = delete;

        TextButton& operator=(const TextButton&) = delete;

        TextButton& operator=(TextButton&&) = delete;

        explicit TextButton(ButtonType buttonType = ButtonType::PushButton) noexcept;

        explicit TextButton(const std::string& text, ButtonType buttonType = ButtonType::PushButton);

        explicit TextButton(const char* text, ButtonType buttonType = ButtonType::PushButton)
            : TextButton(std::string{text}, buttonType) {}

        explicit TextButton(const std::string_view& text, ButtonType buttonType = ButtonType::PushButton)
            : TextButton(std::string{text}, buttonType) {}

        explicit TextButton(const Id &id, ButtonType buttonType = ButtonType::PushButton);

        explicit TextButton(const Id &id, ButtonStateChangeCallback stateChangeCB,
                            ButtonType buttonType = ButtonType::PushButton)
            : TextButton(id, buttonType) {
            mButtonSemantics->setButtonStateChangeCallback(std::move(stateChangeCB));
        }

        explicit TextButton(const std::string& text, ButtonStateChangeCallback stateChangeCB, ButtonType buttonType = ButtonType::PushButton)
                : TextButton(text, buttonType) {
            mButtonSemantics->setButtonStateChangeCallback(std::move(stateChangeCB));
        }

        explicit TextButton(const char* text, ButtonStateChangeCallback stateChangeCB, ButtonType buttonType = ButtonType::PushButton)
                : TextButton(std::string{text}, buttonType) {
            mButtonSemantics->setButtonStateChangeCallback(std::move(stateChangeCB));
        }

        explicit TextButton(const std::string_view& text, ButtonStateChangeCallback stateChangeCB, ButtonType buttonType = ButtonType::PushButton)
                : TextButton(std::string{text}, buttonType) {
            mButtonSemantics->setButtonStateChangeCallback(std::move(stateChangeCB));
        }

        static constexpr std::string_view id = "TextButton";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /**
         * @brief Layout the text button.
         * @param context The graphics Context to use.
         * @param screenRect The available screen Rectangle.
         * @return The requested screen Rectangle.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /**
         * @brief Draw the text button.
         * @details The text is rendered relative the the parent Container.
         * @param context The graphics context to use.
         * @param containerPosition The position of the parent Container.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;
    };

    class TextLabel : public TextButton {
    public:
        ~TextLabel() override = default;

        TextLabel(const TextLabel&) = delete;

        TextLabel(TextLabel&&) = delete;

        TextLabel& operator=(const TextLabel&) = delete;

        TextLabel& operator=(TextLabel&&) = delete;

        explicit TextLabel() noexcept : TextButton(ButtonType::Label) {}

        explicit TextLabel(const std::string& text) noexcept : TextButton(text, ButtonType::Label) {}

        explicit TextLabel(const char* text)
                : TextButton(std::string{text}, ButtonType::Label) {}

        explicit TextLabel(const std::string_view& text)
                : TextButton(std::string{text}, ButtonType::Label) {}

        explicit TextLabel(const Id& id) noexcept : TextButton(id, ButtonType::Label) {}

        static constexpr std::string_view id = "TextLabel";
        std::string_view nodeId() const noexcept override {
            return id;
        }
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
        friend class ImageButtonLayoutManager;

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect);

        ImageId mImageId{ImageId::ThreeDots};

        Size mRequestedSize{};

        gm::RenderFlip mRenderFlip{SDL_FLIP_NONE};

    public:
        ~ImageButton() override = default;

        ImageButton(const ImageButton&) = delete;

        ImageButton(ImageButton&&) = delete;

        ImageButton& operator=(const ImageButton&) = delete;

        ImageButton& operator=(ImageButton&&) = delete;

        explicit ImageButton(ButtonType buttonType = ButtonType::PushButton) noexcept;

        explicit ImageButton(ImageId imageId, ButtonType buttonType = ButtonType::PushButton) noexcept;

        static constexpr std::string_view id = "ImageButton";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /**
         * @brief Layout the image button.
         * @param context The graphics Context to use.
         * @param screenRect The available screen Rectangle.
         * @return The requested screen Rectangle.
         */
        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

        /**
         * @brief Draw the image button.
         * @details The text is rendered relative the the parent Container.
         * @param context The graphics context to use.
         * @param containerPosition The position of the parent Container.
         */
        void draw(gm::Context &context, const Position &containerPosition) override;

        /**
         * @brief Set the image used by the ImageButton.
         * @param imageId
         */
        void setImage(ImageId imageId);

        void setRenderFlip(gm::RenderFlip renderFlip) {
            mRenderFlip = renderFlip;
        }
    };

    class ImageLabel : public ImageButton {
    public:
        ~ImageLabel() override = default;

        ImageLabel(const ImageLabel&) = delete;

        ImageLabel(ImageLabel&&) = delete;

        ImageLabel& operator=(const ImageLabel&) = delete;

        ImageLabel& operator=(ImageLabel&&) = delete;

        explicit ImageLabel() noexcept : ImageButton(ButtonType::Label) {}

        explicit ImageLabel(ImageId imageId) noexcept : ImageButton(imageId, ButtonType::Label) {}

        static constexpr std::string_view id = "ImageLabel";
        std::string_view nodeId() const noexcept override {
            return id;
        }

    };

    class ImageButtonLayoutManager : public LayoutManager {
    protected:
        ImageButton& mImageButton;

    public:

        ImageButtonLayoutManager() = delete;
        ~ImageButtonLayoutManager() override = default;

        explicit ImageButtonLayoutManager(ImageButton& imageButton);

        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                LayoutManager::Itr last) override;
    };
}

