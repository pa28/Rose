//
// Created by richard on 2021-04-27.
//

#pragma once

#include "Visual.h"
#include "Popup.h"

/**
 * @brief Insertion operator to place a Widget in a Container.
 * @tparam ContainerClass The class of the Container.
 * @tparam WidgetClass The class of the Widget.
 * @param container The container.
 * @param widget The Widget.
 * @return The Widget.
 */
template<class ContainerClass, class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<ContainerClass> container, std::shared_ptr<WidgetClass> widget) {
    static_assert(std::is_base_of_v<rose::Visual, ContainerClass> && std::is_base_of_v<rose::Container, ContainerClass>,
                  "ContainerClass must be derived from both rose::Visual and rose::Container.");
    static_assert(std::is_base_of_v<rose::Visual, WidgetClass> && std::is_base_of_v<rose::Node, WidgetClass>,
                  "WidgetClass must be derived from both rose::visual and rose::Node.");
    if constexpr (std::is_base_of_v<rose::PopupWindow, ContainerClass>) {
        container->addWidget(widget);
    } else {
        container->add(widget);
    }
    return widget;
}

