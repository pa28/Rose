/**
 * @file Keyboard.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#pragma once

#include "Frame.h"
#include "Button.h"
#include "Container.h"

namespace rose {

    /**
     * @enum KeyboardMode
     * @brief Keyboard modes (cases)
     */
    enum class KeyboardMode {
        LowerCase,          ///< Lower case letters.
        UpperCase,          ///< Upper case letters.
        Numbers,            ///< Numbers and symbols.
        Symbols,            ///< Numbers and more symbols.
    };

    /**
     * @class KeySpec
     * @brief The specification of the cases, or special function of a key.
     */
    class KeySpec : public std::array<uint32_t,4> {
    public:
        KeySpec() = default;

        /**
         * @brief Constructor
         * @details Construct a KeySpec from four independent values that may be cast to uint32_t.
         * @tparam A
         * @tparam B
         * @tparam C
         * @tparam D
         * @param a
         * @param b
         * @param c
         * @param d
         */
        template<typename A, typename B, typename C, typename D>
        constexpr KeySpec(A a, B b, C c, D d) noexcept
            : std::array<uint32_t,4>( {static_cast<uint32_t>(a),
                                     static_cast<uint32_t>(b),
                                     static_cast<uint32_t>(c),
                                     static_cast<uint32_t>(d)}
                                     ) {}
        /// Construct a KeySpec from a array of char.
        constexpr explicit KeySpec(const std::array<char,5> &c) noexcept
            : KeySpec(c[0], c[1], c[2], c[3]) {}
    };

    /// A row of KeySpec specifications.
    template<size_t N>
    using KeySpecRow = std::array<KeySpec,N>;

    /// Type to help construct a KeySpec from a string.
    using KSS = std::array<char,5>;

    /// Type for a full Keyboard specification.
    template<size_t R, std::size_t N>
    using KeyboardSpec = std::array<std::array<KeySpec,N>,R>;

    class Keyboard;

    /**
     * @class KeyboardPlugin
     * @brief Encapsulate the KeyboardSpec specification and build function.
     */
    class KeyboardPlugin {
    public:
        KeyboardPlugin() = default;
        virtual ~KeyboardPlugin() = default;

        virtual void
        build(shared_ptr <Keyboard> keyboard, Size keySize, shared_ptr <Slot<Button::SignalType>> &slot) const = 0;
    };

    /**
     * @class Keyboard
     * @brief Implement a touchscreen keyboard
     */
    class Keyboard : public Frame {
    protected:
        const KeyboardPlugin &mKeyboardPlugin;  ///< The KeyboardPlugin to use

        KeyboardMode mKeyboardMode{KeyboardMode::LowerCase};    ///< The mode the keyboard is in.

        char mEm{'M'};                          ///< The 'largest' glyph in the Font.
        std::shared_ptr<Column> mKeysGrid;      ///< The Column of key rows.
        Size mKeySize{};                        ///< The size of regular keys, determined by mEm.

        std::string mFontName{};                ///< Key face font name
        int mFontSize{};                        ///< Key face font size

        /// Slot to receive key press signals on.
        std::shared_ptr<Slot<Button::SignalType>> mKeyPressRx{};

    public:

        Keyboard() = delete;

        ~Keyboard() override = default;

        Keyboard(Keyboard &&) = delete;

        Keyboard(const Keyboard &) = delete;

        Keyboard &operator=(Keyboard &&) = delete;

        Keyboard &operator=(const Keyboard &) = delete;

        /**
         * @brief Constructor
         * @param keyboardPlugin The KeyboardPlugin to use.
         */
        explicit Keyboard(const KeyboardPlugin &keyboardPlugin) : mKeyboardPlugin(keyboardPlugin) {}

        /// See Widget::initialLayout()
        Rectangle initialLayout(sdl::Renderer &renderer, Rectangle available) override;

        /// See Widget::draw()
        void draw(sdl::Renderer &renderer, Rectangle parentRect) override;

        /// See Widget::initializeComposite()
        void initializeComposite() override;
    };

    /**
     * @class QUERTY
     * @brief A KeyboardPlugin for a QWERTY keyboard.
     */
    class QUERTY : public KeyboardPlugin {
    protected:
        static const KeyboardSpec<4,11> QWERTYData;     ///< The KeyboardSpec.

    public:
        QUERTY() = default;
        ~QUERTY() override = default;

        /**
         * @brief Build the keyboard from the KeyboardSpec and apply any special layout rules.
         * @param frame The Keyboard Widget to build the keyboard in.
         * @param slot The slot to receive key press signals on.
         */
        void build(shared_ptr <Keyboard> frame, Size keySize, shared_ptr <Slot<Button::SignalType>> &slot) const override;
    };
}

