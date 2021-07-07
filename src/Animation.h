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
#include <map>
#include <vector>
#include "constexpertrig.h"
#include "GraphicsModel.h"
namespace rose {

    /**
     * @enum AnimationEnable
     * @brief An enumeration to enable or disable animation.
     */
    enum class AnimationEnable {
        Disable, Enable,
    };

    namespace ActionCurves {
        static constexpr float FrameRate = 30.f;
        constexpr float ac_sin(int t) {
            return cx_math::sin(cx_math::pi<float> * (float)t / FrameRate);
        }

        constexpr float ac_inv_sin(int t) {
            return 1.f - cx_math::sin(cx_math::pi<float> * (float)t / FrameRate);
        }

        class ActionCurve {
        public:
            ActionCurve() = default;
            virtual ~ActionCurve() = default;

            [[nodiscard]] virtual size_t size() const = 0;

            virtual float operator[](size_t idx) const = 0;
        };

        class PulsePerSecond : public ActionCurve {
        protected:
            static constexpr std::array<float, 30> data{
                    ac_sin(0), ac_sin(2), ac_sin(4), ac_sin(6),
                    ac_sin(8), ac_sin(10), ac_sin(12), ac_sin(14),
                    ac_sin(16), ac_sin(18), ac_sin(20), ac_sin(22),
                    ac_sin(24), ac_sin(26), ac_sin(28), 0.f,
                    0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                    0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                    0.f, 0.f,
            };

        public:
            ~PulsePerSecond() override = default;

            [[nodiscard]] size_t size() const override {
                return data.size();
            }

            float operator[](size_t idx) const override {
                return data[idx];
            }
        };

        class HeartBeat : public ActionCurve {
        protected:
            static constexpr std::array<float, 30> data{
                    ac_sin(0), ac_sin(6),ac_sin(12),ac_sin(18),
                    ac_sin(24), 0.f,0.f, 0.f,0.f, 0.f,
                    ac_sin(0), ac_sin(6),ac_sin(12),ac_sin(18),
                    ac_sin(24), 0.f,0.f, 0.f,0.f, 0.f,
                    0.f, 0.f,0.f, 0.f, 0.f, 0.f,
                    0.f, 0.f,0.f, 0.f,
            };

        public:
            ~HeartBeat() override = default;

            [[nodiscard]] size_t size() const override {
                return data.size();
            }

            float operator[](size_t idx) const override {
                return data[idx];
            }
        };

        class CursorPulse : public ActionCurve {
        protected:
            static constexpr std::array<float, 30> data {
                    ac_inv_sin(0), ac_inv_sin(6),ac_inv_sin(12),ac_inv_sin(18),
                    ac_inv_sin(24), 1.f,1.f, 1.f,1.f, 1.f,
                    1.f,1.f, 1.f,1.f, 1.f, 1.f,
                    1.f,1.f, 1.f,1.f, 1.f, 1.f,
                    1.f,1.f, 1.f,1.f, 1.f, 1.f,
                    1.f,1.f
            };

        public:
            ~CursorPulse() override = default;

            [[nodiscard]] size_t size() const override {
                return data.size();
            }

            float operator[](size_t idx) const override {
                return data[idx];
            }
        };
    }

    template<class Curve, typename ... Args>
    std::unique_ptr<Curve> curve(Args ... args) {
        return std::make_unique<Curve>(args ... );
    }

    class Animation;
    class Animator {
    protected:
        using AnimationList = std::vector<std::pair<std::shared_ptr<Animation>, Position<int>>>;
        std::map<std::shared_ptr<Window>,AnimationList> mAnimations;

        Animator() = default;
    public:
        static Animator& getAnimator() {
            static Animator instance{};
            return instance;
        }

        void set(const std::shared_ptr<Window> &window, std::shared_ptr<Animation> &animation,
                 const Position<int> &position);

        void remove(const std::shared_ptr<Window>& window, std::shared_ptr<Animation>& animation);

        void animate(const std::shared_ptr<Window>& window, gm::Context &context, uint32_t frame);

        explicit operator bool() const noexcept {
            return !mAnimations.empty();
        }

        [[nodiscard]] bool areAnimationsForWindow(const std::shared_ptr<Window>& window) const noexcept {
            return mAnimations.find(window) != mAnimations.end();
        }
    };

    /**
     * @class Animation
     * @brief
     */
    class Animation {
        friend class Animator;
    public:
        using Callback = std::function<void(gm::Context &context, const Position<int> &, uint32_t frame)>;
        using Enable = std::function<void(AnimationEnable animationEnable)>;

        std::unique_ptr<ActionCurves::ActionCurve> mActionCurve{};

    protected:
        Callback mAnimationCallback;

        Enable mAnimationEnableStateCallback;

        AnimationEnable mAnimationEnableState{AnimationEnable::Disable};

    public:
        Animation();

        virtual ~Animation() = default;

        static void setAnimation(const std::shared_ptr<Window> &window, std::shared_ptr<Animation> animation,
                                 const Position<int> &position) {
            Animator::getAnimator().set(window, animation, position);
        }

        static void removeAnimation(const std::shared_ptr<Window>& window, std::shared_ptr<Animation> animation) {
            Animator::getAnimator().remove(window, animation);
        }

        void setActionCurve(std::unique_ptr<ActionCurves::ActionCurve> &&actionCurve) {
            mActionCurve = std::move(actionCurve);
        }

        void setAnimationEnable(AnimationEnable animationEnable) {
            mAnimationEnableState = animationEnable;
            if (mAnimationEnableStateCallback)
                mAnimationEnableStateCallback(animationEnable);
        }
    };
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, std::unique_ptr<rose::ActionCurves::ActionCurve>&& curve) {
    static_assert(std::is_base_of_v<rose::Animation,WidgetClass>, "WidgetClass must be derived from rose::Animation.");
    widget->setActionCurve(std::move(curve));
    return widget;
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::AnimationEnable animationEnable) {
    static_assert(std::is_base_of_v<rose::Animation,WidgetClass>, "WidgetClass must be derived from rose::Animation.");
    widget->setAnimationEnable(animationEnable);
    return widget;
}
