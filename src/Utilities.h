/**
 * @file Utilities.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-06
 */

#pragma once

#include "XDGFilePaths.h"
#include <iterator>
#include <algorithm>

#define XSTR(arg) STR(arg)  ///< Used to stringize the value of an argument
#define STR(arg) #arg       ///< Used to stringize the value of an argument

#define FILE_LOC " -- ", __FILE__, ':', __LINE__    ///< Macro to create a location within a source file

namespace rose {

    template<typename U, typename C>
    bool oneFlagOf(U flag, C container) {
        static_assert(std::is_unsigned_v<U>, "Argument flag must be unsigned type." );
        static_assert(std::is_unsigned_v<typename C::value_type>, "Container must hold an unsigned type.");
        return std::any_of(container.begin(), container.end(), [&flag](typename C::value_type value){
            return flag == value;
        });
    }

    /**
     * @class ReverseContainerView
     * @brief Provide a reverse view of a standard container type.
     * @tparam ContainerType The type of container
     */
    template<class ContainerType>
    class ReverseContainerView {
    protected:
        ContainerType &mContainer;      ///< A reference to the standard container.

    public:
        ReverseContainerView() = delete;

        /**
         * @brief Constructor.
         * @param container The standard container to provide the view of.
         * @param reverse If true reverse the container view, if false do not reverse.
         */
        explicit ReverseContainerView(ContainerType &container) : mContainer(container) {}

        /**
         * @brief Get the begin iterator for the reverse view
         * @return the standard container rbegin iterator
         */
        auto begin() {
            return std::rbegin(mContainer);
        }

        /**
         * @brief Get the end iterator for the reverse view
         * @return the standard container rend iterator
         */
        auto end() {
            return std::rend(mContainer);
        }
    };

    /**
     * @class ContainerView
     * @brief Provide a view of a standard container type.
     * @details Intended to be constructed by classes which have member containers.
     * @tparam ContainerType The type of container
     */
    template<class ContainerType>
    class ContainerView {
    protected:
        ContainerType &mContainer;      ///< A reference to the standard container.

    public:
        ContainerView() = delete;

        /**
         * @brief Constructor.
         * @param container The standard container to provide the view of.
         * @param reverse If true reverse the container view, if false do not reverse.
         */
        explicit ContainerView(ContainerType &container) : mContainer(container) {}

        /**
         * @brief Get the begin iterator for the reverse view
         * @return the standard container rbegin iterator
         */
        auto begin() {
            return std::begin(mContainer);
        }

        /**
         * @brief Get the end iterator for the reverse view
         * @return the standard container rend iterator
         */
        auto end() {
            return std::end(mContainer);
        }
    };

    /**
    * @brief Composite a pack of arguments that are streamable to a string.
    * @tparam Arg First argument type
    * @tparam Args Rest of the argument types
    * @param arg First argument
    * @param args Rest of the arguments
    * @return The composited string
    */
    template<typename Arg, typename... Args>
    std::string StringCompositor(Arg &&arg, Args &&... args) {
        std::stringstream out;
        out << std::forward<Arg>(arg);
        ((out << std::forward<Args>(args)), ...);
        return out.str();
    }

    class Environment {
    protected:
        Environment();

        std::filesystem::path mHomeDirectory;       ///< The user's home directory path
        std::filesystem::path mDataHome;            ///< The user's XDG Data Home path
        std::filesystem::path mConfigHome;          ///< The user's XDG Config Home path
        std::filesystem::path mCacheHome;           ///< The user's XDG Cache Home path
        std::filesystem::path mSharedImages;        ///< Image resources installed with the application.
        XDGFilePaths mFilePaths{};                  ///< XDG Spec file paths.

        std::string mAppName;                       ///< Application name as provided by the system.

    public:
        ~Environment() = default;

        Environment(const Environment &) = delete;

        Environment(Environment &&) = delete;

        Environment& operator=(const Environment &) = delete;

        Environment& operator=(Environment &&) = delete;

        static Environment &getEnvironment() {
            static Environment instance{};
            return instance;
        }

        [[nodiscard]] const std::string& appName() const { return mAppName; }

        [[nodiscard]] const std::filesystem::path& configHome() const { return mConfigHome; }

        [[nodiscard]] const std::filesystem::path& cacheHome() const { return mCacheHome; }

        /**
         * @brief Find the XDG directory for a specified application name.
         * @details If the path location does not exist, and create is set to true, it is created along with all
         * parent directories with permissions set to std::filesystem::perms::all modified by umask(2).
         * @param name The XDG Name.
         * @param appName The application name.
         * @param create Set to true if non-existent directories should be created.
         * @return
         */
        std::filesystem::path getenv_path(XDGFilePaths::XDG_Name name, const std::string &appName, bool create);
    };
}

