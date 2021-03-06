/**
 * @file Utilities.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-06
 */

#pragma once

#include <iterator>

namespace rose {

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

}

