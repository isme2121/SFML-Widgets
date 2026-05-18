#pragma once
#ifndef WIDGETS_HPP
#define WIDGETS_HPP
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <deque>
#include <array>
#include <cmath>

namespace sfml_widgets
{
using namespace std::chrono_literals;

class Widget;
class Button;
class Frame;
class GridCanvas;
class ImageFrame;
class CheckBox;
class Entry;

struct Key;
class WidgetGroup;
class CaptureSequence;
class ImageCache;
class RenderContext;
class CellOutline;
class Cell;

///////////////////////////////////////////////////////////////////
///////////////////         Types               ///////////////////
///////////////////////////////////////////////////////////////////

using Sequence = std::vector<Key>;                                          //!< Represents a sequence of key presses
using SequenceFunction = std::function<void()>;                             //!< Required function structure for key press events
using MouseCallback = std::function<void(std::optional<const sf::Event>)>;  //!< Required function structure for mouse press events
using float_to_milli = std::chrono::duration<float, std::milli>;            //!< casting float to ms

///////////////////////////////////////////////////////////////////
///////////////////         Enums               ///////////////////
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// @brief Bit wise class for updating widget state & control
/// management class
///
///////////////////////////////////////////////////////////////////
enum class FLAGS : std::int32_t
{
    IDLE = 0,                       //!< Default state
    INIT = 1,                       //!< Initalized state
    MODIFIED = 1 << 2,              //!< State: `set automatically after an update`

    RESTRICTED = 1 << 3,            //!< State: `certain functions are limited temporarily`
    HOVERED = 1 << 4,               //!< State: `mouse hovered within widget bounds`
    SELECTED = 1 << 5,              //!< State: `widget is not yet active but can trigger callbacks and set active`
    FOCUSED = 1 << 6,               //!< State: `widget is clicked, only one widget can be focused at a time`
    ACTIVE = 1 << 7,                //!< State: `active widget state as ACTIVE_BUTTON is pressed`
    SET = 1 << 8,                   //!< State: `1 state, used in checkbox and entry`
    RESET = 1 << 9,                 //!< State: `0 state, used in entry`
    TRANSPARENT = 1 << 10,          //!< State: `transparent state, set by AUTOHIDE when not hovering`
    DRAWING = 1 << 11,              //!< State: `drawing state for containers`
    SHORTSKIP = 1 << 12,            //!< State: `skips a process within one function`
    LONGSKIP = 1 << 13,             //!< State: `skips a process until a condition is satisfied, used by Widget`

    INDICATOR = 1 << 14,            //!< Configured flag: `unique widget indicator`
    HIDDEN = 1 << 15,               //!< Configured flag: `invisible widget, does not trigger mouse events`
    BLOCKED = 1 << 16,              //!< Configured flag: `widget becomes non-interactive`
    AUTOSELECT = 1 << 17,           //!< Configured flag: `widget is always selected for keyboard events`
    AUTOHIDE = 1 << 18,             //!< Configured flag: `widget is transparent when not hovered`
    
    CLIPSHADER = 1 << 19,           //!< Flag: `shader for sprites is loaded`
    COLORSHADER = 1 << 20,          //!< Flag: `shader for fill texture is loaded`

    RESIZE = 1 << 21,
    TEXT = 1 << 22,                 //!< Flag: `widget is currently displaying text`
    IMAGE = 1 << 23,                //!< Flag: `widget is currently displaying an image`
    BGIMAGE = 1 << 24,              //!< Flag: `widget has fill texture loaded`
    DEFAULTIMAGE = 1 << 25,         //!< Flag: `default image currently used`
    
