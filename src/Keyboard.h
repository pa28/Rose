/**
 * @file Keyboard.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-30
 */

#pragma once

#include <utility>

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

    /// A key specification of depth N
    template<size_t N>
    using KeySpec = std::array<uint32_t,N>;

    template<size_t N>
    constexpr auto makeKeySpec(const char *values) noexcept {
        auto a = KeySpec<N>{};
        const char *cp = values;
        for (auto &v : a) {
            if (*cp != '\0') {
                v = static_cast<uint32_t>(*cp++);
            } else {
                v = 0;
            }
        }
        return a;
    }

    template<class I, typename Arg, typename ... Args>
    constexpr I makeKeySpecInit(I first, I last, Arg arg, Args ... args) noexcept {
        if (first != last) {
            *first = static_cast<uint32_t>(arg);
            ++first;
            if constexpr (sizeof...(args) > 0) {
                first = makeKeySpecInit(first, last, args...);
            }
        }
        return first;
    }

    template<size_t N, typename ... Args>
    constexpr auto makeKeySpec(Args ... args) noexcept {
        auto a = KeySpec<N>{};
        auto first = a.begin();
        auto last = a.end();
        last = makeKeySpecInit(first, last, args...);
        return a;
    }

    /// A row of KeySpec specifications.
    template<size_t M, size_t N>
    using KeySpecRow = std::array<KeySpec<N>,M>;

    /// Type to help construct a KeySpec from a string.
    using KSS = std::array<char,5>;

    /// Type for a full Keyboard specification.
    template<size_t R, size_t M, size_t N>
    using KeyboardSpec = std::array<std::array<KeySpec<N>,M>,R>;

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
        build(shared_ptr <Keyboard> keyboard, Size keySize, int fontSize, const std::string &fontName) = 0;
    };

    /**
     * @class Keyboard
     * @brief Implement a touchscreen keyboard
     */
    class Keyboard : public Frame {
    protected:
        std::shared_ptr<KeyboardPlugin> mKeyboardPlugin{};  ///< The KeyboardPlugin to use

        KeyboardMode mKeyboardMode{KeyboardMode::LowerCase};    ///< The mode the keyboard is in.

        char mEm{'M'};                          ///< The 'largest' glyph in the Font.
//        std::shared_ptr<Column> mKeysGrid;      ///< The Column of key rows.
        Size mKeySize{};                        ///< The size of regular keys, determined by mEm.

        std::string mFontName{};                ///< Key face font name
        int mFontSize{};                        ///< Key face font size

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
        explicit Keyboard(std::shared_ptr<KeyboardPlugin> keyboardPlugin) : mKeyboardPlugin(std::move(keyboardPlugin)) {}

        /// See Widget::widgetLayout()
        Rectangle widgetLayout(sdl::Renderer &renderer, Rectangle available, uint layoutStage) override;

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
        static constexpr size_t Rows = 4;
        static constexpr size_t KeysPerRow = 11;
        static constexpr size_t Depth = 4;
        static const KeyboardSpec<Rows, KeysPerRow, Depth> QWERTYData;     ///< The KeyboardSpec.

        std::weak_ptr<Keyboard> mKeyboard{};            ///< The parent Keyboard Widget.

        using KeySpecType = KeySpec<Depth>;

        /**
         * @enum KeyboardMode
         * @brief Set the state of the keyboard.
         */
        enum class KeyboardMode : size_t  {
            LowerCase, UpperCase, Numeric, Symbols,
        };

        /**
         * @enum SymbolState
         * @brief The state of symbol selection.
         */
        enum class SymbolState : size_t {
            None, Symbols1, Symbols2,
        };

        uint32_t mTempShiftKey{0};    ///< The temporary shift status
        bool mCapsLock{false};        ///< The state of the caps lock key

        SymbolState mSymbolState{SymbolState::None};            ///< The state of symbol selection
        KeyboardMode mKeyboardMode{KeyboardMode::LowerCase};    ///< Current KeyboardMode

        std::shared_ptr<Slot<Button::SignalType>> rxKey{};
        std::shared_ptr<Slot<Button::SignalType>> rxCtl{};
        std::shared_ptr<Slot<Button::SignalType>> rxToggle{};

        /**
         * @brief Decode a KeySpec and Key index into key Button construction information.
         * @param keyData The KeySpec.
         * @param keyIdx The index into the key.
         * @return A std::tuple with the construction information.
         */
        auto controlKeyData(const KeySpec<Depth> &keyData);

        /**
         * @brief Encode data gathered from user interaction into a current KeyboardMode.
         * @details If the KeyboardMode changes changeKeyboardFace() is called.
         */
        void setKeyboardMode();

        /**
         * @brief Change the Key faces to match the current KeyboardMode.
         */
        void changeKeyboardFace();

    public:
        QUERTY() = default;
        ~QUERTY() override = default;

        /**
         * @brief Build the keyboard from the KeyboardSpec and apply any special layout rules.
         * @param keyboard The Keyboard Widget to build the keyboard in.
         * @param charSlot The slot to receive key press signals on.
         */
        void build(shared_ptr <Keyboard> keyboard, Size keySize, int fontSize, const std::string &fontName) override;
    };

    class NumberPad : public KeyboardPlugin {
    protected:
        static constexpr size_t Rows = 5;
        static constexpr size_t KeysPerRow = 3;
        static const KeyboardSpec<Rows, KeysPerRow, 1> NumberData;     ///< The main number pad KeyboardSpec.

        static constexpr size_t SideCols = 1;
        static constexpr size_t SideKeysPerCol = 5;
        static const KeyboardSpec<SideCols,SideKeysPerCol, 1> SideNumberData;  ///< The side key data.

        std::shared_ptr<Slot<Button::SignalType>> rxKey{};      ///< Slot to process keys

    public:
        NumberPad() = default;
        ~NumberPad() override = default;

        void build(std::shared_ptr<Keyboard> keyboard, Size keySize, int fontSize, const std::string &fontName) override;
    };
}

