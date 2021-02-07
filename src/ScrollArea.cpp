//
// Created by richard on 2020-12-07.
//

#include "Math.h"
#include "Rose.h"
#include "ScrollArea.h"

namespace rose {


    Rectangle ScrollArea::widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) {
        auto scrollAvailable = clampAvailableArea(available, mPos, mSize);
        Rectangle childAvailable{0, 0, MaximumChildSize, MaximumChildSize};
        mHorizontalInd.reset();
        mVerticalInd.reset();

        for (auto &child : mChildren) {
            LayoutHints &childHints{child->layoutHints()};
            childHints.mAssignedRect = child->widgetLayout(renderer, childAvailable, 0);
            if (childHints.mAssignedRect->width() < scrollAvailable.width())
                scrollAvailable.width() = childHints.mAssignedRect->width();
            if (childHints.mAssignedRect->height() < scrollAvailable.height())
                scrollAvailable.height() = childHints.mAssignedRect->height();
        }

        return scrollAvailable;
    }

    void ScrollArea::draw(sdl::Renderer &renderer, Rectangle parentRect) {
        if (mVisible) {
            Rectangle screenRect{};
            screenRect = clampAvailableArea(parentRect, mLayoutHints.mAssignedRect);

            sdl::ClipRectangleGuard clipRectangleGuard(renderer, screenRect);
            for (auto &child : mChildren) {
                LayoutHints &childHints{child->layoutHints()};
                if (!mHorizontalInd || !mVerticalInd) {
                    auto scrollW = (float) mLayoutHints.mAssignedRect->width();
                    auto childW = (float) childHints.mAssignedRect->width();
                    auto scrollH = (float) mLayoutHints.mAssignedRect->height();
                    auto childH = (float) childHints.mAssignedRect->height();
                    mRatioX = scrollW / childW;
                    mRatioY = scrollH / childH;

                    if (mRatioX < 1.0) {
                        mHorIndSize = Size{roundToInt(mRatioX * scrollW), mIndWidth};
                        mHorizontalInd = generateScrollIndicator(renderer, mHorIndSize);
                    }

                    if (mRatioY < 1.0) {
                        mVerIndSize = Size{mIndWidth, roundToInt(mRatioY * scrollH)};
                        mVerticalInd = generateScrollIndicator(renderer, mVerIndSize);
                    }
                }

                auto scrollClientRect = childHints.mAssignedRect;
                scrollClientRect.value() = mScrollOffset + screenRect.getPosition();
                child->draw(renderer, scrollClientRect.value());

                if (SDL_GetTicks() - mLastScrollTick > 10000)
                    mShowHorInd = mShowVerInd = false;

                if (mShowHorInd) {
                    Rectangle hdst{roundToInt((float) - mScrollOffset.x() * mRatioX),
                                      mLayoutHints.mAssignedRect->height() - mIndWidth,
                                      mHorIndSize.width(), mHorIndSize.height()};
                    renderer.renderCopy(mHorizontalInd, hdst + screenRect.getPosition());
                    setNeedsDrawing();
                }

                if (mShowVerInd) {
                    Rectangle vdst{mLayoutHints.mAssignedRect->width() - mIndWidth,
                                      roundToInt((float) - mScrollOffset.y() * mRatioY),
                                      mVerIndSize.width(), mVerIndSize.height()};

                    renderer.renderCopy(mVerticalInd, vdst + screenRect.getPosition());
                    setNeedsDrawing();
                }
            }
        }
    }

    bool ScrollArea::mouseEnterEvent(const Position &mousePos, bool enter) {
        return Container::mouseEnterEvent(mousePos - mLayoutHints.mAssignedRect->getPosition(), enter);
    }

    bool ScrollArea::mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) {
        return Container::mouseButtonEvent(mousePos - mLayoutHints.mAssignedRect->getPosition(), button, down, modifiers);
    }

    bool ScrollArea::mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) {
        return Container::mouseMotionEvent(cursorPosition - mLayoutHints.mAssignedRect->getPosition(), rel, button, modifiers);
    }

