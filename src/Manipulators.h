/** @file Container.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-07
  * @brief The Container Classes
  */

#pragma once

#include "Container.h"
#include "Rose.h"
//#include "ScrollArea.h"
#include "Utilities.h"

template<class WidgetType, class WidgetStore>
inline std::shared_ptr<WidgetType> operator>>(std::shared_ptr<WidgetType> widget, std::shared_ptr<WidgetStore> &variable) {
    static_assert(std::is_base_of_v<WidgetStore,WidgetType>, "WidgetStore must be a base class of WidgetType" );
    variable = widget;
    return widget;
}

/**
 * @brief An insertion operator which will take an un-parented widget and add it to a container.
 * @tparam ParentType The type of the parent Widget
 * @tparam WidgetType The type of the inserted Widget
 * @param parent The parent Widget
 * @param widget The child Widget
 * @return std::shared_ptr<WidgetType>
 */
template<class ParentType, class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<ParentType> parent,
                                              std::shared_ptr<WidgetType> widget) {
    static_assert(std::is_base_of_v<rose::Container,ParentType>, "Parent is not a container type." );
    parent->addChild(widget);
    return widget;
}

/**
 * @brief Handle rose::Manip manipulators inserted to a widget.
 * @tparam WidgetType The type of the widget
 * @param widget The widget
 * @param manip The manipulator
 * @return A WidgetPtr
 */
template<class WidgetType>
inline std::shared_ptr<rose::Container> operator<<(std::shared_ptr<WidgetType> widget, rose::Manip manip) {
    if (manip == rose::Manip::Parent)
        return widget->parent();

    throw std::runtime_error(rose::util::StringCompositor("Unhandled rose::Manip value."));
}

template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::RoseImageId imageId) {
    widget->setImageId(imageId);
    return widget;
}

template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::Elastic elasitc) {
    widget->layoutHints().mElastic = elasitc.mElastic;
    return widget;
}
