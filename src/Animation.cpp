/**
 * @file Animation.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-14
 */

#include "Animation.h"

namespace rose {

    Animation::Animation() = default;

    void Animator::set(const std::shared_ptr<Window> &window, std::shared_ptr<Animation> &animation,
                       const Position &position) {
        if (auto animationList = mAnimations.find(window); animationList != mAnimations.end()) {
            auto itr = std::find_if(animationList->second.begin(), animationList->second.end(),
                                    [&animation](auto ref) -> bool {
                                        return ref.first == animation;
                                    });
            if (itr == animationList->second.end())
                animationList->second.emplace_back(std::make_pair(animation, position));
            else
                itr->second = position;
        } else {
            mAnimations.emplace(window,AnimationList{});
            mAnimations[window].emplace_back(std::make_pair(animation, position));
        }
    }

    void Animator::remove(const std::shared_ptr<Window> &window, std::shared_ptr<Animation> &animation) {
        if (auto animationList = mAnimations.find(window); animationList != mAnimations.end()) {
            animationList->second.erase(std::remove_if(animationList->second.begin(), animationList->second.end(),
                                                       [&animation](auto ref) -> bool {
                                                           return ref.first == animation;
                                                       }));
        }
    }

    void Animator::animate(const std::shared_ptr<Window> &window, gm::Context &context, uint32_t frame) {
        if (auto animationList = mAnimations.find(window); animationList != mAnimations.end()) {
            for (auto &animation : animationList->second) {
                if (animation.first->mAnimationCallback)
                    animation.first->mAnimationCallback(context, animation.second, frame);
            }
        }
    }
}