    GRID = 1 << 26,                 //!< Flag: `grid is enabled for container`
    SCROLL = 1 << 27,               //!< Flag: `scrolling is enabled for scrollable widgets like frame`
    SEPARATORS = 1 << 28,           //!< Flag: `line separators in frame is enabled`
    ZOOM = 1 << 29,                 //!< Flag: `zooming is enabled for canvas`
    PAN = 1 << 30,                  //!< Flag: `panning is enabled for canvas`
    BLOCKBGIMAGE = 1 << 31          //!< Flag: `blocks loading fill texture, clears it with shader if enabled`

};

enum class SeparatorType
{
    NoSeparator,
    Horizontal,
    Vertical
};

enum class DIRECTION
{
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};


///////////////////////////////////////////////////////////////////
///////////////////       Functions             ///////////////////
///////////////////////////////////////////////////////////////////

FLAGS operator &(FLAGS, FLAGS);     //!< Does not set MODIFIED
FLAGS& operator &=(FLAGS&, FLAGS);  //!< Sets MODIFIED
FLAGS operator |(FLAGS, FLAGS);     //!< Does not set MODIFIED
FLAGS& operator |=(FLAGS&, FLAGS);  //!< Sets MODIFIED
FLAGS operator ^(FLAGS, FLAGS);     //!< Does not set MODIFIED
FLAGS& operator ^=(FLAGS&, FLAGS);  //!< Sets MODIFIED
FLAGS& setFlag(FLAGS&, FLAGS);      //!< Sets MODIFIED
FLAGS& resetFlag(FLAGS&, FLAGS);    //!< Sets MODIFIED IF not included in bits
FLAGS& toggle(FLAGS&, FLAGS);       //!< Does not set MODIFIED
bool checkFlag(const FLAGS&, FLAGS);//!< Quick boolean check

///////////////////////////////////////////////////////////////////
/// @brief Updates image size (does not preserve quality)
///
///////////////////////////////////////////////////////////////////
std::optional<sf::Image> resize_image(const sf::Image source, sf::Vector2u output_size);

///////////////////////////////////////////////////////////////////
/// @brief Returns the closest combination of integers to each other
/// that result in `n` when multiplied
///
/// examples:
///         n = 10 ==> { 5, 2},
///         n = 49 ==> { 7, 7},
///         n = 13 ==> {13, 1},
///////////////////////////////////////////////////////////////////
sf::Vector2u get_prime_combination(unsigned int n);

///////////////////////////////////////////////////////////////////
///////////////////         Classes             ///////////////////
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// @brief Represents a `sf::Event::KeyPressed` object, made for
/// implicit constructing
///
///////////////////////////////////////////////////////////////////
struct Key
{
    Key(sf::Keyboard::Key key);
    Key(sf::Keyboard::Key key, bool control);
    Key(sf::Keyboard::Key key, bool control, bool alt, bool shift, bool system);
    Key(sf::Keyboard::Scancode scan_key);
    Key(sf::Keyboard::Scancode scan_key, bool control);
    Key(sf::Keyboard::Scancode scan_key, bool control, bool alt, bool shift, bool system);
    Key(sf::Keyboard::Key key, sf::Keyboard::Scancode scan_key);
    Key(sf::Keyboard::Key key, sf::Keyboard::Scancode scan_key, bool control);
    Key(sf::Keyboard::Key key, sf::Keyboard::Scancode scan_key, bool control, bool alt, bool shift, bool system);
    Key(const sf::Event::KeyPressed& event);
    Key(const Key& other);
    Key(Key&& other) noexcept;
    
    Key& operator=(const Key& other);
    Key& operator=(Key&& other) noexcept;

