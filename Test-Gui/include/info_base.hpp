#pragma once
#include "wrapper_base.hpp"

#if defined(WIDGETS_HPP) && defined(BUILD_TESTS)
#include "button.hpp"
#include "frame.hpp"
#include "label.hpp"
#include "entry.hpp"
#include "checkbox.hpp"
#include "image_frame.hpp"
#include "grid_canvas.hpp"
#include <thread>
#include <atomic>

namespace sfml_widgets
{
/* All derived widget types
*  To fully support a new derived widget type:
*  1. add here
*  2. add to `AccessorLink`
*  3. add default constructor to `InfoWrapper::renderEditSection`
*/
using Widget_t = std::variant<Button, Frame, Label, Entry, CheckBox, ImageFrame, GridCanvas, Canvas>;

// All accessor types collapsed here
using Accessed_t = std::variant<bool, char, float, unsigned int, std::size_t, std::string, sfml_widgets::Button::SIDE,
                           Frame::F_SeparatorInfo, Frame::F_SeparatorType, Frame::F_AfterPreProcess,
                           DIRECTION, sf::Text::Style, Label::HAlign, Label::VAlign, Cell*,
                           sf::String, sf::FloatRect, sf::Angle, sf::Vector2f, sf::Vector2u, sf::Vector2i,
                           std::optional<sf::Vector2u>, Canvas::cell_ptr, const Frame*, const sf::RenderWindow*>;

// Compile time check if a type is part of our variant
template<typename T, typename variant_t> struct is_part_of_variant;
template<typename T, typename... Ts>
struct is_part_of_variant<T, std::variant<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

// For widget type check
template<typename WidgetT>
inline constexpr bool is_widget_type_v = is_part_of_variant<WidgetT, Widget_t>::value;


// Default visitor
template<class... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;


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
        {"Underlined", sf::Text::Underlined}, {"StrikeThrough", sf::Text::StrikeThrough}}
};

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
    {"Zoom", FLAGS::ZOOM, true}, {"Pan", FLAGS::PAN, true}}
};

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

const auto THEME_LITERAL = std::array<VecN<const char*, std::shared_ptr<WidgetTheme>>, 14>{
    {{"Dark", Theme::dark_theme},
    {"Light", Theme::light_theme},
    {"Stale Violet", Theme::stale_violet_theme},
    {"Carbon Cyan", Theme::carbon_cyan_theme},
    {"Midnight Rose", Theme::midnight_rose_theme},
    {"Obsidian Ember", Theme::obsidian_ember_theme},
    {"Emerald Forest", Theme::emerald_forest_theme},
    {"Golden Amber", Theme::golden_amber_theme},
    {"Crimson Ash", Theme::crimson_ash_theme},
    {"Steel Graphite", Theme::steel_graphite_theme},
    {"Lavender Dusk", Theme::lavender_dusk_theme},
    {"Sky Azure", Theme::sky_azure_theme},
    {"Arctic Frost", Theme::arctic_frost_theme},
    {"Sandstone Dune", Theme::sandstone_dune_theme}}
};

const auto TT_STYLE_LITERAL = std::array<VecN<const char*, ToolTipStyle>, 10>{
    {{"Dark", Theme::tooltip_style::TOOLTIP_DEFAULT_DARK},
    {"Light", Theme::tooltip_style::TOOLTIP_DEFAULT_LIGHT},
    {"Emerald Forest", Theme::tooltip_style::TOOLTIP_EMERALD_FOREST},
    {"Golden Amber", Theme::tooltip_style::TOOLTIP_GOLDEN_AMBER},
    {"Crimson Ash", Theme::tooltip_style::TOOLTIP_CRIMSON_ASH},
    {"Steel Graphite", Theme::tooltip_style::TOOLTIP_STEEL_GRAPHITE},
    {"Lavender Dusk", Theme::tooltip_style::TOOLTIP_LAVENDER_DUSK},
    {"Sky Azure", Theme::tooltip_style::TOOLTIP_SKY_AZURE},
    {"Arctic Frost", Theme::tooltip_style::TOOLTIP_ARCTIC_FROST},
    {"Sandstone Dune", Theme::tooltip_style::TOOLTIP_SANDSTONE_DUNE}}
};

