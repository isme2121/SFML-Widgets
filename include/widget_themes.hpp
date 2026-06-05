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
/// @see `__WidgetStyle`, `Widget::linkTheme`, `Widget::unlinkTheme`
///
///////////////////////////////////////////////////////////////////
class WidgetTheme
{
private:
    class P
    {
        P() {}
        friend class WidgetTheme;
    };

    template <typename> friend struct __WidgetStyle; //!< this gets the correct type based on template

    WidgetTheme(P);

    WidgetTheme(P, ButtonStyle _b, TextureStyle _f, CheckboxStyle _c, EntryStyle _e, LabelStyle _l);
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
    /// @param _b style for button
    /// @param _f style for frame, canvas, and image frame (can be differentiated using `setStyle` method)
    /// @param _c style for checkbox
    /// @param _e style for entry
    /// @param _l style for label
    ///
    /// @return shared pointer with the new `WidgetTheme` resource
    ///
    ///////////////////////////////////////////////////////////////////
    static std::shared_ptr<WidgetTheme> make_theme(ButtonStyle _b, TextureStyle _f, CheckboxStyle _c, EntryStyle _e, LabelStyle _l);

    ///////////////////////////////////////////////////////////////////
    /// @brief creates a `WidgetTheme` resource with copy constructor
    ///
    /// The returned object can be assigned to any widget via method
    /// `Widget::linkTheme`, each assigned widget remains synced in style
    /// until the shared counter goes to zero.
    ///
    /// @param t reference to existing `WidgetTheme` object
    ///
    /// @return shared pointer with the new `WidgetTheme` resource
    ///
    ///////////////////////////////////////////////////////////////////
    static std::shared_ptr<WidgetTheme> make_theme(const WidgetTheme& t);

    ///////////////////////////////////////////////////////////////////
    /// @brief copies the content of parameter t to this object
    ///
    /// @param t reference to existing `WidgetTheme` object
    ///
    ///////////////////////////////////////////////////////////////////
    void copy_theme(const WidgetTheme& t);

    ///////////////////////////////////////////////////////////////////
    /// @brief returns current assigned style for template widget type
    ///
    /// @tparam WIDGET_DERIVED_TYPE derived widget type
    /// 
    /// @return matching style type for template type
    /// 
    ///////////////////////////////////////////////////////////////////
    template <typename WIDGET_DERIVED_TYPE>
    typename __WidgetStyle<WIDGET_DERIVED_TYPE>::StyleType getStyle()
    {
        return __WidgetStyle<WIDGET_DERIVED_TYPE>::get(*this);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief assigns parameter style to the template widget type
    ///
    /// @tparam WIDGET_DERIVED_TYPE derived widget type
    /// 
    /// @param _t style object optimal for template type
    ///
    ///////////////////////////////////////////////////////////////////
    template <typename WIDGET_DERIVED_TYPE>
    void setStyle(typename __WidgetStyle<WIDGET_DERIVED_TYPE>::StyleType _t)
    {
        __WidgetStyle<WIDGET_DERIVED_TYPE>::set(*this, _t);
        ++__WidgetStyle<WIDGET_DERIVED_TYPE>::token(*this);
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
        return __WidgetStyle<WIDGET_DERIVED_TYPE>::token(*this);
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
};

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
}
}
#endif