    bool operator==(const Key& other);
    bool operator!=(const Key& other);

private:
        sf::Keyboard::Key code{sf::Keyboard::Key::Unknown};
        sf::Keyboard::Scancode scancode{sf::Keyboard::Scancode::Unknown};
        bool control{};
        bool alt{};
        bool shift{};
        bool system{};
};

///////////////////////////////////////////////////////////////////
/// @brief Static class for storing images used within widgets by paths
///
/// @see `ImageReferenceCounter`
///
///////////////////////////////////////////////////////////////////
class ImageCache
{
public:
    static ImageCache& getInstance()
    {
        static ImageCache image_buffer_instance;
        return image_buffer_instance;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Fetch image pointer from memory or loads from disk if not
    /// found
    ///
    /// @param image_path relative path to image as a look up key
    /// @param increment_ref specify whether to increment reference counter of image,
    /// set to false if widget already reference's the image.
    ///
    /// @return read only image pointer if image is found on disk or memory,
    /// otherwise nullopt
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<const sf::Image*> getImage(const std::string& image_path, bool increment_ref = true);

    ///////////////////////////////////////////////////////////////////
    /// @brief Called by widgets after clearing their texture image
    ///
    /// Decrements reference counter for a pre-loaded image, and frees
    /// the image memory if no widget is using it.
    ///
    ///////////////////////////////////////////////////////////////////
    void decrementRefCounter(const std::string& image_path);

private:

    struct ImageReferenceCounter
    {
        sf::Image image;
        std::size_t ref_count;
    };

    std::unordered_map<std::string, ImageReferenceCounter> m_cachedImages;
};



///////////////////////////////////////////////////////////////////
/// @brief Master object for `Widget` objects
///
/// A master handles drawing the widget, can be a `RenderTexture` or
/// a `Frame` object.
///
///////////////////////////////////////////////////////////////////
class RenderContext
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the master using a RW object
    /// 
    /// @param window root window that draws the widget
    ///
    ///////////////////////////////////////////////////////////////////
    explicit RenderContext(sf::RenderWindow& window);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the master using a Frame widget for
    /// widgets placed on top of frames
    ///
    /// @param frame master frame object
    ///
    ///////////////////////////////////////////////////////////////////
    explicit RenderContext(Frame& frame);

    RenderContext(RenderContext&& other) noexcept;
    RenderContext& operator=(RenderContext&& other) noexcept;

    RenderContext(const RenderContext& other) = delete;
    RenderContext& operator=(const RenderContext& other) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget
    ///
    /// @param __widget reference to widget
    /// @param __state rendering state object for the widget
    ///
    ///////////////////////////////////////////////////////////////////
    void draw(const sf::Drawable& __widget, const sf::RenderStates __state = sf::RenderStates::Default);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the relative mouse position based on view
    ///
    /// @param mouse_position captured mouse position
    ///
    /// @return mouse coordinates relative to view
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f mapMousePosition(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the pointer to the main window
    ///
    ///////////////////////////////////////////////////////////////////
    void setWindowPtr(sf::RenderWindow* __winScreenPtr);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the pointer to the root frame
    ///
    ///////////////////////////////////////////////////////////////////
    void setFramePtr(Frame* __frPtr);

    ///////////////////////////////////////////////////////////////////
    /// @brief returns a readonly pointer to window object if available
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::RenderWindow* getWindowPtr() const; 

    ///////////////////////////////////////////////////////////////////
    /// @brief returns a pointer to frame if available
    ///
    ///////////////////////////////////////////////////////////////////
    Frame* getFramePtr() const; 

private:
    sf::RenderWindow* m_window;
    Frame* m_frame;
};

///////////////////////////////////////////////////////////////////
/// @brief Group of widgets sharing the focus resource
///
/// Designed to construct within a shared pointer object, each widget
/// referenced by the same group share special rules when it comes to
/// gaining or losing focus on a widget.
///
/// Use `make_group` static method to construct a new instance:
///
/// ```cpp
///     auto w_group1 = sfml_widgets::WidgetGroup::make_group();
///     if(w_group1)
///     {
///         # assigning widgets to share focus if one is clicked
///         button198320.linkWidget(w_group1);
///         entry67.linkWidget(w_group1);
///         # both widgets would fire their focus callback together if focused
///         w_group1.groupFocusCallback = true;
///         # only the widget that lost focus would fire their proper callback
///         w_group1.groupFocuslossCallback = false;
///     }
///     ...
/// ```
///
/// @note `WidgetGroup` cannot be copied or moved and is lifetime safe, meanwhile
/// `Widget` is responsible for move / lifetime operations with it's group
///
///////////////////////////////////////////////////////////////////
class WidgetGroup
{
    WidgetGroup() = default;
public:
    static std::shared_ptr<WidgetGroup> make_group()
    {
        return std::shared_ptr<WidgetGroup>(new WidgetGroup());
    }

