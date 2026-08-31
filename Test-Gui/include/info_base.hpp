#pragma once
#include "wrapper_base.hpp"

#if defined(WIDGETS_HPP) && defined(COMPILE_TESTS)
#include "button.hpp"
#include "frame.hpp"
#include "label.hpp"
#include "entry.hpp"
#include "checkbox.hpp"
#include "image_frame.hpp"
#include "grid_canvas.hpp"
#include <type_traits>
#include <utility>
#include <map>


/*
*   TYPES: Widget_t, Accessed_t, accessor_ptr::Getter_t, accessor_ptr::Setter_t, LookupMap_t
*
*   Classes: accessor_ptr<>, AccessorLink<>, BaseAccessorLink<>, is_part_of_variant<>, Info
*
*   Functions: set_lookup, invoke_setter, get_base_widget
*/
namespace sfml_widgets
{
// Default visitor
template<class... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};

template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;

// All derived widget types
using Widget_t = std::variant<Button, Frame, Label, Entry, CheckBox, ImageFrame, GridCanvas>;

// Compile time check if a type is part of our variant
template<typename T, typename variant_t> struct is_part_of_variant;
template<typename T, typename... Ts>
struct is_part_of_variant<T, std::variant<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

// For widget type check
template<typename WidgetT>
inline constexpr bool is_widget_type_v = is_part_of_variant<WidgetT, Widget_t>::value;

// All accessor types collapsed here
using Accessed_t = std::variant<bool, char, float, unsigned int, std::size_t, std::string, sfml_widgets::Button::SIDE,
                           Frame::F_SeparatorInfo, Frame::F_SeparatorType, Frame::F_AfterPreProcess,
                           DIRECTION, sf::Text::Style, Label::HAlign, Label::VAlign, Cell*,
                           sf::String, sf::FloatRect, sf::Angle, sf::Vector2f, sf::Vector2u, sf::Vector2i,
                           std::optional<sf::Vector2u>>;


///////////////////////////////////////////////////////////////////
/// @brief Storage for getter + setter pointers, meant to store
/// getter + setter functions for a single `Accessed_t` type
///
/// @tparam WidgetT derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT> 
struct accessor_ptr
{
    using Getter_t = Accessed_t (*)(const WidgetT&);             //!< Getter lambda pointer type
    using Setter_t = bool (*)(WidgetT&, const Accessed_t&);      //!< Setter lambda pointer type
    
    Getter_t getter{nullptr};
    Setter_t setter{nullptr};

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a lambda to the proper class type
    /// @tparam Fn lambda pointer type
    /// @throw asserts if type does not match
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename Fn> void assign(Fn&& func) noexcept(false)
    {
        if constexpr(std::is_convertible_v<Fn, Getter_t>)
            getter = std::forward<Fn>(func);

        else if constexpr(std::is_convertible_v<Fn, Setter_t>)
            setter = std::forward<Fn>(func);

        else
            static_assert(!sizeof(Fn*), "Callable does not match the proper function type.");
    }
};

// Hash-map type for accessor lookup
template<typename WidgetT>
using LookupMap_t = std::map<std::string, accessor_ptr<WidgetT>>;

///////////////////////////////////////////////////////////////////
/// @brief Necessary details for a single `LookupMap_t` key
///
///////////////////////////////////////////////////////////////////
struct Info
{
    std::string info_key;
    std::array<char, 128> info_text{};
    std::optional<Accessed_t> info_val{std::nullopt};
    bool info_has_setter{false};