constexpr auto BUILTIN_SHADERS_LITERAL = std::array<VecN<const char*, std::string_view>, 3>{
    {{"Button Shader", BUTTON_SHADER_FRAG},
     {"Sprite Shader", SPRITE_SHADER_FRAG},
     {"Texture Shader", TEXTURE_SHADER_FRAG}}
};

constexpr auto LINE_TYPE_LITERAL = std::array<VecN<const char*>, 3>{"None", "Open", "Dotted"};
}

///////////////////////////////////////////////////////////////////
/// @brief Holds accessor information in `InfoWidget` class
///
///////////////////////////////////////////////////////////////////
struct Accessor_ptr
{
using Getter_t = Accessed_t (*)(const void*);
using Setter_t = bool (*)(void*, const Accessed_t&);
using Resetter_t = bool (*)(void*);
struct Accessor_attrib
{
    std::optional<Accessed_t> val{std::nullopt};        //!< updated by getter
    std::optional<std::string> hint{std::nullopt};      
    std::optional<sf::Vector2f> frange{std::nullopt};   //!< float range
    std::optional<sf::Vector2u> uirange{std::nullopt};  //!< uint range
    std::array<char, 96> text_content{};                //!< temp storage for string types
    EnumSection section{EnumSection::Any};
    bool is_path{false};
};

    std::string key;
    Getter_t getter{nullptr};
    Setter_t setter{nullptr};
    Resetter_t resetter{nullptr};
    Accessor_attrib attrib;

