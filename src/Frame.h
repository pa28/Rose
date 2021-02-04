/** @file Frame.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-21
  * @section LICENSE
  *
  * @section DESCRIPTION
  * @brief The Frame Class.
  */


#pragma once

#include "Constants.h"
#include "Math.h"
#include "Rose.h"
#include "Signals.h"
#include "SingleChild.h"
#include "Texture.h"
#include "Surface.h"

namespace rose {

    /**
     * @struct ScaleBackgroundElement
     * @brief An element in a blended Scale background image.
     */
    struct ScaleBackgroundElement {
        float mLength;          ///< The position along the progress axis the segment applies to.
        float mBlend;           ///< The width of the previous segment and this segment blended.
        color::HSVA mColor;     ///< The color of the segment specified in HSVA

        ScaleBackgroundElement() = delete;

        /**
         * @brief Constructor
         * @param length The length of the color area.
         * @param blend The length of the blended portion of this area.
         * @param color The color of this area.
         */
        constexpr ScaleBackgroundElement(float length, float blend, color::HSVA color) : mLength(length), mBlend(blend),
                                                                                         mColor(color) {}
    };

    template<size_t N>
    using ScaleBackgroundArray = std::array<ScaleBackgroundElement, N>;

    static constexpr ScaleBackgroundArray<3> CpuScale = {
            ScaleBackgroundElement{0.5f, 0.1f, Theme::dGreen},
            ScaleBackgroundElement{0.7f, 0.1f, Theme::dYellow},
            ScaleBackgroundElement{1.0f, 0.1f, Theme::dRed}
    };

    /**
     * @class Frame
     * @brief A Container Widget that takes one child and places a frame around it.
     */
    class Frame : public SingleChild {
    protected:
        /**
         * @brief Draw the background for the Frame.
         * @details The Frame background is the background color and the border.
         * @param renderer The renderer to use.
         * @param src The source rectangle.
         * @param dst The destination rectangle.
         */
        void drawBackground(sdl::Renderer &renderer, Rectangle &src, Rectangle &dst);

        sdl::Texture mTexture;       ///< The texture for the drawable (background).

        color::RGBA mBaseColor{},                    ///< The base colour
        mTopColor{},                           ///< The top shadow colour
        mBotColor{},                           ///< The bottom shadow colour
        mLeftColor{},                          ///< the left shadow colour
        mRightColor{},                         ///< the right shodow colour
        mInvertColor{};                        ///< the invert colour
        bool mTextureValid{};                  ///< True if the texture is valid, false if the texture must be recreated.
        BorderStyle mBorder{};                 ///< The type of border.
        CornerStyle mCornerStyle;              ///< The style used in border colours
        DrawBackground mDrawBackground;        ///< If true, the background (mTexture) is drawn).
        bool mInvert{};                        ///< True if the colours should be used to make the drawable look inverted.
        Gradient mGradient{Gradient::None};    ///< The type of Gradient to apply as background.

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
        trimCorners(sdl::Surface &surface, color::RGBA color, Frame::SelectedCorners selectedCorners, Size cornerSize,
                    Size frameSize);

