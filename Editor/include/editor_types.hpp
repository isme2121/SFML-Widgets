#pragma once
#include "widget.hpp"
#if defined(WIDGETS_HPP) && defined(BUILD_EDITOR_GUI)
#include "button.hpp"
#include "frame.hpp"
#include "label.hpp"
#include "entry.hpp"
#include "checkbox.hpp"
#include "image_frame.hpp"
#include "grid_canvas.hpp"

/*
* Contains: type definitions, pre-declarations, core must-maintain objects
*
* Widget_t ==> all derived widget types
*
* Accessed_t ==> types used by widgets getter/setter methods
*
* LookupVec_t ==> getter/setter/resetter + GUI attribs for a single data type
*
* is_part_of_variant ; is_widget_type_v ==> compile time check, used in "wrapper_ptr/WidgetWrapper"
*
* accessor_visitor ==> used in WidgetEditor 
*
* BuiltinBindFlags ==> multiple widget functionalities from bindings
*
* VecN ==> tuple-like object (it's a tuple)
*
* Pre-Declarations for all available types.
*/

namespace sfml_widgets
{
// defined in "wrapper_ptr.hpp"
template<class WidgetT> class WidgetWrapper;
class wrapper_ptr;

// defined in "editor_accessors.hpp"
class Accessor_ptr;
enum class EnumSection : std::uint8_t;
template<class WidgetT> struct AccessorLink;
template<class WidgetT> struct BaseAccessorLink;

// defined in "editor_cache.hpp"
struct CachedAttributes;
struct ToolTipObject;
struct ToolTipCache;
struct ShaderCache;
struct CanvasCache;
struct DialogThread;
struct LabeledGroup;

// Storage type for accessor lookup
using LookupVec_t = std::vector<Accessor_ptr>;

/// Widget cache type in `WidgetEditor`
using CacheMap_t = std::unordered_map<const Widget*, CachedAttributes>;

/*
*  All derived widget types, To fully support a new derived widget type:
*  1. add here
*  2. add any new accessor types to `Accessed_t` & their visit overload in `WidgetEditor::renderRow` in "editor.hpp"
*  3. add to `AccessorLink` in "editor_accessors.hpp"
*  4. add default constructor to `WidgetEditor::renderEditSection`
*/
using Widget_t = std::variant<Button, Frame, Label, Entry, CheckBox, ImageFrame, GridCanvas>;

// All accessor types collapsed here
using Accessed_t = std::variant<bool, char, float, unsigned int, std::size_t, std::string, sfml_widgets::Button::SIDE,
                           Frame::F_SeparatorInfo, Frame::F_SeparatorType, Frame::F_AfterPreProcess,
                           DIRECTION, sf::Text::Style, Label::HAlign, Label::VAlign,
                           sf::String, sf::FloatRect, sf::Angle, sf::Vector2f, sf::Vector2u, sf::Vector2i,
                           std::optional<sf::Vector2u>, GridCanvas::cell_ptr, const Frame*, const sf::RenderWindow*>;


// Compile time check if a type is part of our variant
template<typename T, typename variant_t> struct is_part_of_variant;
template<typename T, typename... Ts>
struct is_part_of_variant<T, std::variant<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

// For widget type check
template<typename WidgetT>
inline constexpr bool is_widget_type_v = is_part_of_variant<WidgetT, Widget_t>::value;

// Default visitor
template<class... Ts>
struct accessor_visitor : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
accessor_visitor(Ts...) -> accessor_visitor<Ts...>;

///////////////////////////////////////////////////////////////////
/// @brief Widget bindings in `WidgetEditor`
///
/// @note to add a new flag, first add here with an incremented bit shift, then
/// add to "editor_literals.hpp"-> `QUICK_BINDINGS_LITERAL` for UI rendering,
/// followed up by the implementation of the binding in `WidgetEditor::addWidget` method.
///
///////////////////////////////////////////////////////////////////
enum class BuiltinBindFlags : std::uint8_t
{
    None = 0,
    Drag = 1 << 0,
    ScrollResize = 1 << 1,
    RightClickDelete = 1 << 2,
    HighlightOnHover = 1 << 3
};

constexpr BuiltinBindFlags operator|(BuiltinBindFlags first, BuiltinBindFlags second)
{
    return static_cast<BuiltinBindFlags>(static_cast<std::uint8_t>(first) | static_cast<std::uint8_t>(second));
}
constexpr BuiltinBindFlags operator&(BuiltinBindFlags first, BuiltinBindFlags second)
{
    return static_cast<BuiltinBindFlags>(static_cast<std::uint8_t>(first) & static_cast<std::uint8_t>(second));
}
constexpr BuiltinBindFlags operator~(BuiltinBindFlags first)
{
    return static_cast<BuiltinBindFlags>(~static_cast<std::uint8_t>(first));
}
constexpr bool isBuiltinFlagSet(BuiltinBindFlags& self, BuiltinBindFlags other)
{
    return (self & other) == other;
}

namespace enum_literals
{
///////////////////////////////////////////////////////////////////
/// @brief Custom attributes type
///
/// @tparam Ts each attribute type
///
///////////////////////////////////////////////////////////////////
template<typename... Ts>
struct VecN
{
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructed with thoughts and prayers
    ///
    /// @param args values types and order must match the object template
    /// types
    ///
    ///////////////////////////////////////////////////////////////////
    constexpr VecN(Ts... args) : attrib(args...) {}

    ///////////////////////////////////////////////////////////////////
    /// @brief Return the proper attribute relative to it's index on the
    /// object
    ///
    /// @tparam I attribute index
    /// @return pre-declared type
    /// @throw asserts if index is out of range
    ///
    ///////////////////////////////////////////////////////////////////
    template<std::size_t I> decltype(auto) get() const
    {
        return std::get<I>(attrib);
    }

private:
    std::tuple<Ts...> attrib;
};
}
}

#endif