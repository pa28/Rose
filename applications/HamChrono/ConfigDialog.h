/**
 * @file ConfigDialog.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-02-14
 */

#pragma once

#include <regex>
#include <utility>

#include "PopupWindow.h"

namespace rose {

/**
 * @class ConfigDialog
 * @brief
 */
    class ConfigDialog : public PopupWindow {
    protected:
        static constexpr std::string_view mTitle = "Configure";
        static constexpr std::string_view CallPattern = "[A-Z]+[0-9][A-Z]+";
        static constexpr std::string_view FloatPattern = "[+-]?([0-9]*[.])?[0-9]+";

        void qthConfigure(shared_ptr <Container> &parent);

        std::shared_ptr<std::regex> mCallRegex{};
        std::shared_ptr<std::regex> mFloatRegex{};

    public:
        ConfigDialog() = delete;
        ~ConfigDialog() override = default;
        ConfigDialog(ConfigDialog &&) = delete;
        ConfigDialog(const ConfigDialog &) = delete;
        ConfigDialog& operator=(ConfigDialog &&) = delete;
        ConfigDialog& operator=(const ConfigDialog &) = delete;

        /**
         * @brief Construct a PopupWindow which covers the entire screen.
         * @param parent the Rose application object.
         */
        explicit ConfigDialog(shared_ptr <Rose> parent) : PopupWindow(std::move(parent)) {
            mWindowTitle = mTitle;
        }

        /**
         * @brief Construct a Window of specified position.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param position the Window size.
         */
        ConfigDialog(shared_ptr <Rose> parent, const Position &position) : PopupWindow(std::move(parent), position) {
            mWindowTitle = mTitle;
        }

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param size the Window size.
         */
        ConfigDialog(shared_ptr <Rose> parent, const Position &pos, const Size &size)
            : PopupWindow(std::move(parent), pos, size) {
            mWindowTitle = mTitle;
        }

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

    };
}

