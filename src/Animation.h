/**
 * @file Animation.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-14
 */

#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>
#include "constexpertrig.h"
#include "GraphicsModel.h"
namespace rose {

    static constexpr float FrameRate = 30.f;
    constexpr float ac_sin(int t) {
        return cx_math::sin(cx_math::pi<float> * (float)t / FrameRate);
    }

    namespace ActionCurves {
        static constexpr std::array<float, 30> PositiveSine{
                ac_sin(0), ac_sin(2), ac_sin(4), ac_sin(6),
                ac_sin(8), ac_sin(10), ac_sin(12), ac_sin(14),
                ac_sin(16), ac_sin(18), ac_sin(20), ac_sin(22),
                ac_sin(24), ac_sin(26), ac_sin(28), 0.f,
                0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                0.f, 0.f,
        };

        static constexpr std::array<float, 38> HeartBeat{
                ac_sin(0), ac_sin(4), ac_sin(8), ac_sin(12),
                ac_sin(16), ac_sin(20), ac_sin(24), ac_sin(28),
                0.f, 0.f, 0.f,
                ac_sin(0), ac_sin(4), ac_sin(8), ac_sin(12),
                ac_sin(16), ac_sin(20), ac_sin(24), ac_sin(28),
                0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                0.f, 0.f,0.f, 0.f, 0.f, 0.f,
                0.f, 0.f,0.f, 0.f,0.f, 0.f,
                0.f,
        };
    }

    class Animation;
    class Animator {
    protected:
        std::vector<std::pair<std::shared_ptr<Animation>,Position>> mAnimations{};

        Animator() = default;
    public:
        ~Animator() = default;

        static Animator& getAnimator() {
            static Animator instance{};
            return instance;
        }

        void set(std::shared_ptr<Animation>& animation, const Position& position);

        void remove(std::shared_ptr<Animation>& animation);

        void animate(gm::Context &context, uint32_t frame);

        explicit operator bool() const noexcept {
            return !mAnimations.empty();
        }
    };

    /**
     * @class Animation
     * @brief
     */
    class Animation {
        friend class Animator;
    public:
        using Callback = std::function<void(gm::Context& context, const Position&, uint32_t frame)>;
    protected:
        Callback mAnimate;
    public:
        Animation();

        virtual ~Animation() = default;

        static void setAnimation(std::shared_ptr<Animation> animation, const Position& position) {
            Animator::getAnimator().set(animation, position);
        }

        static void removeAnimation(std::shared_ptr<Animation>& animation) {
            Animator::getAnimator().remove(animation);
        }

        void animationCallback(Callback cb) {
            mAnimate = std::move(cb);
        }
    };
}