    WidgetGroup(const WidgetGroup&) = delete;
    WidgetGroup& operator=(const WidgetGroup&) = delete;
    WidgetGroup(WidgetGroup&&) = delete;
    WidgetGroup& operator=(WidgetGroup&&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if a widget belongs to the group
    ///
    ///////////////////////////////////////////////////////////////////
    bool contains(const Widget* w) const;

    bool groupFocusCallback = false;        //!< Focusing on a group of widgets triggers their focus callbacks all at once
    bool groupFocuslossCallback = true;     //!< Losing focus on a group of widget triggers their unfocus callbacks all at once

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Adds a widget to the group, does nothing if the widget
    /// is already there
    ///
    ///////////////////////////////////////////////////////////////////
    void add(Widget* w);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a widget from the group, does nothing if the widget
    /// does not belong in the first place
    ///
    ///////////////////////////////////////////////////////////////////
    void remove(Widget* w);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the focus callback for all widgets in the group
    ///
    ///////////////////////////////////////////////////////////////////
    void focusOn(std::optional<const sf::Event> event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the unfocus callback for all the widgets in the group
    ///
    ///////////////////////////////////////////////////////////////////
    void focusOff(std::optional<const sf::Event> event);

    std::vector<Widget*> m_cult;    //!< ooga booga

    friend class Widget;
};

///////////////////////////////////////////////////////////////////
///@brief Keyboard event management class used by `Widget`
///
///////////////////////////////////////////////////////////////////
class CaptureSequence
{
///////////////////////////////////////////////////////////////////
/// @brief Stores the triggering sequence and assigned callback for
/// when a keyboard event is triggered
///
/// @see `CaptureSequence`
///
///////////////////////////////////////////////////////////////////
struct SequenceEntry
{
    std::string tag;
    Sequence sequence;
    SequenceFunction callback;
    
    bool operator==(const std::string other_tag);
    bool operator==(const SequenceEntry& other);
};

public:
    CaptureSequence() = default;

    ///////////////////////////////////////////////////////////////////
    /// @brief Adds a sequence of key presses to memory
    ///
    /// Takes a vector of scancode objects to define a sequence of events
    /// with the correct order.
    ///
    /// @param name sequence's reference tag
    /// @param sequence vector of physical key presses
    /// @param callback sequence's function, must not accept any parameters
    ///
    ///////////////////////////////////////////////////////////////////
    void addSequence(const std::string& name, Sequence sequence, SequenceFunction callback);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Remove a sequence from memory
    ///
    /// Remove the first found sequence with entered name, returns if 
    /// name is not found.
    ///
    /// @param name string name used when assigning the sequence
    ///
    ///////////////////////////////////////////////////////////////////
    void deleteSequence(const std::string& name);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks and fires the callback if a sequence matches
    ///
    /// Takes the Key press object from the main window events and stores
    /// it internally as a sequence, if the sequence of events match a saved
    /// sequence then fires a callback for that sequence.
    ///
    /// @param _event Event object
    ///
    /// @return true if a sequence was matched, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////
    bool checkEvents(const sf::Event& _event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears the current sequence
    ///
    ///////////////////////////////////////////////////////////////////
    void reset();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current stored sequence entered by the user
    ///
    /// @return a vector of physical key press objects.
    ///
    ///////////////////////////////////////////////////////////////////
    const Sequence& getCurrentSequence() const;

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Main sequence comparing method
    ///
    /// @return true if a sequence is matched, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////
    inline bool compareSequence();

    ///////////////////////////////////////////////////////////////////
    /// Member variables
    ///////////////////////////////////////////////////////////////////

    std::vector<SequenceEntry> m_sequences;                     //!< Stored sequences
    Sequence m_current_sequence;                                //!< Current sequence
    size_t m_max_sequence = 0;                                  //!< Max number of keys needed to match a saved sequence
};

///////////////////////////////////////////////////////////////////
/// @brief Color theme representing all the states of a widget
///
///////////////////////////////////////////////////////////////////
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

namespace constants
{
constexpr float PI = 3.14159265f;
constexpr float DEGREE90 = 90.f;
constexpr float DEGREE180 = 180.f;
constexpr float DEGREE270 = 270.f;
constexpr float START_ANGLES[4] =
{
    DEGREE180 * PI / DEGREE180,  // TL
    DEGREE270 * PI / DEGREE180,  // TR
    0.f,                         // BR
    DEGREE90  * PI / DEGREE180   // BL
};
}

namespace themes
{
constexpr ButtonStyle BUTTON_STALE_VIOLET =
{
    {30,  27,  46},         // default_color
    {45,  40,  72},         // hover_color
    {76,  63,  160},        // active_color
    {22,  22,  32},         // blocked_color
    {30,  27,  46,  80},    // transparent_color

    {210, 205, 240},        // default_text_color
    {225, 220, 255},        // hover_text_color
    {255, 252, 255},        // active_text_color
    {90,  88,  110},        // blocked_text_color
    {180, 175, 210, 120},   // transparent_text_color

    {90,  80,  180},        // default_outline_color
    {120, 108, 220},        // hover_outline_color
    {150, 138, 240},        // active_outline_color
    {55,  52,  80},         // blocked_outline_color
    {90,  80,  180, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_CARBON_CYAN =
{
    {20,  22,  26},         // default_color
    {28,  32,  38},         // hover_color
    {12,  60,  72},         // active_color
    {18,  18,  20},         // blocked_color
    {20,  22,  26,  80},    // transparent_color

    {200, 240, 245},        // default_text_color
    {220, 248, 252},        // hover_text_color
    {240, 252, 255},        // active_text_color
    {70,  80,  82},         // blocked_text_color
    {150, 200, 210, 120},   // transparent_text_color

    {30,  180, 200},        // default_outline_color
    {50,  210, 225},        // hover_outline_color
    {80,  230, 240},        // active_outline_color
    {40,  55,  58},         // blocked_outline_color
    {30,  180, 200, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_OBSIDIAN_EMBER =
{
    {22,  20,  18},         // default_color
    {34,  28,  22},         // hover_color
    {80,  42,  14},         // active_color
    {20,  18,  16},         // blocked_color
    {22,  20,  18,  80},    // transparent_color

    {240, 220, 200},        // default_text_color
    {252, 235, 215},        // hover_text_color
    {255, 242, 225},        // active_text_color
    {80,  72,  62},         // blocked_text_color
    {180, 150, 120, 120},   // transparent_text_color

    {200, 100, 40},         // default_outline_color
    {230, 120, 50},         // hover_outline_color
    {250, 140, 60},         // active_outline_color
    {55,  45,  35},         // blocked_outline_color
    {200, 100, 40,  120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr ButtonStyle BUTTON_MIDNIGHT_ROSE =
{
    {22,  18,  26},         // default_color
    {36,  26,  40},         // hover_color
    {80,  24,  50},         // active_color
    {20,  16,  22},         // blocked_color
    {22,  18,  26,  80},    // transparent_color

    {240, 200, 215},        // default_text_color
    {252, 215, 230},        // hover_text_color
    {255, 230, 242},        // active_text_color
    {80,  65,  75},         // blocked_text_color
    {170, 130, 150, 120},   // transparent_text_color

    {180, 60,  100},        // default_outline_color
    {210, 80,  120},        // hover_outline_color
    {240, 100, 140},        // active_outline_color
    {55,  40,  52},         // blocked_outline_color
    {180, 60,  100, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_STALE_VIOLET =
{
    {30,  27,  46},         // default_color
    {45,  40,  72},         // hover_color
    {76,  63,  160},        // active_color
    {22,  22,  32},         // blocked_color
    {30,  27,  46,  80},    // transparent_color

    {90,  80,  180},        // default_outline_color
    {120, 108, 220},        // hover_outline_color
    {150, 138, 240},        // active_outline_color
    {55,  52,  80},         // blocked_outline_color
    {90,  80,  180, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_CARBON_CYAN =
{
    {20,  22,  26},         // default_color
    {28,  32,  38},         // hover_color
    {12,  60,  72},         // active_color
    {18,  18,  20},         // blocked_color
    {20,  22,  26,  80},    // transparent_color

    {30,  180, 200},        // default_outline_color
    {50,  210, 225},        // hover_outline_color
    {80,  230, 240},        // active_outline_color
    {40,  55,  58},         // blocked_outline_color
    {30,  180, 200, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_OBSIDIAN_EMBER =
{
    {22,  20,  18},         // default_color
    {34,  28,  22},         // hover_color
    {80,  42,  14},         // active_color
    {20,  18,  16},         // blocked_color
    {22,  20,  18,  80},    // transparent_color

    {200, 100, 40},         // default_outline_color
    {230, 120, 50},         // hover_outline_color
    {250, 140, 60},         // active_outline_color
    {55,  45,  35},         // blocked_outline_color
    {200, 100, 40,  120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr TextureStyle STYLE_MIDNIGHT_ROSE =
{
    {22,  18,  26},         // default_color
    {36,  26,  40},         // hover_color
    {80,  24,  50},         // active_color
    {20,  16,  22},         // blocked_color
    {22,  18,  26,  80},    // transparent_color

    {180, 60,  100},        // default_outline_color
    {210, 80,  120},        // hover_outline_color
    {240, 100, 140},        // active_outline_color
    {55,  40,  52},         // blocked_outline_color
    {180, 60,  100, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
    2.0f,                   // hovered_outline_thickness
    2.0f,                   // active_outline_thickness
    1.0f,                   // blocked_outline_thickness
    2.0f                    // transparent_outline_thickness
};

constexpr CheckboxStyle CHECKBOX_STALE_VIOLET =
{
    {76,  63,  160},        // on_color
    {30,  27,  46},         // off_color
    {45,  40,  72},         // hover_color
    {22,  22,  32},         // blocked_color
    {30,  27,  46,  80},    // transparent_color

    {255, 252, 255},        // text_color
    {225, 220, 255},        // hover_text_color
    {90,  88,  110},        // blocked_text_color
    {180, 175, 210, 120},   // transparent_text_color

    {150, 138, 240},        // outline_color
    {120, 108, 220},        // hover_outline_color
    {55,  52,  80},         // blocked_outline_color
    {90,  80,  180, 120},   // transparent_outline_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_CARBON_CYAN =
{
    {12,  60,  72},         // on_color
    {20,  22,  26},         // off_color
    {28,  32,  38},         // hover_color
    {18,  18,  20},         // blocked_color
    {20,  22,  26,  80},    // transparent_color

    {240, 252, 255},        // text_color
    {220, 248, 252},        // hover_text_color
    {70,  80,  82},         // blocked_text_color
    {150, 200, 210, 120},   // transparent_text_color

    {80,  230, 240},        // outline_color
    {50,  210, 225},        // hover_outline_color
    {40,  55,  58},         // blocked_outline_color
    {30,  180, 200, 120},   // transparent_outline_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_OBSIDIAN_EMBER =
{
    {80,  42,  14},         // on_color
    {22,  20,  18},         // off_color
    {34,  28,  22},         // hover_color
    {20,  18,  16},         // blocked_color
    {22,  20,  18,  80},    // transparent_color

    {255, 242, 225},        // text_color
    {252, 235, 215},        // hover_text_color
    {80,  72,  62},         // blocked_text_color
    {180, 150, 120, 120},   // transparent_text_color

    {250, 140, 60},         // outline_color
    {230, 120, 50},         // hover_outline_color
    {55,  45,  35},         // blocked_outline_color
    {200, 100, 40,  120},   // transparent_outline_color

    2.0f                    // outline_thickness
};

constexpr CheckboxStyle CHECKBOX_MIDNIGHT_ROSE =
{
    {80,  24,  50},         // on_color
    {22,  18,  26},         // off_color
    {36,  26,  40},         // hover_color
    {20,  16,  22},         // blocked_color
    {22,  18,  26,  80},    // transparent_color

    {255, 230, 242},        // text_color
    {252, 215, 230},        // hover_text_color
    {80,  65,  75},         // blocked_text_color
    {170, 130, 150, 120},   // transparent_text_color

    {240, 100, 140},        // outline_color
    {210, 80,  120},        // hover_outline_color
    {55,  40,  52},         // blocked_outline_color
    {180, 60,  100, 120},   // transparent_outline_color

    2.0f,                   // outline_thickness
};

constexpr EntryStyle ENTRY_SLATE_VIOLET =
{
    {30,  27,  46},         // fill_color
    {45,  40,  72},         // fill_hover_color
    {76,  63,  160},        // fill_active_color
    {22,  22,  32},         // fill_blocked_color
    {30,  27,  46,  80},    // fill_transparent_color

    {210, 205, 240},        // text_default_color
    {225, 220, 255},        // text_hover_color
    {255, 252, 255},        // text_active_color
    {90,  88,  110},        // text_blocked_color
    {180, 175, 210, 120},   // text_transparent_color

    {80,  75,  120},        // bg_text_default_color
    {95,  88,  145},        // bg_text_hover_color
    {120, 108, 180},        // bg_text_active_color
    {50,  48,  75},         // bg_text_blocked_color
    {80,  75,  120, 80},    // bg_text_transparent_color

    {90,  80,  180},        // outline_color
    {120, 108, 220},        // outline_hover_color
    {150, 138, 240},        // outline_active_color
    {55,  52,  80},         // outline_blocked_color
    {90,  80,  180, 120},   // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_CARBON_CYAN =
{
    {20,  22,  26},         // fill_color
    {28,  32,  38},         // fill_hover_color
    {12,  60,  72},         // fill_active_color
    {18,  18,  20},         // fill_blocked_color
    {20,  22,  26,  80},    // fill_transparent_color

    {200, 240, 245},        // text_default_color
    {220, 248, 252},        // text_hover_color
    {240, 252, 255},        // text_active_color
    {70,  80,  82},         // text_blocked_color
    {150, 200, 210, 120},   // text_transparent_color

    {40,  90,  100},        // bg_text_default_color
    {50,  110, 120},        // bg_text_hover_color
    {30,  130, 150},        // bg_text_active_color
    {35,  42,  44},         // bg_text_blocked_color
    {40,  90,  100, 80},    // bg_text_transparent_color

    {30,  180, 200},        // outline_color
    {50,  210, 225},        // outline_hover_color
    {80,  230, 240},        // outline_active_color
    {40,  55,  58},         // outline_blocked_color
    {30,  180, 200, 120},   // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_OBSIDIAN_EMBER =
{
    {22,  20,  18},         // fill_color
    {34,  28,  22},         // fill_hover_color
    {80,  42,  14},         // fill_active_color
    {20,  18,  16},         // fill_blocked_color
    {22,  20,  18,  80},    // fill_transparent_color

    {240, 220, 200},        // text_default_color
    {252, 235, 215},        // text_hover_color
    {255, 242, 225},        // text_active_color
    {80,  72,  62},         // text_blocked_color
    {180, 150, 120, 120},   // text_transparent_color

    {100, 60,  30},         // bg_text_default_color
    {120, 75,  35},         // bg_text_hover_color
    {150, 90,  40},         // bg_text_active_color
    {50,  42,  32},         // bg_text_blocked_color
    {100, 60,  30,  80},    // bg_text_transparent_color

    {200, 100, 40},         // outline_color
    {230, 120, 50},         // outline_hover_color
    {250, 140, 60},         // outline_active_color
    {55,  45,  35},         // outline_blocked_color
    {200, 100, 40,  120},   // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
};

constexpr EntryStyle ENTRY_MIDNIGHT_ROSE =
{
    {22,  18,  26},         // fill_color
    {36,  26,  40},         // fill_hover_color
    {80,  24,  50},         // fill_active_color
    {20,  16,  22},         // fill_blocked_color
    {22,  18,  26,  80},    // fill_transparent_color

    {240, 200, 215},        // text_default_color
    {252, 215, 230},        // text_hover_color
    {255, 230, 242},        // text_active_color
    {80,  65,  75},         // text_blocked_color
    {170, 130, 150, 120},   // text_transparent_color

    {100, 50,  75},         // bg_text_default_color
    {120, 60,  90},         // bg_text_hover_color
    {150, 50,  100},        // bg_text_active_color
    {55,  38,  50},         // bg_text_blocked_color
    {100, 50,  75,  80},    // bg_text_transparent_color

    {180, 60,  100},        // outline_color
    {210, 80,  120},        // outline_hover_color
    {240, 100, 140},        // outline_active_color
    {55,  40,  52},         // outline_blocked_color
    {180, 60,  100, 120},   // outline_transparent_color

    2.0f,                   // outline_default_thickness
    2.0f,                   // outline_hover_thickness
    2.0f,                   // outline_active_thickness
    1.0f,                   // outline_blocked_thickness
    1.0f                    // outline_transparent_thickness
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
}

namespace parameters
{
// indicator | hidden | blocked | autohide | autoselect | grid | scroll | pan | block bg image | resize
inline const FLAGS MODIFIABLE = FLAGS::INDICATOR | FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::AUTOHIDE |
                                FLAGS::AUTOSELECT | FLAGS::GRID | FLAGS::SCROLL | FLAGS::ZOOM |
                                FLAGS::PAN | FLAGS::BLOCKBGIMAGE;


constexpr bool UNSELECT_WHEN_HOLD = false;                                  //!< Should holding a widget unselect it? true by default
constexpr bool STACK_MOUSE_CALLBACKS = false;                               //!< Allows mouse events to trigger on multiple widgets inside a frame (stacked widgets)
constexpr bool STACK_FRAME_WITH_WIDGETS = false;                            //!< Allows the frame callback to trigger after a child callback

constexpr sf::Mouse::Button ACTIVE_MOUSE_BUTTON = sf::Mouse::Button::Left;  //!< Mouse button for event triggering
constexpr std::chrono::milliseconds QUEUE_DELAY = 150ms;                    //!< Delay between keyboard events
constexpr std::chrono::milliseconds BLINK_PERIOD = 1000ms;

constexpr std::size_t CHAR_LIMIT = 15;                                      //!< Limit of characters in widgets with text
constexpr unsigned int CORNER_POINTS = 16;                                  //!< Number of vertices in a rounded widget corner 
constexpr float SCROLL_RATIO = 0.05f;                                       //!< Frame scrolls by 5% of it's height on each scroll
constexpr float MAX_ZOOM_LEVEL = 5.f;                                       //!< Default zoom level for `GridCanvas` showing full view
constexpr float MIN_ZOOM_LEVEL = 1.f;                                       //!< Smallest view possible while zooming in `GridCanvas`
constexpr float MAX_DEADZONE_RATIO = 0.1f;                                  //!< Max surface occupation percentage for deadzone in `GridCanvas`

///////////////////////////////////////////////////////////////////
///////////////////          Paths              ///////////////////
///////////////////////////////////////////////////////////////////

inline const sf::Font font("assets/fonts/default.otf"); //!< temporary font

// Place holder image
inline const std::string DEFAULT_IMAGE_PATH{"assets/images/paper.png"};
// Clip button's image + opacity effect, uniform => texture, size, radius, image_opacity
inline const std::string SPRITE_CLIP_COLOR_SHADER_PATH{"assets/shaders/button_clip_opacity.frag"};
// Clip sprite + opacity effect, uniform => texture, size, radius, opacity
inline const std::string SPRITE_FULLCLIP_OPACITY_SHADER_PATH{"assets/shaders/sprite_clip_opacity.frag"};
// Shader for updating a texture's color and opacity based on input if it has texture, uniform => texture, bg_color, opacity, outline_color, has_texture
inline const std::string TEXTURE_COLOR_SHADER_PATH{"assets/shaders/texture_color.frag"};
}
}
#endif