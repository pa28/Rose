/** @file Utilities.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-13
  * @brief Miscellaneous utility classes and functions.
  */

#pragma once

#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <chrono>
#include <SDL.h>
#include <stack>
#include "Constants.h"

namespace rose::util {

    template<class Iterator>
    struct iterator_pair {
        Iterator mBegin, mEnd;

        Iterator begin() const { return mBegin; }
        Iterator end() const { return mEnd; }
    };

    template<class C>
    using iterator_t = decltype(std::begin(std::declval<C&>()));

    template<class C>
    iterator_pair<iterator_t<C>> offset(C& container, size_t skip) {
        return {std::next(container.begin(), skip), container.end()};
    }

    template<typename T, size_t N>
    std::ostream & printScreenMetric(std::ostream &strm, const std::array<T,N> &metric) {
        strm << '(' << metric.front();
        for (auto &i : offset(metric,1)) {
            strm << ',' << i;
        }
        return strm << ')';
    }

    /**
     * @brief Convert a filesystem time to a system clock time point.
     * @tparam T The type of the file time point.
     * @param fileTimePoint The value of the file time point.
     * @return std::chrono::system_clock::time_point.
     */
    template<typename T>
    extern std::chrono::system_clock::time_point fileClockToSystemClock(T fileTimePoint) {
        using namespace std::chrono_literals;
        using namespace std::chrono;

        auto sysWriteTime = time_point_cast<system_clock::duration>(
                fileTimePoint - decltype(fileTimePoint)::clock::now() +
                system_clock::now());
        return sysWriteTime;
    }

    /**
     * @struct FontMetrics
     * @brief The size metrics that pertain to a particular font.
     */
    struct FontMetrics {
        int fontAscent,     ///< The height above the base line.
        fontDescent,        ///< The length of descenders below the baseline a negative number.
        fontHeight,         ///< The total height of the font (ascent - descent
        fontLineSkip;       ///< The size of a line advance for the font.
    };

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
        explicit ReverseContainerView(ContainerType &container, bool reverse = true) : mContainer(container) {}

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
     * @namespace rose::util
     * @brief Utility Functions
     */

#define XSTR(arg) STR(arg)  ///< Used to stringize the value of an argument
#define STR(arg) #arg       ///< Used to stringize the value of an argument

#define FILE_LOC " -- ", __FILE__, ':', __LINE__    ///< Macro to create a location within a source file

    /**
     * @brief A type to encode the location of an error: file name and line number.
     */
    using DebugTuple = std::optional<std::tuple<const std::string_view, unsigned int>>;

    /**
     * @brief A macro to turn file location on or off.
     */
#define DEBUG_TUPLE(use) use ? std::make_optional(std::make_tuple(std::string_view{__FILE__},__LINE__)) : (std::nullopt)

    /**
     * @brief Comute the boolean exclusive or of a with b.
     * @param a
     * @param b
     * @return
     */
    static bool bxor(bool a, bool b) { return b == !a; }

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
}
