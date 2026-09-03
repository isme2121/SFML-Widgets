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
#include <tuple>

/*
*   TYPES: Widget_t, Accessed_t, accessor_ptr::Getter_t, accessor_ptr::Setter_t, LookupVec_t
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

// For UI classification
enum class EnumSection : std::uint8_t
{
    Any,
    Base,
    State,
    Text,
    Image,
    Pointer
};

///////////////////////////////////////////////////////////////////
/// @brief Widget bindings in `InfoWrapper`
///
/// Adding a value here is not enough, it should also get included to
/// `QUICK_BINDINGS_LITERAL` array for literal, followed up by the implementation
/// of the binding in `InfoWrapper::addWidget` method.
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
    constexpr VecN(Ts... args) : attrib(args...)
    {

    }

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

constexpr auto BUTTON_SIDE_LITERAL = std::array<VecN<const char*>, 3>{"Center", "Left", "Right"};
constexpr auto SEPARATOR_TYPE_LITERAL = std::array<VecN<const char*>, 3>{"NoSeparator", "Horizontal", "Vertical"};
constexpr auto AFTER_PREPROCESS_LITERAL = std::array<VecN<const char*>, 3>{"NO_SKIP", "SKIP_CALLBACK", "SKIP_ALL"};
constexpr auto DIRECTION_LITERAL = std::array<VecN<const char*>, 4>{"TOP", "BOTTOM", "LEFT", "RIGHT"};
constexpr auto HALIGN_LITERAL = std::array<VecN<const char*>, 3>{"Left", "Center", "Right"};
constexpr auto VALIGN_LITERAL = std::array<VecN<const char*>, 3>{"Top", "Middle", "Bottom"};

constexpr auto TEXT_STYLE_LITERAL = std::array<VecN<const char*, sf::Text::Style>, 4>{
        {{"Bold", sf::Text::Bold}, {"Italic", sf::Text::Italic},
        {"Underlined", sf::Text::Underlined}, {"StrikeThrough", sf::Text::StrikeThrough}}};

constexpr auto FLAG_BITS_LITERAL = std::array<VecN<const char*, FLAGS, bool>, 32>{
    {{"IDLE", FLAGS::IDLE, false}, {"MODIFIED", FLAGS::MODIFIED, false},
    {"FOCUSED", FLAGS::FOCUSED, false}, {"HOVERED", FLAGS::HOVERED, false},
    {"ALPHAREDUCED", FLAGS::ALPHAREDUCED, false},{"SELECTED", FLAGS::SELECTED, false},
    {"RESTRICTED", FLAGS::RESTRICTED, false}, {"ACTIVE", FLAGS::ACTIVE, false},
    {"DRAWING", FLAGS::DRAWING, false}, {"SHORTSKIP", FLAGS::SHORTSKIP, false},
    {"LONGSKIP", FLAGS::LONGSKIP, false}, {"UPDATE", FLAGS::UPDATE, false},
    {"SET", FLAGS::SET, false}, {"RESET", FLAGS::RESET, false},
    {"TEXT", FLAGS::TEXT, false}, {"IMAGE", FLAGS::IMAGE, false},
    {"SEPARATORS", FLAGS::SEPARATORS, false}, {"Hidden", FLAGS::HIDDEN, true},
    {"Blocked", FLAGS::BLOCKED, true}, {"Transparent", FLAGS::TRANSPARENT, true},
    {"Resize", FLAGS::RESIZE, true}, {"AlwaysSelected", FLAGS::ALWAYSSELECTED, true},
    {"AutoHide", FLAGS::AUTOHIDE, true}, {"AutoTransparent", FLAGS::AUTOTRANSPARENT, true},
    {"NoBgImage", FLAGS::NOBGIMAGE, true}, {"NoClipShader", FLAGS::NOCLIPSHADER, true},
    {"NoColorShader", FLAGS::NOCOLORSHADER, true}, {"Indicator", FLAGS::INDICATOR, true},
    {"Grid", FLAGS::GRID, true}, {"Scroll", FLAGS::SCROLL, true},
    {"Zoom", FLAGS::ZOOM, true}, {"Pan", FLAGS::PAN, true}}};

constexpr auto ENUM_SECTION_LITERAL = std::array<VecN<const char*, EnumSection>, 6>{
        {{"Base", EnumSection::Base},
        {"State", EnumSection::State},
        {"Text", EnumSection::Text},
        {"Image", EnumSection::Image},
        {"Pointer", EnumSection::Pointer},
        {"Any", EnumSection::Any}}
};

constexpr auto QUICK_BINDINGS_LITERAL = std::array<VecN<const char*, BuiltinBindFlags>, 4>{
    {{"Drag to move", BuiltinBindFlags::Drag},
    {"Scroll to resize", BuiltinBindFlags::ScrollResize},
    {"Right-click delete", BuiltinBindFlags::RightClickDelete},
    {"Highlight on hover", BuiltinBindFlags::HighlightOnHover}}
};
}

// All accessor types collapsed here
using Accessed_t = std::variant<bool, char, float, unsigned int, std::size_t, std::string, sfml_widgets::Button::SIDE,
                           Frame::F_SeparatorInfo, Frame::F_SeparatorType, Frame::F_AfterPreProcess,
                           DIRECTION, sf::Text::Style, Label::HAlign, Label::VAlign, Cell*,
                           sf::String, sf::FloatRect, sf::Angle, sf::Vector2f, sf::Vector2u, sf::Vector2i,
                           std::optional<sf::Vector2u>>;


///////////////////////////////////////////////////////////////////
/// @brief Holds accessor information in `InfoWidget` class
///
/// @tparam WidgetT derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT> 
struct accessor_ptr
{
    using Getter_t = Accessed_t (*)(const WidgetT&);             //!< Getter lambda pointer type
    using Setter_t = bool (*)(WidgetT&, const Accessed_t&);      //!< Setter lambda pointer type
    
    std::string key;                        //!< lookup key
    Getter_t getter{nullptr};
    Setter_t setter{nullptr};
    EnumSection section{EnumSection::Any};

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

// Storage type for accessor lookup
template<typename WidgetT>
using LookupVec_t = std::vector<accessor_ptr<WidgetT>>;

///////////////////////////////////////////////////////////////////
/// @brief Holds accessor information in `InfoWrapper` class
///
///////////////////////////////////////////////////////////////////
struct Info
{
    std::string info_key;                               //!< indentifier
    EnumSection info_section{EnumSection::Any};         //!< section in UI
    std::array<char, 128> info_text{};                  //!< temporary data storage
    std::optional<Accessed_t> info_val{std::nullopt};   //!< value available if getter provided
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
/// @param vec lookup storage
/// @param key string key
/// @param section classification type
/// @param fn function pointer of getter/setter type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT, typename Fn>
void set_lookup(LookupVec_t<WidgetT>& vec, const std::string& key, EnumSection section, Fn&& fn)
{
    auto it = std::find_if(vec.begin(), vec.end(),
        [&](const accessor_ptr<WidgetT>& w)
        {
            return w.key == key;
        });

    // key already registered
    if(it != vec.end())
    {
        it->assign(std::forward<Fn>(fn));
    }
    // create key
    else
    {
        accessor_ptr<WidgetT> accessor;
        accessor.key = key;
        accessor.section = section;
        accessor.assign(std::forward<Fn>(fn));
        vec.push_back(std::move(accessor));
    }
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
    static void link(LookupVec_t<WidgetT>&) {}
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

    static void link(LookupVec_t<WidgetT>& map)
    {
        /* Getters */
        set_lookup<WidgetT>(map, "position", EnumSection::Base,+ [](const WidgetT& w) -> Accessed_t {return w.getWidgetPosition();});
        set_lookup<WidgetT>(map, "absolute_position", EnumSection::Base,+ [](const WidgetT& w) -> Accessed_t {return w.getAbsolutePosition();});
        set_lookup<WidgetT>(map, "size", EnumSection::Base,+ [](const WidgetT& w) -> Accessed_t {return w.getWidgetSize();});
        set_lookup<WidgetT>(map, "rotation", EnumSection::Base,+ [](const WidgetT& w) -> Accessed_t {return w.getWidgetRotation();});
        set_lookup<WidgetT>(map, "bounds", EnumSection::Base,+ [](const WidgetT& w) -> Accessed_t {return w.getBounds();});
        set_lookup<WidgetT>(map, "absolute_bounds", EnumSection::Base,+ [](const WidgetT& w) -> Accessed_t {return w.getAbsoluteBounds();});
        set_lookup<WidgetT>(map, "is_hidden", EnumSection::State,+ [](const WidgetT& w) -> Accessed_t {return w.isHidden();});

        /* Setters */
        set_lookup<WidgetT>(map, "position", EnumSection::Base,+ [](WidgetT& w, const Accessed_t& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&]{return w.setWidgetPosition(*p);});
            return false;
        });
        set_lookup<WidgetT>(map, "size", EnumSection::Base,+ [](WidgetT& w, const Accessed_t& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&]{return w.setWidgetSize(*p);});
            return false;
        });
        set_lookup<WidgetT>(map, "rotation", EnumSection::Base,+ [](WidgetT& w, const Accessed_t& v) -> bool
        {
            if(const sf::Angle* p = std::get_if<sf::Angle>(&v))
                return invoke_setter([&]{return w.setWidgetRotation(p->asDegrees());});
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
    static void link(LookupVec_t<Button>& vec)
    {
        /* Getters */
        set_lookup<Button>(vec, "text", EnumSection::Text, + [](const Button& w) -> Accessed_t  {return w.getText();});
        set_lookup<Button>(vec, "text_size", EnumSection::Text, + [](const Button& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<Button>(vec, "text_limit", EnumSection::Text, + [](const Button& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<Button>(vec, "text_position", EnumSection::Text, + [](const Button& w) -> Accessed_t {return w.getTextPosition();});
        set_lookup<Button>(vec, "icon_path", EnumSection::Image, + [](const Button& w) -> Accessed_t {return w.getIconPath();});
        set_lookup<Button>(vec, "icon_position", EnumSection::Image, + [](const Button& w) -> Accessed_t {return w.getIconPosition();});

        /* Setters */
        set_lookup<Button>(vec, "text", EnumSection::Text, + [](Button& w, const Accessed_t& v) -> bool
        {
            if(const sf::String* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<Button>(vec, "text_size", EnumSection::Text, +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*f);});
            return false;
        });
        set_lookup<Button>(vec, "text_size", EnumSection::Text, +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const unsigned int* f = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*f);});
            return false;
        });
        set_lookup<Button>(vec, "text_position", EnumSection::Text, +[](Button& w, const Accessed_t& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return w.setTextPosition(*side);});
            return false;
        });
        set_lookup<Button>(vec, "icon_position", EnumSection::Image, +[](Button& w, const Accessed_t& v) -> bool
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
    static void link(LookupVec_t<Frame>& map)
    {
        /* Getters */
        set_lookup<Frame>(map, "child_count", EnumSection::State, + [](const Frame& w) -> Accessed_t {return w.getChildCount();});
        set_lookup<Frame>(map, "scroll_offset", EnumSection::State, + [](const Frame& w) -> Accessed_t {return w.getScrollOffset();});
        set_lookup<Frame>(map, "scroll_speed", EnumSection::State, + [](const Frame& w) -> Accessed_t {return w.getScrollSpeed();});
        set_lookup<Frame>(map, "separator_info", EnumSection::State, + [](const Frame& w) -> Accessed_t {return w.getSeparatorInfo();});
        set_lookup<Frame>(map, "process_mode", EnumSection::State, + [](const Frame& w) -> Accessed_t {return w.getProcessMode();});

        /* Setters */
        set_lookup<Frame>(map, "grid", EnumSection::State, +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.setGrid(*s);});
            return false;
        });
        set_lookup<Frame>(map, "separator_info", EnumSection::State, +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_SeparatorInfo>(&v))
                return invoke_setter([&]{return w.setSeparators((*s).type, (*s).ratio, (*s).dist);});
            return false;
        });
        set_lookup<Frame>(map, "process_mode", EnumSection::State, +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_AfterPreProcess>(&v))
                return invoke_setter([&]{return w.setProcessMode(*s);});
            return false;
        });
        set_lookup<Frame>(map, "scroll_speed", EnumSection::State, +[](Frame& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setScrollSpeed(*s);});
            return false;
        });
        set_lookup<Frame>(map, "scroll", EnumSection::State, +[](Frame& w, const Accessed_t& v) -> bool
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
    static void link(LookupVec_t<CheckBox>& map)
    {
        /* Getters */
        set_lookup<CheckBox>(map, "state", EnumSection::State ,+ [](const CheckBox& w) -> Accessed_t {return w.getState();});
        set_lookup<CheckBox>(map, "text_size", EnumSection::Text ,+ [](const CheckBox& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<CheckBox>(map, "text_limit", EnumSection::Text ,+ [](const CheckBox& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<CheckBox>(map, "text_padding", EnumSection::Text ,+ [](const CheckBox& w) -> Accessed_t {return w.getTextPadding();});
        set_lookup<CheckBox>(map, "text_position", EnumSection::Text ,+ [](const CheckBox& w) -> Accessed_t {return w.getTextPosition();});

        /* Setters */
        set_lookup<CheckBox>(map, "text", EnumSection::Text ,+[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_size", EnumSection::Text ,+[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_limit", EnumSection::Text ,+[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_position", EnumSection::Text ,+[](CheckBox& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<DIRECTION>(&v))
                return invoke_setter([&]{return w.setTextPosition(*s);});
            return false;
        });
        set_lookup<CheckBox>(map, "text_padding", EnumSection::Text ,+[](CheckBox& w, const Accessed_t& v) -> bool
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
    static void link(LookupVec_t<Entry>& map)
    {
        /* Getters */
        set_lookup<Entry>(map, "text", EnumSection::Text ,+ [](const Entry& w) -> Accessed_t {return w.getText();});
        set_lookup<Entry>(map, "text_size", EnumSection::Text ,+ [](const Entry& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<Entry>(map, "text_limit", EnumSection::Text ,+ [](const Entry& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<Entry>(map, "background_text", EnumSection::Text ,+ [](const Entry& w) -> Accessed_t {return w.getBackGroundText();});
        set_lookup<Entry>(map, "blink_ratio", EnumSection::State ,+ [](const Entry& w) -> Accessed_t {return w.getBlinkUpRatio();});
        set_lookup<Entry>(map, "cursor_index", EnumSection::State ,+ [](const Entry& w) -> Accessed_t {return w.getCursorIndex();});
        set_lookup<Entry>(map, "placeholder_char", EnumSection::State ,+ [](const Entry& w) -> Accessed_t {return w.getPlaceholderChar();});
        set_lookup<Entry>(map, "text_offset", EnumSection::Text ,+ [](const Entry& w) -> Accessed_t {return w.getTextOffsetRatio();});
        set_lookup<Entry>(map, "cursor_width", EnumSection::State ,+ [](const Entry& w) -> Accessed_t {return w.getCursorWidth();});
        set_lookup<Entry>(map, "scroll_offset", EnumSection::State ,+ [](const Entry& w) -> Accessed_t {return w.getScrollOffset();});

        /* Setters */
        set_lookup<Entry>(map, "text", EnumSection::Text ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<Entry>(map, "text_size", EnumSection::Text ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        set_lookup<Entry>(map, "text_limit", EnumSection::Text ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        set_lookup<Entry>(map, "background_text", EnumSection::Text ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setBackGroundText(*s);});
            return false;
        });
        set_lookup<Entry>(map, "blink_ratio", EnumSection::State ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setBLinkUpRatio(*s);});
            return false;
        });
        set_lookup<Entry>(map, "cursor_index", EnumSection::State ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setCursorIndex(*s);});
            return false;
        });
        set_lookup<Entry>(map, "placeholder_char", EnumSection::State ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<char>(&v))
                return invoke_setter([&]{return w.setPlaceholderChar(*s);});
            return false;
        });
        set_lookup<Entry>(map, "text_offset", EnumSection::Text ,+[](Entry& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextOffsetRatio(*s);});
            return false;
        });
        set_lookup<Entry>(map, "cursor_width", EnumSection::State ,+[](Entry& w, const Accessed_t& v) -> bool
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
    static void link(LookupVec_t<ImageFrame>& map)
    {
        /* Getters */
        set_lookup<ImageFrame>(map, "image_path", EnumSection::Image ,+ [](const ImageFrame& w) -> Accessed_t {return w.getImagePath();});
        set_lookup<ImageFrame>(map, "image_size", EnumSection::Image ,+ [](const ImageFrame& w) -> Accessed_t {return w.getOriginalImageSize();});
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
    static void link(LookupVec_t<Label>& map)
    {
        /* Getters */
        set_lookup<Label>(map, "text", EnumSection::Text ,+ [](const Label& w) -> Accessed_t {return w.getText();});
        set_lookup<Label>(map, "text_size", EnumSection::Text ,+ [](const Label& w) -> Accessed_t {return w.getTextSize();});
        set_lookup<Label>(map, "text_limit", EnumSection::Text ,+ [](const Label& w) -> Accessed_t {return w.getTextLimit();});
        set_lookup<Label>(map, "text_style", EnumSection::Text ,+ [](const Label& w) -> Accessed_t {return w.getTextStyle();});
        set_lookup<Label>(map, "h_align", EnumSection::State ,+ [](const Label& w) -> Accessed_t {return w.getHorizontalAlignment();});
        set_lookup<Label>(map, "v_align", EnumSection::State ,+ [](const Label& w) -> Accessed_t {return w.getVerticalAlignment();});
        set_lookup<Label>(map, "line_spacing", EnumSection::State ,+ [](const Label& w) -> Accessed_t {return w.getLineSpacing();});
        set_lookup<Label>(map, "letter_spacing", EnumSection::State ,+ [](const Label& w) -> Accessed_t {return w.getLetterSpacing();});
        set_lookup<Label>(map, "padding_ratio", EnumSection::State ,+ [](const Label& w) -> Accessed_t {return w.getPaddingRatio();});

        /* Setters */
        set_lookup<Label>(map, "text", EnumSection::Text ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        set_lookup<Label>(map, "text_limit", EnumSection::Text ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        set_lookup<Label>(map, "text_size", EnumSection::Text ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        set_lookup<Label>(map, "text_style", EnumSection::Text ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Text::Style>(&v))
                return invoke_setter([&]{return w.setTextStyle(*s);});
            return false;
        });
        set_lookup<Label>(map, "h_align", EnumSection::State ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Label::HAlign>(&v))
                return invoke_setter([&]{return w.setHorizontalAlignment(*s);});
            return false;
        });
        set_lookup<Label>(map, "v_align", EnumSection::State ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Label::VAlign>(&v))
                return invoke_setter([&]{return w.setVerticalAlignment(*s);});
            return false;
        });
        set_lookup<Label>(map, "line_spacing", EnumSection::State ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setLineSpacing(*s);});
            return false;
        });
        set_lookup<Label>(map, "letter_spacing", EnumSection::State ,+[](Label& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setLetterSpacing(*s);});
            return false;
        });
        set_lookup<Label>(map, "padding_ratio", EnumSection::State ,+[](Label& w, const Accessed_t& v) -> bool
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
    static void link(LookupVec_t<GridCanvas>& map)
    {
        /* Getters */
        set_lookup<GridCanvas>(map, "local_pixel", EnumSection::State ,+ [](const GridCanvas& w) -> Accessed_t {return w.getLocalPixel(sf::Mouse::getPosition(*w.getRootWindow()));});
        set_lookup<GridCanvas>(map, "view_pixel", EnumSection::State ,+ [](const GridCanvas& w) -> Accessed_t {return w.getViewPixel(sf::Mouse::getPosition(*w.getRootWindow()));});
        set_lookup<GridCanvas>(map, "local_to_view", EnumSection::State ,+ [](const GridCanvas& w) -> Accessed_t {return w.mapLocalToView(w.getLocalPixel(sf::Mouse::getPosition(*w.getRootWindow())));});
        set_lookup<GridCanvas>(map, "coordinates", EnumSection::State ,+ [](const GridCanvas& w) -> Accessed_t {return w.mapCoordinates(sf::Mouse::getPosition(*w.getRootWindow()));});
        set_lookup<GridCanvas>(map, "selected_cell", EnumSection::Pointer ,+ [](const GridCanvas& w) -> Accessed_t {return w.getSelectedCellPtr();});
        set_lookup<GridCanvas>(map, "clicked_cell", EnumSection::Pointer ,+ [](const GridCanvas& w) -> Accessed_t {return w.getClickedCellPtr();});
        set_lookup<GridCanvas>(map, "cell_count", EnumSection::State ,+ [](const GridCanvas& w) -> Accessed_t {return w.getCellsCount();});

        /* Setters */
        set_lookup<GridCanvas>(map, "insert", EnumSection::Any ,+[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.insert(*s);});
            return false;
        });
        set_lookup<GridCanvas>(map, "remove", EnumSection::Any ,+[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.remove(*s);});
            return false;
        });
        set_lookup<GridCanvas>(map, "dead_zone", EnumSection::State ,+[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setDeadZone(*s);});
            return false;
        });
        set_lookup<GridCanvas>(map, "is_taken", EnumSection::State ,+[](GridCanvas& w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.isTaken(*s);});
            return false;
        });
    }
};
}

#endif