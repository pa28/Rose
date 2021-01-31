/** @file Tab.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-07
  */

#pragma once

#include <utility>

#include "SingleChild.h"

/**
 * ToDo: There is an issue that the initial scroll interaction is lost if the click/press lands on a
 * Widget and not a Container (by user observation).
 */

namespace rose {
    /**
     * class ScrollArea
     * brief A Container that provides scrolling to a single child.
     */
    class ScrollArea : public SingleChild {
    protected:
        std::optional<SDL_TimerID> sdlTimerId{};        ///< Timer to turn off scroll indicators.
        int mIndWidth{10};               ///< The width of the indicator bars.

        Position mScrollOffset;      ///< The current scroll offset
        uint32_t mLastScrollTick;       ///< Used to track how fast the scroll wheel is turning.

        sdl::Texture mHorizontalInd{};  ///< An indicator of the position of the Horizontal Scroll.
        sdl::Texture mVerticalInd{};    ///< An indicator of the position of the Vertical Scroll.

        Size mHorIndSize{};          ///< The size of the horizontal indicator
        Size mVerIndSize{};          ///< The size of the vertical indicator

        float mRatioX{};                ///< The horizontal ratio
        float mRatioY{};                ///< The vertical ratio

        bool mShowHorInd{};             ///< If True, display the horizontal scroll indicator.
        bool mShowVerInd{};             ///< If True, display the vertical scroll indicator.

        /// Generate a scroll indicator.
        sdl::Texture generateScrollIndicator(sdl::Renderer &renderer, Size indicatorSize);

        void startTimer();              ///< Start the time to hid the scroll indicators.

        static constexpr int MaximumChildSize = 4096;   ///< The maximum width and height of a widget behind a ScrollArea.

    public:
        /**
         * @brief Constructor
         */
        ScrollArea() : SingleChild(), mScrollOffset(0, 0) {
            mSupportsDrag = true;
            mLastScrollTick = 0;
            mClassName = "ScrollArea";
            mSupportsScrollWheel = true;
        }

        ~ScrollArea() override = default;

        /**
         * @brief The SDL_Timer callback
         * @param interval The timer interval in milliseconds.
         * @param param A pointer to the ScrollArea
         * @return the interval.
         */
        static Uint32 TimerCallbackStub(Uint32 interval, void *param) {
            auto scrollArea = static_cast<ScrollArea *>(param);
            scrollArea->scrollIndicatorsOff();
            return interval;
        }

        /**
         * @brief Timer callback to turn scroll indicators off.
         */
        void scrollIndicatorsOff() {
            if (sdlTimerId) {
                SDL_RemoveTimer(sdlTimerId.value());
                sdlTimerId.reset();
            }
            mShowHorInd = mShowVerInd = false;
            setNeedsDrawing();
        }

        /**
         * @brief See Widget::draw
         */
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /**
         * @brief Determine the desired size of the child widgets.
         * @details If not overridden the default is to call initialLayout() for each child and return
         * the current value of mSize of the Container.
         * @param renderer The Renderer to use
         * @param available The Rectangle available to layout in.
         * @return
         */
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /**
         * @brief Set the scroll offset of the ScrollArea
         * @param scrollOffset the offset position.
         */
        void setScrollOffset(Position scrollOffset) {
            mScrollOffset = scrollOffset;
            setNeedsDrawing();
        }

        /**
         * @brief Find the Widget which uniquely contains the position.
         * @details This version translates positions across scrolled boundaries.
         * @param pos the position the Widget must contain.
         * @return A reference to the Widget found, if any.
         */
        std::shared_ptr<Widget> findWidget(const Position &pos) override;

        /**
         * @brief Get the current scroll offset of the ScrollArea
         * @return the scroll offset position.
         */
        Position getScrollOffset() const { return mScrollOffset; }

        /// Handle a mouse enter event.
        bool mouseEnterEvent(const Position &p, bool enter) override;

        /// Handle a mouse button event (default implementation: propagate to children)
        bool mouseButtonEvent(const Position &mousePos, int button, bool down, int modifiers) override;

        /// Handle a mouse motion event (default implementation: propagate to children)
        bool mouseMotionEvent(const Position &cursorPosition, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse drag event (default implementation: do nothing)
        bool mouseDragEvent(const Position &mousePos, const Position &rel, int button, int modifiers) override;

        /// Handle a mouse scroll event (default implementation: propagate to children)
        bool scrollEvent(const Position &mousePos, int32_t x, int32_t y) override;

        /// Handle a keyboard event (default implementation: do nothing)
        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

        /// Handle text input (UTF-32 format) (default implementation: do nothing)
        bool keyboardCharacterEvent(unsigned int codepoint) override;
    };
}


