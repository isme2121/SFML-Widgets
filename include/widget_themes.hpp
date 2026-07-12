#pragma once
#include "widgets.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Color style representing all the states of a widget
///
/// Some widgets have their own styles, while some others share one
/// common type `TextureStyle`, the correct type for each widget is the
/// parameter for the derived widget method `setTheme`
///
///////////////////////////////////////////////////////////////////
struct TextureStyle
{
    sf::Color fill_color;
    sf::Color fill_hover_color;
    sf::Color fill_active_color;
    sf::Color fill_blocked_color;
    sf::Color fill_transparent_color;

    sf::Color outline_color;
    sf::Color outline_hover_color;
    sf::Color outline_active_color;
    sf::Color outline_blocked_color;
    sf::Color outline_transparent_color;

    float outline_thickness;
    float outline_hover_thickness;
    float outline_active_thickness;
    float outline_blocked_thickness;
    float outline_transparent_thickness;
};

struct ButtonStyle
{
    sf::Color fill_color;
    sf::Color fill_hover_color;
    sf::Color fill_active_color;
    sf::Color fill_blocked_color;
    sf::Color fill_transparent_color;

    sf::Color text_color;
    sf::Color text_hover_color;
    sf::Color text_active_color;
    sf::Color text_blocked_color;
    sf::Color text_transparent_color;

    sf::Color outline_color;
    sf::Color outline_hover_color;
    sf::Color outline_active_color;
    sf::Color outline_blocked_color;
    sf::Color outline_transparent_color;

    float outline_thickness;
    float outline_hover_thickness;
    float outline_active_thickness;
    float outline_blocked_thickness;
    float outline_transparent_thickness;
};

struct CheckboxStyle
{
    sf::Color fill_on_color;
    sf::Color fill_off_color;
    sf::Color fill_hover_color;
    sf::Color fill_blocked_color;
    sf::Color fill_trasnparent_color;
    
    sf::Color text_color;
    sf::Color text_hover_color;
    sf::Color text_blocked_color;
    sf::Color text_transparent_color;

    sf::Color outline_color;
    sf::Color outline_hover_color;
    sf::Color outline_blocked_color;
    sf::Color outline_transparent_color;

    float outline_thickness;
};

struct EntryStyle
{
    sf::Color fill_color;
    sf::Color fill_hover_color;
    sf::Color fill_active_color;
    sf::Color fill_blocked_color;
    sf::Color fill_transparent_color;

    sf::Color text_color;
    sf::Color text_hover_color;
    sf::Color text_active_color;
    sf::Color text_blocked_color;
    sf::Color text_transparent_color;

    sf::Color bg_text_color;
    sf::Color bg_text_hover_color;
    sf::Color bg_text_active_color;
    sf::Color bg_text_blocked_color;
    sf::Color bg_text_transparent_color;

    sf::Color outline_color;
    sf::Color outline_hover_color;
    sf::Color outline_active_color;
    sf::Color outline_blocked_color;
    sf::Color outline_transparent_color;

    float outline_thickness;
    float outline_hover_thickness;
    float outline_active_thickness;
    float outline_blocked_thickness;
    float outline_transparent_thickness;
};

struct LabelStyle
{
    sf::Color fill_color = sf::Color::Transparent;
    sf::Color fill_hover_color = sf::Color::Transparent;
    sf::Color fill_active_color = sf::Color::Transparent;
    sf::Color fill_blocked_color = sf::Color::Transparent;
    sf::Color fill_transparent_color = sf::Color::Transparent;

    sf::Color text_color;
    sf::Color text_hover_color;
    sf::Color text_active_color;
    sf::Color text_blocked_color;
    sf::Color text_transparent_color;

    sf::Color text_outline_color;
    sf::Color text_outline_hover_color;
    sf::Color text_outline_active_color;
    sf::Color text_outline_blocked_color;
    sf::Color text_outline_transparent_color;

    float text_outline_thickness;
    float text_outline_hover_thickness;
    float text_outline_active_thickness;
    float text_outline_blocked_thickness;
    float text_outline_transparent_thickness;
};

struct ToolTipStyle
{
    sf::Color fill_color;
    sf::Color outline_color;
    float outline_thickness;

    sf::Color text_color;
    sf::Color text_outline_color;
    float text_outline_thickness;
};

struct CellColor
{
    sf::Color bg_color;
    sf::Color top_color;
    sf::Color bottom_color;
    sf::Color left_color;
    sf::Color right_color;

    float top_outline_thickness;
    float bottom_outline_thickness;
    float left_outline_thickness;
    float right_outline_thickness;
};

///////////////////////////////////////////////////////////////////
/// @brief Shared theme between widget types
///
/// Constructed using `WidgetTheme::make_theme` static method, any
/// widget assigned to the `shared_ptr<WidgetTheme>` automatically
/// updates it's style to the assigned style with the proper type.
///
/// @see `WidgetStyle`, `Widget::linkTheme`, `Widget::unlinkTheme`
///
///////////////////////////////////////////////////////////////////
class WidgetTheme
{
private:
    // prevents constructing from WidgetStyle class
    class P
    {
        P() {}
        friend class WidgetTheme;
    };

    template <typename> friend struct WidgetStyle; //!< template implementation for each widget type

    WidgetTheme(P);

    WidgetTheme(P, ButtonStyle s_button, TextureStyle s_texture, CheckboxStyle s_checkbox, EntryStyle s_entry, LabelStyle s_label);
    ///////////////////////////////////////////////////////////////////
    /// @brief Various styles for each widget type
    ///////////////////////////////////////////////////////////////////

    ButtonStyle m_buttonTheme;
    TextureStyle m_frameTheme;
    TextureStyle m_gridCanvasTheme;
    TextureStyle m_imageFrameTheme;
    CheckboxStyle m_checkboxTheme;
    EntryStyle m_entryTheme;
    LabelStyle m_labelTheme;

    ///////////////////////////////////////////////////////////////////
    /// @brief Update tokens for each type, increments during each change
    ///////////////////////////////////////////////////////////////////

    std::size_t m_buttonToken{0};
    std::size_t m_frameToken{0};
    std::size_t m_gridCanvasToken{0};
    std::size_t m_imageFrameToken{0};
    std::size_t m_checkboxToken{0};
    std::size_t m_entryToken{0};
    std::size_t m_labelToken{0};

public:

    ///////////////////////////////////////////////////////////////////
    /// @brief creates and returns a shared pointer of type `WidgetTheme`
    ///
    /// The returned object can be assigned to any widget via method
    /// `Widget::linkTheme`, each assigned widget remains synced in style
    /// until the shared counter goes to zero.
    ///
    /// @param s_button style for button
    /// @param s_texture style for frame, canvas, and image frame (can be differentiated using `setStyle` method)
    /// @param s_checkbox style for checkbox
    /// @param s_entry style for entry
    /// @param s_label style for label
    ///
    /// @return shared pointer with the new `WidgetTheme` resource
    ///
    ///////////////////////////////////////////////////////////////////
    static std::shared_ptr<WidgetTheme> make_theme(ButtonStyle s_button, TextureStyle s_texture, CheckboxStyle s_checkbox, EntryStyle s_entry, LabelStyle s_label);

    ///////////////////////////////////////////////////////////////////
    /// @brief creates a `WidgetTheme` resource with copy constructor
    ///
    /// The returned object can be assigned to any widget via method
    /// `Widget::linkTheme`, each assigned widget remains synced in style
    /// until the shared counter goes to zero.
    ///
    /// @param theme reference to existing `WidgetTheme` object
    ///
    /// @return shared pointer with the new `WidgetTheme` resource
    ///
    ///////////////////////////////////////////////////////////////////
    static std::shared_ptr<WidgetTheme> make_theme(const WidgetTheme& theme);

    ///////////////////////////////////////////////////////////////////
    /// @brief copies the content of parameter theme to this object
    ///
    /// @param theme reference to existing `WidgetTheme` object
    ///
    ///////////////////////////////////////////////////////////////////
    void copy_theme(const WidgetTheme& theme);

