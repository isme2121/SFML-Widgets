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

namespace sfml_widgets
{
class Widget;
class Button;
class Frame;
class GridCanvas;
class ImageFrame;
class CheckBox;
class Entry;
class Label;
class ToolTip;
class WidgetGroup;
template <typename DERIVED_TYPE> struct WidgetStyle;
class CaptureSequence;
struct Key;
class RenderContext;
class CellOutline;
class Cell;

///////////////////////////////////////////////////////////////////
///////////////////         Types               ///////////////////
///////////////////////////////////////////////////////////////////

using Sequence = std::vector<Key>;                                          //!< Represents a sequence of key presses
using SequenceFunction = std::function<void()>;                             //!< Required function structure for key press events
using MouseCallback = std::function<void(std::optional<sf::Event>)>;  //!< Required function structure for mouse press events

///////////////////////////////////////////////////////////////////
///////////////////         Enums               ///////////////////
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// @brief Bit wise class for updating widget state & control
/// management class
///
/// The class is divided to multiple sections with each serving a
/// separate role, their current states can be checked with
/// `Widget::isFlag` & `Widget::isFlags` methods.
///
/// States are for internal flow control and are managed by the class,
/// Flags however can be toggled using `Widget::configure` method,
/// Derived flags are implemented in only a number of widgets while
/// Base flags are universal for all widgets.
///
/// Flags are modified using bit operators such as `|`, `&`, `^` and
/// more.
///
///////////////////////////////////////////////////////////////////
enum class FLAGS : std::int32_t
{
// Main properties for flow control
    IDLE = 1 << 0,                  //!< State: `default state`
    INIT = 1 << 1,                  //!< State: `initialized state`
    MODIFIED = 1 << 2,              //!< State: `set automatically after an update`
    FOCUSED = 1 << 3,               //!< State: `focus resource obtained from clicking a widget by default`

// Base states, controlled within the base class
    HOVERED = 1 << 4,               //!< State: `widget is hovered`
    ALPHAREDUCED = 1 << 5,          //!< State: `widget is transparent`
    SELECTED = 1 << 6,              //!< State: `widget is selected and can trigger callbacks`
    RESTRICTED = 1 << 7,            //!< State: `widget is blocking certain functions`
    ACTIVE = 1 << 8,                //!< State: `widget is pressed with the active mouse button or held`

// Derived states, controlled within derived classes
    DRAWING = 1 << 9,               //!< State: `widget is drawing` @see `Frame`
    SHORTSKIP = 1 << 10,            //!< State: `widget can skip a repetitive process` @see `Frame`, `CheckBox`
    LONGSKIP = 1 << 11,             //!< State: `widget can skip a process based on internal conditions` @see `Frame`
    SET = 1 << 12,                  //!< State: `widget's 1 state` @see `CheckBox`, `Entry`
    RESET = 1 << 13,                //!< State: `widget's 0 state` @see `CheckBox`, `Entry`
    TEXT = 1 << 14,                 //!< State: `widget is currently displaying text` @see `Button`
    IMAGE = 1 << 15,                //!< State: `widget is currently displaying an image (unrelated to background image)` @see `Button`
    SEPARATORS = 1 << 16,           //!< State: `widget has separator lines enabled` @see `Frame`

// Base configurable flags
    HIDDEN = 1 << 17,               //!< Configured flag: `widget is invisible`
    BLOCKED = 1 << 18,              //!< Configured flag: `widget is non-interactive`
    TRANSPARENT = 1 << 19,          //!< Configured flag: `widget is always transparent`
    RESIZE = 1 << 20,               //!< Configured Flag: `widget can automatically resize with the window`
    ALWAYSSELECTED = 1 << 21,           //!< Configured flag: `widget is always selected for keyboard events`
    AUTOHIDE = 1 << 22,             //!< Configured flag: `widget is hidden when not hovered`
    AUTOTRANSPARENT = 1 << 23,      //!< Configured flag: `widget is transparent when not hovered`
    NOBGIMAGE = 1 << 24,            //!< Configured Flag: `widget cannot select a background image`
    NOCLIPSHADER = 1 << 25,         //!< Configured Flag: `widget cannot load a clipping shader`
    NOCOLORSHADER = 1 << 26,        //!< Configured Flag: `widget cannot load a bg color shader`

// Derived configurable flags
    INDICATOR = 1 << 27,            //!< Configured flag: `widget has a visible indicator` @see `Entry`, `GridCanvas`
    GRID = 1 << 28,                 //!< Configured Flag: `grid is enabled for widget` @see `Frame`, `GridCanvas`
    SCROLL = 1 << 29,               //!< Configured Flag: `widget has scrolling enabled` @see `Frame`
    ZOOM = 1 << 30,                 //!< Configured Flag: `zooming is enabled for widget` @see `GridCanvas`
    PAN = 1 << 31                   //!< Configured Flag: `panning is enabled for widget` @see `GridCanvas`
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

FLAGS operator &(FLAGS, FLAGS);         //!< Does not set MODIFIED
FLAGS& operator &=(FLAGS&, FLAGS);      //!< Sets MODIFIED
FLAGS operator |(FLAGS, FLAGS);         //!< Does not set MODIFIED
FLAGS& operator |=(FLAGS&, FLAGS);      //!< Sets MODIFIED
FLAGS operator ^(FLAGS, FLAGS);         //!< Does not set MODIFIED
FLAGS& operator ^=(FLAGS&, FLAGS);      //!< Sets MODIFIED
FLAGS& setFlag(FLAGS&, FLAGS);          //!< Sets MODIFIED
FLAGS& resetFlag(FLAGS&, FLAGS);        //!< Sets MODIFIED IF not included in bits
FLAGS& toggle(FLAGS&, FLAGS);           //!< Does not set MODIFIED
bool checkFlag(const FLAGS&, FLAGS);    //!< Quick boolean check, returns true if any bit is set in both sides
bool checkFlags(const FLAGS&, FLAGS);   //!< Quick boolean check, returns true if all bits on right are set in left, otherwise false

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
    
