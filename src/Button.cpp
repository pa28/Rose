/**
 * @file Button.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-22
 */

#include "Application.h"
#include "Button.h"
#include "Settings.h"
#include "Theme.h"

namespace rose {

    ButtonFrame::ButtonFrame() noexcept : Frame(), mButtonSemantics(*this) {
        mPadding = Padding{Theme::getTheme().ButtonPadding};
    }


    TextButton::TextButton() noexcept : ButtonFrame(), Text() {
        mPointSize = Theme::getTheme().ButtonPointSize;
        mFontName = Theme::getTheme().BoldFont;

        mLayoutManager = std::make_unique<TextButtonLayoutManager>(*this);

        mButtonSemantics.setButtonDisplayCallback([&](ButtonDisplayState buttonDisplayState){
            switch (buttonDisplayState) {
                case ButtonDisplayState::Active:
                    mInvert = true;
                    std::cout << "Display state: Active\n";
                    break;
                case ButtonDisplayState::Inactive:
                    mInvert = false;
                    std::cout << "Display state: Inactive\n";
                    break;
                case ButtonDisplayState::PressedActive:
                    mInvert = false;
                    std::cout << "Display state: PressedActive\n";
                    break;
                case ButtonDisplayState::PressedInactive:
                    mInvert = true;
                    std::cout << "Display state: PressedInactive\n";
                    break;
            }
            getApplication().redrawBackground();
            mInactiveBG.reset();
            if (mBorderStyle != BorderStyle::None)
                mBorder.reset();
        });

        mButtonSemantics.setButtonStateChangeCallback([&](ButtonStateChange buttonStateChange){
            switch (buttonStateChange) {
                case ButtonStateChange::Pushed:
                    std::cout << "Button state: Pushed\n";
                    break;
                case ButtonStateChange::Off:
                    std::cout << "Button state: Off\n";
                    break;
                case ButtonStateChange::On:
                    std::cout << "Button state: On\n";
                    break;
            }
        });
    }

    TextButton::TextButton(const std::string &text, ButtonType buttonType) : TextButton() {
        mText = text;
        mButtonSemantics.setButtonType(buttonType);
    }

    TextButton::TextButton(const Id &id, ButtonType buttonType) : TextButton() {
        mId = id;
        Settings &settings{Settings::getSettings()};
        mText = settings.getValue(id.idString, std::string{id.idString});
        mButtonSemantics.setButtonType(buttonType);
    }

    Rectangle TextButton::layout(gm::Context &context, const Rectangle &screenRect) {
        return Frame::layout(context, screenRect);
    }

    void TextButton::draw(gm::Context &context, const Position &containerPosition) {
        Frame::draw(context, containerPosition);

        if (!mTexture) {
            createTextureBlended(context);
        }

        auto drawPosition = drawPadding(containerPosition) + mPos + mFramePadding.position() + Position{mFrameWidth};

        if (mTexture) {
            std::cout << __PRETTY_FUNCTION__ << drawPosition << '+' << mPos << '+' << mFramePadding.position()
            << '+' << Position{mFrameWidth} << '\n';
            Rectangle dst{drawPosition, mTextSize};
            context.renderCopy(mTexture, dst);
        }
    }

    Rectangle TextButton::layoutContent(gm::Context &context, const Rectangle &screenRect) {
        createTextureBlended(context);
        return Rectangle{Position::Zero, mTextSize};
    }

    TextButtonLayoutManager::TextButtonLayoutManager(TextButton &textButton) : LayoutManager(), mTextButton(textButton) {
        mMaxContent = 0;
    }

    Rectangle
    TextButtonLayoutManager::layoutContent(gm::Context &context, const Rectangle &screenRect, LayoutManager::Itr,
                                           LayoutManager::Itr) {
        return mTextButton.layoutContent(context, screenRect);
    }

    ImageButton::ImageButton() noexcept : ButtonFrame(), Image() {
        mButtonSemantics.setButtonDisplayCallback([&](ButtonDisplayState buttonDisplayState){
            std::cout << __PRETTY_FUNCTION__ << ' ' << (int)buttonDisplayState << '\n';
        });

        mButtonSemantics.setButtonStateChangeCallback([&](ButtonStateChange buttonStateChange){
            std::cout << __PRETTY_FUNCTION__ << ' ' << (int)buttonStateChange << "\n\n";
        });
    }

    ImageButton::ImageButton(ImageId imageId) noexcept: ImageButton() {
        mImageId = imageId;
    }
}