    bool operator==(const std::string& other)
    {
        return info_key == other;
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Assigns a function pointer to the proper accessor type
/// in map
///
/// Creates key value pair if it doesn't exist.
///
/// @tparam WidgetT derived widget type
/// @tparam Fn function pointer type
/// @param mp lookup map
/// @param key string key
/// @param val function pointer of getter/setter type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT, typename Fn>
void set_lookup(LookupMap_t<WidgetT>& mp, const std::string& key, Fn&& val)
{
    accessor_ptr<WidgetT>& wfp = mp[key];
    wfp.assign(val);
}

///////////////////////////////////////////////////////////////////
/// @brief Runs fn function and returns a boolean
///
/// For lambda type matching, `invoke_setter` may always return a boolean
/// for any setter method
///
///////////////////////////////////////////////////////////////////
template<typename Fn>
bool invoke_setter(Fn&& fn)
{
    if constexpr(std::is_void_v<std::invoke_result_t<Fn>>)
    {
        std::forward<Fn>(fn)();
        return true;
    }
    else
    {
        return std::forward<Fn>(fn)();
    }
}

///////////////////////////////////////////////////////////////////
/// @brief Get the base class address for variant widget types
///
///////////////////////////////////////////////////////////////////
constexpr Widget* get_base_widget(Widget_t& w)
{
    return std::visit([&](auto& w) -> Widget*
    {
        return &w;
    }, w);
}

///////////////////////////////////////////////////////////////////
/// @brief Links accessor member functions explicitly to map
///
/// @tparam WidgetT derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
struct AccessorsLink
{
    static void link(LookupMap_t<WidgetT>&) {}
};

///////////////////////////////////////////////////////////////////
/// @brief Links base `Widget` accessors to widget
///
/// @tparam WidgetT derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
struct BaseAccessorsLink
{
    static_assert(is_widget_type_v<WidgetT>, "Template type does not match any supported widget derived type.");

    static void link(LookupMap_t<WidgetT>& map)
    {
        /* Getters */
        set_lookup<WidgetT>(map, "position", + [](const WidgetT& w) -> Accessed_t {return w.getWidgetPosition();});
        set_lookup<WidgetT>(map, "absolute_position", + [](const WidgetT& w) -> Accessed_t {return w.getAbsolutePosition();});
        set_lookup<WidgetT>(map, "size", + [](const WidgetT& w) -> Accessed_t {return w.getWidgetSize();});
        set_lookup<WidgetT>(map, "rotation", + [](const WidgetT& w) -> Accessed_t {return w.getWidgetRotation();});
        set_lookup<WidgetT>(map, "bounds", + [](const WidgetT& w) -> Accessed_t {return w.getBounds();});
        set_lookup<WidgetT>(map, "absolute_bounds", + [](const WidgetT& w) -> Accessed_t {return w.getAbsoluteBounds();});
        set_lookup<WidgetT>(map, "is_hidden", + [](const WidgetT& w) -> Accessed_t {return w.isHidden();});

        /* Setters */
        set_lookup<WidgetT>(map, "position", + [](WidgetT& w, const Accessed_t& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&]{return w.setWidgetPosition(*p);});
            return false;
        });
        set_lookup<WidgetT>(map, "size", + [](WidgetT& w, const Accessed_t& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&]{return w.setWidgetSize(*p);});
            return false;
        });
        set_lookup<WidgetT>(map, "rotation", + [](WidgetT& w, const Accessed_t& v) -> bool
        {
            if(const float* p = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setWidgetRotation(*p);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Button`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Button>
{
    static void link(LookupMap_t<Button>& map)
    {
        /* Getters */
        set_lookup<Button>(map, "text", + [](const Button& w) -> Accessed_t  {return w.getText();});
        set_lookup<Button>(map, "text_size", + [](const Button& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<Button>(map, "text_limit", + [](const Button& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<Button>(map, "text_position", + [](const Button& w) -> Accessed_t {return w.getTextPosition();});
        set_lookup<Button>(map, "icon_path", + [](const Button& w) -> Accessed_t {return w.getIconPath();});
        set_lookup<Button>(map, "icon_position", + [](const Button& w) -> Accessed_t {return w.getIconPosition();});

        /* Setters */
        set_lookup<Button>(map, "text", + [](Button& w, const Accessed_t& v) -> bool
        {
            if(const sf::String* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<Button>(map, "text_size", +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*f);});
            return false;
        });
        set_lookup<Button>(map, "text_size", +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const unsigned int* f = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*f);});
            return false;
        });
        set_lookup<Button>(map, "text_position", +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return w.setTextPosition(*side);});
            return false;
        });
        set_lookup<Button>(map, "icon_position", +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return w.setIconPosition(*side);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Frame`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Frame>
{
    static void link(LookupMap_t<Frame>& map)
    {
        /* Getters */
        set_lookup<Frame>(map, "child_count", + [](const Frame& w) -> Accessed_t {return w.getChildCount();});
        set_lookup<Frame>(map, "scroll_offset", + [](const Frame& w) -> Accessed_t {return w.getScrollOffset();});
        set_lookup<Frame>(map, "scroll_speed", + [](const Frame& w) -> Accessed_t {return w.getScrollSpeed();});
        set_lookup<Frame>(map, "separator_info", + [](const Frame& w) -> Accessed_t {return w.getSeparatorInfo();});
        set_lookup<Frame>(map, "process_mode", + [](const Frame& w) -> Accessed_t {return w.getProcessMode();});

        /* Setters */
        set_lookup<Frame>(map, "grid", +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.setGrid(*s);});
            return false;
        });
        set_lookup<Frame>(map, "separator_info", +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_SeparatorInfo>(&v))
                return invoke_setter([&]{return w.setSeparators((*s).type, (*s).ratio, (*s).dist);});
            return false;
        });
        set_lookup<Frame>(map, "process_mode", +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_AfterPreProcess>(&v))
                return invoke_setter([&]{return w.setProcessMode(*s);});
            return false;
        });
        set_lookup<Frame>(map, "scroll_speed", +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setScrollSpeed(*s);});
            return false;
        });
        set_lookup<Frame>(map, "scroll", +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.scroll(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `CheckBox`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<CheckBox>
{
    static void link(LookupMap_t<CheckBox>& map)
    {
        /* Getters */
        set_lookup<CheckBox>(map, "state", + [](const CheckBox& w) -> Accessed_t {return w.getState();});
        set_lookup<CheckBox>(map, "text_size", + [](const CheckBox& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<CheckBox>(map, "text_limit", + [](const CheckBox& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<CheckBox>(map, "text_padding", + [](const CheckBox& w) -> Accessed_t {return w.getTextPadding();});
        set_lookup<CheckBox>(map, "text_position", + [](const CheckBox& w) -> Accessed_t {return w.getTextPosition();});

        /* Setters */
        set_lookup<CheckBox>(map, "text", +[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_size", +[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_limit", +[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_position", +[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<DIRECTION>(&v))
                return invoke_setter([&]{return w.setTextPosition(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_padding", +[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextPadding(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Entry`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Entry>
{
    static void link(LookupMap_t<Entry>& map)
    {
        /* Getters */
        set_lookup<Entry>(map, "text", + [](const Entry& w) -> Accessed_t {return w.getText();});
        set_lookup<Entry>(map, "text_size", + [](const Entry& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<Entry>(map, "text_limit", + [](const Entry& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<Entry>(map, "background_text", + [](const Entry& w) -> Accessed_t {return w.getBackGroundText();});
        set_lookup<Entry>(map, "blink_ratio", + [](const Entry& w) -> Accessed_t {return w.getBlinkUpRatio();});
        set_lookup<Entry>(map, "cursor_index", + [](const Entry& w) -> Accessed_t {return w.getCursorIndex();});
        set_lookup<Entry>(map, "placeholder_char", + [](const Entry& w) -> Accessed_t {return w.getPlaceholderChar();});
        set_lookup<Entry>(map, "text_offset", + [](const Entry& w) -> Accessed_t {return w.getTextOffsetRatio();});
        set_lookup<Entry>(map, "cursor_width", + [](const Entry& w) -> Accessed_t {return w.getCursorWidth();});
        set_lookup<Entry>(map, "scroll_offset", + [](const Entry& w) -> Accessed_t {return w.getScrollOffset();});

        /* Setters */
        set_lookup<Entry>(map, "text", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<Entry>(map, "text_size", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        set_lookup<Entry>(map, "text_limit", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        set_lookup<Entry>(map, "background_text", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setBackGroundText(*s);});
            return false;
        });
        set_lookup<Entry>(map, "blink_ratio", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setBLinkUpRatio(*s);});
            return false;
        });
        set_lookup<Entry>(map, "cursor_index", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setCursorIndex(*s);});
            return false;
        });
        set_lookup<Entry>(map, "placeholder_char", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<char>(&v))
                return invoke_setter([&]{return w.setPlaceholderChar(*s);});
            return false;
        });
        set_lookup<Entry>(map, "text_offset", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextOffsetRatio(*s);});
            return false;
        });
        set_lookup<Entry>(map, "cursor_width", +[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setCursorWidth(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `ImageFrame`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<ImageFrame>
{
    static void link(LookupMap_t<ImageFrame>& map)
    {
        /* Getters */
        set_lookup<ImageFrame>(map, "image_path", + [](const ImageFrame& w) -> Accessed_t {return w.getImagePath();});
        set_lookup<ImageFrame>(map, "image_size", + [](const ImageFrame& w) -> Accessed_t {return w.getOriginalImageSize();});
        /* Setters */
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Label`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Label>
{
    static void link(LookupMap_t<Label>& map)
    {
        /* Getters */
        set_lookup<Label>(map, "text", + [](const Label& w) -> Accessed_t {return w.getText();});
        set_lookup<Label>(map, "text_size", + [](const Label& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<Label>(map, "text_limit", + [](const Label& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<Label>(map, "text_style", + [](const Label& w) -> Accessed_t {return w.getTextStyle();});
        set_lookup<Label>(map, "h_align", + [](const Label& w) -> Accessed_t {return w.getHorizontalAlignment();});
        set_lookup<Label>(map, "v_align", + [](const Label& w) -> Accessed_t {return w.getVerticalAlignment();});
        set_lookup<Label>(map, "line_spacing", + [](const Label& w) -> Accessed_t {return w.getLineSpacing();});
        set_lookup<Label>(map, "letter_spacing", + [](const Label& w) -> Accessed_t {return w.getLetterSpacing();});
        set_lookup<Label>(map, "padding_ratio", + [](const Label& w) -> Accessed_t {return w.getPaddingRatio();});

        /* Setters */
        set_lookup<Label>(map, "text", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<Label>(map, "text_limit", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        set_lookup<Label>(map, "text_size", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        set_lookup<Label>(map, "text_style", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Text::Style>(&v))
                return invoke_setter([&]{return w.setTextStyle(*s);});
            return false;
        });
        set_lookup<Label>(map, "h_align", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Label::HAlign>(&v))
                return invoke_setter([&]{return w.setHorizontalAlignment(*s);});
            return false;
        });
        set_lookup<Label>(map, "v_align", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Label::VAlign>(&v))
                return invoke_setter([&]{return w.setVerticalAlignment(*s);});
            return false;
        });
        set_lookup<Label>(map, "line_spacing", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setLineSpacing(*s);});
            return false;
        });
        set_lookup<Label>(map, "letter_spacing", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setLetterSpacing(*s);});
            return false;
        });
        set_lookup<Label>(map, "padding_ratio", +[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setPaddingRatio(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `GridCanvas`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<GridCanvas>
{
    static void link(LookupMap_t<GridCanvas>& map)
    {
        /* Getters */
        set_lookup<GridCanvas>(map, "local_pixel", + [](const GridCanvas& w) -> Accessed_t {return w.getLocalPixel(sf::Mouse::getPosition(*w.getRootWindow()));});
        set_lookup<GridCanvas>(map, "view_pixel", + [](const GridCanvas& w) -> Accessed_t {return w.getViewPixel(sf::Mouse::getPosition(*w.getRootWindow()));});
        set_lookup<GridCanvas>(map, "local_to_view", + [](const GridCanvas& w) -> Accessed_t {return w.mapLocalToView(w.getLocalPixel(sf::Mouse::getPosition(*w.getRootWindow())));});
        set_lookup<GridCanvas>(map, "coordinates", + [](const GridCanvas& w) -> Accessed_t {return w.mapCoordinates(sf::Mouse::getPosition(*w.getRootWindow()));});
        set_lookup<GridCanvas>(map, "selected_cell", + [](const GridCanvas& w) -> Accessed_t {return w.getSelectedCellPtr();});
        set_lookup<GridCanvas>(map, "clicked_cell", + [](const GridCanvas& w) -> Accessed_t {return w.getClickedCellPtr();});
        set_lookup<GridCanvas>(map, "cell_count", + [](const GridCanvas& w) -> Accessed_t {return w.getCellsCount();});

        /* Setters */
        set_lookup<GridCanvas>(map, "insert", +[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.insert(*s);});
            return false;
        });
        set_lookup<GridCanvas>(map, "remove", +[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.remove(*s);});
            return false;
        });
        set_lookup<GridCanvas>(map, "dead_zone", +[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setDeadZone(*s);});
            return false;
        });
        set_lookup<GridCanvas>(map, "is_taken", +[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.isTaken(*s);});
            return false;
        });
    }
};
}

#endif