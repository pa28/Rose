/**
 * @file SingleChild.h
 *
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-01-05
 */

#pragma once

#include <utility>

#include "Container.h"

namespace rose {

    /**
     * @class ContainerChildView
     * @brief Provide a range view of a Container Widget's children.
     * @tparam ContainerType The type of Container Widget
     */
    template<class ContainerType>
    class ContainerChildView {
    protected:
        friend class SingleChild;
        ContainerType &mContainer;      ///< A reference to the Container Widget.

        /**
         * @brief Constructor
         * @param container The Container Widget to view.
         */
        explicit ContainerChildView(std::shared_ptr<ContainerType> &container)
                : mContainer(container->front()->template as<Container>().get()) {
            static_assert(std::is_base_of_v<Container, ContainerType>,
                          "rose::ContainerChildView requires an object derived from rose::Container." );
        }

    public:
        ContainerChildView() = delete;

        /// Get the begin iterator of the Container Widget.
        auto begin() { return mContainer.begin(); }

        /// Get the end iterator of the Container Widget.
        auto end() { return mContainer.end(); }

        /// Get the front object of the Container Widget.
        auto front() { return mContainer.front(); }
    };

    /**
     * @class SingleChild
     * @brief A container Widget that supports only one child Widget.
     */
    class SingleChild : public Container {
    protected:

    public:
        SingleChild();

        ~SingleChild() override = default;

        SingleChild(const SingleChild &) = delete;

        SingleChild &operator=(const SingleChild &) = delete;

        SingleChild(SingleChild &&) = delete;

        SingleChild &operator=(SingleChild &&) = delete;

        std::shared_ptr<Widget> getSingleChild() {
            if (mChildren.empty())
                return nullptr;
            return mChildren.front()->getWidget();
        }

        /**
         * @brief Create a Container view of the child.
         * @tparam ContainerType The type of Container Widget.
         * @return std::optional<ContainerChildView> if child is set and of type ContainerType, empty otherwise.
         */
        template<class ContainerType>
        std::optional<ContainerChildView<ContainerType>> containerView() {
            if (auto child = front()->as<ContainerType>(); child) {
                return ContainerChildView<ContainerType>{child};
            }
            return std::nullopt;
        }

        /**
         * @brief Add a child widget to this container
         * @details This container is set as the widget's parent, the widget is added to this containers child list,
         * and the widgets initializeComposite method is called.
         * @param widget The child widget to add
         */
        void addChild(const std::shared_ptr<Widget>& widget) override {
            if (mChildren.empty()) {
                auto wParent = widget->weak_from_this();
                widget->mParent = as<Container>();
                widget->mHasParent = true;
                widget->mRose = mRose;
                mChildren.push_back(widget);
                widget->initializeComposite();
            } else
                throw std::runtime_error("More than one child added to SinglChild Container.");
        }

        /**
         * @brief A convenience operation to get the child of a SingleChild Container.
         * @details If the child is set, an attempt is made to cast it to WidgetType.
         * @tparam WidgetType The desired return Widget type.
         * @return std::shared_prt<WidgetType> if child is sent and derived from WidgetType, empty otherwise.
         */
        template<class WidgetType>
        std::shared_ptr<WidgetType> getSingleChild() {
            if (!mChildren.empty())
                return mChildren.front()->as<WidgetType>();
            return nullptr;
        }

        /**
         * @brief A convenience operation to get the child of a SingleChild Container.
         * @details If the child is set, an attempt is made to cast it to WidgetType.
         * @tparam WidgetType The desired return Widget type.
         * @return std::shared_prt<WidgetType> if child is sent and derived from WidgetType, empty otherwise.
         */
        template<class WidgetType>
        std::shared_ptr<WidgetType> getSingleChild() const {
            if (!mChildren.empty())
                return mChildren.front()->as<WidgetType>();
            return nullptr;
        }
    };
}
