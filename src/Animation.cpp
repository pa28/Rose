/**
 * @file Animation.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-14
 */

#include "Animation.h"

namespace rose {

    Animation::Animation() = default;

    void Animator::set(std::shared_ptr<Animation> &animation, const Position& position) {
        auto itr = std::find_if(mAnimations.begin(), mAnimations.end(), [&animation](auto ref) -> bool {
            return ref.first == animation;
        });
        if (itr == mAnimations.end())
            mAnimations.emplace_back(std::make_pair(animation,position));
        else
            itr->second = position;
    }

    void Animator::remove(std::shared_ptr<Animation> &animation) {
        mAnimations.erase(std::remove_if(mAnimations.begin(), mAnimations.end(), [&animation](auto ref) -> bool {
            return ref.first == animation;
        }),mAnimations.end());
    }

    void Animator::animate(gm::Context &context, uint32_t frame) {
        for (auto &animation : mAnimations) {
            if (animation.first->mAnimationCallback)
                animation.first->mAnimationCallback(context, animation.second, frame);
        }
    }
}