        /**
         * @brief Render corner images as part of a border around the frame.
         * @param renderer The Renderer to use
         * @param selectedCorners The corners to render, others are left undecorated.
         * @param corner The RoseImageId of the corner Texture sheet
         * @param size The destination rectangle for the complete frame.
         */
        void
        renderSelectedCorners(sdl::Renderer &renderer, SelectedCorners selectedCorners, RoseImageId corner,
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
        renderSelectedSides(sdl::Renderer &renderer, Frame::SelectedSides selectedSides, UseBorder useBorder,
                            RoseImageId corner, const Size &size, int extend = 0);

        /**
         * @brief Draw only the frame image, do not draw children.
         * @details This is split from Frame::Draw() to allow Widgets derived from Frame to control how their
         * children are drawn.
         * @param renderer The Renderer used to draw the Frame.
         * @param widgetRect The Rectangle available to draw in.
         */
        void drawFrameOnly(sdl::Renderer &renderer, Rectangle widgetRect);

    public:
        ~Frame() override = default;
        Frame(Frame &&) = delete;
        Frame(const Frame &) = delete;
        Frame& operator=(Frame &&) = delete;
        Frame& operator=(const Frame &) = delete;

        /**
         * @brief Constructor
         */
        Frame();

        /**
         * @brief Constructor
         * @param padding Padding value to use on all sides.
         */
        explicit Frame(int padding);

        /**
         * @brief Constructor
         * @param padding Padding object to set padding.
         */
        explicit Frame(Padding padding);

        /**
         * @brief Determine the desired size of the child widget.
         * @details If not overridden the default is to call widgetLayout() for the child and return
         * the current value of mSize of the Container.
         * @param renderer The Renderer to use.
         * @param available The Rectangle available for layout.
         * @return Rectangle The layout Rectangle.
         */
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

        /**
         * @brief Draw this Widget's contribution to the scene.
         * @details Draws the background and frame if these are enabled.
         * @param renderer
         * @param parentRect The layout size computed for this widget.
         * @param parentPosition The layout position computed for this widget.
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Get the colour set used by the Frame.
         * @return An array of 6 colours: Base, Top, Bottom, Left, Right and Invert.
         */
        std::array<color::RGBA, 6> getColorSet() const {
            return std::array<color::RGBA, 6>{mBaseColor, mTopColor, mBotColor,
                                        mLeftColor, mRightColor, mInvertColor};
        }

        /**
         * @brief Set the colour set used by the Frame
         * @param colors An array of 6 colours: Base, Top, Bottom, Left, Right and Invert.
         */
        void setColorSet(const std::array<color::RGBA, 6> &colors) {
            mBaseColor = colors[0];
            mTopColor = colors[1];
            mBotColor = colors[2];
            mLeftColor = colors[3];
            mRightColor = colors[4];
            mInvertColor = colors[5];
            mTextureValid = false;
        }

        /**
         * @brief Set the colour set used by the Frame.
         * @param baseColorHSLA The HSLA encoded base color.
         * @param mods An array of five modification values for the Lightness parameter to generate the
         * remaining 5 colours.
         */
        void setColorSet(const color::HSVA &baseColorHSLA, const std::array<float, 5> &mods) {
            mBaseColor = color::RGBA{baseColorHSLA};
            mTopColor = color::RGBA{baseColorHSLA.modValue(mods[0])};
            mBotColor = color::RGBA{baseColorHSLA.modValue(mods[1])};
            mLeftColor = color::RGBA{baseColorHSLA.modValue(mods[2])};
            mRightColor = color::RGBA{baseColorHSLA.modValue(mods[3])};
            mInvertColor = color::RGBA{baseColorHSLA.modValue(mods[4])};
            mTextureValid = false;
        }

        /**
         * @brief Set the size of the Frame.
         * @param sizeInt
         */
        void setSize(Size size) override {
            mTextureValid = false;
            Widget::setSize(size);
        }

        /**
         * @brief Get the Frame BorderStyle
         * @return the BorderStyle type.
         */
        [[maybe_unused]] BorderStyle getBorder() const noexcept{ return mBorder; }

        /**
         * @brief Set the BorderStyle of the Frame
         * @param border the BorderStyle type.
         */
        void setBorder(BorderStyle border) noexcept {
            mBorder = border;
            mTextureValid = false;
        }

        /**
         * @brief Set the BorderStyle and return a std::shared_ptr to this.
         * @param border
         * @return std::shared_ptr<Frame>
         */
        std::shared_ptr<Frame> withBorder(BorderStyle border) {
            setBorder(border);
            return getWidget<Frame>();
        }

        /**
         * @brief Get the interior (managed) size of a Container
         * @return
         */
//        Rectangle interiorRectangle() override {
//            if (mLayoutHints.mAssignedRect) {
//                auto rect = mLayoutHints.mAssignedRect.value();
//                rect.x() += mFrameWidth;
//                rect.y() += mFrameWidth;
//                rect.width() -= mFrameWidth * 2;
//                rect.height() -= mFrameWidth * 2;
//                return rect;
//            }
//            return Rectangle{mPos, mSize};
//        }

        /**
         * @brief Get the border CornerStyle
         * @return the CornerStyle.
         */
        [[maybe_unused]] CornerStyle getCornerStyle() const { return mCornerStyle; }

        /**
         * @brief Set the border CornerStyle
         * @param cornerStyle the CornerStyle
         */
        void setCornerStyle(CornerStyle cornerStyle) {
            mCornerStyle = cornerStyle;
            mTextureValid = false;
        }

        /**
         * @brief Set the border CornerStyle and return a std::shared_ptr<Frame> to this.
         * @param cornerStyle the CornerStyle
         * @return std::shared_ptr<Frame>
         */
        std::shared_ptr<Frame> withCornerStyle(CornerStyle cornerStyle) {
            setCornerStyle(cornerStyle);
            return getWidget<Frame>();
        }

        /**
         * @brief Get the DrawBackground state.
         * @details If DrawBackground state is None the background is not drawn.
         * @return the DrawBackground state.
         */
        [[maybe_unused]] DrawBackground getDrawBackground() const { return mDrawBackground; }

        /**
         * @brief Set the DrawBackground state.
         * @details If DrawBackground state is None the background is not drawn.
         * @param drawBackground the DrawBackground state.
         */
        void setDrawBackground(DrawBackground drawBackground) {
            mDrawBackground = drawBackground;
            mTextureValid = false;
        }

        /**
         * @brief Set the DrawBackground state and return a std::shared_ptr<Frame> to this.
         * @details If DrawBackground state is None the background is not drawn.
         * @param drawBackground the DrawBackground state.
         * @return std::shared_ptr<Frame>
         */
        std::shared_ptr<Frame> withDrawBackground(DrawBackground drawBackground) {
            setDrawBackground(drawBackground);
            return getWidget<Frame>();
        }

        /**
         * @brief Get the Invert state.
         * @details If the Invert state is true, the border if drawn at all, is drawn inverted.
         * @return the Invert state.
         */
        [[maybe_unused]] bool getInvert() const { return mInvert; }

        /**
         * @brief Set the Invert state.
         * @details If the Invert state is true, the border if drawn at all, is drawn inverted.
         * @param invert the new Invert state.
         */
        void setInvert(bool invert) {
            if (mInvert != invert) {
                mInvert = invert;
                mTextureValid = false;
                setNeedsDrawing();
            }
        }

        /**
         * @brief Set the Invert state and return a std::shared_ptr<Frame> to this.
         * @param invert the new Invert state
         * @return std::shared_ptr<Frame>
         */
        std::shared_ptr<Frame> withInvert(bool invert) {
            setInvert(invert);
            return getWidget<Frame>();
        }

        /**
         * @brief A slot to receive invert transmissions on.
         * @details The boolean value received is used to set the invert state of the Frame. This is intended
         * to be used to indicate the state of buttons, but can also be used for other purposes.
         */
        std::shared_ptr<Slot<bool>> rxInvert{};   //!< A slot to receive invert transmissions on.

        /**
         * @brief See Widget::initializeComposite
         */
        void initializeComposite() override;

        /**
         * @brief Render a selected Gradient.
         * @param renderer The Renderer to use.
         * @param gradient The desired pre-defined Gradient.
         * @param dst The destination Rectangle.
         * @param orientation Rendering Orientation, if Orientation::Vertical the pre-defined Gradient is rotated
         * 90 degrees counter clockwise.
         */
        void
        drawGradientBackground(sdl::Renderer &renderer, Gradient gradient, Rectangle dst, Orientation orientation,
                               float boundary = 0.5f);

        /**
         * @struct LinearInterpolator
         * @brief Provide an linear interpolated value between between two extrema values.
         * @tparam T The type of the dependent data value, the value being interpolated.
         * @tparam U The type of the independent data value, the index marking the distance between extrema.
         */
        template<typename T, typename U>
        struct LinearInterpolator {
            bool zeroRange;                         ///< True of the range between extrema is zero.
            T zeroData;                             ///< The dependent value corresponding to zero, the left extrema.
            using working_type = double;            ///< The type used for computation.
            working_type dataMin, dataRange;        ///< The minimum and range of the dependent values.
            working_type indexMin, indexRange;      ///< The minimum and range of the independent values.

            LinearInterpolator() = delete;

            /**
             * @brief Constructor
             * @param dataFirst The first (left hand) dependent value.
             * @param dataLast The last (right hand) dependent value.
             * @param indexFirst The index of dataFirst.
             * @param indexLast The index of dataLast.
             */
            LinearInterpolator(T dataFirst, T dataLast, U indexFirst, U indexLast)
                : dataMin(dataFirst), dataRange(dataLast-dataFirst),
                  indexMin(indexFirst), indexRange(indexLast-indexFirst) {
                zeroRange = dataFirst == dataLast;
                zeroData = dataFirst;
            }

            /**
             * @brief Calculate the interpolation of the dependent value at index.
             * @param index The independent value to compute the interpolation at.
             * @return The interpolated dependent value.
             */
            T operator()(U index) {
                if (zeroRange)
                    return zeroData;

                auto i = (static_cast<working_type>(index) - indexMin) / indexRange;
                auto r = dataRange * i + dataMin;
                if constexpr (std::is_integral_v<T>) {
                    return roundTo<T>(r);
                } else {
                    return static_cast<T>(r);
                }
            }
        };

        /**
         * @brief Compute a Hue, Saturation, Value, Alpha gradiant.
         * @details At each pixel index of the Rectangle dst, a line of the color generated from an interpolation of
         * each component between start and end colors is drawn perpendicular to the orientation axis.
         * @param renderer The Renderer
         * @param dst The rectangle to fill with the gradiant.
         * @param startColor The start Color
         * @param endColor The end Color
         * @param orientation Whether the gradiant is horizontal (along the x axis) or vertical (along the y axis).
         */
        static void
        renderHSVAGradiant(sdl::Renderer &renderer, Rectangle dst, color::HSVA startColor, color::HSVA endColor,
                           Orientation orientation) {
            sdl::DrawColorGuard colorGuard{renderer, color::RGBA(startColor)};
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal: {
                    LinearInterpolator liH{startColor.hue(), endColor.hue(), 0, dst.width()};
                    LinearInterpolator liS{startColor.saturation(), endColor.saturation(), 0, dst.width()};
                    LinearInterpolator liV{startColor.value(), endColor.value(), 0, dst.width()};
                    LinearInterpolator liA{startColor.alpha(), endColor.alpha(), 0, dst.width()};

                    for (int i = 0; i < dst.width(); ++i) {
                        color::HSVA shade;
                        shade.hue() = liH(i);
                        shade.saturation() = liS(i);
                        shade.value() = liV(i);
                        shade.alpha() = liA(i);
                        colorGuard.setDrawColor(color::RGBA{shade});
                        SDL_RenderDrawLine(renderer.get(), dst.x()+i, dst.y(), dst.x()+i, dst.y()+dst.height());
                    }
                }
                    break;
                case Orientation::Vertical: {
                    LinearInterpolator liH{endColor.hue(), startColor.hue(), 0, dst.height()};
                    LinearInterpolator liS{endColor.saturation(), startColor.saturation(), 0, dst.height()};
                    LinearInterpolator liV{endColor.value(), startColor.value(), 0, dst.height()};
                    LinearInterpolator liA{endColor.alpha(), startColor.alpha(), 0, dst.height()};

                    for (int i = 0; i < dst.width(); ++i) {
                        color::HSVA shade;
                        shade.hue() = liH(i);
                        shade.saturation() = liS(i);
                        shade.value() = liV(i);
                        shade.alpha() = liA(i);
                        colorGuard.setDrawColor(color::RGBA{shade});
                        SDL_RenderDrawLine(renderer.get(), dst.x(), dst.y()+i, dst.x()+dst.width(), dst.y()+i);
                    }
                }
                    break;
            }
        }

