//
// Created by richard on 2020-10-21.
//

#include "Label.h"


namespace rose {

    Label::Label() = default;

    Label::Label(const std::string &text, RoseImageId badge) {
        mText = text;
        mBadge = badge;
    }

    void Label::initializeComposite() {
        Widget::initializeComposite();
        auto theme = Widget::rose()->theme();
        mLayoutHints.mElastic = true;
        mFontSize = theme.mFontPointSize;
        mFontName = theme.mDefaultFontName;
        mTextColor = theme.mTextColour;
        mLabelBadgeSpace = theme.mLabelBadgeSpace;
        mTextureDirty = true;
        mTextRx = std::make_shared<Slot<std::string>>();
        mTextRx->setCallback([=](uint32_t, const std::string &text){
            setText(text);
        });
        mClassName = "Label";
    }

    Rectangle Label::initialLayout(sdl::Renderer &renderer, Rectangle available) {
        auto labelAvailable = clampAvailableArea(available, mPos, mSize);

        if (!mFont) {
            fetchFont();
        }

        int w = 0, h = 0;
        TTF_SizeUTF8(mFont->get(), mText.c_str(), &w, &h);

        if (mText.empty() && mBadge == RoseImageInvalid) {
            mTextSize = Size{max(w, h), max(w, h)};
        } else if (!mText.empty() && mBadge == RoseImageInvalid) {
            mTextSize = Size{w, h};
        } else if (mText.empty() && mBadge != RoseImageInvalid) {
            SDL_Rect badgeRect;
            rose()->imageRepository(mBadge).getRectangle(badgeRect);
            mTextSize = Size{max(h, badgeRect.w), max(h, badgeRect.w)};
        } else {
            SDL_Rect badgeRect;
            rose()->imageRepository(mBadge).getRectangle(badgeRect);
            mTextSize = Size{w + badgeRect.w + mLabelBadgeSpace, h};
        }

        if (!mPos)
            mPos = Position::Zero;
        mSize = Size{mTextSize.width(), mFontMetrics.fontHeight};
        return Rectangle{mPos, mSize};
    }

