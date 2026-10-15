#pragma once
#include "editor_types.hpp"
#if defined(WIDGETS_HPP) && defined(BUILD_EDITOR_GUI)
#include <thread>
#include <atomic>

/*
* Contains: cache data objects for accessors that can't be obtained from a widget &
* are displayed in the GUI
*
* ToolTipObject ==> a tooltip text/image attributes
*
* ToolTipCache ==> cached tooltip attributes
*
* ShaderCache ==> cached shader attributes
*
* CanvasCache ==> cached grid canvas attributes
*
* CachedAttributes ==> a joint object for all cached attributes
*
* DialogThread ==> static separate thread for a file dialog
*
* LabeledGroup ==> widget group reflection + group object
*/

namespace sfml_widgets
{
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

///////////////////////////////////////////////////////////////////
/// @brief for `GridCanvas` class only
///
///////////////////////////////////////////////////////////////////
struct CanvasCache
{
    // lines
    unsigned int line_x0{0}, line_y0{0}, line_x1{0}, line_y1{0};
    unsigned int line_thickness{1};
    bool line_use_default_color{true};
    float line_color[4]{1.f, 1.f, 1.f, 1.f};
    int line_type_idx{0};

    // selected Cell editor
    int cell_side_idx{0};
    float cell_fill_color[4]{1.f, 1.f, 1.f, 1.f};
    unsigned int cell_outline_thickness{1};
    bool cell_use_default_outline_color{true};
    float cell_outline_color[4]{1.f, 1.f, 1.f, 1.f};
    int cell_outline_type_idx{0};
    std::array<char, 256> cell_texture_path{};

    // quick draw cached accessors
    float quick_draw_pen_color[4]{1.f, 0.f, 0.f, 1.f};
    bool quick_draw_enabled{true};
    bool quick_draw_skip_identical{false};
    bool quick_draw_show_pen{true};
    bool quick_draw_use_outline_color{false};
};

///////////////////////////////////////////////////////////////////
/// @brief Memory attributes for any widget
///
/// These attributes can't be obtained from the widget itself, so they
/// are stored and maintained externally, used in `WidgetEditor` class,
/// and they are not accurate by default.
///
///////////////////////////////////////////////////////////////////
struct CachedAttributes
{
    BuiltinBindFlags flags{BuiltinBindFlags::None};
    ShaderCache clip_shader_cache;
    ShaderCache color_shader_cache;
    ToolTipCache tooltip;
    CanvasCache GridCanvas;
    int theme_idx{-1};                                      //!< -1 == None
    sf::Vector2f grab_offset{0.f, 0.f};
    std::string label;
};

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
}

#endif