    ///////////////////////////////////////////////////////////////////
    /// @brief returns current assigned style for template widget type
    ///
    /// @tparam WIDGET_DERIVED_TYPE derived widget type
    /// 
    /// @return matching style type for template type
    /// 
    ///////////////////////////////////////////////////////////////////
    template <typename WIDGET_DERIVED_TYPE>
    typename WidgetStyle<WIDGET_DERIVED_TYPE>::StyleType getStyle()
    {
        return WidgetStyle<WIDGET_DERIVED_TYPE>::get(*this);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief assigns parameter style to the template widget type
    ///
    /// @tparam WIDGET_DERIVED_TYPE derived widget type
    /// 
    /// @param s_widget style object optimal for template type
    ///
    ///////////////////////////////////////////////////////////////////
    template <typename WIDGET_DERIVED_TYPE>
    void setStyle(typename WidgetStyle<WIDGET_DERIVED_TYPE>::StyleType s_widget)
    {
        WidgetStyle<WIDGET_DERIVED_TYPE>::set(*this, s_widget);
        ++WidgetStyle<WIDGET_DERIVED_TYPE>::token(*this);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief returns the token reserved for the template type
    ///
    /// Widgets use the method to check for any updates in their type related
    /// style.
    ///
    /// @tparam WIDGET_DERIVED_TYPE widget type
    ///
    /// @return generated token
    ///
    ///////////////////////////////////////////////////////////////////
    template <typename WIDGET_DERIVED_TYPE>
    std::size_t getToken() noexcept
    {
        return WidgetStyle<WIDGET_DERIVED_TYPE>::token(*this);
    }

    WidgetTheme(const WidgetTheme& other) = delete;
    WidgetTheme& operator=(const WidgetTheme& other) = delete;
    WidgetTheme(WidgetTheme&&) = delete;
    WidgetTheme& operator=(WidgetTheme&&) = delete;

};

namespace Theme
{
namespace button_style
{
constexpr ButtonStyle BUTTON_LIGHT_STYLE =
{
    {245, 245, 247, 255},   // bg_color
    {228, 228, 233, 255},   // hover_bg_color
    {207, 224, 255, 255},   // active_bg_color
    {220, 220, 223, 255},   // blocked_bg_color
    {255, 255, 255,  15},   // transparent_bg_color
 
    {30, 30, 35, 255},      // default_text_color
    {30, 30, 35, 255},      // hover_text_color
    {30, 30, 35, 255},      // active_text_color
    {168, 168, 172, 255},   // blocked_text_color
    {30, 30, 35, 30},       // transparent_text_color
 
    {205, 205, 212, 255},   // default_outline_color
    {205, 205, 212, 255},   // hover_outline_color
    {205, 205, 212, 255},   // active_outline_color
    {215, 215, 218, 255},   // blocked_outline_color
    {185, 185, 198, 70},    // transparent_outline_color
 
    0.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle MENUBAR_BUTTON_LIGHT =
{
    {230, 230, 234, 255},   // bg_color
    {214, 214, 220, 255},   // hover_bg_color
    {190, 212, 252, 255},   // active_bg_color
    {222, 222, 226, 255},   // blocked_bg_color
    {238, 238, 241, 15},    // transparent_bg_color
 
    {42, 42, 48, 255},      // default_text_color
    {42, 42, 48, 255},      // hover_text_color
    {42, 42, 48, 255},      // active_text_color
    {162, 162, 168, 255},   // blocked_text_color
    {42, 42, 48, 30},       // transparent_text_color
 
    {192, 192, 200, 255},   // default_outline_color
    {152, 152, 166, 255},   // hover_outline_color
    {88, 145, 255, 255},    // active_outline_color
    {208, 208, 213, 255},   // blocked_outline_color
    {178, 178, 192,  68},   // transparent_outline_color
 
    0.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_DARK_STYLE =
{
    {28, 28, 32, 255},      // bg_color
    {44, 44, 52, 255},      // hover_bg_color
    {20, 50, 110, 255},     // active_bg_color
    {34, 34, 38, 255},      // blocked_bg_color
    {28, 28, 32, 15},       // transparent_bg_color
 
    {218, 215, 208, 255},   // default_text_color
    {218, 215, 208, 255},   // hover_text_color
    {218, 215, 208, 255},   // active_text_color
    {88, 88, 98, 255},      // blocked_text_color
    {200, 198, 192, 30},    // transparent_text_color
 
    {58, 58, 68, 255},      // default_outline_color
    {90, 90, 108, 255},     // hover_outline_color
    {85, 138, 255, 255},    // active_outline_color
    {44, 44, 50, 255},      // blocked_outline_color
    {78, 78, 95, 72},       // transparent_outline_color
 
    0.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle MENUBAR_BUTTON_DARK =
{
    {16, 16, 20, 255},      // bg_color
    {28, 28, 34, 255},      // hover_bg_color
    {14, 40, 92, 255},      // active_bg_color
    {20, 20, 24, 255},      // blocked_bg_color
    {16, 16, 20, 15},       // transparent_bg_color
 
    {198, 195, 188, 255},   // default_text_color
    {198, 195, 188, 255},   // hover_text_color
    {198, 195, 188, 255},   // active_text_color
    {66, 66, 76, 255},      // blocked_text_color
    {182, 179, 172, 30},    // transparent_text_color
 
    {36, 36, 44, 255},      // default_outline_color
    {62, 62, 78, 255},      // hover_outline_color
    {85, 138, 255, 255},    // active_outline_color
    {26, 26, 30, 255},      // blocked_outline_color
    {52, 52, 68, 58},       // transparent_outline_color
 
    0.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_STALE_VIOLET =
{
    {30, 27, 46},           // default_color
    {45, 40, 72},           // hover_color
    {76, 63, 160},          // active_color
    {22, 22, 32},           // blocked_color
    {30, 27, 46, 80},       // transparent_color

    {210, 205, 240},        // default_text_color
    {225, 220, 255},        // hover_text_color
    {255, 252, 255},        // active_text_color
    {90, 88, 110},          // blocked_text_color
    {180, 175, 210, 120},   // transparent_text_color

    {90, 80, 180},          // default_outline_color
    {120, 108, 220},        // hover_outline_color
    {150, 138, 240},        // active_outline_color
    {55, 52, 80},           // blocked_outline_color
    {90, 80, 180, 120},     // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_CARBON_CYAN =
{
    {20, 22, 26},           // default_color
    {28, 32, 38},           // hover_color
    {12, 60, 72},           // active_color
    {18, 18, 20},           // blocked_color
    {20, 22, 26, 80},       // transparent_color

    {200, 240, 245},        // default_text_color
    {220, 248, 252},        // hover_text_color
    {240, 252, 255},        // active_text_color
    {70, 80, 82},           // blocked_text_color
    {150, 200, 210, 120},   // transparent_text_color

    {30, 180, 200},         // default_outline_color
    {50, 210, 225},         // hover_outline_color
    {80, 230, 240},         // active_outline_color
    {40, 55, 58},           // blocked_outline_color
    {30, 180, 200, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_OBSIDIAN_EMBER =
{
    {22, 20, 18},           // default_color
    {34, 28, 22},           // hover_color
    {80, 42, 14},           // active_color
    {20, 18, 16},           // blocked_color
    {22, 20, 18, 80},       // transparent_color

    {240, 220, 200},        // default_text_color
    {252, 235, 215},        // hover_text_color
    {255, 242, 225},        // active_text_color
    {80, 72, 62},           // blocked_text_color
    {180, 150, 120, 120},   // transparent_text_color

    {200, 100, 40},         // default_outline_color
    {230, 120, 50},         // hover_outline_color
    {250, 140, 60},         // active_outline_color
    {55, 45, 35},           // blocked_outline_color
    {200, 100, 40, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_MIDNIGHT_ROSE =
{
    {22, 18, 26},           // default_color
    {36, 26, 40},           // hover_color
    {80, 24, 50},           // active_color
    {20, 16, 22},           // blocked_color
    {22, 18, 26, 80},       // transparent_color

    {240, 200, 215},        // default_text_color
    {252, 215, 230},        // hover_text_color
    {255, 230, 242},        // active_text_color
    {80, 65, 75},           // blocked_text_color
    {170, 130, 150, 120},   // transparent_text_color

    {180, 60, 100},         // default_outline_color
    {210, 80, 120},         // hover_outline_color
    {240, 100, 140},        // active_outline_color
    {55, 40, 52},           // blocked_outline_color
    {180, 60, 100, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_EMERALD_FOREST =
{
    {24, 32, 26},
    {47, 54, 49},
    {36, 97, 64},
    {18, 24, 20},
    {24, 32, 26, 80},

    {206, 218, 211},
    {166, 201, 182},
    {228, 235, 231},
    {103, 118, 108},
    {206, 218, 211, 120},

    {38, 109, 71},
    {71, 131, 99},
    {103, 153, 126},
    {17, 49, 32},
    {38, 109, 71, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr ButtonStyle BUTTON_GOLDEN_AMBER =
{
    {34, 28, 18},
    {56, 51, 42},
    {128, 95, 30},
    {26, 21, 14},
    {34, 28, 18, 80},

    {223, 217, 205},
    {218, 200, 164},
    {237, 234, 228},
    {123, 115, 100},
    {223, 217, 205, 120},

    {145, 107, 32},
    {162, 129, 65},
    {178, 151, 99},
    {65, 48, 14},
    {145, 107, 32, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr ButtonStyle BUTTON_CRIMSON_ASH =
{
    {32, 20, 20},
    {54, 44, 44},
    {122, 39, 41},
    {24, 15, 15},
    {32, 20, 20, 80},

    {222, 207, 207},
    {215, 169, 170},
    {237, 229, 229},
    {121, 102, 102},
    {222, 207, 207, 120},

    {138, 43, 45},
    {156, 75, 76},
    {173, 107, 108},
    {62, 19, 20},
    {138, 43, 45, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr ButtonStyle BUTTON_STEEL_GRAPHITE =
{
    {24, 26, 30},
    {47, 49, 52},
    {69, 80, 96},
    {18, 20, 22},
    {24, 26, 30, 80},

    {212, 215, 217},
    {185, 192, 200},
    {231, 233, 234},
    {122, 124, 127},
    {212, 215, 217, 120},

    {77, 90, 108},
    {104, 115, 130},
    {130, 140, 152},
    {35, 40, 49},
    {77, 90, 108, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr ButtonStyle BUTTON_LAVENDER_DUSK =
{
    {28, 24, 34},
    {51, 47, 56},
    {98, 80, 123},
    {21, 18, 26},
    {28, 24, 34, 80},

    {218, 214, 222},
    {202, 192, 215},
    {235, 232, 237},
    {131, 127, 137},
    {218, 214, 222, 120},

    {111, 90, 139},
    {133, 115, 156},
    {154, 140, 174},
    {50, 40, 63},
    {111, 90, 139, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr ButtonStyle BUTTON_SKY_AZURE =
{
    {18, 24, 34},
    {42, 47, 56},
    {41, 85, 139},
    {14, 18, 26},
    {18, 24, 34, 80},

    {207, 215, 225},
    {170, 195, 225},
    {229, 233, 238},
    {117, 127, 139},
    {207, 215, 225, 120},

    {45, 96, 158},
    {76, 120, 173},
    {108, 144, 187},
    {20, 43, 71},
    {45, 96, 158, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr ButtonStyle BUTTON_ARCTIC_FROST =
{
    {246, 248, 250},
    {229, 231, 232},
    {209, 225, 241},
    {231, 233, 235},
    {246, 248, 250, 15},

    {30, 30, 35},
    {30, 30, 35},
    {30, 30, 35},
    {168, 168, 172},
    {30, 30, 35, 30},

    {212, 213, 215},
    {197, 198, 200},
    {144, 186, 225},
    {212, 213, 215},
    {212, 213, 215, 70},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr ButtonStyle BUTTON_SANDSTONE_DUNE =
{
    {250, 244, 235},
    {232, 227, 219},
    {240, 217, 202},
    {235, 229, 221},
    {250, 244, 235, 15},

    {30, 30, 35},
    {30, 30, 35},
    {30, 30, 35},
    {168, 168, 172},
    {30, 30, 35, 30},

    {215, 210, 202},
    {200, 195, 188},
    {222, 170, 144},
    {215, 210, 202},
    {215, 210, 202, 70},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};
}

namespace texture_style
{
constexpr TextureStyle DEFAULT_LIGHT_STYLE =
{
    {245, 245, 247, 255},   // bg_color
    {245, 245, 247, 255},   // hover_bg_color
    {245, 245, 247, 255},   // active_bg_color
    {220, 220, 223, 255},   // blocked_bg_color
    {255, 255, 255, 15},    // transparent_bg_color

    {205, 205, 212, 255},   // outline_color
    {205, 205, 212, 255},   // hover_outline_color
    {205, 205, 212, 255},   // active_outline_color
    {215, 215, 218, 255},   // blocked_outline_color
    {185, 185, 198, 70},    // transparent_outline_color

    1.0f,                   // outline_thickness          
    1.0f,                   // hovered_outline_thickness
    1.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f,                   // transparent_outline_thickness  
};

constexpr TextureStyle CANVAS_LIGHT_STYLE =
{
    {245, 245, 247, 255},   // bg_color
    {245, 245, 247, 255},   // hover_bg_color
    {245, 245, 247, 255},   // active_bg_color
    {220, 220, 223, 255},   // blocked_bg_color
    {255, 255, 255, 15},    // transparent_bg_color

    {205, 205, 212, 255},   // outline_color
    {205, 205, 212, 255},   // hover_outline_color
    {205, 205, 212, 255},   // active_outline_color
    {215, 215, 218, 255},   // blocked_outline_color
    {185, 185, 198, 70},    // transparent_outline_color

    1.0f,                   // outline_thickness          
    1.0f,                   // hovered_outline_thickness
    1.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f,                   // transparent_outline_thickness  
};

constexpr sfml_widgets::TextureStyle DEFAULT_DARK_STYLE =
{
    {28, 28, 32, 255},      // bg_color
    {28, 28, 32, 255},      // hover_bg_color
    {28, 28, 32, 255},      // active_bg_color
    {34, 34, 38, 255},      // blocked_bg_color
    {28, 28, 32, 15},       // transparent_bg_color

    {58, 58, 68, 255},      // outline_color
    {58, 58, 68, 255},      // hover_outline_color
    {58, 58, 68, 255},      // active_outline_color
    {44, 44, 50, 255},      // blocked_outline_color
    {78, 78, 95, 72},       // transparent_outline_color

    1.0f,                   // outline_thickness
    1.0f,                   // hovered_outline_thickness
    1.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f,                   // transparent_outline_thickness
};


 
constexpr TextureStyle MENUBAR_TEXTURE_DARK =
{
    {16, 16, 20, 255},      // bg_color
    {16, 16, 20, 255},      // hover_bg_color
    {16, 16, 20, 255},      // active_bg_color
    {20, 20, 24, 255},      // blocked_bg_color
    {16, 16, 20, 15},       // transparent_bg_color
 
    {36, 36, 44, 255},      // default_outline_color
    {36, 36, 44, 255},      // hover_outline_color
    {36, 36, 44, 255},      // active_outline_color
    {26, 26, 30, 255},      // blocked_outline_color
    {52, 52, 68, 58},       // transparent_outline_color
 
    0.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f,                   // transparent_outline_thickness
};



constexpr sfml_widgets::TextureStyle CANVAS_DARK_STYLE =
{
    {28, 28, 32, 255},      // bg_color
    {28, 28, 32, 255},      // hover_bg_color
    {28, 28, 32, 255},      // active_bg_color
    {34, 34, 38, 255},      // blocked_bg_color
    {28, 28, 32, 15},       // transparent_bg_color

    {58, 58, 68, 255},      // outline_color
    {58, 58, 68, 255},      // hover_outline_color
    {58, 58, 68, 255},      // active_outline_color
    {44, 44, 50, 255},      // blocked_outline_color
    {78, 78, 95, 72},       // transparent_outline_color

    1.0f,                   // outline_thickness
    1.0f,                   // hovered_outline_thickness
    1.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f,                   // transparent_outline_thickness
};

constexpr TextureStyle MENUBAR_TEXTURE_LIGHT =
{
    {230, 230, 234, 255},   // bg_color
    {230, 230, 234, 255},   // hover_bg_color
    {230, 230, 234, 255},   // active_bg_color
    {222, 222, 226, 255},   // blocked_bg_color
    {238, 238, 241, 15},    // transparent_bg_color
 
    {192, 192, 200, 255},   // default_outline_color
    {152, 152, 166, 255},   // hover_outline_color
    {88, 145, 255, 255},    // active_outline_color
    {208, 208, 213, 255},   // blocked_outline_color
    {178, 178, 192, 68},    // transparent_outline_color
 
    0.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f,                   // transparent_outline_thickness
};

constexpr TextureStyle STYLE_STALE_VIOLET =
{
    {30, 27, 46},           // default_color
    {45, 40, 72},           // hover_color
    {76, 63, 160},          // active_color
    {22, 22, 32},           // blocked_color
    {30, 27, 46, 80},       // transparent_color

    {90, 80, 180},          // default_outline_color
    {120, 108, 220},        // hover_outline_color
    {150, 138, 240},        // active_outline_color
    {55, 52, 80},           // blocked_outline_color
    {90, 80, 180, 120},     // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_CARBON_CYAN =
{
    {20, 22, 26},           // default_color
    {28, 32, 38},           // hover_color
    {12, 60, 72},           // active_color
    {18, 18, 20},           // blocked_color
    {20, 22, 26, 80},       // transparent_color

    {30, 180, 200},         // default_outline_color
    {50, 210, 225},         // hover_outline_color
    {80, 230, 240},         // active_outline_color
    {40, 55, 58},           // blocked_outline_color
    {30, 180, 200, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_OBSIDIAN_EMBER =
{
    {22, 20, 18},           // default_color
    {34, 28, 22},           // hover_color
    {80, 42, 14},           // active_color
    {20, 18, 16},           // blocked_color
    {22, 20, 18, 80},       // transparent_color

    {200, 100, 40},         // default_outline_color
    {230, 120, 50},         // hover_outline_color
    {250, 140, 60},         // active_outline_color
    {55, 45, 35},           // blocked_outline_color
    {200, 100, 40, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_MIDNIGHT_ROSE =
{
    {22, 18, 26},           // default_color
    {36, 26, 40},           // hover_color
    {80, 24, 50},           // active_color
    {20, 16, 22},           // blocked_color
    {22, 18, 26, 80},       // transparent_color

    {180, 60, 100},         // default_outline_color
    {210, 80, 120},         // hover_outline_color
    {240, 100, 140},        // active_outline_color
    {55, 40, 52},           // blocked_outline_color
    {180, 60, 100, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle TEXTURE_EMERALD_FOREST =
{
    {24, 32, 26},
    {47, 54, 49},
    {36, 97, 64},
    {18, 24, 20},
    {24, 32, 26, 80},

    {38, 109, 71},
    {71, 131, 99},
    {103, 153, 126},
    {17, 49, 32},
    {38, 109, 71, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr TextureStyle TEXTURE_GOLDEN_AMBER =
{
    {34, 28, 18},
    {56, 51, 42},
    {128, 95, 30},
    {26, 21, 14},
    {34, 28, 18, 80},

    {145, 107, 32},
    {162, 129, 65},
    {178, 151, 99},
    {65, 48, 14},
    {145, 107, 32, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr TextureStyle TEXTURE_CRIMSON_ASH =
{
    {32, 20, 20},
    {54, 44, 44},
    {122, 39, 41},
    {24, 15, 15},
    {32, 20, 20, 80},

    {138, 43, 45},
    {156, 75, 76},
    {173, 107, 108},
    {62, 19, 20},
    {138, 43, 45, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr TextureStyle TEXTURE_STEEL_GRAPHITE =
{
    {24, 26, 30},
    {47, 49, 52},
    {69, 80, 96},
    {18, 20, 22},
    {24, 26, 30, 80},

    {77, 90, 108},
    {104, 115, 130},
    {130, 140, 152},
    {35, 40, 49},
    {77, 90, 108, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr TextureStyle TEXTURE_LAVENDER_DUSK =
{
    {28, 24, 34},
    {51, 47, 56},
    {98, 80, 123},
    {21, 18, 26},
    {28, 24, 34, 80},

    {111, 90, 139},
    {133, 115, 156},
    {154, 140, 174},
    {50, 40, 63},
    {111, 90, 139, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr TextureStyle TEXTURE_SKY_AZURE =
{
    {18, 24, 34},
    {42, 47, 56},
    {41, 85, 139},
    {14, 18, 26},
    {18, 24, 34, 80},

    {45, 96, 158},
    {76, 120, 173},
    {108, 144, 187},
    {20, 43, 71},
    {45, 96, 158, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr TextureStyle TEXTURE_ARCTIC_FROST =
{
    {246, 248, 250},
    {229, 231, 232},
    {209, 225, 241},
    {231, 233, 235},
    {246, 248, 250, 15},

    {212, 213, 215},
    {197, 198, 200},
    {144, 186, 225},
    {212, 213, 215},
    {212, 213, 215, 70},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr TextureStyle TEXTURE_SANDSTONE_DUNE =
{
    {250, 244, 235},
    {232, 227, 219},
    {240, 217, 202},
    {235, 229, 221},
    {250, 244, 235, 15},

    {215, 210, 202},
    {200, 195, 188},
    {222, 170, 144},
    {215, 210, 202},
    {215, 210, 202, 70},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};
}

namespace checkbox_style
{
constexpr CheckboxStyle CHECKBOX_LIGHT_STYLE =
{
    {28, 28, 32, 255},      // on_color
    {245, 245, 247, 255},   // off_color
    {245, 245, 247, 255},   // hover_color
    {220, 220, 223, 255},   // blocked_color
    {255, 255, 255, 15},    // transparent_color

    {30, 30, 35, 255},      // text_color
    {30, 30, 35, 255},      // hover_text_color
    {168, 168, 172, 255},   // blocked_text_color
    {30, 30, 35, 30},       // transparent_text_color

    {205, 205, 212, 255},   // outline_color
    {205, 205, 212, 255},   // hover_outline_color
    {215, 215, 218, 255},   // blocked_outline_color
    {185, 185, 198, 70},    // transparent_outline_color

    5.0f,                   // outline_thickness          
};

constexpr CheckboxStyle CHECKBOX_DARK_STYLE =
{
    {245, 245, 247, 255},   // on_color
    {28, 28, 32, 255},      // off_color
    {28, 28, 32, 255},      // hover_color
    {34, 34, 38, 255},      // blocked_color
    {28, 28, 32, 15},       // transparent_color

    {218, 215, 208, 255},   // text_color
    {218, 215, 208, 255},   // hover_text_color
    {88, 88, 98, 255},      // blocked_text_color
    {200, 198, 192, 30},    // transparent_text_color

    {205, 205, 212, 255},   // outline_color
    {205, 205, 212, 255},   // hover_outline_color
    {215, 215, 218, 255},   // blocked_outline_color
    {185, 185, 198, 70},    // transparent_outline_color

    5.0f,                   // outline_thickness          
};

constexpr CheckboxStyle CHECKBOX_STALE_VIOLET =
{
    {76, 63, 160},          // on_color
    {30, 27, 46},           // off_color
    {45, 40, 72},           // hover_color
    {22, 22, 32},           // blocked_color
    {30, 27, 46, 80},       // transparent_color

    {255, 252, 255},        // text_color
    {225, 220, 255},        // hover_text_color
    {90, 88, 110},          // blocked_text_color
    {180, 175, 210, 120},   // transparent_text_color

    {150, 138, 240},        // outline_color
    {120, 108, 220},        // hover_outline_color
    {55, 52, 80},           // blocked_outline_color
    {90, 80, 180, 120},     // transparent_outline_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_CARBON_CYAN =
{
    {12, 60, 72},           // on_color
    {20, 22, 26},           // off_color
    {28, 32, 38},           // hover_color
    {18, 18, 20},           // blocked_color
    {20, 22, 26, 80},       // transparent_color

    {240, 252, 255},        // text_color
    {220, 248, 252},        // hover_text_color
    {70, 80, 82},           // blocked_text_color
    {150, 200, 210, 120},   // transparent_text_color

    {80, 230, 240},         // outline_color
    {50, 210, 225},         // hover_outline_color
    {40, 55, 58},           // blocked_outline_color
    {30, 180, 200, 120},    // transparent_outline_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_OBSIDIAN_EMBER =
{
    {80, 42, 14},           // on_color
    {22, 20, 18},           // off_color
    {34, 28, 22},           // hover_color
    {20, 18, 16},           // blocked_color
    {22, 20, 18, 80},       // transparent_color

    {255, 242, 225},        // text_color
    {252, 235, 215},        // hover_text_color
    {80, 72, 62},           // blocked_text_color
    {180, 150, 120, 120},   // transparent_text_color

    {250, 140, 60},         // outline_color
    {230, 120, 50},         // hover_outline_color
    {55, 45, 35},           // blocked_outline_color
    {200, 100, 40, 120},    // transparent_outline_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_MIDNIGHT_ROSE =
{
    {80, 24, 50},           // on_color
    {22, 18, 26},           // off_color
    {36, 26, 40},           // hover_color
    {20, 16, 22},           // blocked_color
    {22, 18, 26, 80},       // transparent_color

    {255, 230, 242},        // text_color
    {252, 215, 230},        // hover_text_color
    {80, 65, 75},           // blocked_text_color
    {170, 130, 150, 120},   // transparent_text_color

    {240, 100, 140},        // outline_color
    {210, 80,  120},        // hover_outline_color
    {55, 40, 52},           // blocked_outline_color
    {180, 60, 100, 120},    // transparent_outline_color

    2.0f,                   // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_EMERALD_FOREST =
{
    {43, 132, 85},           // fill_on_color
    {24, 32, 26},           // fill_off_color
    {47, 54, 49},           // fill_hover_color
    {18, 24, 20},           // fill_blocked_color
    {24, 32, 26, 80},           // fill_trasnparent_color (sic — matches real field name)

    {206, 218, 211},           // text_color
    {166, 201, 182},           // text_hover_color
    {103, 118, 108},           // text_blocked_color
    {206, 218, 211, 120},           // text_transparent_color

    {38, 109, 71},           // outline_color
    {71, 131, 99},           // outline_hover_color
    {17, 49, 32},           // outline_blocked_color
    {38, 109, 71, 120},           // outline_transparent_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_GOLDEN_AMBER =
{
    {179, 132, 37},           // fill_on_color
    {34, 28, 18},           // fill_off_color
    {56, 51, 42},           // fill_hover_color
    {26, 21, 14},           // fill_blocked_color
    {34, 28, 18, 80},           // fill_trasnparent_color (sic — matches real field name)

    {223, 217, 205},           // text_color
    {218, 200, 164},           // text_hover_color
    {123, 115, 100},           // text_blocked_color
    {223, 217, 205, 120},           // text_transparent_color

    {145, 107, 32},           // outline_color
    {162, 129, 65},           // outline_hover_color
    {65, 48, 14},           // outline_blocked_color
    {145, 107, 32, 120},           // outline_transparent_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_CRIMSON_ASH =
{
    {171, 50, 52},           // fill_on_color
    {32, 20, 20},           // fill_off_color
    {54, 44, 44},           // fill_hover_color
    {24, 15, 15},           // fill_blocked_color
    {32, 20, 20, 80},           // fill_trasnparent_color (sic — matches real field name)

    {222, 207, 207},           // text_color
    {215, 169, 170},           // text_hover_color
    {121, 102, 102},           // text_blocked_color
    {222, 207, 207, 120},           // text_transparent_color

    {138, 43, 45},           // outline_color
    {156, 75, 76},           // outline_hover_color
    {62, 19, 20},           // outline_blocked_color
    {138, 43, 45, 120},           // outline_transparent_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_STEEL_GRAPHITE =
{
    {93, 110, 132},           // fill_on_color
    {24, 26, 30},           // fill_off_color
    {47, 49, 52},           // fill_hover_color
    {18, 20, 22},           // fill_blocked_color
    {24, 26, 30, 80},           // fill_trasnparent_color (sic — matches real field name)

    {212, 215, 217},           // text_color
    {185, 192, 200},           // text_hover_color
    {122, 124, 127},           // text_blocked_color
    {212, 215, 217, 120},           // text_transparent_color

    {77, 90, 108},           // outline_color
    {104, 115, 130},           // outline_hover_color
    {35, 40, 49},           // outline_blocked_color
    {77, 90, 108, 120},           // outline_transparent_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_LAVENDER_DUSK =
{
    {136, 110, 171},           // fill_on_color
    {28, 24, 34},           // fill_off_color
    {51, 47, 56},           // fill_hover_color
    {21, 18, 26},           // fill_blocked_color
    {28, 24, 34, 80},           // fill_trasnparent_color (sic — matches real field name)

    {218, 214, 222},           // text_color
    {202, 192, 215},           // text_hover_color
    {131, 127, 137},           // text_blocked_color
    {218, 214, 222, 120},           // text_transparent_color

    {111, 90, 139},           // outline_color
    {133, 115, 156},           // outline_hover_color
    {50, 40, 63},           // outline_blocked_color
    {111, 90, 139, 120},           // outline_transparent_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_SKY_AZURE =
{
    {54, 118, 196},           // fill_on_color
    {18, 24, 34},           // fill_off_color
    {42, 47, 56},           // fill_hover_color
    {14, 18, 26},           // fill_blocked_color
    {18, 24, 34, 80},           // fill_trasnparent_color (sic — matches real field name)

    {207, 215, 225},           // text_color
    {170, 195, 225},           // text_hover_color
    {117, 127, 139},           // text_blocked_color
    {207, 215, 225, 120},           // text_transparent_color

    {45, 96, 158},           // outline_color
    {76, 120, 173},           // outline_hover_color
    {20, 43, 71},           // outline_blocked_color
    {45, 96, 158, 120},           // outline_transparent_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_ARCTIC_FROST =
{
    {181, 208, 234},           // fill_on_color
    {246, 248, 250},           // fill_off_color
    {229, 231, 232},           // fill_hover_color
    {231, 233, 235},           // fill_blocked_color
    {246, 248, 250, 15},           // fill_trasnparent_color (sic — matches real field name)

    {30, 30, 35},           // text_color
    {30, 30, 35},           // text_hover_color
    {168, 168, 172},           // text_blocked_color
    {30, 30, 35, 30},           // text_transparent_color

    {212, 213, 215},           // outline_color
    {197, 198, 200},           // outline_hover_color
    {212, 213, 215},           // outline_blocked_color
    {212, 213, 215, 70},           // outline_transparent_color

    0.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_SANDSTONE_DUNE =
{
    {232, 197, 177},           // fill_on_color
    {250, 244, 235},           // fill_off_color
    {232, 227, 219},           // fill_hover_color
    {235, 229, 221},           // fill_blocked_color
    {250, 244, 235, 15},           // fill_trasnparent_color (sic — matches real field name)

    {30, 30, 35},           // text_color
    {30, 30, 35},           // text_hover_color
    {168, 168, 172},           // text_blocked_color
    {30, 30, 35, 30},           // text_transparent_color

    {215, 210, 202},           // outline_color
    {200, 195, 188},           // outline_hover_color
    {215, 210, 202},           // outline_blocked_color
    {215, 210, 202, 70},           // outline_transparent_color

    0.0f                    // outline_thickness
};

}

namespace entry_style
{

constexpr EntryStyle ENTRY_DARK_STYLE =
{
    {28, 28, 32, 255},      // bg_color
    {44, 44, 52, 255},      // hover_bg_color
    {20, 50, 110, 255},     // active_bg_color
    {34, 34, 38, 255},      // blocked_bg_color
    {28, 28, 32, 15},       // transparent_bg_color
 
    {218, 215, 208, 255},   // default_text_color
    {218, 215, 208, 255},   // hover_text_color
    {218, 215, 208, 255},   // active_text_color
    {88, 88, 98, 255},      // blocked_text_color
    {200, 198, 192, 30},    // transparent_text_color
 
    {218, 215, 208, 255},   // bg_text_default_color
    {218, 215, 208, 255},   // bg_text_hover_color
    {218, 215, 208, 255},   // bg_text_active_color
    {88, 88, 98, 255},      // bg_text_blocked_color
    {200, 198, 192, 30},    // bg_text_transparent_color

    {58, 58, 68, 255},      // default_outline_color
    {90, 90, 108, 255},     // hover_outline_color
    {85, 138, 255, 255},    // active_outline_color
    {44, 44, 50, 255},      // blocked_outline_color
    {78, 78, 95, 72},       // transparent_outline_color
 
    1.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f                    // transparent_outline_thickness
};

constexpr EntryStyle ENTRY_LIGHT_STYLE =
{
    {245, 245, 247, 255},   // bg_color
    {228, 228, 233, 255},   // hover_bg_color
    {207, 224, 255, 255},   // active_bg_color
    {220, 220, 223, 255},   // blocked_bg_color
    {255, 255, 255, 15},    // transparent_bg_color
 
    {30, 30, 35, 255},      // default_text_color
    {30, 30, 35, 255},      // hover_text_color
    {30, 30, 35, 255},      // active_text_color
    {168, 168, 172, 255},   // blocked_text_color
    {30, 30, 35, 30},       // transparent_text_color
 
    {30, 30, 35, 255},      // bg_text_default_color
    {30, 30, 35, 255},      // bg_text_hover_color
    {30, 30, 35, 255},      // bg_text_active_color
    {168, 168, 172, 255},   // bg_text_blocked_color
    {30, 30, 35, 30},       // bg_text_transparent_color

    {205, 205, 212, 255},   // default_outline_color
    {205, 205, 212, 255},   // hover_outline_color
    {205, 205, 212, 255},   // active_outline_color
    {215, 215, 218, 255},   // blocked_outline_color
    {185, 185, 198, 70},    // transparent_outline_color
 
    1.0f,                   // outline_thickness
    0.0f,                   // hovered_outline_thickness
    0.0f,                   // active_outline_thickness
    0.0f,                   // blocked_outline_thickness
    0.0f                    // transparent_outline_thickness
};

constexpr EntryStyle ENTRY_STALE_VIOLET =
{
    {30, 27, 46},           // fill_color
    {45, 40, 72},           // fill_hover_color
    {76, 63, 160},          // fill_active_color
    {22, 22, 32},           // fill_blocked_color
    {30, 27, 46, 80},       // fill_transparent_color

    {210, 205, 240},        // text_default_color
    {225, 220, 255},        // text_hover_color
    {255, 252, 255},        // text_active_color
    {90, 88, 110},          // text_blocked_color
    {180, 175, 210, 120},   // text_transparent_color

    {80, 75, 120},          // bg_text_default_color
    {95, 88, 145},          // bg_text_hover_color
    {120, 108, 180},        // bg_text_active_color
    {50, 48, 75},           // bg_text_blocked_color
    {80, 75, 120, 80},      // bg_text_transparent_color

    {90, 80, 180},          // outline_color
    {120, 108, 220},        // outline_hover_color
    {150, 138, 240},        // outline_active_color
    {55, 52, 80},           // outline_blocked_color
    {90, 80, 180, 120},     // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_CARBON_CYAN =
{
    {20, 22, 26},           // fill_color
    {28, 32, 38},           // fill_hover_color
    {12, 60, 72},           // fill_active_color
    {18, 18, 20},           // fill_blocked_color
    {20, 22, 26, 80},       // fill_transparent_color

    {200, 240, 245},        // text_default_color
    {220, 248, 252},        // text_hover_color
    {240, 252, 255},        // text_active_color
    {70, 80, 82},           // text_blocked_color
    {150, 200, 210, 120},   // text_transparent_color

    {40, 90, 100},          // bg_text_default_color
    {50, 110, 120},         // bg_text_hover_color
    {30, 130, 150},         // bg_text_active_color
    {35, 42, 44},           // bg_text_blocked_color
    {40, 90, 100, 80},      // bg_text_transparent_color

    {30, 180, 200},         // outline_color
    {50, 210, 225},         // outline_hover_color
    {80, 230, 240},         // outline_active_color
    {40, 55, 58},           // outline_blocked_color
    {30, 180, 200, 120},    // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_OBSIDIAN_EMBER =
{
    {22, 20, 18},           // fill_color
    {34, 28, 22},           // fill_hover_color
    {80, 42, 14},           // fill_active_color
    {20, 18, 16},           // fill_blocked_color
    {22, 20, 18, 80},       // fill_transparent_color

    {240, 220, 200},        // text_default_color
    {252, 235, 215},        // text_hover_color
    {255, 242, 225},        // text_active_color
    {80, 72, 62},           // text_blocked_color
    {180, 150, 120, 120},   // text_transparent_color

    {100, 60, 30},          // bg_text_default_color
    {120, 75, 35},          // bg_text_hover_color
    {150, 90, 40},          // bg_text_active_color
    {50, 42, 32},           // bg_text_blocked_color
    {100, 60, 30, 80},      // bg_text_transparent_color

    {200, 100, 40},         // outline_color
    {230, 120, 50},         // outline_hover_color
    {250, 140, 60},         // outline_active_color
    {55, 45, 35},           // outline_blocked_color
    {200, 100, 40, 120},    // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_MIDNIGHT_ROSE =
{
    {22, 18, 26},           // fill_color
    {36, 26, 40},           // fill_hover_color
    {80, 24, 50},           // fill_active_color
    {20, 16, 22},           // fill_blocked_color
    {22, 18, 26,  80},      // fill_transparent_color

    {240, 200, 215},        // text_default_color
    {252, 215, 230},        // text_hover_color
    {255, 230, 242},        // text_active_color
    {80, 65, 75},           // text_blocked_color
    {170, 130, 150, 120},   // text_transparent_color

    {100, 50, 75},          // bg_text_default_color
    {120, 60, 90},          // bg_text_hover_color
    {150, 50, 100},         // bg_text_active_color
    {55, 38, 50},           // bg_text_blocked_color
    {100, 50, 75, 80},      // bg_text_transparent_color

    {180, 60, 100},         // outline_color
    {210, 80, 120},         // outline_hover_color
    {240, 100, 140},        // outline_active_color
    {55, 40, 52},           // outline_blocked_color
    {180, 60, 100, 120},    // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_EMERALD_FOREST =
{
    {24, 32, 26},
    {47, 54, 49},
    {36, 97, 64},
    {18, 24, 20},
    {24, 32, 26, 80},

    {206, 218, 211},
    {166, 201, 182},
    {228, 235, 231},
    {103, 118, 108},
    {206, 218, 211, 120},

    {136, 140, 137},
    {121, 135, 127},
    {161, 163, 162},
    {82, 94, 86},
    {136, 140, 137, 120},

    {38, 109, 71},
    {71, 131, 99},
    {103, 153, 126},
    {17, 49, 32},
    {38, 109, 71, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr EntryStyle ENTRY_GOLDEN_AMBER =
{
    {34, 28, 18},
    {56, 51, 42},
    {128, 95, 30},
    {26, 21, 14},
    {34, 28, 18, 80},

    {223, 217, 205},
    {218, 200, 164},
    {237, 234, 228},
    {123, 115, 100},
    {223, 217, 205, 120},

    {142, 140, 136},
    {145, 138, 124},
    {165, 164, 162},
    {98, 92, 80},
    {142, 140, 136, 120},

    {145, 107, 32},
    {162, 129, 65},
    {178, 151, 99},
    {65, 48, 14},
    {145, 107, 32, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr EntryStyle ENTRY_CRIMSON_ASH =
{
    {32, 20, 20},
    {54, 44, 44},
    {122, 39, 41},
    {24, 15, 15},
    {32, 20, 20, 80},

    {222, 207, 207},
    {215, 169, 170},
    {237, 229, 229},
    {121, 102, 102},
    {222, 207, 207, 120},

    {141, 136, 136},
    {141, 123, 124},
    {164, 161, 161},
    {97, 82, 82},
    {141, 136, 136, 120},

    {138, 43, 45},
    {156, 75, 76},
    {173, 107, 108},
    {62, 19, 20},
    {138, 43, 45, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr EntryStyle ENTRY_STEEL_GRAPHITE =
{
    {24, 26, 30},
    {47, 49, 52},
    {69, 80, 96},
    {18, 20, 22},
    {24, 26, 30, 80},

    {212, 215, 217},
    {185, 192, 200},
    {231, 233, 234},
    {122, 124, 127},
    {212, 215, 217, 120},

    {139, 140, 140},
    {132, 134, 138},
    {162, 163, 164},
    {98, 99, 102},
    {139, 140, 140, 120},

    {77, 90, 108},
    {104, 115, 130},
    {130, 140, 152},
    {35, 40, 49},
    {77, 90, 108, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr EntryStyle ENTRY_LAVENDER_DUSK =
{
    {28, 24, 34},
    {51, 47, 56},
    {98, 80, 123},
    {21, 18, 26},
    {28, 24, 34, 80},

    {218, 214, 222},
    {202, 192, 215},
    {235, 232, 237},
    {131, 127, 137},
    {218, 214, 222, 120},

    {142, 140, 143},
    {141, 137, 146},
    {164, 163, 165},
    {105, 102, 110},
    {142, 140, 143, 120},

    {111, 90, 139},
    {133, 115, 156},
    {154, 140, 174},
    {50, 40, 63},
    {111, 90, 139, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr EntryStyle ENTRY_SKY_AZURE =
{
    {18, 24, 34},
    {42, 47, 56},
    {41, 85, 139},
    {14, 18, 26},
    {18, 24, 34, 80},

    {207, 215, 225},
    {170, 195, 225},
    {229, 233, 238},
    {117, 127, 139},
    {207, 215, 225, 120},

    {138, 140, 143},
    {127, 137, 149},
    {162, 163, 165},
    {94, 102, 111},
    {138, 140, 143, 120},

    {45, 96, 158},
    {76, 120, 173},
    {108, 144, 187},
    {20, 43, 71},
    {45, 96, 158, 120},

    2.0f,
    2.0f,
    2.0f,
    1.0f,
    2.0f
};

constexpr EntryStyle ENTRY_ARCTIC_FROST =
{
    {246, 248, 250},
    {229, 231, 232},
    {209, 225, 241},
    {231, 233, 235},
    {246, 248, 250, 15},

    {30, 30, 35},
    {30, 30, 35},
    {30, 30, 35},
    {168, 168, 172},
    {30, 30, 35, 30},

    {150, 150, 155},
    {150, 150, 155},
    {150, 150, 155},
    {190, 190, 192},
    {150, 150, 155, 30},

    {212, 213, 215},
    {197, 198, 200},
    {144, 186, 225},
    {212, 213, 215},
    {212, 213, 215, 70},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr EntryStyle ENTRY_SANDSTONE_DUNE =
{
    {250, 244, 235},
    {232, 227, 219},
    {240, 217, 202},
    {235, 229, 221},
    {250, 244, 235, 15},

    {30, 30, 35},
    {30, 30, 35},
    {30, 30, 35},
    {168, 168, 172},
    {30, 30, 35, 30},

    {150, 150, 155},
    {150, 150, 155},
    {150, 150, 155},
    {190, 190, 192},
    {150, 150, 155, 30},

    {215, 210, 202},
    {200, 195, 188},
    {222, 170, 144},
    {215, 210, 202},
    {215, 210, 202, 70},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};
}

namespace label_style
{
constexpr LabelStyle LABEL_LIGHT_STYLE =
{
    {0, 0, 0, 0},           // fill_color
    {195, 195, 210, 20},    // fill_hover_color
    {178, 208, 255, 30},    // fill_active_color
    {0, 0, 0, 0},           // fill_blocked_color
    {0, 0, 0, 0},           // fill_transparent_color

    {25, 25, 30, 255},      // text_color
    {25, 25, 30, 255},      // text_hover_color
    {25, 25, 30, 255},      // text_active_color
    {160, 160, 168, 255},   // text_blocked_color
    {25, 25, 30, 25},       // text_transparent_color

    {75, 75, 88, 200},      // text_outline_color
    {75, 75, 88, 200},      // text_outline_hover_color
    {75, 75, 88, 200},      // text_outline_active_color
    {138, 138, 148, 130},   // text_outline_blocked_color
    {75, 75, 88, 0},        // text_outline_transparent_color

    0.f,                    // text_outline_thickness
    0.f,                    // text_outline_hover_thickness
    0.f,                    // text_outline_active_thickness
    0.f,                    // text_outline_blocked_thickness
    0.f                     // text_outline_transparent_thickness
};

constexpr LabelStyle LABEL_DARK_STYLE =
{
    {0, 0, 0, 0},           // fill_color
    {44, 44, 54, 20},       // fill_hover_color
    {22, 52, 115, 30},      // fill_active_color
    {0, 0, 0, 0},           // fill_blocked_color
    {0, 0, 0, 0},           // fill_transparent_color

    {218, 215, 208, 255},   // text_color
    {232, 229, 223, 255},   // text_hover_color
    {218, 215, 208, 255},   // text_active_color
    {86, 86, 96, 255},      // text_blocked_color
    {200, 197, 192, 25},    // text_transparent_color

    {8, 6, 4, 210},         // text_outline_color
    {8, 6, 4, 210},         // text_outline_hover_color
    {8, 6, 4, 210},         // text_outline_active_color
    {20, 20, 26, 130},      // text_outline_blocked_color
    {8, 6, 4, 0},           // text_outline_transparent_color

    0.f,                    // text_outline_thickness
    0.f,                    // text_outline_hover_thickness
    0.f,                    // text_outline_active_thickness
    0.f,                    // text_outline_blocked_thickness
    0.f                     // text_outline_transparent_thickness
};

constexpr LabelStyle LABEL_SLATE_VIOLET =
{
    {0, 0, 0, 0},           // fill_color
    {45, 40, 72, 22},       // fill_hover_color
    {76, 63, 160, 38},      // fill_active_color
    {0, 0, 0, 0},           // fill_blocked_color
    {0, 0, 0, 0},           // fill_transparent_color

    {210, 205, 240, 255},   // text_color
    {225, 220, 255, 255},   // text_hover_color
    {255, 252, 255, 255},   // text_active_color
    { 90,  88, 110, 255},   // text_blocked_color
    {180, 175, 210, 38},    // text_transparent_color

    {18, 12, 38, 220},      // text_outline_color
    {18, 12, 38, 220},      // text_outline_hover_color
    {18, 12, 38, 220},      // text_outline_active_color
    {22, 20, 35, 140},      // text_outline_blocked_color
    {18, 12, 38, 0},        // text_outline_transparent_color

    0.f,                    // text_outline_thickness
    0.f,                    // text_outline_hover_thickness
    0.f,                    // text_outline_active_thickness
    0.f,                    // text_outline_blocked_thickness
    0.f                     // text_outline_transparent_thickness
};

constexpr LabelStyle LABEL_CARBON_CYAN =
{
    {0, 0, 0, 0},           // fill_color
    {28, 32, 38, 22},       // fill_hover_color
    {12, 60, 72, 38},       // fill_active_color
    {0, 0, 0, 0},           // fill_blocked_color
    {0, 0, 0, 0},           // fill_transparent_color

    {200, 240, 245, 255},   // text_color
    {220, 248, 252, 255},   // text_hover_color
    {240, 252, 255, 255},   // text_active_color
    {70, 80, 82, 255},      // text_blocked_color
    {150, 200, 210, 38},    // text_transparent_color

    { 8, 28, 32, 220},      // text_outline_color
    { 8, 28, 32, 220},      // text_outline_hover_color
    { 8, 28, 32, 220},      // text_outline_active_color
    {18, 24, 26, 140},      // text_outline_blocked_color
    { 8, 28, 32, 0},        // text_outline_transparent_color

    0.f,                    // text_outline_thickness
    0.f,                    // text_outline_hover_thickness
    0.f,                    // text_outline_active_thickness
    0.f,                    // text_outline_blocked_thickness
    0.f                     // text_outline_transparent_thickness
};

constexpr LabelStyle LABEL_OBSIDIAN_EMBER =
{
    {0, 0, 0, 0},           // fill_color
    {34, 28, 22, 22},       // fill_hover_color
    {80, 42, 14, 38},       // fill_active_color
    {0, 0, 0, 0},           // fill_blocked_color
    {0, 0, 0, 0},           // fill_transparent_color

    {240, 220, 200, 255},   // text_color
    {252, 235, 215, 255},   // text_hover_color
    {255, 242, 225, 255},   // text_active_color
    {80, 72, 62, 255},      // text_blocked_color
    {180, 150, 120, 38},    // text_transparent_color

    {28, 18, 8, 220},       // text_outline_color
    {28, 18, 8, 220},       // text_outline_hover_color
    {28, 18, 8, 220},       // text_outline_active_color
    {32, 24, 18, 140},      // text_outline_blocked_color
    {28, 18, 8, 0},         // text_outline_transparent_color

    0.f,                    // text_outline_thickness
    0.f,                    // text_outline_hover_thickness
    0.f,                    // text_outline_active_thickness
    0.f,                    // text_outline_blocked_thickness
    0.f                     // text_outline_transparent_thickness
};

constexpr LabelStyle LABEL_MIDNIGHT_ROSE =
{
    {0, 0, 0, 0},           // fill_color
    {36, 26, 40, 22},       // fill_hover_color
    {80, 24, 50, 38},       // fill_active_color
    {0, 0, 0, 0},           // fill_blocked_color
    {0, 0, 0, 0},           // fill_transparent_color

    {240, 200, 215, 255},   // text_color
    {252, 215, 230, 255},   // text_hover_color
    {255, 230, 242, 255},   // text_active_color
    {80, 65, 75, 255},      // text_blocked_color
    {170, 130, 150, 38},    // text_transparent_color

    {30, 10, 18, 220},      // text_outline_color
    {30, 10, 18, 220},      // text_outline_hover_color
    {30, 10, 18, 220},      // text_outline_active_color
    {32, 20, 26, 140},      // text_outline_blocked_color
    {30, 10, 18, 0},        // text_outline_transparent_color

    0.f,                    // text_outline_thickness
    0.f,                    // text_outline_hover_thickness
    0.f,                    // text_outline_active_thickness
    0.f,                    // text_outline_blocked_thickness
    0.f                     // text_outline_transparent_thickness
};

constexpr LabelStyle LABEL_EMERALD_FOREST =
{
    {0, 0, 0, 0},
    {36, 97, 64, 22},
    {36, 97, 64, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {206, 218, 211},
    {166, 201, 182},
    {228, 235, 231},
    {103, 118, 108},
    {206, 218, 211, 25},

    {38, 109, 71, 210},
    {38, 109, 71, 210},
    {38, 109, 71, 210},
    {27, 76, 50, 130},
    {38, 109, 71, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_GOLDEN_AMBER =
{
    {0, 0, 0, 0},
    {128, 95, 30, 22},
    {128, 95, 30, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {223, 217, 205},
    {218, 200, 164},
    {237, 234, 228},
    {123, 115, 100},
    {223, 217, 205, 25},

    {145, 107, 32, 210},
    {145, 107, 32, 210},
    {145, 107, 32, 210},
    {102, 75, 22, 130},
    {145, 107, 32, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_CRIMSON_ASH =
{
    {0, 0, 0, 0},
    {122, 39, 41, 22},
    {122, 39, 41, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {222, 207, 207},
    {215, 169, 170},
    {237, 229, 229},
    {121, 102, 102},
    {222, 207, 207, 25},

    {138, 43, 45, 210},
    {138, 43, 45, 210},
    {138, 43, 45, 210},
    {97, 30, 31, 130},
    {138, 43, 45, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_STEEL_GRAPHITE =
{
    {0, 0, 0, 0},
    {69, 80, 96, 22},
    {69, 80, 96, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {212, 215, 217},
    {185, 192, 200},
    {231, 233, 234},
    {122, 124, 127},
    {212, 215, 217, 25},

    {77, 90, 108, 210},
    {77, 90, 108, 210},
    {77, 90, 108, 210},
    {54, 63, 76, 130},
    {77, 90, 108, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_LAVENDER_DUSK =
{
    {0, 0, 0, 0},
    {98, 80, 123, 22},
    {98, 80, 123, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {218, 214, 222},
    {202, 192, 215},
    {235, 232, 237},
    {131, 127, 137},
    {218, 214, 222, 25},

    {111, 90, 139, 210},
    {111, 90, 139, 210},
    {111, 90, 139, 210},
    {78, 63, 97, 130},
    {111, 90, 139, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_SKY_AZURE =
{
    {0, 0, 0, 0},
    {41, 85, 139, 22},
    {41, 85, 139, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {207, 215, 225},
    {170, 195, 225},
    {229, 233, 238},
    {117, 127, 139},
    {207, 215, 225, 25},

    {45, 96, 158, 210},
    {45, 96, 158, 210},
    {45, 96, 158, 210},
    {31, 67, 111, 130},
    {45, 96, 158, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_ARCTIC_FROST =
{
    {0, 0, 0, 0},
    {209, 225, 241, 22},
    {209, 225, 241, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {30, 30, 35},
    {30, 30, 35},
    {30, 30, 35},
    {168, 168, 172},
    {30, 30, 35, 25},

    {212, 213, 215, 210},
    {212, 213, 215, 210},
    {212, 213, 215, 210},
    {148, 149, 150, 130},
    {212, 213, 215, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};

constexpr LabelStyle LABEL_SANDSTONE_DUNE =
{
    {0, 0, 0, 0},
    {240, 217, 202, 22},
    {240, 217, 202, 38},
    {0, 0, 0, 0},
    {0, 0, 0, 0},

    {30, 30, 35},
    {30, 30, 35},
    {30, 30, 35},
    {168, 168, 172},
    {30, 30, 35, 25},

    {215, 210, 202, 210},
    {215, 210, 202, 210},
    {215, 210, 202, 210},
    {150, 147, 141, 130},
    {215, 210, 202, 0},

    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f
};
};

namespace tooltip_style
{
constexpr ToolTipStyle TOOLTIP_DEFAULT_DARK = 
{
    {28, 28, 32},           // fill_color
    {205, 205, 212},        // outline_color
    1.0f,                   // outline_thickness

    {218, 215, 208},        // text_color
    {42, 42, 48},           // text_outline_color
    0.0f                    // text_outline_thickness
};

constexpr ToolTipStyle TOOLTIP_DEFAULT_LIGHT = 
{
    {245, 245, 247},        // fill_color
    {58, 58, 68},           // outline_color
    1.0f,                   // outline_thickness

    {30, 30, 35},           // text_color
    {218, 215, 208},        // text_outline_color
    0.0f                    // text_outline_thickness
};

constexpr ToolTipStyle TOOLTIP_EMERALD_FOREST =
{
    {22, 29, 23},
    {38, 109, 71},
    1.5f,
    {206, 218, 211},
    {7, 10, 8},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_GOLDEN_AMBER =
{
    {31, 25, 16},
    {145, 107, 32},
    1.5f,
    {223, 217, 205},
    {10, 8, 5},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_CRIMSON_ASH =
{
    {29, 18, 18},
    {138, 43, 45},
    1.5f,
    {222, 207, 207},
    {10, 6, 6},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_STEEL_GRAPHITE =
{
    {22, 23, 27},
    {77, 90, 108},
    1.5f,
    {212, 215, 217},
    {7, 8, 9},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_LAVENDER_DUSK =
{
    {25, 22, 31},
    {111, 90, 139},
    1.5f,
    {218, 214, 222},
    {8, 7, 10},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_SKY_AZURE =
{
    {16, 22, 31},
    {45, 96, 158},
    1.5f,
    {207, 215, 225},
    {5, 7, 10},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_ARCTIC_FROST =
{
    {246, 248, 250},
    {212, 213, 215},
    1.5f,
    {30, 30, 35},
    {249, 250, 252},
    0.0f
};

constexpr ToolTipStyle TOOLTIP_SANDSTONE_DUNE =
{
    {250, 244, 235},
    {215, 210, 202},
    1.5f,
    {30, 30, 35},
    {252, 247, 241},
    0.0f
};
}

constexpr CellColor DEFAULT_CELL_COLOR =
{
    sf::Color::White,
    sf::Color::Black,
    sf::Color::Black,
    sf::Color::Black,
    sf::Color::Black,
    1.f, 1.f, 1.f, 1.f
};

constexpr sfml_widgets::CellColor LIGHT_CELL = {
    sf::Color::Transparent,
    sf::Color(205, 205, 212),
    sf::Color(205, 205, 212),
    sf::Color(205, 205, 212),
    sf::Color(205, 205, 212),
    0.f, 0.f, 1.f, 1.f
};

constexpr sfml_widgets::CellColor DARK_CELL = {
    sf::Color::Transparent,
    sf::Color(58,  58,  68),
    sf::Color(58,  58,  68),
    sf::Color(58,  58,  68),
    sf::Color(58,  58,  68),
    1.f, 1.f, 0.f, 0.f
};

inline static std::shared_ptr<WidgetTheme> dark_theme = WidgetTheme::make_theme(button_style::BUTTON_DARK_STYLE, texture_style::DEFAULT_DARK_STYLE, checkbox_style::CHECKBOX_DARK_STYLE, entry_style::ENTRY_DARK_STYLE, label_style::LABEL_DARK_STYLE);
inline static std::shared_ptr<WidgetTheme> light_theme = WidgetTheme::make_theme(button_style::BUTTON_LIGHT_STYLE, texture_style::DEFAULT_LIGHT_STYLE, checkbox_style::CHECKBOX_LIGHT_STYLE, entry_style::ENTRY_LIGHT_STYLE, label_style::LABEL_LIGHT_STYLE);
inline static std::shared_ptr<WidgetTheme> stale_violet_theme = WidgetTheme::make_theme(button_style::BUTTON_STALE_VIOLET, texture_style::STYLE_STALE_VIOLET, checkbox_style::CHECKBOX_STALE_VIOLET, entry_style::ENTRY_STALE_VIOLET, label_style::LABEL_SLATE_VIOLET);
inline static std::shared_ptr<WidgetTheme> carbon_cyan_theme = WidgetTheme::make_theme(button_style::BUTTON_CARBON_CYAN, texture_style::STYLE_CARBON_CYAN, checkbox_style::CHECKBOX_CARBON_CYAN, entry_style::ENTRY_CARBON_CYAN, label_style::LABEL_CARBON_CYAN);
inline static std::shared_ptr<WidgetTheme> midnight_rose_theme = WidgetTheme::make_theme(button_style::BUTTON_MIDNIGHT_ROSE, texture_style::STYLE_MIDNIGHT_ROSE, checkbox_style::CHECKBOX_MIDNIGHT_ROSE, entry_style::ENTRY_MIDNIGHT_ROSE, label_style::LABEL_MIDNIGHT_ROSE);
inline static std::shared_ptr<WidgetTheme> obsidian_ember_theme = WidgetTheme::make_theme(button_style::BUTTON_OBSIDIAN_EMBER, texture_style::STYLE_OBSIDIAN_EMBER, checkbox_style::CHECKBOX_OBSIDIAN_EMBER, entry_style::ENTRY_OBSIDIAN_EMBER, label_style::LABEL_OBSIDIAN_EMBER);
inline static std::shared_ptr<WidgetTheme> emerald_forest_theme = WidgetTheme::make_theme(button_style::BUTTON_EMERALD_FOREST, texture_style::TEXTURE_EMERALD_FOREST, checkbox_style::CHECKBOX_EMERALD_FOREST, entry_style::ENTRY_EMERALD_FOREST, label_style::LABEL_EMERALD_FOREST);
inline static std::shared_ptr<WidgetTheme> golden_amber_theme = WidgetTheme::make_theme(button_style::BUTTON_GOLDEN_AMBER, texture_style::TEXTURE_GOLDEN_AMBER, checkbox_style::CHECKBOX_GOLDEN_AMBER, entry_style::ENTRY_GOLDEN_AMBER, label_style::LABEL_GOLDEN_AMBER);
inline static std::shared_ptr<WidgetTheme> crimson_ash_theme = WidgetTheme::make_theme(button_style::BUTTON_CRIMSON_ASH, texture_style::TEXTURE_CRIMSON_ASH, checkbox_style::CHECKBOX_CRIMSON_ASH, entry_style::ENTRY_CRIMSON_ASH, label_style::LABEL_CRIMSON_ASH);
inline static std::shared_ptr<WidgetTheme> steel_graphite_theme = WidgetTheme::make_theme(button_style::BUTTON_STEEL_GRAPHITE, texture_style::TEXTURE_STEEL_GRAPHITE, checkbox_style::CHECKBOX_STEEL_GRAPHITE, entry_style::ENTRY_STEEL_GRAPHITE, label_style::LABEL_STEEL_GRAPHITE);
inline static std::shared_ptr<WidgetTheme> lavender_dusk_theme = WidgetTheme::make_theme(button_style::BUTTON_LAVENDER_DUSK, texture_style::TEXTURE_LAVENDER_DUSK, checkbox_style::CHECKBOX_LAVENDER_DUSK, entry_style::ENTRY_LAVENDER_DUSK, label_style::LABEL_LAVENDER_DUSK);
inline static std::shared_ptr<WidgetTheme> sky_azure_theme = WidgetTheme::make_theme(button_style::BUTTON_SKY_AZURE, texture_style::TEXTURE_SKY_AZURE, checkbox_style::CHECKBOX_SKY_AZURE, entry_style::ENTRY_SKY_AZURE, label_style::LABEL_SKY_AZURE);
inline static std::shared_ptr<WidgetTheme> arctic_frost_theme = WidgetTheme::make_theme(button_style::BUTTON_ARCTIC_FROST, texture_style::TEXTURE_ARCTIC_FROST, checkbox_style::CHECKBOX_ARCTIC_FROST, entry_style::ENTRY_ARCTIC_FROST, label_style::LABEL_ARCTIC_FROST);
inline static std::shared_ptr<WidgetTheme> sandstone_dune_theme = WidgetTheme::make_theme(button_style::BUTTON_SANDSTONE_DUNE, texture_style::TEXTURE_SANDSTONE_DUNE, checkbox_style::CHECKBOX_SANDSTONE_DUNE, entry_style::ENTRY_SANDSTONE_DUNE, label_style::LABEL_SANDSTONE_DUNE);
}
}
#endif