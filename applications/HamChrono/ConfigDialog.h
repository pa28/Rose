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
#include "TextField.h"

namespace rose {

/**
 * @class ConfigDialog
 * @brief Present a dialog to allow the user to configure the program.
 */
    class ConfigDialog : public Dialog {
    protected:
        static constexpr std::string_view mTitle = "Configure";
        static constexpr std::string_view CallPattern = "[A-Z]+[0-9][A-Z]+";
        static constexpr std::string_view FloatPattern = "[+-]?([0-9]*[.])?[0-9]+";

        static constexpr std::array<DialogActionButton,1> mActionButtons = {
                DialogActionButton{ActionButtonClose, DialogClose }
        };

        void qthConfigure(shared_ptr <Row> &parent);

        std::array<std::shared_ptr<TextField>,3> mTextFields{};

        std::shared_ptr<std::regex> mCallRegex{};
        std::shared_ptr<std::regex> mFloatRegex{};

        std::shared_ptr<Slot<Button::SignalType>> mActionButtonSlot{};

    public:
        ConfigDialog() = delete;
        ~ConfigDialog() override = default;
        ConfigDialog(ConfigDialog &&) = delete;
        ConfigDialog(const ConfigDialog &) = delete;
        ConfigDialog& operator=(ConfigDialog &&) = delete;
        ConfigDialog& operator=(const ConfigDialog &) = delete;

        /**
         * @brief Construct a Dialog which covers the entire screen.
         * @param parent the Rose application object.
         */
        explicit ConfigDialog(shared_ptr <Rose> parent) : Dialog(std::move(parent)) {
            mWindowTitle = mTitle;
            mSupportsDrag = true;
        }

        /**
         * @brief Construct a Window of specified position.
         * @details The position is clamped to be inside the screen.
         * @param parent the Rose application object.
         * @param position the Window size.
         */
        ConfigDialog(shared_ptr <Rose> parent, const Position &position) : Dialog(std::move(parent), position) {
            mWindowTitle = mTitle;
            mSupportsDrag = true;
        }

        /**
         * @brief Construct a Window of specified position and size.
         * @param parent the Rose application object.
         * @param pos the Window position
         * @param size the Window size.
         */
        ConfigDialog(shared_ptr <Rose> parent, const Position &pos, const Size &size)
            : Dialog(std::move(parent), pos, size) {
            mWindowTitle = mTitle;
            mSupportsDrag = true;
        }

        /**
         * @brief See Widget::initializeComposite()
         */
        void initializeComposite() override;

    };
}

