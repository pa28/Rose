/**
 * @file Frame.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-12
 */

#include "Frame.h"
#include "Utilities.h"

namespace rose {
    void FrameElements::trimCorners(gm::Surface &surface, color::RGBA color, FrameElements::SelectedCorners selectedCorners,
                                    Size cornerSize,
                                    Size frameSize) {
        auto trimColor = color;
        trimColor.a() = 0.f;
        auto trimCorner = [&surface, &trimColor](int x0, int y0, int xw, int yh, int R2) {
            for (int x = x0; x0 < xw ? x < xw : x > xw; x0 < xw ? ++x : --x)
                for (int y = y0; y0 < yh ? y < yh : y > yh; y0 < yh ? ++y : --y) {
                    int xr = xw - x;
                    int yr = yh - y;
                    int r2 = xr * xr + yr * yr;
                    if (r2 > R2) {
                        surface.pixel(x, y) = gm::mapRGBA(surface.get()->format, trimColor);
                    }
                }
        };

        cornerSize.w /= 2;
        cornerSize.h /= 2;
        int R2 = cornerSize.w * cornerSize.w;
        if (selectedCorners & SelectedCorners::TopLeftCorner)
            trimCorner(0, 0, cornerSize.w, cornerSize.h, R2);
        if (selectedCorners & SelectedCorners::TopRightCorner)
            trimCorner(frameSize.w - 1, 0, frameSize.w - cornerSize.w - 1, cornerSize.h, R2);
        if (selectedCorners & SelectedCorners::BottomLeftCorner)
            trimCorner(0, frameSize.h - 1, cornerSize.w, frameSize.h - cornerSize.h - 1, R2);
        if (selectedCorners & SelectedCorners::BottomRightCorner)
            trimCorner(frameSize.w - 1, frameSize.h - 1, frameSize.w - cornerSize.w - 1,
                       frameSize.h - cornerSize.h - 1,
                       R2);
    }

    void FrameElements::renderSelectedCorners(gm::Context& context, SelectedCorners selectedCorners, ImageId corner,
                                              const Size &size) {
        auto cornerSize = ImageStore::getStore().size(corner);

        auto w = cornerSize.w / 2;
        auto h = cornerSize.h / 2;

        for (SelectedCorners s = SelectedCorners::BottomRightCorner; (s & SelectedCorners::AllCorners) != 0;
             s = SelectedCorners(s << 1U)) {
            Rectangle src{0, 0, w, h}, dst{0, 0, w, h};
            if (s & selectedCorners) {
                if (s & SelectedCorners::TopLeftCorner) {
                } else if (s & SelectedCorners::TopRightCorner) {
                    src.x = w;
                    src.y = 0;
                    dst.x = size.w - w;
                    dst.y = 0;
                } else if (s & SelectedCorners::BottomLeftCorner) {
                    src.x = 0;
                    src.y = w;
                    dst.x = 0;
                    dst.y = size.h - w;
                } else if (s & SelectedCorners::BottomRightCorner) {
                    src.x = w;
                    src.y = w;
                    dst.x = size.w - w;
                    dst.y = size.h - w;
                }
                ImageStore::getStore().renderCopy(context, corner, src, dst);
            }
        }
    }

    void
    FrameElements::renderSelectedSides(gm::Context& context, FrameElements::SelectedSides selectedSides, UseBorder useBorder,
                                       ImageId corner, const Size &size, int extend) {
        auto cornerSize = ImageStore::getStore().size(corner);
        Rectangle fill0, fill1;
        color::RGBA color0, color1;

        for (SelectedSides s = SelectedSides::TopSide; (s & SelectedSides::AllSides) != 0; s = SelectedSides(s << 1U)) {
            if ((s & selectedSides) & SelectedSides::TopSide) {
                fill0.x = fill1.x = (extend < 0 ? 0 : cornerSize.w / 2);
                fill0.y = 0;
                fill1.y = mFrameWidth / 2;
                fill0.w = fill1.w = size.w - cornerSize.w / (extend ? 2 : 1);
                fill0.h = fill1.h = mFrameWidth / 2;
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
                fill0.x = fill1.x = (extend < 0 ? 0 : cornerSize.w / 2);
                fill0.y = size.h - mFrameWidth;
                fill1.y = fill0.y + mFrameWidth / 2;
                fill0.w = fill1.w = size.w - cornerSize.w / (extend ? 2 : 1);
                fill0.h = fill1.h = mFrameWidth / 2;
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
                fill0.x = 0;
                fill1.x = mFrameWidth / 2;
                fill0.y = fill1.y = (extend < 0 ? 0 : cornerSize.w / 2);
                fill0.w = fill1.w = mFrameWidth / 2;
                fill0.h = fill1.h = size.h - cornerSize.h / (extend ? 2 : 1);
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
                fill0.x = size.w - mFrameWidth;
                fill1.x = size.w - mFrameWidth / 2;
                fill0.y = fill1.y = (extend < 0 ? 0 : cornerSize.w / 2);
                fill0.w = fill1.w = mFrameWidth / 2;
                fill0.h = fill1.h = size.h - cornerSize.h / (extend ? 2 : 1);
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
                context.fillRect(fill0, color0);
                context.fillRect(fill1, color1);
            }
        }
    }