    bool operator==(const std::string& other) const
    {
        return key == other;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a lambda to the proper class type
    ///
    /// @tparam Fn lambda pointer type
    /// @throw asserts if type does not match
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename Fn>
    void assign_accessor(Fn&& func) noexcept(false)
    {
        if constexpr(std::is_convertible_v<Fn, Getter_t>)
            getter = std::forward<Fn>(func);

        else if constexpr(std::is_convertible_v<Fn, Setter_t>)
            setter = std::forward<Fn>(func);

        else if constexpr(std::is_convertible_v<Fn, Resetter_t>)
            resetter = std::forward<Fn>(func);

        else
            static_assert(!sizeof(Fn*), "Callable does not match the proper function type.");
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns accessor attributes
    ///
    ///////////////////////////////////////////////////////////////////
    void assign_attributes(EnumSection key_section = EnumSection::Any, const std::string& hint = "", bool is_path = false,
                           std::optional<sf::Vector2f> frange = std::nullopt, std::optional<sf::Vector2u> uintrange = std::nullopt)
    {
        attrib.section = key_section;
        if(!hint.empty()) attrib.hint = hint;
        attrib.is_path = is_path;
        attrib.frange = frange;
        attrib.uirange = uintrange;
    }
};

// Storage type for accessor lookup
using LookupVec_t = std::vector<Accessor_ptr>;

///////////////////////////////////////////////////////////////////
/// @brief Assigns a function pointer to the proper accessor type
/// in map
///
/// Creates key value pair if it doesn't exist.
///
/// @tparam Fn function pointer type
/// @param vec lookup storage
/// @param key string key
/// @param fn function pointer of getter/setter type
/// @param key_section classification type
/// @param hint hint for input
/// @param is_path does key expect a disk path?
/// @param frange float range
/// @param uintrange uint range
///
/// @note Any parameter after fn only applies on the first call
/// for a key on the same vec, a second call only applies fn to key. this
/// is done because i'm tired boss
///
///////////////////////////////////////////////////////////////////
template<typename Fn>
void set_lookup(LookupVec_t& vec, const std::string& key, Fn&& fn, EnumSection key_section = EnumSection::Any, const std::string& hint = "", bool is_path = false,
                std::optional<sf::Vector2f> frange = std::nullopt, std::optional<sf::Vector2u> uintrange = std::nullopt)
{
    auto it = std::find_if(vec.begin(), vec.end(), [&](const Accessor_ptr& w)
            {
                return w.key == key;
            });

    if(it != vec.end())
    {
        it->assign_accessor(std::forward<Fn>(fn));
    }
    else
    {
        Accessor_ptr accessor;
        accessor.key = key;
        accessor.assign_accessor(std::forward<Fn>(fn));
        accessor.assign_attributes(key_section, hint, is_path, frange, uintrange);
        vec.push_back(std::move(accessor));
    }
}

///////////////////////////////////////////////////////////////////
/// @brief Represents a `ToolTip` class rendered object, the attributes
/// maintain a reference to the object itself and it's state
///
///////////////////////////////////////////////////////////////////
struct ToolTipObject
{
    std::size_t id{0};
    TT_RenderedType type{TT_RenderedType::Text};
    std::string pending_image_path;
    sf::Vector2u image_size_param{0, 0};
};

///////////////////////////////////////////////////////////////////
/// @brief Cache object for `ToolTip`
///
///////////////////////////////////////////////////////////////////
struct ToolTipCache
{
    ToolTip* ptr{nullptr};
    std::vector<ToolTipObject> content;
    std::optional<std::size_t> selected_id{std::nullopt};
    std::size_t style_idx{0};
};

///////////////////////////////////////////////////////////////////
/// @brief cache for shader
///
///////////////////////////////////////////////////////////////////
struct ShaderCache
{
    Widget::W_LoadType mode{Widget::W_LoadType::Memory};
    int combo_idx{-1};                                      //!< -1 == None
    std::array<char, 256> file_path{};
};

struct CanvasCache
{
    unsigned int line_x0{0}, line_y0{0}, line_x1{0}, line_y1{0};
    unsigned int line_thickness{1};
    bool line_use_default_color{true};
    float line_color[4]{1.f, 1.f, 1.f, 1.f};
    int line_type_idx{0};

    // Cell editor
    int cell_side_idx{0};
    float cell_fill_color[4]{1.f, 1.f, 1.f, 1.f};
    unsigned int cell_outline_thickness{1};
    bool cell_use_default_outline_color{true};
    float cell_outline_color[4]{1.f, 1.f, 1.f, 1.f};
    int cell_outline_type_idx{0};
    std::array<char, 256> cell_texture_path{};
};

///////////////////////////////////////////////////////////////////
/// @brief Memory attributes for any widget
///
/// These attributes can't be obtained from the widget itself, so they
/// are stored and maintained externally, used in `InfoWrapper` class,
/// and they are not accurate by default.
///
///////////////////////////////////////////////////////////////////
struct CachedAttributes
{
    BuiltinBindFlags flags{BuiltinBindFlags::None};
    ShaderCache clip_shader_cache;
    ShaderCache color_shader_cache;
    ToolTipCache tooltip;
    CanvasCache canvas;
    int theme_idx{-1};                                      //!< -1 == None
    sf::Vector2f grab_offset{0.f, 0.f};
    std::string label;
};

/// Widget cache type in `InfoWrapper`
using CacheMap_t = std::unordered_map<const Widget*, CachedAttributes>;

struct DialogThread
{
    std::thread thread;
    std::string protected_path;                 //!< obtained from thread
    std::optional<std::string> safe_path;       //!< main thread storage
    
    std::mutex guard;
    std::atomic<bool> is_open{false};
    std::atomic<bool> path_found{false};

    std::string req_key;
    unsigned int req_token{0};
};

// Widget group with a label
struct LabeledGroup
{
    std::string group_name;
    std::shared_ptr<WidgetGroup> group_ptr;
};

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
    static void link(LookupVec_t&) {}
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

    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "position" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getWidgetPosition();}, EnumSection::Base);
        set_lookup(vec, "position" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setWidgetPosition(*p);});
            return false;
        });
        set_lookup(vec, "position" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setWidgetPosition(sf::Vector2f{0.f, 0.f});});
        });

        set_lookup(vec, "absolute position" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getAbsolutePosition();}, EnumSection::Base);

        set_lookup(vec, "size" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getWidgetSize();}, EnumSection::Base);
        set_lookup(vec, "size" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setWidgetSize(*p);});
            return false;
        });

        set_lookup(vec, "rotation" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getWidgetRotation();}, EnumSection::Base);
        set_lookup(vec, "rotation" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const sf::Angle* p = std::get_if<sf::Angle>(&v))
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setWidgetRotation(p->asDegrees());});
            return false;
        });
        set_lookup(vec, "rotation" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setWidgetRotation(0.f);});
        });

        set_lookup(vec, "corner radius" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getCornerRadius();}, EnumSection::Base);
        set_lookup(vec, "corner radius" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const float* p = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setCornerRadius(*p);});
            return false;
        });
        set_lookup(vec, "corner radius" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->setCornerRadius(0.f);});
        });

        set_lookup(vec, "bounds" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getBounds();}, EnumSection::Base);

        set_lookup(vec, "absolute bounds" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getAbsoluteBounds();}, EnumSection::Base);

        set_lookup(vec, "hidden" ,+ [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->isHidden();}, EnumSection::State);

        set_lookup(vec, "background texture" ,+ [](void* w, const Accessed_t& v) -> bool
            {
                if(const auto* s = std::get_if<sf::String>(&v))
                    return invoke_setter([&]{return static_cast<WidgetT*>(w)->setBackgroundTexture(*s, "");});
                return false;
            }, EnumSection::Image, "Path to image", true);

        set_lookup(vec, "background texture" ,+ [](void* w) -> bool
            {
                return invoke_setter([&]{return static_cast<WidgetT*>(w)->resetBackgroundTexture();});
            });

        set_lookup(vec, "root window", + [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getRootWindow();}, EnumSection::Pointer);
        set_lookup(vec, "root frame", + [](const void* w) -> Accessed_t {return static_cast<const WidgetT*>(w)->getRootFrame();}, EnumSection::Pointer);
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Button`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Button>
{
    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "text" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getText();}, EnumSection::Text);
        set_lookup(vec, "text" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const sf::String* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setText(*s);});
            return false;
        });
        set_lookup(vec, "text" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<Button*>(w)->setText("");});
        });

        set_lookup(vec, "text size" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getTextSize();}, EnumSection::Text);
        set_lookup(vec, "text size" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const unsigned int* f = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setTextSize(*f);});
            return false;
        });

        set_lookup(vec, "text size ratio" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setTextSize(*f);});
            return false;
        }, EnumSection::Text, "Float [0.f, 1.f]", false, sf::Vector2f{0.f, 0.999f});
    
        set_lookup(vec, "icon size" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getIconSizeRatio();}, EnumSection::Image, "Float [0.f, 1.f]", false, sf::Vector2f{0.f, 1.f});
        set_lookup(vec, "icon size" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setIconSizeRatio(*f);});
            return false;
        });
        set_lookup(vec, "icon size" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<Button*>(w)->setIconSizeRatio(0.33f);});
        });

        set_lookup(vec, "text limit" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getTextLimit();}, EnumSection::Text);
        set_lookup(vec, "text limit" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const unsigned int* f = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setTextLimit(*f);});
            return false;
        });

        set_lookup(vec, "text position" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getTextPosition();}, EnumSection::Text);
        set_lookup(vec, "text position" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setTextPosition(*side);});
            return false;
        });

        set_lookup(vec, "icon path" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getIconPath();}, EnumSection::Image, "Image path", true);
        set_lookup(vec, "icon path" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setIcon(*s);});
            return false;
        });
        set_lookup(vec, "icon path" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<Button*>(w)->clearIcon();});
        });

        set_lookup(vec, "icon position" ,+ [](const void* w) -> Accessed_t {return static_cast<const Button*>(w)->getIconPosition();}, EnumSection::Image);
        set_lookup(vec, "icon position" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return static_cast<Button*>(w)->setIconPosition(*side);});
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
    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "child count" ,+ [](const void* w) -> Accessed_t {return static_cast<const Frame*>(w)->getChildCount();}, EnumSection::State);

        set_lookup(vec, "scroll offset" ,+ [](const void* w) -> Accessed_t {return static_cast<const Frame*>(w)->getScrollOffset();}, EnumSection::State);

        set_lookup(vec, "scroll speed" ,+ [](const void* w) -> Accessed_t {return static_cast<const Frame*>(w)->getScrollSpeed();}, EnumSection::State, "", false, sf::Vector2f{0.01f, 1.f});
        set_lookup(vec, "scroll speed" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Frame*>(w)->setScrollSpeed(*s);});
            return false;
        });
    
        set_lookup(vec, "separator info" ,+ [](const void* w) -> Accessed_t {return static_cast<const Frame*>(w)->getSeparatorInfo();}, EnumSection::State);
        set_lookup(vec, "separator info" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_SeparatorInfo>(&v))
                return invoke_setter([&]{return static_cast<Frame*>(w)->setSeparators((*s).type, (*s).ratio, (*s).dist);});
            return false;
        });
    
        set_lookup(vec, "process mode" ,+ [](const void* w) -> Accessed_t {return static_cast<const Frame*>(w)->getProcessMode();}, EnumSection::State);
        set_lookup(vec, "process mode" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_AfterPreProcess>(&v))
                return invoke_setter([&]{return static_cast<Frame*>(w)->setProcessMode(*s);});
            return false;
        });

        set_lookup(vec, "grid" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return static_cast<Frame*>(w)->setGrid(*s);});
            return false;
        }, EnumSection::State, "Vector2u{cols, rows}");

        set_lookup(vec, "scroll" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Frame*>(w)->scroll(*s);});
            return false;
        }, EnumSection::State, "Float");

        set_lookup(vec, "root window", + [](const void* w) -> Accessed_t {return static_cast<const Frame*>(w)->getWindowPtr();}, EnumSection::Pointer);
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `CheckBox`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<CheckBox>
{
    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "state" ,+ [](const void* w) -> Accessed_t {return static_cast<const CheckBox*>(w)->getState();}, EnumSection::State);

        set_lookup(vec, "text size" ,+ [](const void* w) -> Accessed_t {return static_cast<const CheckBox*>(w)->getTextSize();}, EnumSection::Text);
        set_lookup(vec, "text size" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<CheckBox*>(w)->setTextSize(*s);});
            return false;
        });

        set_lookup(vec, "text size ratio",+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<CheckBox*>(w)->setTextSize(*f);});
            return false;
        }, EnumSection::Text, "Float [0.f, 2.f]", false, sf::Vector2f{0.f, 2.f});

        set_lookup(vec, "text limit" ,+ [](const void* w) -> Accessed_t {return static_cast<const CheckBox*>(w)->getTextLimit();}, EnumSection::Text);
        set_lookup(vec, "text limit" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<CheckBox*>(w)->setTextLimit(*s);});
            return false;
        });

        set_lookup(vec, "text padding" ,+ [](const void* w) -> Accessed_t {return static_cast<const CheckBox*>(w)->getTextPadding();}, EnumSection::Text, "", false, sf::Vector2f{0.f, 5.f});
        set_lookup(vec, "text padding" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<CheckBox*>(w)->setTextPadding(*s);});
            return false;
        });

        set_lookup(vec, "text position" ,+ [](const void* w) -> Accessed_t {return static_cast<const CheckBox*>(w)->getTextPosition();}, EnumSection::Text);
        set_lookup(vec, "text position" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<DIRECTION>(&v))
                return invoke_setter([&]{return static_cast<CheckBox*>(w)->setTextPosition(*s);});
            return false;
        });

        set_lookup(vec, "text" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<CheckBox*>(w)->setText(*s);});
            return false;
        }, EnumSection::Text, "String");
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Entry`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Entry>
{
    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "text" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getText();}, EnumSection::Text);
        set_lookup(vec, "text" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setText(*s);});
            return false;
        });
        set_lookup(vec, "text" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<Entry*>(w)->setText("");});
        });

        set_lookup(vec, "text size" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getTextSize();}, EnumSection::Text);
        set_lookup(vec, "text size" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setTextSize(*s);});
            return false;
        });

        set_lookup(vec, "text size ratio" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setTextSize(*f);});
            return false;
        }, EnumSection::Text, "Float [0.f, 1.f]", false, sf::Vector2f{0.f, 1.f});

        set_lookup(vec, "text limit" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getTextLimit();}, EnumSection::Text);
        set_lookup(vec, "text limit" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<std::size_t>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setTextLimit(*s);});
            return false;
        });

        set_lookup(vec, "background text" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getBackGroundText();}, EnumSection::Text);
        set_lookup(vec, "background text" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setBackGroundText(*s);});
            return false;
        });

        set_lookup(vec, "blink ratio" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getBlinkUpRatio();}, EnumSection::State, "", false, sf::Vector2f{0.f, 1.f});
        set_lookup(vec, "blink ratio" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setBLinkUpRatio(*s);});
            return false;
        });

        set_lookup(vec, "cursor index" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getCursorIndex();}, EnumSection::State);
        set_lookup(vec, "cursor index" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setCursorIndex(*s);});
            return false;
        });

        set_lookup(vec, "placeholder char" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getPlaceholderChar();}, EnumSection::State);
        set_lookup(vec, "placeholder char" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<char>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setPlaceholderChar(*s);});
            return false;
        });

        set_lookup(vec, "text offset" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getTextOffsetRatio();}, EnumSection::Text, "", false, sf::Vector2f{0.f, 0.999f});
        set_lookup(vec, "text offset", + [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setTextOffsetRatio(*s);});
            return false;
        });

        set_lookup(vec, "cursor width" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getCursorWidth();}, EnumSection::State);
        set_lookup(vec, "cursor width" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Entry*>(w)->setCursorWidth(*s);});
            return false;
        });

        set_lookup(vec, "scroll offset" ,+ [](const void* w) -> Accessed_t {return static_cast<const Entry*>(w)->getScrollOffset();}, EnumSection::State);
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `ImageFrame`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<ImageFrame>
{
    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "image path" ,+ [](const void* w) -> Accessed_t {return static_cast<const ImageFrame*>(w)->getImagePath();}, EnumSection::Image, "Image path", true);
        set_lookup(vec, "image path" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<ImageFrame*>(w)->setImage(*s);});
            return false;
        });
        set_lookup(vec, "image path" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<ImageFrame*>(w)->clearImage();});
        });

        set_lookup(vec, "original image size" ,+ [](const void* w) -> Accessed_t {return static_cast<const ImageFrame*>(w)->getOriginalImageSize();}, EnumSection::Image);
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Label`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Label>
{
    static void link(LookupVec_t& vec)
    {
        set_lookup(vec, "text" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getText();}, EnumSection::Text);
        set_lookup(vec, "text" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setText(*s);});
            return false;
        });
        set_lookup(vec, "text" ,+ [](void* w) -> bool
        {
                return invoke_setter([&]{return static_cast<Label*>(w)->setText("");});
        });

        set_lookup(vec, "text size" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getTextSize();}, EnumSection::Text);
        set_lookup(vec, "text size" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setTextSize(*s);});
            return false;
        });

        set_lookup(vec, "text size ratio", +[](void* w, const Accessed_t& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setTextSize(*f);});
            return false;
        }, EnumSection::Text, "Float [0.f, 1.f]", false, sf::Vector2f{0.f, 1.f});

        set_lookup(vec, "text limit" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getTextLimit();}, EnumSection::Text);
        set_lookup(vec, "text limit" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setTextLimit(*s);});
            return false;
        });

        set_lookup(vec, "text style" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getTextStyle();}, EnumSection::Text);
        set_lookup(vec, "text style" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Text::Style>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setTextStyle(*s);});
            return false;
        });

        set_lookup(vec, "H align" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getHorizontalAlignment();}, EnumSection::State);
        set_lookup(vec, "H align" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Label::HAlign>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setHorizontalAlignment(*s);});
            return false;
        });

        set_lookup(vec, "V align" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getVerticalAlignment();}, EnumSection::State);
        set_lookup(vec, "V align" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<Label::VAlign>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setVerticalAlignment(*s);});
            return false;
        });

        set_lookup(vec, "line spacing" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getLineSpacing();}, EnumSection::State);
        set_lookup(vec, "line spacing" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setLineSpacing(*s);});
            return false;
        });

        set_lookup(vec, "letter spacing" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getLetterSpacing();}, EnumSection::State);
        set_lookup(vec, "letter spacing" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setLetterSpacing(*s);});
            return false;
        });

        set_lookup(vec, "padding ratio" ,+ [](const void* w) -> Accessed_t {return static_cast<const Label*>(w)->getPaddingRatio();}, EnumSection::State, "", false, sf::Vector2f{0.f, 1.f});
        set_lookup(vec, "padding ratio" ,+ [](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Label*>(w)->setPaddingRatio(*s);});
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
    static void link(LookupVec_t& vec)
    {
        /* Getters */
        set_lookup(vec, "local pixel" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->getLocalPixel(sf::Mouse::getPosition(*static_cast<const GridCanvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "view pixel" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->getViewPixel(sf::Mouse::getPosition(*static_cast<const GridCanvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "local to view" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->mapLocalToView(sf::Mouse::getPosition(*static_cast<const GridCanvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "coordinates" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->mapCoordinates(sf::Mouse::getPosition(*static_cast<const GridCanvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "selected cell" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->getSelectedCellPtr();}, EnumSection::Pointer);
        set_lookup(vec, "clicked cell" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->getClickedCellPtr();}, EnumSection::Pointer);
        set_lookup(vec, "cell count" ,+ [](const void* w) -> Accessed_t {return static_cast<const GridCanvas*>(w)->getCellsCount();}, EnumSection::State);

        /* Setters */
        set_lookup(vec, "insert" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return static_cast<GridCanvas*>(w)->insert(*s);});
            return false;
        }, EnumSection::Any, "Vector2u{row, col}");
        set_lookup(vec, "remove" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return static_cast<GridCanvas*>(w)->remove(*s);});
            return false;
        }, EnumSection::Any, "Vector2u{row, col}");
        set_lookup(vec, "dead zone" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<GridCanvas*>(w)->setDeadZone(*s);});
            return false;
        }, EnumSection::State, "Float [0.f, 0.1f]");
        set_lookup(vec, "is taken" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return static_cast<GridCanvas*>(w)->isTaken(*s);});
            return false;
        }, EnumSection::State, "Vector2u{row, col}");
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Links accessors for derived widget type `Canvas`
///
///////////////////////////////////////////////////////////////////
template<>
struct AccessorsLink<Canvas>
{
    static void link(LookupVec_t& vec)
    {
        /* Getters */
        set_lookup(vec, "local pixel" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->getLocalPixel(sf::Mouse::getPosition(*static_cast<const Canvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "view pixel" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->getViewPixel(sf::Mouse::getPosition(*static_cast<const Canvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "local to view" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->mapLocalToView(sf::Mouse::getPosition(*static_cast<const Canvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "coords to grid" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->mapCoordsToGrid(sf::Mouse::getPosition(*static_cast<const Canvas*>(w)->getRootWindow()));}, EnumSection::State);
        set_lookup(vec, "hovered cell" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->getHoveredCell();}, EnumSection::Pointer);
        set_lookup(vec, "selected cell" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->getSelectedCell();}, EnumSection::Pointer);
        set_lookup(vec, "cell count" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->getCellsCount();}, EnumSection::State);
        set_lookup(vec, "cell max count" ,+ [](const void* w) -> Accessed_t {return static_cast<const Canvas*>(w)->getMaxCellsCount();}, EnumSection::State);

        /* Setters */
        set_lookup(vec, "dead zone" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setDeadZone(*s);});
            return false;
        }, EnumSection::State, "Float [0.f, 0.1f]");

        set_lookup(vec, "join mode" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<bool>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setOpenLineJoinMode(*s);});
            return false;
        }, EnumSection::State, "true/false");

        set_lookup(vec, "door width" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setOpenLineDoorWidth(*s);});
            return false;
        }, EnumSection::State, "Float (default=0.4)");

        set_lookup(vec, "stub ratio" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setOpenLineStubRatio(*s);});
            return false;
        }, EnumSection::State, "Float (default=0.1)");

        set_lookup(vec, "dotted line spacing" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setDottedLineSpacing(*s);});
            return false;
        }, EnumSection::State, "Float (default=3)");

        set_lookup(vec, "dotted line radius" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setDottedLineRadius(*s);});
            return false;
        }, EnumSection::State, "Float (default=0.3)");

        set_lookup(vec, "dotted line row height" ,+[](void* w, const Accessed_t& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return static_cast<Canvas*>(w)->setDottedLineRowHeight(*s);});
            return false;
        }, EnumSection::State, "Float (default=2.5f)");
    }
};
}

#endif