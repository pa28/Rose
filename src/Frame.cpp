/** @file Frame.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-21
  * @brief A container which can place a frame around its contents.
  * @details The Frame container only supports one child, which may be another container.
  */

#include <SDL2/SDL.h>
#include "Frame.h"
#include "Theme.h"
#include "Rose.h"
#include "Signals.h"
#include "Surface.h"

namespace rose {
    Frame::Frame() :
            mTextureValid(false),
            mDrawBackground(DrawBackground::None),
            mInvert(false),
            mCornerStyle(CornerStyle::Unset) {
    }

    void Frame::initializeComposite() {
        mClassName = "Frame";
        Container::initializeComposite();

        auto theme = Widget::rose()->theme();
        mBaseColor = theme.mBaseColor;
        mTopColor = theme.mTopColor;
        mBotColor = theme.mBotColor;
        mLeftColor = theme.mLeftColor;
        mRightColor = theme.mRightColor;
        mInvertColor = theme.mInvertColor;
        if (mFrameWidth < 0) mFrameWidth = theme.mBorderWidth;
        if (unset(mBorder)) mBorder = theme.mBorder;
        if (unset(mCornerStyle)) mCornerStyle = theme.mCornerStyle;

        // Create the Invert receive slot.
        rxInvert = std::make_shared<Slot<bool>>();

        // Add a callback to set the state of the mInvert member.
        rxInvert->setCallback([&](uint32_t sn, bool state) {
            setInvert(state);
        });
    }

    Rectangle Frame::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto frameAvailable = clampAvailableArea(available, mPos, mSize);
        frameAvailable.width() -= mFrameWidth * 2;
        frameAvailable.height() -= mFrameWidth * 2;
        Rectangle layout{};
        for (auto &child : mChildren) {
            LayoutHints& childHints{child->layoutHints()};
            layout = child->widgetLayout(renderer, frameAvailable, 0);
            childHints.mAssignedRect = layout;
            childHints.mAssignedRect->x() += mFrameWidth;
            childHints.mAssignedRect->y() += mFrameWidth;

            layout.width() += mFrameWidth * 2;
            layout.height() += mFrameWidth * 2;
        }