    bool operator==(const std::string& other_tag);
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
    /// @param key_event Event object
    ///
    /// @return true if a sequence was matched, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////
    bool checkEvents(const sf::Event& key_event);

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

    RenderContext(RenderContext&& other) = delete;
    RenderContext& operator=(RenderContext&& other) = delete;

    RenderContext(const RenderContext& other) = delete;
    RenderContext& operator=(const RenderContext& other) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget
    ///
    /// @param widget reference to widget
    /// @param state rendering state object for the widget
    ///
    ///////////////////////////////////////////////////////////////////
    void draw(const sf::Drawable& widget, const sf::RenderStates state = sf::RenderStates::Default);

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
    /// @brief Updates root window's view dimensions after a resize event
    ///
    /// If a window pointer is not available then it updates the frame's
    /// root window view.
    ///
    ///////////////////////////////////////////////////////////////////
    void setWindowView(const sf::View& v);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the pointer to the main window
    ///
    ///////////////////////////////////////////////////////////////////
    void setWindowPtr(sf::RenderWindow* win_ptr);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the pointer to the root frame
    ///
    ///////////////////////////////////////////////////////////////////
    void setFramePtr(Frame* frame_ptr);

    ///////////////////////////////////////////////////////////////////
    /// @brief returns a readonly pointer to window object if available
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::RenderWindow* getWindowPtr() const; 

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to the root window
    ///
    /// The widget calling this method gets the `sf::RenderWindow` address
    /// of the window drawing it.
    ///
    ///////////////////////////////////////////////////////////////////
    sf::RenderWindow* getRootWindowPtr();

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
///         w_group1->groupFocusCallback = true;
///         # only the widget that lost focus would fire their proper callback
///         w_group1->groupFocuslossCallback = false;
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
    static std::shared_ptr<WidgetGroup> make_group();

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
    /// @return true if the addition was successful, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool add(Widget* w);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a widget from the group, does nothing if the widget
    /// does not belong in the first place
    ///
    ///////////////////////////////////////////////////////////////////
    bool remove(Widget* w);

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the focus callback for all widgets in the group
    ///
    ///////////////////////////////////////////////////////////////////
    void focusOn(std::optional<sf::Event> event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the unfocus callback for all the widgets in the group
    ///
    ///////////////////////////////////////////////////////////////////
    void focusOff(std::optional<sf::Event> event);

    std::vector<Widget*> m_cult;    //!< ooga booga

    friend class Widget;
};

namespace constants
{
constexpr std::size_t KSTALE = std::numeric_limits<std::size_t>::max();
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

namespace parameters
{
// hidden | blocked | resize | autohide | autoselect | block bg image | indicator |  grid | scroll | zoom | pan
inline const FLAGS MODIFIABLE = FLAGS::INDICATOR | FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::RESIZE |
                                FLAGS::AUTOHIDE | FLAGS::ALWAYSSELECTED | FLAGS::TRANSPARENT | FLAGS::AUTOTRANSPARENT |
                                FLAGS::GRID | FLAGS::SCROLL | FLAGS::ZOOM | FLAGS::PAN |
                                FLAGS::NOBGIMAGE | FLAGS::NOCLIPSHADER | FLAGS::NOCOLORSHADER;


constexpr std::size_t CHAR_LIMIT = 15;                                      //!< Limit of characters in widgets with text
constexpr unsigned int CHAR_SIZE = 12;                                      //!< Default character size for text used by widgets

///////////////////////////////////////////////////////////////////
///////////////////          Paths              ///////////////////
///////////////////////////////////////////////////////////////////

inline const sf::Font font("assets/fonts/default.otf"); //!< temporary font

// Clip button's image + opacity effect, uniform => texture, size, radius, image_opacity
constexpr std::string_view SPRITE_CLIP_COLOR_SHADER_PATH = "assets/shaders/button_clip_opacity.frag";
// Clip sprite + opacity effect, uniform => texture, size, radius, opacity
constexpr std::string_view SPRITE_FULLCLIP_OPACITY_SHADER_PATH = "assets/shaders/sprite_clip_opacity.frag";
// Shader for updating a texture's color and opacity based on input if it has texture, uniform => texture, bg_color, opacity, outline_color, has_texture
constexpr std::string_view TEXTURE_COLOR_SHADER_PATH = "assets/shaders/texture_color.frag";
}
}
#endif