    std::tuple<UseBorder, FrameElements::SelectedCorners> FrameElements::decoration() {
        SelectedCorners selectedCorners = NoCorners;
        UseBorder useBorder;
        switch (mBorderStyle) {
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

        return std::make_tuple(useBorder, selectedCorners);
    }

    gm::Texture FrameElements::createBackgroundTexture(gm::Context &context, Rectangle &src, Rectangle &dst,
                                                       const color::RGBA &color) {
        auto [useBorder,selectedCorners] = decoration();

        gm::Surface surface{dst.w, dst.h, 32, SDL_PIXELFORMAT_RGBA8888};
        if (!surface)
            throw gm::SurfaceRuntimeError(StringCompositor(__PRETTY_FUNCTION__ , " (", dst.w, ',', ") :", SDL_GetError()));

        Rectangle rectangle{mFrameWidth, mFrameWidth, dst.w - mFrameWidth*2, dst.h - mFrameWidth*2};
        surface.fillRectangle(rectangle, color);

        if (mCornerStyle == CornerStyle::Round) {
            auto cornerSize = ImageStore::getStore().size(ImageId::BevelOutRoundCorners);
            trimCorners(surface, color, selectedCorners, cornerSize, dst.size());
        }

        auto texture = surface.toTexture(context);
        return std::move(texture);
    }

    void FrameElements::drawBackground(gm::Context &context, Rectangle &src, Rectangle &dst) {
        gm::Texture texture{context, src.size()};

        auto [useBorder,selectedCorners] = decoration();

        texture.setBlendMode(SDL_BLENDMODE_NONE);
        mBorder = gm::Texture{context, dst.size()};

        gm::RenderTargetGuard renderTargetGuard(context, mBorder);
        context.renderCopy(texture);
        mBorder.setBlendMode(SDL_BLENDMODE_BLEND);

        ImageId roundCnr = ImageId::NoImage;
        ImageId squareCnr = ImageId::NoImage;

        switch (useBorder) {
            case None:
                break;
            case BevelOut:
                roundCnr = ImageId::BevelOutRoundCorners;
                squareCnr = ImageId::BevelOutSquareCorners;
                break;
            case BevelIn:
                roundCnr = ImageId::BevelInRoundCorners;
                squareCnr = ImageId::BevelInSquareCorners;
                break;
            case NotchIn:
                roundCnr = ImageId::NotchInRoundCorners;
                squareCnr = ImageId::NotchInSquareCorners;
                break;
            case NotchOut:
                roundCnr = ImageId::NotchOutRoundCorners;
                squareCnr = ImageId::NotchOutSquareCorners;
                break;
        }

        if (mBorderStyle != BorderStyle::None && mBorderStyle != BorderStyle::Unset) {
            auto corner = mCornerStyle == CornerStyle::Round ? roundCnr : squareCnr;
            if (mBorderStyle == BorderStyle::Notch || mBorderStyle == BorderStyle::Bevel || mBorderStyle == BorderStyle::BevelIn) {
                renderSelectedCorners(context, SelectedCorners::AllCorners, corner, dst.size());
                renderSelectedSides(context, SelectedSides::AllSides, useBorder, corner, dst.size(), 0);
            } else if (mBorderStyle == BorderStyle::TabTop) {
                renderSelectedCorners(context, SelectedCorners::TopCorners, corner, dst.size());
                renderSelectedSides(context, SelectedSides::TopSide, useBorder, corner, dst.size(), 0);
                renderSelectedSides(context, (SelectedSides) (SelectedSides::LeftSide | SelectedSides::RightSide),
                                    useBorder, corner, dst.size(), 1);
                if (mInvert) {
                    renderSelectedCorners(context, SelectedCorners::BottomCorners, squareCnr, dst.size());
                    renderSelectedSides(context, SelectedSides::BotSide, useBorder, corner, dst.size(), 0);
                }
            } else if (mBorderStyle == BorderStyle::TabLeft) {
                renderSelectedCorners(context, SelectedCorners::LeftCorners, corner, dst.size());
                renderSelectedSides(context, SelectedSides::LeftSide, useBorder, corner, dst.size(), 0);
                renderSelectedSides(context, (SelectedSides) (SelectedSides::TopSide | SelectedSides::BotSide),
                                    useBorder, corner, dst.size(), 1);
                if (mInvert) {
                    renderSelectedCorners(context, SelectedCorners::RightCorners, squareCnr, dst.size());
                    renderSelectedSides(context, SelectedSides::RightSide, useBorder, corner, dst.size(), 0);
                }
            } else if (mBorderStyle == BorderStyle::TabPage) {
                renderSelectedCorners(context, SelectedCorners::BottomCorners, corner, dst.size());
                renderSelectedSides(context, SelectedSides::BotSide, useBorder, corner, dst.size(), 0);
                renderSelectedSides(context, (SelectedSides) (SelectedSides::LeftSide | SelectedSides::RightSide),
                                    useBorder, corner, dst.size(), -1);
            }
        }
    }

    gm::Texture
    FrameElements::createBackgroundMask(gm::Context &context, const Size size, int frameWidth, bool roundCorners) {
        gm::Texture mask{context, size};
        mask.setBlendMode(SDL_BLENDMODE_NONE);
        ImageStore &is{ImageStore::getStore()};

        {
            gm::RenderTargetGuard renderTargetGuard(context, mask);
            Rectangle filterFill{frameWidth, frameWidth, size.w - frameWidth * 2, size.h - frameWidth * 2};
            context.fillRect(filterFill, color::RGBA::OpaqueBlack);

            if (roundCorners) {
                auto trimSize = is.size(ImageId::RoundCornerTrim) / 2;
                Rectangle trimSrc{Position::Zero, trimSize}, trimDst{Position::Zero, trimSize};

                is.renderCopy(context, ImageId::RoundCornerTrim, trimSrc, trimDst);
                trimDst.x = size.w - trimSrc.w;
                trimSrc.x = trimSize.w;
                is.renderCopy(context, ImageId::RoundCornerTrim, trimSrc, trimDst);
                trimDst.y = size.h - trimSrc.h;
                trimSrc.y = trimSize.h;
                is.renderCopy(context, ImageId::RoundCornerTrim, trimSrc, trimDst);
                trimDst.x = 0;
                trimSrc.x = 0;
                is.renderCopy(context, ImageId::RoundCornerTrim, trimSrc, trimDst);
            }
            mask.setBlendMode(SDL_BLENDMODE_BLEND);
        }

        return std::move(mask);
    }

    void FrameElements::colorBackgroundMask(gm::Context &context, gm::Texture &mask, const color::RGBA &base,
                                            const color::RGBA &active, float value) {
        value = std::clamp(value, 0.0f, 1.0f);
        Rectangle dst{Position::Zero, mask.getSize()};
        gm::RenderTargetGuard renderTargetGuard(context, mask);
        context.setDrawBlendMode(SDL_BLENDMODE_ADD);
        auto interpolated = base.interpolate(active, value);
        context.fillRect(dst, interpolated);
        context.setDrawBlendMode(SDL_BLENDMODE_BLEND);
        mask.setBlendMode(SDL_BLENDMODE_BLEND);
        std::cout << ' ' << value << '\t' << interpolated << base << active << '\n';
    }

    void FrameElements::drawFrame(gm::Context &context, Rectangle widgetRect) {
        Rectangle src{0, 0, widgetRect.w, widgetRect.h};
        Rectangle dst{widgetRect};

        if (mBorderStyle != BorderStyle::None) {
            if (!mBorder) {
                drawBackground(context, src, dst);
                mBorder.setBlendMode(SDL_BLENDMODE_BLEND);
            }
        }

        if (!mAnimagedBG) {
            mAnimagedBG = createBackgroundMask(context, src.size(), mFrameWidth, mCornerStyle == CornerStyle::Round);
            colorBackgroundMask(context, mAnimagedBG, mActiveColor, mInactiveColor, 0.);
        }

        if (!mInactiveBG) {
            mInactiveBG = createBackgroundMask(context, src.size(), mFrameWidth, mCornerStyle == CornerStyle::Round);
            colorBackgroundMask(context, mInactiveBG, mInactiveColor, mActiveColor, 0.);
        }

        context.renderCopy(mBorder, dst);
        context.renderCopy(mInactiveBG, dst);
        mAnimagedBG.setAlphaMod(mColorValue);
        context.renderCopy(mAnimagedBG, dst);
    }

    Rectangle
    FrameLayoutManager::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                      LayoutManager::Itr last) {
        if (first == last) {
            return Rectangle{};
        }

        Rectangle layoutRect{};
        if (auto manager = std::dynamic_pointer_cast<Manager>(*first); manager) {
            layoutRect = manager->layout(context, screenRect);
            manager->setScreenRectangle(layoutRect);
        } else if (auto widget = std::dynamic_pointer_cast<Widget>(*first); widget) {
            layoutRect = widget->layout(context, screenRect);
            widget->setScreenRectangle(layoutRect);
        }
        return layoutRect;
    }

    void Frame::drawAnimate(gm::Context &context, const Position &containerPosition) {
        drawFrame(context, Rectangle{containerPosition + mPos, mSize});
        Manager::draw(context, containerPosition + mFramePadding.position() + Position{mFrameWidth});
    }

    Rectangle Frame::layout(gm::Context &context, const Rectangle &screenRect) {
        auto contentRect = Manager::layout(context, screenRect - mFramePadding.size() - Size{mFrameWidth*2});
        contentRect = contentRect + mFramePadding.size() + Size(mFrameWidth*2);
        return contentRect;
    }
}