        /**
         * @brief Compute a Red, Green, Blue, Alpha gradiant.
         * @details At each pixel index of the Rectangle dst, a line of the color generated from an interpolation of
         * each component between start and end colors is drawn perpendicular to the orientation axis.
         * @param renderer The Renderer
         * @param dst The rectangle to fill with the gradiant.
         * @param startColor The start Color
         * @param endColor The end Color
         * @param orientation Whether the gradiant is horizontal (along the x axis) or vertical (along the y axis).
         */
        static void
        renderColorGradiant(sdl::Renderer &renderer, Rectangle dst, color::RGBA startColor, color::RGBA endColor,
                          Orientation orientation) {
            sdl::DrawColorGuard colorGuard{renderer, color::RGBA(startColor)};
            switch (orientation) {
                case Orientation::Unset:
                case Orientation::Horizontal: {
                    LinearInterpolator liR{startColor.r(), endColor.r(), 0, dst.width()};
                    LinearInterpolator liG{startColor.g(), endColor.g(), 0, dst.width()};
                    LinearInterpolator liB{startColor.b(), endColor.b(), 0, dst.width()};
                    LinearInterpolator liA{startColor.a(), endColor.a(), 0, dst.width()};

                    for (int i = 0; i < dst.width(); ++i) {
                        color::RGBA shade;
                        shade.r() = liR(i);
                        shade.g() = liG(i);
                        shade.b() = liB(i);
                        shade.a() = liA(i);
                        colorGuard.setDrawColor(color::RGBA{shade});
                        SDL_RenderDrawLine(renderer.get(), dst.x()+i, dst.y(), dst.x()+i, dst.y()+dst.height());
                    }
                }
                    break;
                case Orientation::Vertical: {
                    LinearInterpolator liR{endColor.r(), startColor.r(), 0, dst.height()};
                    LinearInterpolator liG{endColor.g(), startColor.g(), 0, dst.height()};
                    LinearInterpolator liB{endColor.b(), startColor.b(), 0, dst.height()};
                    LinearInterpolator liA{endColor.a(), startColor.a(), 0, dst.height()};

                    for (int i = 0; i < dst.width(); ++i) {
                        color::RGBA shade;
                        shade.r() = liR(i);
                        shade.g() = liG(i);
                        shade.b() = liB(i);
                        shade.a() = liA(i);
                        colorGuard.setDrawColor(color::RGBA{shade});
                        renderer.drawLine(dst.x(), dst.y()+i, dst.x()+dst.width(), dst.y()+i);
                    }
                }
                    break;
            }
        }

