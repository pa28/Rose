/** @file Container.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-07
  * @brief The Container Classes
  */

#pragma once

#include "Container.h"
#include "Rose.h"
#include "Types.h"
#include "Utilities.h"

/**
 * @addtogroup WidgetManip
 * @{
 */

/**
 * @brief A Widget manipulator to set the minimum orthogonal size of a widget that supports that parameter.
 * @tparam WidgetClass The type of Widget to manipulate.
 * @param widget The Widget to manipulate.
 * @param mo The minimum orthogonal value.
 * @return The manipulated Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::MinimumOrthogonal mo) {
    widget->setMinimumOrthogonal(mo.mMinimumOrthogonal);
    return widget;
}

/**
 * @brief A Widget manipulator to set the fill to end flag of a widget that supports that parameter.
 * @tparam WidgetClass The type of Widget to manipulate.
 * @param widget The Widget to manipulate.
 * @param fillToEnd The value of the flag.
 * @return The manipulated Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::FillToEnd fillToEnd) {
    widget->containerLayoutHints().fillToEnd = fillToEnd.mFillToEnd;
    return widget;
}

/**
 * @brief A Widget manipulator to set the FontSize.
 * @tparam WidgetClass The type of Widget to manipulate.
 * @param widget The Widget to manipulate.
 * @param fontSize The font size in points (pixels).
 * @return The manipulated Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::FontSize fontSize) {
    widget->setFontSize(fontSize);
    return widget;
}

/**
 * @brief A Widget manipulator to set the FontName
 * @tparam WidgetClass The type of Widget to manipulate.
 * @param widget The Widget to manipulate.
 * @param fontName Then name of the True Type font to use.
 * @return The manipulated Widget.
 */
template<class WidgetClass>
inline std::shared_ptr<WidgetClass> operator<<(std::shared_ptr<WidgetClass> widget, rose::FontName fontName) {
    widget->setFontName(fontName.fontName);
    return widget;
}

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

/**
 * @brief Set an ImageId on a Widget.
 * @tparam WidgetType The type of Widget.
 * @param widget The Widget.
 * @param imageId The ImageId.
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::RoseImageId imageId) {
    widget->setImageId(imageId);
    return widget;
}

/**
 * @brief Set the Elastic state of a Widget.
 * @tparam WidgetType The type of Widget
 * @param widget The Widget.
 * @param elastic The elastic value to set.
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::Elastic elastic) {
    widget->layoutHints().mElastic = elastic;
    return widget;
}

/**
 * @brief Set the SignalToken value on a Widget
 * @tparam WidgetType The Widget type.
 * @param widget The Widget
 * @param token The SignalToken
 * @return The Widget.
 */
template<class WidgetType>
inline std::shared_ptr<WidgetType> operator<<(std::shared_ptr<WidgetType> widget, rose::SignalToken token) {
    widget->setSignalToken(token);
    return widget;
}

/** @} */
