#pragma once
#include "editor_types.hpp"
#if defined(WIDGETS_HPP) && defined(BUILD_EDITOR_GUI)

/*
* Contains: reflection for sfml widgets types
*/

namespace sfml_widgets
{
namespace enum_literals
{
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

#ifdef CANVAS_MORE_LINE_TYPES
constexpr auto LINE_TYPE_LITERAL = std::array<VecN<const char*>, 3>{NORMAL_LINE_LABEL, OPEN_LINE_LABEL, DOTS_LINE_LABEL};
#else
constexpr auto LINE_TYPE_LITERAL = std::array<VecN<const char*>, 1>{NORMAL_LINE_LABEL};
#endif
}
}

#endif