        /**
         * @brief Draw a gradient background given a range [first ... last) of ScaleBackgroundElement objects.
         * @tparam InputIterator The iterator type providing the range.
         * @param renderer The Renderer.
         * @param first The iterator of the first background element.
         * @param last The iterator one past the last background element.
         * @param dst The destination rectangle.
         * @param orientation The orientation.
         */
        template<class InputIterator>
        void drawGradientBackground(sdl::Renderer &renderer, InputIterator first, InputIterator last,
                                    Rectangle dst, Orientation orientation) {
            static_assert(std::is_same_v<typeof((*first)), ScaleBackgroundElement> &
                          std::is_same_v<typeof((*last)), ScaleBackgroundElement>,
                          "Iterator arguments not of type ScaleBackgroundElement.");

            sdl::ClipRectangleGuard clipRectangleGuard{renderer, dst};

            if (first != last) {
                if (last - first == 1) {
                    renderer.fillRect(dst, color::RGBA{first->mColor});
                } else {
                    Rectangle segment{dst};
                    int previousEnd = 0;
                    float previousLength = 0.0f;
                    switch (orientation) {
                        case Orientation::Unset:
                        case Orientation::Horizontal:
                            break;
                        case Orientation::Vertical:
                            segment.y() += segment.height();
                            break;
                    }

                    auto idx = first;
                    auto prevIdx = last;
                    while (idx != last) {
                        switch (orientation) {
                            case Orientation::Unset:
                            case Orientation::Horizontal:
                                segment.width() = roundToInt((idx->mLength - previousLength) * (float) dst.width());
                                break;
                            case Orientation::Vertical:
                                segment.height() = dst.height() -
                                                   roundToInt((idx->mLength - previousLength) * (float) dst.height());
                                break;
                        }

                        renderer.fillRect(segment, color::RGBA{idx->mColor});

                        if (idx != first) {
                            // Render gradient from last segment to this.

                            Rectangle blendRect{};
                            switch (orientation) {
                                case Orientation::Unset:
                                case Orientation::Horizontal:
                                    blendRect.width() = roundToInt(idx->mBlend * (float) dst.width());
                                    blendRect.height() = dst.height();
                                    blendRect.x() = previousEnd - blendRect.width()/2;
                                    blendRect.y() = dst.y();
                                    break;
                                case Orientation::Vertical:
                                    blendRect.width() = dst.width();
                                    blendRect.height() = roundToInt(idx->mBlend * (float) dst.height());
                                    blendRect.x() = dst.x();
                                    blendRect.y() = previousEnd + blendRect.height()/2;
                                    break;
                            }
                            renderHSVAGradiant(renderer, blendRect, (idx - 1)->mColor, idx->mColor,
                                               orientation);
                        }

                        switch (orientation) {
                            case Orientation::Unset:
                            case Orientation::Horizontal:
                                segment.x() += segment.width();
                                previousEnd = segment.x();
                                break;
                            case Orientation::Vertical:
                                segment.y() -= segment.height();
                                previousEnd = segment.y();
                                break;
                        }

                        previousLength = idx->mLength;
                        prevIdx = idx;
                        ++idx;
                    }
                }
            }
        }
    };
}

/**
 * @addtogroup WidgetManip
 * @{
 */

/**
 * @brief A Frame manipulator to set the Frame Corner Style.
 * @tparam WidgetClass The type of widget
 * @param widget the widget
 * @param cornerStyle the CornerStyle
 * @return a std::shared_ptr to this widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::CornerStyle cornerStyle) {
    widget->setCornerStyle(cornerStyle);
    return widget;
}

/**
 * @brief A Frame manipulator to set the Frame BorderStyle.
 * @tparam WidgetClass The type of widget
 * @param widget the widget
 * @param border the BorderStyle
 * @return a std::shared_ptr to this widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::BorderStyle border) {
    widget->setBorder(border);
    return widget;
}

/**
 * @brief A Frame manipulator to set if and how the background is drawn.
 * @tparam WidgetClass The type of widget
 * @param widget the widget
 * @param drawBackground the background drawing specification.
 * @return a std::shared_ptr to this widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass>
operator<<(std::shared_ptr<WidgetClass> widget, rose::DrawBackground drawBackground) {
    widget->setDrawBackground(drawBackground);
    return widget;
}

/** @} */
