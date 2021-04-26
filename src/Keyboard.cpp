/**
 * @file Keyboard.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-26
 */

#include "Keyboard.h"
#include "ImageStore.h"
#include "Application.h"

namespace rose {

    void Keyboard::addedToContainer() {
        Node::addedToContainer();
        auto grid = getNode<Grid>();
        for( auto& key : keyDataRow0) {
            if (key[0] >= ' ')
                grid << wdg<LetterKey>(key);
            else
                grid << wdg<ImageKey>(key);
        }

        back()->getNode<Visual>() << LayoutHint{LayoutHint::GridLayoutHint::EndStride, 0};

        bool first = true;
        for( auto& key : keyDataRow1) {
            if (key[0] >= ' ')
                grid << wdg<LetterKey>(key);
            else
                grid << wdg<ImageKey>(key);
            if (first) {
                back()->getNode<Visual>() << LayoutHint{LayoutHint::GridLayoutHint::AxisOffset, 50};
                first = false;
            }
        }

        back()->getNode<Visual>()
            << LayoutHint{LayoutHint::GridLayoutHint::EndStride, 0}
            << LayoutHint{LayoutHint::GridLayoutHint::AxisSize, 150};

        for( auto& key : keyDataRow2) {
            if (key[0] >= ' ')
                grid << wdg<LetterKey>(key);
            else
                grid << wdg<ImageKey>(key);
        }
        back()->getNode<Visual>()
            << LayoutHint{LayoutHint::GridLayoutHint::EndStride, 0};

        for( auto& key : keyDataRow3) {
            if (key[0] > ' ')
                grid << wdg<LetterKey>(key);
            else if (key[0] == ' ')
                grid << wdg<LetterKey>(key) << LayoutHint{LayoutHint::GridLayoutHint::AxisSize, 600};
            else
                grid << wdg<ImageKey>(key);
        }
    }

    void ImageKey::addedToContainer() {
        setKeyImage();
    }

    void ImageKey::setKeyImage() {
        switch (mFaceData[mKeyState]) {
            case Keyboard::CR:
                setImage(ImageId::LevelDown);
                setRenderFlip(gm::RenderFlip{SDL_FLIP_HORIZONTAL});
                break;
            case Keyboard::BS:
                setImage(ImageId::Back);
                break;
            case Keyboard::CapLock:
                setImage(ImageId::Lock);
                break;
            case Keyboard::CapUnlock:
                setImage(ImageId::LockOpen);
                break;
            case Keyboard::RightSh:
            case Keyboard::LeftSh:
                setImage(ImageId::UpBold);
                break;
            case Keyboard::KB:
                setImage(ImageId::Keyboard);
                break;
            case Keyboard::Dots2:
                setImage(ImageId::TwoDots);
                break;
            case Keyboard::Dots3:
                setImage(ImageId::ThreeDots);
                break;
            case Keyboard::LeftArw:
                setImage(ImageId::Left);
                break;
            case Keyboard::RightArw:
                setImage(ImageId::Right);
                break;
        }
        getApplication().redrawBackground();
    }
}