        return layout;
    }

    void Frame::drawFrameOnly(sdl::Renderer &renderer, Rectangle widgetRect) {
        Rectangle src{0, 0, widgetRect.width(), widgetRect.height()};
        Rectangle dst{widgetRect};

        // If there is a reason to do the background at all
        if (mDrawBackground != DrawBackground::None || mBorder != BorderStyle::None) {
            if (!mTexture || !mTextureValid)
                drawBackground(renderer, src, dst);

            mTexture.setBlendMOde(SDL_BLENDMODE_BLEND);
            renderer.renderCopy(mTexture, dst);
        }
    }

    void Frame::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            drawFrameOnly(renderer, widgetRect);

            for (auto &child : mChildren) {
                child->draw(renderer, widgetRect);
            }
        }
    }

    void Frame::trimCorners(sdl::Surface &surface, color::RGBA color, Frame::SelectedCorners selectedCorners,
                            Size cornerSize,
                            Size frameSize) {
        auto trimCorner = [&surface, &color](int x0, int y0, int xw, int yh, int R2) {
            for (int x = x0; x0 < xw ? x < xw : x > xw; x0 < xw ? ++x : --x)
                for (int y = y0; y0 < yh ? y < yh : y > yh; y0 < yh ? ++y : --y) {
                    int xr = xw - x;
                    int yr = yh - y;
                    int r2 = xr * xr + yr * yr;
                    if (r2 > R2) {
                        surface.pixel(x, y) = sdl::mapRGBA(surface.get()->format, color);
                    }
                }
        };
        cornerSize.width() /= 2;
        cornerSize.height() /= 2;
        int R2 = cornerSize.width() * cornerSize.width();
        if (selectedCorners & SelectedCorners::TopLeftCorner)
            trimCorner(0, 0, cornerSize.width(), cornerSize.height(), R2);
        if (selectedCorners & SelectedCorners::TopRightCorner)
            trimCorner(frameSize.width() - 1, 0, frameSize.width() - cornerSize.width() - 1, cornerSize.height(), R2);
        if (selectedCorners & SelectedCorners::BottomLeftCorner)
            trimCorner(0, frameSize.height() - 1, cornerSize.width(), frameSize.height() - cornerSize.height() - 1, R2);
        if (selectedCorners & SelectedCorners::BottomRightCorner)
            trimCorner(frameSize.width() - 1, frameSize.height() - 1, frameSize.width() - cornerSize.width() - 1,
                       frameSize.height() - cornerSize.height() - 1,
                       R2);
    }

    void Frame::renderSelectedCorners(sdl::Renderer &renderer, SelectedCorners selectedCorners, RoseImageId corner,
                                      const Size &size) {
        auto sRose = rose();
        auto cornerSize = sRose->imageRepository(corner).getSize();

        auto w = cornerSize.width() / 2;
        auto h = cornerSize.height() / 2;

        for (SelectedCorners s = SelectedCorners::BottomRightCorner; (s & SelectedCorners::AllCorners) != 0;
             s = SelectedCorners(s << 1U)) {
            SDL_Rect src{0, 0, w, h}, dst{0, 0, w, h};
            if (s & selectedCorners) {
                if (s & SelectedCorners::TopLeftCorner) {
                } else if (s & SelectedCorners::TopRightCorner) {
                    src.x = w;
                    src.y = 0;
                    dst.x = size.width() - w;
                    dst.y = 0;
                } else if (s & SelectedCorners::BottomLeftCorner) {
                    src.x = 0;
                    src.y = w;
                    dst.x = 0;
                    dst.y = size.height() - w;
                } else if (s & SelectedCorners::BottomRightCorner) {
                    src.x = w;
                    src.y = w;
                    dst.x = size.width() - w;
                    dst.y = size.height() - w;
                }
                sRose->imageRepository().renderCopy(renderer, corner, src, dst);
            }
        }
    }

    void
    Frame::renderSelectedSides(sdl::Renderer &renderer, Frame::SelectedSides selectedSides, UseBorder useBorder,
                               RoseImageId corner, const Size &size, int extend) {
        auto cornerSize = rose()->imageRepository(corner).getSize();
        Rectangle fill0, fill1;
        color::RGBA color0, color1;

        for (SelectedSides s = SelectedSides::TopSide; (s & SelectedSides::AllSides) != 0; s = SelectedSides(s << 1U)) {
            if ((s & selectedSides) & SelectedSides::TopSide) {
                fill0.x() = fill1.x() = (extend < 0 ? 0 : cornerSize.width() / 2);
                fill0.y() = 0;
                fill1.y() = mFrameWidth / 2;
                fill0.width() = fill1.width() = size.width() - cornerSize.width() / (extend ? 2 : 1);
                fill0.height() = fill1.height() = mFrameWidth / 2;
                switch (useBorder) {
                    case UseBorder::BevelOut:
                        color0 = color1 = mTopColor;
                        break;
                    case UseBorder::BevelIn:
                        color0 = color1 = mBotColor;
                        break;
                    case UseBorder::NotchIn:
                        color0 = mBotColor;
                        color1 = mTopColor;
                        break;
                    case UseBorder::NotchOut:
                        color0 = mTopColor;
                        color1 = mBotColor;
                        break;
                    case UseBorder::None:
                        break;
                }
            } else if ((s & selectedSides) & SelectedSides::BotSide) {
                fill0.x() = fill1.x() = (extend < 0 ? 0 : cornerSize.width() / 2);
                fill0.y() = size.height() - mFrameWidth;
                fill1.y() = fill0.y() + mFrameWidth / 2;
                fill0.width() = fill1.width() = size.width() - cornerSize.width() / (extend ? 2 : 1);
                fill0.height() = fill1.height() = mFrameWidth / 2;
                switch (useBorder) {
                    case UseBorder::BevelOut:
                        color0 = color1 = mBotColor;
                        break;
                    case UseBorder::BevelIn:
                        color0 = color1 = mTopColor;
                        break;
                    case UseBorder::NotchIn:
                        color0 = mBotColor;
                        color1 = mTopColor;
                        break;
                    case UseBorder::NotchOut:
                        color0 = mTopColor;
                        color1 = mBotColor;
                        break;
                    case UseBorder::None:
                        break;
                }
            } else if ((s & selectedSides) & SelectedSides::LeftSide) {
                fill0.x() = 0;
                fill1.x() = mFrameWidth / 2;
                fill0.y() = fill1.y() = (extend < 0 ? 0 : cornerSize.width() / 2);
                fill0.width() = fill1.width() = mFrameWidth / 2;
                fill0.height() = fill1.height() = size.height() - cornerSize.height() / (extend ? 2 : 1);
                switch (useBorder) {
                    case UseBorder::BevelOut:
                        color0 = color1 = mLeftColor;
                        break;
                    case UseBorder::BevelIn:
                        color0 = color1 = mRightColor;
                        break;
                    case UseBorder::NotchIn:
                        color0 = mRightColor;
                        color1 = mLeftColor;
                        break;
                    case UseBorder::NotchOut:
                        color0 = mLeftColor;
                        color1 = mRightColor;
                        break;
                    case UseBorder::None:
                        break;
                }
            } else if ((s & selectedSides) & SelectedSides::RightSide) {
                fill0.x() = size.width() - mFrameWidth;
                fill1.x() = size.width() - mFrameWidth / 2;
                fill0.y() = fill1.y() = (extend < 0 ? 0 : cornerSize.width() / 2);
                fill0.width() = fill1.width() = mFrameWidth / 2;
                fill0.height() = fill1.height() = size.height() - cornerSize.height() / (extend ? 2 : 1);
                switch (useBorder) {
                    case UseBorder::BevelOut:
                        color0 = color1 = mRightColor;
                        break;
                    case UseBorder::BevelIn:
                        color0 = color1 = mLeftColor;
                        break;
                    case UseBorder::NotchIn:
                        color0 = mRightColor;
                        color1 = mLeftColor;
                        break;
                    case UseBorder::NotchOut:
                        color0 = mLeftColor;
                        color1 = mRightColor;
                        break;
                    case UseBorder::None:
                        break;
                }
            }

            if (s & selectedSides) {
                renderer.fillRect(fill0, color0);
                renderer.fillRect(fill1, color1);
            }
        }
    }

    void Frame::drawBackground(sdl::Renderer &renderer, Rectangle &src, Rectangle &dst) {
        SelectedCorners selectedCorners = NoCorners;
        UseBorder useBorder;
        switch (mBorder) {
            case BorderStyle::Unset:
            case BorderStyle::None:
                useBorder = None;
                selectedCorners = AllCorners;
                break;
            case BorderStyle::Bevel:
                useBorder = mInvert ? BevelIn : BevelOut;
                selectedCorners = AllCorners;
                break;
            case BorderStyle::BevelIn:
                useBorder = mInvert ? BevelOut : BevelIn;
                selectedCorners = AllCorners;
                break;
            case BorderStyle::Notch:
                useBorder = mInvert ? NotchOut : NotchIn;
                selectedCorners = AllCorners;
                break;
            case BorderStyle::TabLeft:
                useBorder = NotchIn;
                selectedCorners = LeftCorners;
                break;
            case BorderStyle::TabTop:
                useBorder = NotchIn;
                selectedCorners = TopCorners;
                break;
            case BorderStyle::TabPage:
                useBorder = NotchIn;
                selectedCorners = BottomCorners;
                break;
        }

        sdl::Surface surface{dst.width(), dst.height(), 32, SDL_PIXELFORMAT_RGBA8888};
        if (!surface)
            std::cout << __PRETTY_FUNCTION__ << ' ' << SDL_GetError() << '\n';

        color::RGBA color;
        if (mBorder == BorderStyle::Notch)
            color = mBaseColor;
        else
            color = !mInvert ? mBaseColor : mInvertColor;

        RoseImageId roundCnr = RoseImageInvalid;
        RoseImageId squareCnr = RoseImageInvalid;

        switch (useBorder) {
            case None:
                break;
            case BevelOut:
                roundCnr = RoseImageId::BevelOutRoundCorners;
                squareCnr = RoseImageId::BevelOutSquareCorners;
                break;
            case BevelIn:
                roundCnr = RoseImageId::BevelInRoundCorners;
                squareCnr = RoseImageId::BevelInSquareCorners;
                break;
            case NotchIn:
                roundCnr = RoseImageId::NotchInRoundCorners;
                squareCnr = RoseImageId::NotchInSquareCorners;
                break;
            case NotchOut:
                roundCnr = RoseImageId::NotchOutRoundCorners;
                squareCnr = RoseImageId::NotchOutSquareCorners;
                break;
        }

        surface.fillRectangle(color);

        if (mCornerStyle == CornerStyle::Round) {
            auto cornerSize = rose()->imageRepository(BevelOutRoundCorners).getSize();
            trimCorners(surface, color, selectedCorners, cornerSize, dst.getSize());
        }

        auto texture = surface.toTexture(renderer);
        texture.setBlendMOde(SDL_BLENDMODE_NONE);
        mTexture = sdl::Texture{renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                         dst.width(), dst.height()};

        sdl::RenderTargetGuard renderTargetGuard(renderer, mTexture);
        renderer.renderCopy(texture);
        mTexture.setBlendMOde(SDL_BLENDMODE_BLEND);

        if (mBorder != BorderStyle::None && mBorder != BorderStyle::Unset) {
            auto corner = mCornerStyle == CornerStyle::Round ? roundCnr : squareCnr;
            if (mBorder == BorderStyle::Notch || mBorder == BorderStyle::Bevel || mBorder == BorderStyle::BevelIn) {
                renderSelectedCorners(renderer, SelectedCorners::AllCorners, corner, dst.getSize());
                renderSelectedSides(renderer, SelectedSides::AllSides, useBorder, corner, dst.getSize(), 0);
            } else if (mBorder == BorderStyle::TabTop) {
                renderSelectedCorners(renderer, SelectedCorners::TopCorners, corner, dst.getSize());
                renderSelectedSides(renderer, SelectedSides::TopSide, useBorder, corner, dst.getSize(), 0);
                renderSelectedSides(renderer, (SelectedSides) (SelectedSides::LeftSide | SelectedSides::RightSide),
                                    useBorder, corner, dst.getSize(), 1);
                if (mInvert) {
                    renderSelectedCorners(renderer, SelectedCorners::BottomCorners, squareCnr, dst.getSize());
                    renderSelectedSides(renderer, SelectedSides::BotSide, useBorder, corner, dst.getSize(), 0);
                }
            } else if (mBorder == BorderStyle::TabLeft) {
                renderSelectedCorners(renderer, SelectedCorners::LeftCorners, corner, dst.getSize());
                renderSelectedSides(renderer, SelectedSides::LeftSide, useBorder, corner, dst.getSize(), 0);
                renderSelectedSides(renderer, (SelectedSides) (SelectedSides::TopSide | SelectedSides::BotSide),
                                    useBorder, corner, dst.getSize(), 1);
                if (mInvert) {
                    renderSelectedCorners(renderer, SelectedCorners::RightCorners, squareCnr, dst.getSize());
                    renderSelectedSides(renderer, SelectedSides::RightSide, useBorder, corner, dst.getSize(), 0);
                }
            } else if (mBorder == BorderStyle::TabPage) {
                renderSelectedCorners(renderer, SelectedCorners::BottomCorners, corner, dst.getSize());
                renderSelectedSides(renderer, SelectedSides::BotSide, useBorder, corner, dst.getSize(), 0);
                renderSelectedSides(renderer, (SelectedSides) (SelectedSides::LeftSide | SelectedSides::RightSide),
                                    useBorder, corner, dst.getSize(), -1);
            }
        }

        mTextureValid = true;
    }

    void
    Frame::drawGradientBackground(sdl::Renderer &renderer, Gradient gradient, Rectangle dst, Orientation orientation,
                                  float boundary) {
        auto &theme = rose()->theme();
        std::vector<ScaleBackgroundElement> scaleBackgroundElements{};

        switch (gradient) {
            case Gradient::None:
                break;
            case Gradient::LightToDark:
                scaleBackgroundElements.emplace_back(0.5, 0.2, rose::Theme::dWhite);
                scaleBackgroundElements.emplace_back(1.0, 0.9, rose::Theme::dBlack);
                break;
            case Gradient::DarkToLight:
                scaleBackgroundElements.emplace_back(0.5, 0.2, rose::Theme::dBlack);
                scaleBackgroundElements.emplace_back(1.0, 0.9, rose::Theme::dWhite);
                break;
            case Gradient::GreenToRed:
                scaleBackgroundElements.emplace_back(1.0, 0.2, rose::Theme::dGreen);
                scaleBackgroundElements.emplace_back(0.5, 0.9, rose::Theme::dRed);
                break;
            case Gradient::RedToGreen:
                scaleBackgroundElements.emplace_back(0.5, 0.2, rose::Theme::dRed);
                scaleBackgroundElements.emplace_back(1.0, 0.9, rose::Theme::dGreen);
                break;
            case Gradient::GreenYellowRed:
                scaleBackgroundElements.emplace_back(0.6, 0.2, rose::Theme::dGreen);
                scaleBackgroundElements.emplace_back(0.8, 0.2, rose::Theme::dYellow);
                scaleBackgroundElements.emplace_back(1.0, 0.2, rose::Theme::dRed);
                break;
            case Gradient::RedYellowGreen:
                scaleBackgroundElements.emplace_back(0.2, 0.2, rose::Theme::dRed);
                scaleBackgroundElements.emplace_back(0.4, 0.2, rose::Theme::dYellow);
                scaleBackgroundElements.emplace_back(1.0, 0.2, rose::Theme::dGreen);
                break;
        }

        drawGradientBackground(renderer, scaleBackgroundElements.begin(), scaleBackgroundElements.end(), dst, orientation);
    }
}
