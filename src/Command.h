/**
 * @file Command.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-24
 */

#pragma once

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "Utilities.h"

namespace rose {

    /**
     * @class Command
     * @brief Execute a program and gather the output.
     */
    class Command {
    protected:
        std::string mResult{};      ///< The output of the command.

        /**
         * @brief Close the pipe and keep the exit status
         */
        struct PipeClose {
            int exitStatus{};           ///< The status code returned by pclose(3)

            void operator()(FILE *pipe) {
                exitStatus = pclose(pipe);
            }
        };

        PipeClose mPipeClose{};                     ///< The pipe closer.
        std::unique_ptr<FILE, PipeClose> mPipe;     ///< The pipe from the command.

    public:
        Command() = delete;

        /**
         * @brief Constructor.
         * @param command The command to execute. See popen()
         * @details The pipe is opened with a call to popen(command,"r"). A call to wait will pause execution
         * and gather the output and the exit status when the command exits.
         */
        explicit Command(const char* command) : mPipe(popen(command, "r"), mPipeClose){

        }

        /**
         * @brief Gather the command output and wait for it to exit.
         * @return The exit status gathered by pclose().
         */
        int wait() {
            std::array<char, 128> buffer{};

            while (fgets(buffer.data(), buffer.size(), mPipe.get()) != nullptr) {
                mResult += buffer.data();
            }

            mPipe.release();

            return statusCode();
        }

        /**
         * @brief Get the command exit status code.
         * @return The exit status gathered by pclose().
         */
        int statusCode() {
            return mPipeClose.exitStatus;
        }

        /**
         * @brief Get the output of the command.
         * @return A std::string with the command output.
         */
        const std::string& result() const { return mResult; }
    };
}