    void Label::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            auto sRose = rose();
            auto widgetRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            if (mTextureDirty || mBadgeDirty) {
                initialLayout(renderer, parentRect);
                mTextureDirty = mBadgeDirty = false;

                if (mText.empty() && mBadge == RoseImageInvalid) {
                    mTextureDirty = mBadgeDirty = false;
                    mTexture.reset(nullptr);
                } else if (!mText.empty() && mBadge == RoseImageInvalid) {
                    auto fg = mTextColor.toSdlColor();
                    sdl::Surface surface{TTF_RenderUTF8_Blended(mFont.value().get(), mText.c_str(), fg)};
                    mTextSize = Size{surface->w, surface->h};
                    mTexture = surface.toTexture(renderer);

                    int maxY = 0;
                    for (auto &c : mText) {
                        int maxy;
                        TTF_GlyphMetrics(mFont.value().get(), c, nullptr, nullptr, nullptr, &maxy, nullptr);
                        maxY = std::max(maxY, maxy);
                    }
                    mLayoutHints.mBaseLine = maxY;
                } else if (mText.empty() && mBadge != RoseImageInvalid) {
                    auto badgeRect = sRose->imageRepository(mBadge).getRectangle();

                    sdl::TextureData composite{renderer, SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,
                                                                 mSize->width(), mSize->height()};

                    composite.setBlendMOde(SDL_BLENDMODE_BLEND);
                    sdl::RenderTargetGuard renderTargetGuard(renderer, composite);
                    sdl::DrawColorGuard drawColorGuard(renderer, color::RGBA::TransparentBlack);
                    renderer.renderClear();

                    Rectangle src{
                            badgeRect.width() <= mSize->width() ? 0 : (badgeRect.width() - mSize->width()) / 2,
                            badgeRect.height() <= mSize->height() ? 0 : (badgeRect.height() - mSize->height()) / 2,
                            badgeRect.width() <= mSize->width() ? badgeRect.width() : mSize->width(),
                            badgeRect.height() <= mSize->height() ? badgeRect.height() : mSize->height()
                    };
                    Rectangle dst{
                            badgeRect.width() <= mSize->width() ? (mSize->width() - badgeRect.width()) / 2 : 0,
                            badgeRect.height() <= mSize->height() ? (mSize->height() - badgeRect.height()) / 2 : 0,
                            badgeRect.width() <= mSize->width() ? badgeRect.width() : mSize->width(),
                            badgeRect.height() <= mSize->height() ? badgeRect.height() : mSize->height()
                    };
                    sRose->imageRepository().renderCopy(renderer, mBadge, src, dst);
                    mTexture = std::move(composite);
                } else {
                    auto fg = mTextColor.toSdlColor();
                    sdl::Surface surface{TTF_RenderUTF8_Blended(mFont.value().get(), mText.c_str(), fg)};
                    mTextSize = Size{surface->w, surface->h};
                    mTexture = surface.toTexture(renderer);

                    Rectangle badgeRect = sRose->imageRepository(mBadge).getRectangle();

                    sdl::TextureData composite{SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                                                 SDL_TEXTUREACCESS_TARGET,
                                                                 mSize->width(), mSize->height())};

                    composite.setBlendMOde(SDL_BLENDMODE_BLEND);
                    sdl::RenderTargetGuard renderTargetGuard(renderer, composite);
                    sdl::DrawColorGuard drawColorGuard(renderer, color::RGBA::TransparentBlack);
                    renderer.renderClear();

                    Rectangle src{
                            0,
                            badgeRect.height() <= mSize->height() ? 0 : (badgeRect.height() - mSize->height()) / 2,
                            badgeRect.width() <= mSize->width() ? badgeRect.width() : mSize->width(),
                            badgeRect.height() <= mSize->height() ? badgeRect.height() : mSize->height()
                    };
                    Rectangle dst{
                            mBadgeRight ? mSize->width() - badgeRect.width() : 0,
                            badgeRect.height() <= mSize->height() ? (mSize->height() - badgeRect.height()) / 2 : 0,
                            badgeRect.width(), mTextSize.height()
                    };
                    sRose->imageRepository().renderCopy(renderer, mBadge, src, dst);
                    dst.x() = mBadgeRight ? 0 : badgeRect.width() + mLabelBadgeSpace;
                    dst.y() = 0;
                    dst.width() = mTextSize.width();
                    dst.height() = mTextSize.height();
                    renderer.renderCopy(mTexture, dst);
                    mTexture = std::move(composite);
                }
            }

            Rectangle dst{widgetRect.getPosition(), mSize.value()};

            switch (mLayoutHints.mLabelHorAlign) {
                case LabelHorizontalAlignment::Unset:
                case LabelHorizontalAlignment::Left:
                    break;
                case LabelHorizontalAlignment::Right:
                    if (parentRect.width() > mSize->width())
                        dst.x() += mLayoutHints.mAssignedRect->width() - mSize->width();
                    break;
                case LabelHorizontalAlignment::Center:
                    if (parentRect.width() > mSize->width())
                        dst.x() += (mLayoutHints.mAssignedRect->width() - mSize->width()) / 2;
                    break;
            }

            switch (mLayoutHints.mLabelVerAlign) {
                case LabelVerticalAlignment::Unset:
                case LabelVerticalAlignment::Center:
                    if (parentRect.height() > mSize->height())
                        dst.y() += (mLayoutHints.mAssignedRect->height() - mSize->height()) / 2;
                    break;
                case LabelVerticalAlignment::Top:
                case LabelVerticalAlignment::Baseline:
                    // Layout position will be provided by the parent Container so
                    // the label should align as if by LabelVerticalAlignment::Top
                    break;
                case LabelVerticalAlignment::Bottom:
                    if (parentRect.height() > mSize->height())
                        dst.y() += mLayoutHints.mAssignedRect->height() - mSize->height();
                    break;
            }

            renderer.renderCopy(mTexture, dst);
        }
    }

    void Label::fetchFont() {
        auto sRose = rose();
        if (!mFont) {
            mFont = sRose->fontCache().getFont(mFontName, mFontSize);
            if (!mFont) {
                mFont = sRose->fontCache().getFont(sRose->theme().mDefaultFontName, mFontSize);
                if (!mFont)
                    throw std::runtime_error(StringCompositor("Neither font", mFontName, " nor default font found: ",
                                                              FILE_LOC));
            }
        }
        mFontMetrics.fontAscent = TTF_FontAscent(mFont.value().get());
        mFontMetrics.fontDescent = TTF_FontDescent(mFont.value().get());
        mFontMetrics.fontHeight = TTF_FontHeight(mFont.value().get());
        mFontMetrics.fontLineSkip = TTF_FontLineSkip(mFont.value().get());
    }

    void Label::createTexture(sdl::Renderer &renderer) {
        if (!mFont)
            fetchFont();

        auto fg = mTextColor.toSdlColor();
        sdl::Surface surface{TTF_RenderText_Blended(mFont.value().get(), mText.c_str(), fg)};
        mTextSize = Size{surface->w, surface->h};
        mTexture.reset(SDL_CreateTextureFromSurface(renderer.get(), surface.get()));
    }

    void Label::compositeBadge(sdl::Renderer &renderer) {
        auto sRose = rose();
        SDL_Rect badgeSrc;
        sRose->imageRepository(mBadge).getRectangle(badgeSrc);

        sdl::TextureData composite{renderer, SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,
                                                     mTextSize.width() + badgeSrc.w + mLabelBadgeSpace,
                                                     mTextSize.height()};

        composite.setBlendMOde(SDL_BLENDMODE_BLEND);
        sdl::RenderTargetGuard compositeGuard{renderer, composite};
        sdl::DrawColorGuard drawColorGuard{renderer, color::RGBA::TransparentBlack};
        renderer.renderClear();

        Rectangle dst{mBadgeRight ? 0 : badgeSrc.w + mLabelBadgeSpace, 0,
                         mTextSize.width(), mTextSize.height()};
        renderer.renderCopy(mTexture, dst);

        dst.x() = mBadgeRight ? mTextSize.width() + mLabelBadgeSpace : 0;
        dst.width() = badgeSrc.w;
        if (badgeSrc.h < dst.height()) {
            dst.y() = (dst.height() - badgeSrc.h) / 2;
            dst.height() = badgeSrc.h;
        } else {
            badgeSrc.y = (badgeSrc.h - dst.height()) / 2;
            badgeSrc.h = dst.height();
        }
        sRose->imageRepository().renderCopy(renderer, mBadge, dst);

        mTextSize.width() = mTextSize.width() + badgeSrc.w + mLabelBadgeSpace;
        mTexture = std::move(composite);
    }

    std::tuple<int, int, int, int, int> rose::Label::getGlyphMetrics(char glyph) {
//        int minx{}, maxx{}, miny{}, maxy{}, advance{};
//        fetchFont();
//        TTF_GlyphMetrics(mFont.value().get(), glyph, &minx, &maxx, &miny, &maxy, &advance);
//        return std::make_tuple(minx, maxx, miny, maxy, advance);
        return rose::getGlyphMetrics(mFont, glyph);
    }

    std::tuple<int, int, int, int> rose::Label::getFontMetrics() {
        int fontHeight{}, fontAscent{}, fontDescent{}, fontLineSkip{};
        fetchFont();
        fontHeight = TTF_FontHeight(mFont.value().get());
        fontAscent = TTF_FontAscent(mFont.value().get());
        fontDescent = TTF_FontDescent(mFont.value().get());
        fontLineSkip = TTF_FontLineSkip(mFont.value().get());
        return std::make_tuple(fontHeight, fontAscent, fontDescent, fontLineSkip);
    }
}
