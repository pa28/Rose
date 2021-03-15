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

    namespace ActionCurves {
        static constexpr float FrameRate = 30.f;
        constexpr float ac_sin(int t) {
            return cx_math::sin(cx_math::pi<float> * (float)t / FrameRate);
        }

        class ActionCurve {
        public:
            ActionCurve() = default;
            virtual ~ActionCurve() = default;

            [[nodiscard]] virtual size_t size() const = 0;

            virtual float operator[](size_t idx) const = 0;
        };

        class PositiveSine : public ActionCurve {
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
            PositiveSine() = default;

            ~PositiveSine() override = default;

            [[nodiscard]] size_t size() const override {
                return data.size();
            }

            float operator[](size_t idx) const override {
                return data[idx];
            }
        };

        class HeartBeat : public ActionCurve {
        protected:
            static constexpr std::array<float, 38> data{
                    ac_sin(0), ac_sin(4), ac_sin(8), ac_sin(12),
                    ac_sin(16), ac_sin(20), ac_sin(24), ac_sin(28),
                    0.f, 0.f, 0.f,
                    ac_sin(0), ac_sin(4), ac_sin(8), ac_sin(12),
                    ac_sin(16), ac_sin(20), ac_sin(24), ac_sin(28),
                    0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                    0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                    0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                    0.f,
            };

        public:
            HeartBeat() = default;

            ~HeartBeat() override = default;

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
        std::unique_ptr<ActionCurves::ActionCurve> mActionCurve{};

    protected:
        Callback mAnimate;
    public:
        Animation();

        virtual ~Animation() = default;

        static void setAnimation(std::shared_ptr<Animation> animation, const Position& position) {
            Animator::getAnimator().set(animation, position);
        }

        static void removeAnimation(std::shared_ptr<Animation> animation) {
            Animator::getAnimator().remove(animation);
        }

        void animationCallback(Callback cb) {
            mAnimate = std::move(cb);
        }

        void setActionCurve(std::unique_ptr<ActionCurves::ActionCurve> &&actionCurve) {
            mActionCurve = std::move(actionCurve);
        }
    };
}

template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, std::unique_ptr<rose::ActionCurves::ActionCurve>&& curve) {
    static_assert(std::is_base_of_v<rose::Animation,WidgetClass>, "WidgetClass must be derived from rose::Animation.");
    widget->setActionCurve(std::move(curve));
    return widget;
}