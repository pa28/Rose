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

    void FrameElements::drawBackground(gm::Context &context, Rectangle &src, Rectangle &dst) {
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

        gm::Surface surface{dst.w, dst.h, 32, SDL_PIXELFORMAT_RGBA8888};
        if (!surface)
            throw gm::SurfaceRuntimeError(StringCompositor(__PRETTY_FUNCTION__ , " (", dst.w, ',', ") :", SDL_GetError()));

        color::RGBA color;
        if (mBorderStyle == BorderStyle::Notch)
            color = mBaseColor;
        else
            color = !mInvert ? mBaseColor : mInvertColor;

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

        surface.fillRectangle(color);

        if (mCornerStyle == CornerStyle::Round) {
            auto cornerSize = ImageStore::getStore().size(ImageId::BevelOutRoundCorners);
            trimCorners(surface, color, selectedCorners, cornerSize, dst.size());
        }

        auto texture = surface.toTexture(context);
        gm::TextureSetBlendMode(texture, SDL_BLENDMODE_NONE);
        mTexture = gm::CreateTexture(context, dst.size());

        gm::RenderTargetGuard renderTargetGuard(context, mTexture);
        context.renderCopy(texture);
        gm::TextureSetBlendMode(mTexture, SDL_BLENDMODE_BLEND);

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

    void FrameElements::drawFrame(gm::Context &context, Rectangle widgetRect) {
        Rectangle src{0, 0, widgetRect.w, widgetRect.h};
        Rectangle dst{widgetRect};

        if (!mTexture)
            drawBackground(context, src, dst);

        gm::TextureSetBlendMode(mTexture, SDL_BLENDMODE_BLEND);
        context.renderCopy(mTexture, dst);
    }

    Rectangle
    FrameLayoutManager::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr first,
                                      LayoutManager::Itr last) {
        Rectangle layoutRect{};
        if (auto manager = std::dynamic_pointer_cast<Manager>(*first); manager) {
            layoutRect = manager->layout(context, screenRect);
        } else if (auto widget = std::dynamic_pointer_cast<Widget>(*first); widget) {
            layoutRect = widget->layout(context, screenRect);
        }
        return layoutRect;
    }

    void Frame::draw(gm::Context &context, const Position &containerPosition) {
        drawFrame(context, Rectangle{containerPosition + mPos, mSize});
        Manager::draw(context, containerPosition + mFramePadding.position() + Position{mFrameWidth});
    }

    Rectangle Frame::layout(gm::Context &context, const Rectangle &screenRect) {
        auto contentRect = Manager::layout(context, screenRect - mFramePadding.size() - Size{mFrameWidth*2});
        contentRect = contentRect + mFramePadding.size() + Size(mFrameWidth*2);
        return contentRect;
    }
}