//    bool ScrollArea::keyboardEvent(int key, int scancode, int action, int modifiers) {
//        return Container::keyboardEvent(key, scancode, action, modifiers);
//    }

    bool ScrollArea::keyboardCharacterEvent(unsigned int codepoint) {
        return Container::keyboardCharacterEvent(codepoint);
    }

    bool ScrollArea::mouseDragEvent(const Position &mousePos, const Position &rel, int button, int modifiers) {
        mLastScrollTick = SDL_GetTicks();
        if (mChildren.empty())
            return false;

        auto size = mLayoutHints.mAssignedRect->getSize();

        for (auto &child : mChildren) {
            auto childHints = child->layoutHints();
            child->mouseEnterEvent(mousePos - mLayoutHints.mAssignedRect->getPosition(), false);
            auto childSize = childHints.mAssignedRect->getSize();
            int clampX = 0, clampY = 0;
            if (childSize.width() > size.width())
                clampX = size.width() - childSize.width();
            if (childSize.height() > size.height())
                clampY = size.height() - childSize.height();
            mScrollOffset.x() = std::clamp<int>(mScrollOffset.x() + rel.x(), clampX, 0);
            mScrollOffset.y() = std::clamp<int>(mScrollOffset.y() + rel.y(), clampY, 0);
        }

        startTimer();

        return true;
    }

    bool ScrollArea::scrollEvent(const Position &mousePos, int32_t x, int32_t y) {
        if (mChildren.empty())
            return false;

        auto size = mLayoutHints.mAssignedRect->getSize();

        auto ticks = SDL_GetTicks();
        int multiplier = 1;
        auto dTicks = ticks - mLastScrollTick;
        mLastScrollTick = ticks;

        if (dTicks > 250)
            multiplier = 1;
        else if (dTicks > 50)
            multiplier = 4;
        else if (dTicks > 10)
            multiplier = 8;
        else
            multiplier = 16;

        for (auto &child : mChildren) {
            auto childHints = child->layoutHints();
            auto childSize = childHints.mAssignedRect->getSize();
            int clampX = 0, clampY = 0;
            if (childSize.width() > size.width())
                clampX = size.width() - childSize.width();
            if (childSize.height() > size.height())
                clampY = size.height() - childSize.height();
            if (clampY != 0) {
                mScrollOffset.y() = std::clamp<int>(mScrollOffset.y() + y * multiplier, clampY, 0);
            } else {
                mScrollOffset.x() = std::clamp<int>(mScrollOffset.x() + y * multiplier, clampX, 0);
            }
        }

        startTimer();

        return true;
    }

    sdl::Texture ScrollArea::generateScrollIndicator(sdl::Renderer &renderer, Size indicatorSize) {
        auto color = Widget::rose()->theme().dBaseColorHSLA.contrasting().toRGBA().toSdlColor();
        sdl::Surface surface{SDL_CreateRGBSurfaceWithFormat(0, indicatorSize.width(), indicatorSize.height(),
                                                            32, SDL_PIXELFORMAT_RGBA8888)};
        SDL_FillRect(surface.get(), nullptr,
                     SDL_MapRGBA(surface.get()->format, color.r, color.g, color.b, 0xC0));
        return sdl::Texture{SDL_CreateTextureFromSurface(renderer.get(), surface.get())};
    }

    void ScrollArea::startTimer() {
        setNeedsDrawing();
        if (sdlTimerId) {
            SDL_RemoveTimer(sdlTimerId.value());
            mShowVerInd = mShowHorInd = true;
            sdlTimerId = SDL_AddTimer(1000, TimerCallbackStub, this);
        } else {
            mShowVerInd = mShowHorInd = true;
            sdlTimerId = SDL_AddTimer(1000, TimerCallbackStub, this);
        }
    }

    std::shared_ptr<Widget> ScrollArea::findWidget(const Position &pos) {
        auto relPos = pos - mLayoutHints.mAssignedRect->getPosition() - mScrollOffset;
        for (auto &child : ReverseContainerView(mChildren)) {
            auto childHints = child->layoutHints();
            if (childHints.mAssignedRect->contains(relPos)) {
                if (auto container = child->as<Container>(); container)
                    return container->findWidget(relPos);
                else
                    return child;
            }
        }

        return getWidget();
    }
}

