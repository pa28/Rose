/**
 * @file Frame.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-12
 */

#pragma once

#include <cstdint>
#include "Animation.h"
#include "Color.h"
#include "ImageStore.h"
#include "Visual.h"

namespace rose {

    /**
     * @enum BorderStyle
     * @brief The types of border supported.
     */
    enum class BorderStyle {
        Unset,      ///< Not set to a valid value.
        None,       ///< No border
        Bevel,      ///< A beveled out border
        BevelIn,    ///< A beveled in border
        Notch,      ///< A notch border
        TabTop,     ///< TabButton border, tabs on top
        TabLeft,    ///< TabButton border, tabs on left
        TabPage,    ///< TabPage border.
    };

    /**
     * @enum CornerStyle
     * @brief Types of corners supported.
     */
    enum class CornerStyle {
        Unset,      ///< Not set to a valid value.
        Square,     ///< Square corners
        Round,      ///< Round corners.
    };

    /**
     * @enum UseBorder
     * @brief The type of border to draw
     */
    enum UseBorder {
        None,       ///< No border
        BevelOut,   ///< A beveled border that gives the illusion the frame stands up from the display.
        BevelIn,    ///< A beveled border that gives the illusion the frame is sunk into the display.
        NotchOut,   ///< A notched border that looks like a ridge surrounding the frame.
        NotchIn,    ///< A notched border that looks like a trench surrounding the frame.
    };

    /**
     * @class FrameElements
     * @brief Encapsulation of the visual elements of a Frame.
     */
    class FrameElements {
    protected:
        color::RGBA mBaseColor{color::DarkBaseColor};
        color::RGBA mInvertColor{color::DarkInvertColor};
        color::RGBA mTopColor{color::DarkTopColor};
        color::RGBA mBotColor{color::DarkBotColor};
        color::RGBA mLeftColor{color::DarkLeftColor};
        color::RGBA mRightColor{color::DarkRightColor};
        color::RGBA mActiveColor{color::DarkBaseColor};
        color::RGBA mInactiveColor{color::DarkBaseColor};
        float mColorValue{}, mLastColorValue{};
        int mFrameWidth{2};
        Padding mFramePadding{};
        BorderStyle mBorderStyle{BorderStyle::None};
        CornerStyle mCornerStyle{CornerStyle::Round};
        bool mInvert{};
        gm::Texture mBorder{};
        gm::Texture mInactiveBG{};
        gm::Texture mAnimagedBG{};

        /**
         * @enum SelectedCorners
         * @brief Specify corners selected for some process, usually drawing.
         */
        enum SelectedCorners : uint32_t {
            NoCorners = 0,
            TopLeftCorner = 0x8,
            TopRightCorner = 0x4,
            TopCorners = TopLeftCorner | TopRightCorner,
            BottomLeftCorner = 0x2,
            BottomRightCorner = 0x1,
            BottomCorners = BottomLeftCorner | BottomRightCorner,
            LeftCorners = TopLeftCorner | BottomLeftCorner,
            RightCorners = TopRightCorner | BottomRightCorner,
            AllCorners = 0xf,
        };

        /**
         * @enum SelectedSides
         * @brief Specify sides selected for some process, usually drawing.
         */
        enum SelectedSides : uint32_t {
            NoSides = 0,
            TopSide = 1,
            BotSide = 2,
            LeftSide = 4,
            RightSide = 8,
            AllSides = TopSide | BotSide | LeftSide | RightSide,
            TabTopSides = TopSide | LeftSide | RightSide,
        };

        /**
         * @brief Trim corners on a background surface to allow for rounded corners.
         * @param surface The surface to trim
         * @param color The background fill color.
         * @param selectedCorners Which corners should be trimmed, others are left square.
         * @param cornerSize The size of the corner in piexels
         * @param frameSize The size of the frame.
         */
        static void
        trimCorners(gm::Surface &surface, color::RGBA color, FrameElements::SelectedCorners selectedCorners, Size cornerSize,
                    Size frameSize);

        /**
         * @brief Render corner images as part of a border around the frame.
         * @param renderer The Renderer to use
         * @param selectedCorners The corners to render, others are left undecorated.
         * @param corner The RoseImageId of the corner Texture sheet
         * @param size The destination rectangle for the complete frame.
         */
        static void
        renderSelectedCorners(gm::Context &context, SelectedCorners selectedCorners, ImageId corner,
                              const Size &size);

        /**
         * @brief Render sides as part of a border around the frame.
         * @param renderer The Renderer to use
         * @param selectedSides The sides to render, others are left undecorated.
         * @param useBorder The border style used.
         * @param corner The RoseImageId of the corner Texture sheet, to get the size of corner images.
         * @param size The destination rectangle for the complete frame.
         * @param extend If true, extend the side(s) in the direction of increasing Y or X (down or right).
         */
        void
        renderSelectedSides(gm::Context &context, FrameElements::SelectedSides selectedSides, UseBorder useBorder,
                            ImageId corner, const Size &size, int extend = 0);

        /**
         * @brief Draw the background for the Frame.
         * @details The Frame background is the background color and the border.
         * @param renderer The renderer to use.
         * @param src The source rectangle.
         * @param dst The destination rectangle.
         */
        void drawBackground(gm::Context &context, Rectangle &src, Rectangle &dst);

