/**
 * @file TestWidget.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-24
 */

#pragma once

#include "Button.h"
#include "Visual.h"

namespace rose {

    /**
     * @class TestWidget
     * @brief
     */
    class TestWidget : public Widget {
    protected:
        color::RGBA mColor;
        ButtonSemantics mButtonSemantics;

    public:
        TestWidget() : mButtonSemantics(static_cast<Widget&>(*this)) {
            mButtonSemantics.setButtonDisplayCallback([](ButtonDisplayState buttonDisplayState){
                switch (buttonDisplayState) {
                    case rose::ButtonDisplayState::Active:
                        std::cout << __PRETTY_FUNCTION__ << " Active\n";
                        break;
                    case rose::ButtonDisplayState::Inactive:
                        std::cout << __PRETTY_FUNCTION__ << " Inactive\n";
                        break;
                    case rose::ButtonDisplayState::PressedInactive:
                        std::cout << __PRETTY_FUNCTION__ << " Pressed Inactive\n";
                        break;
                    case rose::ButtonDisplayState::PressedActive:
                        std::cout << __PRETTY_FUNCTION__ << " Pressed Active\n";
                        break;
                }
            });
        }

        ~TestWidget() override = default;

        TestWidget(const TestWidget &) = delete;

        TestWidget(TestWidget &&) = delete;

        TestWidget &operator=(const TestWidget &) = delete;

        TestWidget &operator=(TestWidget &&) = delete;

        explicit TestWidget(color::RGBA c) : TestWidget() {
            mSemanticGesture = SemanticGesture::Key | SemanticGesture::Click | SemanticGesture::Scroll | SemanticGesture::Drag;
            mColor = c;
        }

        TestWidget(Size size, color::RGBA c) : TestWidget() {
            mSemanticGesture = SemanticGesture::Key | SemanticGesture::Click | SemanticGesture::Scroll | SemanticGesture::Drag;
            mPreferredSize = size;
            mColor = c;
        }

        static constexpr std::string_view id = "TestWidget";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        /// Draw the visual.
        void draw(gm::Context &context, const Position<int>& containerPosition) override {
            Rectangle dst{containerPosition + mPos, mSize};
            context.fillRect(dst, mColor);
        }

        /// Layout the visual.
        Rectangle layout(rose::gm::Context &context, const Rectangle &screenRect) override {
            return Rectangle{mPreferredPos, mPreferredSize};
        }
    };

}