        gm::Texture
        createBackgroundTexture(gm::Context &context, Rectangle &src, Rectangle &dst, const color::RGBA &color);

        std::tuple<UseBorder,SelectedCorners> decoration();

        /**
         * @brief Draw the Frame and background.
         * @param renderer The Renderer used to draw the Frame.
         * @param widgetRect The Rectangle available to draw in.
         */
        void drawFrame(gm::Context &context, Rectangle widgetRect);

    public:
        FrameElements() = default;
        virtual ~FrameElements() = default;

        explicit FrameElements(int padding) : mFramePadding(padding) {}

        /// Set the BorderStyle
        void set(const BorderStyle borderStyle) {
            mBorderStyle = borderStyle;
            mBorder.reset();
        }

        /// Set the CornerStyle
        void set(const CornerStyle cornerStyle) {
            mCornerStyle = cornerStyle;
            mBorder.reset();
            mAnimagedBG.reset();
            mInactiveBG.reset();
        }

        /// Set the state, true = inverted.
        void setState(bool state) {
            mInvert = state;
            mBorder.reset();
            mAnimagedBG.reset();
            mInactiveBG.reset();
        }

        /// Set the active color.
        void setAnimateColor(const color::RGBA &color) {
            mActiveColor = color;
            mAnimagedBG.reset();
        }

        /// Set the background color.
        void setInactiveColor(const color::RGBA &color) {
            mInactiveColor = color;
            mInactiveBG.reset();
        }

        [[nodiscard]] bool getState() const { return mInvert; }

        static gm::Texture
        createBackgroundMask(gm::Context &context, Size size, int frameWidth, bool roundCorners = false);

        static void colorBackgroundMask(gm::Context &context, gm::Texture &mask, const color::RGBA &base,
                                        const color::RGBA &active, float value);
    };

    class FrameLayoutManager : public LayoutManager {
    public:
        FrameLayoutManager() {
            mMaxContent = 1;
        }

        ~FrameLayoutManager() override = default;

        /// Layout the contents of the associated manager.
        Rectangle layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                        LayoutManager::Itr last) override;

    };

    class Frame : public Manager, public FrameElements, public Animation {
    protected:

    public:
        Frame() noexcept : Manager(), FrameElements() {
            mLayoutManager = std::make_unique<FrameLayoutManager>();
            mAnimationCallback = [&](gm::Context &context, const Position &position, uint32_t frame) {
                if (mActionCurve) {
                    auto idx = frame % mActionCurve->size();
                    mColorValue = (*mActionCurve)[idx];
//                    if (mColorValue != mLastColorValue) {
                        drawAnimate(context, position);
                        mLastColorValue = mColorValue;
//                    }
                } else {
                    removeAnimation(getNode<Animation>());
                }
            };

            mAnimationEnableStateCallback = [&](AnimationEnable animationEnable){
                if (animationEnable == AnimationEnable::Disable && mAnimationEnableState == AnimationEnable::Enable) {
                    removeAnimation(getNode<Animation>());
                }
            };
        }

        ~Frame() override = default;

        explicit Frame(int padding) noexcept : Frame() {
            mPadding = Padding{padding};
        }

        void drawAnimate(gm::Context &context, const Position &containerPosition);

        void draw(gm::Context &context, const Position &containerPosition) override {
            if (mActionCurve && mAnimationEnableState == AnimationEnable::Enable)
                setAnimation(getNode<Animation>(), containerPosition);
            drawAnimate(context,containerPosition);
        }

        Rectangle layout(gm::Context &context, const Rectangle &screenRect) override;

    };

    enum class FrameColorType {
        InactiveColor,
        AnimateColor,
    };

    struct FrameColor {
        FrameColorType type{};
        color::RGBA rgba{};

        explicit FrameColor(FrameColorType type, color::RGBA color) : type(type), rgba(color) {}
    };
}

template<typename ManagerClass>
inline std::shared_ptr<ManagerClass> operator<<(std::shared_ptr<ManagerClass> manager, const rose::BorderStyle borderStyle) {
    static_assert(std::is_base_of_v<rose::FrameElements,ManagerClass>, "ManagerClass must be derived from rose::FrameElements." );
    manager->set(borderStyle);
    return manager;
}

template<typename ManagerClass>
inline std::shared_ptr<ManagerClass> operator<<(std::shared_ptr<ManagerClass> manager, const rose::CornerStyle cornerStyle) {
    static_assert(std::is_base_of_v<rose::FrameElements,ManagerClass>, "ManagerClass must be derived from rose::FrameElements." );
    manager->set(cornerStyle);
    return manager;
}

template <class ManagerClass>
inline std::shared_ptr<ManagerClass> operator<<(std::shared_ptr<ManagerClass> manager, const rose::FrameColor& frameColor) {
    static_assert(std::is_base_of_v<rose::FrameElements,ManagerClass>, "ManagerClass must be derived from rose::FrameElements." );
    switch (frameColor.type) {
        case rose::FrameColorType::InactiveColor:
            manager->setInactiveColor(frameColor.rgba);
            break;
        case rose::FrameColorType::AnimateColor:
            manager->setAnimateColor(frameColor.rgba);
            break;
    }
    return manager;
}
