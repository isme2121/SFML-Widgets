#pragma once

#ifndef WIDGETS_HPP
#define WIDGETS_HPP
#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include "shaders.hpp"
#include "fonts.hpp"

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
class NewToolTip;
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
using MouseCallback = std::function<void(std::optional<sf::Event>)>;        //!< Required function structure for mouse press events

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
    MODIFIED = 1 << 1,              //!< State: `set automatically after an update`
    FOCUSED = 1 << 2,               //!< State: `focus resource obtained from clicking a widget by default`

// Base states, controlled within the base class
    HOVERED = 1 << 3,               //!< State: `widget is hovered`
    ALPHAREDUCED = 1 << 4,          //!< State: `widget is transparent`
    SELECTED = 1 << 5,              //!< State: `widget is selected and can trigger callbacks`
    RESTRICTED = 1 << 6,            //!< State: `widget is blocking certain functions`
    ACTIVE = 1 << 7,                //!< State: `widget is pressed with the active mouse button or held`

// Derived states, controlled within derived classes
    DRAWING = 1 << 8,               //!< State: `widget is drawing` @see `Frame`
    SHORTSKIP = 1 << 9,             //!< State: `widget can skip a repetitive process` @see `Frame`, `CheckBox`
    LONGSKIP = 1 << 10,             //!< State: `widget can skip a process based on internal conditions` @see `Frame`
    UPDATE = 1 << 11,               //!< State: `widget requests an update process`
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
    ALWAYSSELECTED = 1 << 21,       //!< Configured flag: `widget is always selected for keyboard events`
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

constexpr FLAGS operator&(FLAGS first, FLAGS second)
{
    return static_cast<FLAGS>(static_cast<std::int32_t>(first) & static_cast<std::int32_t>(second));
}

constexpr FLAGS operator|(FLAGS first, FLAGS second)
{
    return static_cast<FLAGS>(static_cast<std::int32_t>(first) | static_cast<std::int32_t>(second));
}

constexpr FLAGS operator^(FLAGS first, FLAGS second)
{
    return static_cast<FLAGS>(static_cast<std::int32_t>(first) ^ static_cast<std::int32_t>(second));
}

// Sets `FLAGS::MODIFIED`
constexpr FLAGS& operator&=(FLAGS& self, FLAGS other)
{
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) & static_cast<std::int32_t>(other)) | FLAGS::MODIFIED;
    return self;
}

// Sets `FLAGS::MODIFIED`
constexpr FLAGS& operator|=(FLAGS& self, FLAGS other)
{
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) | static_cast<std::int32_t>(other)) | FLAGS::MODIFIED;
    return self;
}

// Sets `FLAGS::MODIFIED`
constexpr FLAGS& operator^=(FLAGS& self, FLAGS other)
{
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) ^ static_cast<std::int32_t>(other)) | FLAGS::MODIFIED;
    return self;
}

// Sets `FLAGS::MODIFIED`
constexpr FLAGS& setFlag(FLAGS& self, FLAGS bits)
{
    self |= bits;    //!< sets MODIFIED
    return self;
};

// Checks if atleast one bit from bits is set in self
constexpr bool checkFlag(const FLAGS& self, FLAGS bits)
{
    return static_cast<bool>(self & bits);
}

// Checks if all bits are set in self
constexpr bool checkFlags(const FLAGS& self, FLAGS bits)
{
    return (self & bits) == bits;
}

// Sets `FLAGS::MODIFIED` IF it's not stated in bits
constexpr FLAGS& resetFlag(FLAGS& self, FLAGS bits)
{
    std::int32_t mask = ~static_cast<std::int32_t>(bits);
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) & mask);
    if(!checkFlag(bits, FLAGS::MODIFIED)) self |= FLAGS::MODIFIED;
    return self;
};

// Does not set `FLAGS::MODIFIED`
constexpr FLAGS& toggle(FLAGS& self, FLAGS bits)
{
    self = self ^ bits;
    return self;
}

// Checks if a flag can be passed in `Widget::configure`
constexpr bool isModifiable(FLAGS);

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
/// @brief Represents a `sf::Event::KeyPressed` object, used in
/// `Widget::BindShortCut` to create a sequence of key presses
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
        bool control{false};
        bool alt{false};
        bool shift{false};
        bool system{false};
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
    sf::String tag;
    Sequence sequence;
    SequenceFunction callback;
    
    bool operator==(const sf::String& other_tag);
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
    void addSequence(const sf::String& name, Sequence sequence, SequenceFunction callback);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Remove a sequence from memory
    ///
    /// Remove the first found sequence with entered name, returns if 
    /// name is not found.
    ///
    /// @param name string name used when assigning the sequence
    ///
    ///////////////////////////////////////////////////////////////////
    void deleteSequence(const sf::String& name);

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
    std::size_t m_max_sequence{0};                              //!< Max number of keys needed to match a saved sequence
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
    sf::RenderWindow* m_window{nullptr};
    Frame* m_frame{nullptr};
};

///////////////////////////////////////////////////////////////////
/// @brief Group of widgets sharing the focus resource
///
/// Designed to construct within a shared pointer object, each widget
/// referenced by the same group share special rules when it comes to
/// gaining or losing focus on a widget.
///
/// A widget group achieves synced functionality between multiple widgets of
/// the same group depending on the configurations @see `groupFocusCallback`, `groupFocuslossCallback`.
///
/// - `groupFocusCallback`: Once a widget from an existing group gains focus, choose to trigger `Focus Gain`
/// callback for all group widgets at once with `true`, or trigger only the focused widget callback with `false`.
///
/// - `groupFocuslossCallback`: Once the group loses focus, choose to trigger `Focus Loss` callback for all
/// group widgets at once with `true`, or trigger for only the single widget that lost the resource with `false`.
///
/// Use `make_group` static method to construct a new instance:
///
/// ```cpp
///     auto widget_group = sfml_widgets::WidgetGroup::make_group();
///     if(widget_group)
///     {
///         # assigning widgets to share focus if one is clicked
///         button_1.linkWidget(widget_group);
///         entry_1.linkWidget(widget_group);
///         # both widgets would fire their focus callback together if focused
///         widget_group->groupFocusCallback = true;
///         # only the widget that lost focus would fire their proper callback
///         widget_group->groupFocuslossCallback = false;
///     }
///     ...
/// ```
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

    bool groupFocusCallback{false};        //!< Focusing on a group of widgets triggers their focus callbacks all at once, `false` by default
    bool groupFocuslossCallback{true};     //!< Losing focus on a group of widget triggers their unfocus callbacks all at once, `true` by default

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
constexpr float PI{3.14159265f};
constexpr float DEGREE90{90.f};
constexpr float DEGREE180{180.f};
constexpr float DEGREE270{270.f};
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
// hidden | blocked | resize | autohide | autoselect | transparent | autotransparent | block bg image | block clip shader | block color shader | indicator |  grid | scroll | zoom | pan
constexpr FLAGS MODIFIABLE{FLAGS::INDICATOR | FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::RESIZE |
                                FLAGS::AUTOHIDE | FLAGS::ALWAYSSELECTED | FLAGS::TRANSPARENT | FLAGS::AUTOTRANSPARENT |
                                FLAGS::GRID | FLAGS::SCROLL | FLAGS::ZOOM | FLAGS::PAN |
                                FLAGS::NOBGIMAGE | FLAGS::NOCLIPSHADER | FLAGS::NOCOLORSHADER};


constexpr unsigned int CHAR_LIMIT{15};                                       //!< Limit of characters in widgets with text
constexpr unsigned int CHAR_SIZE{18};                                       //!< Default character size for text used by widgets
constexpr unsigned int MAX_STACK_IMAGE_SIZE{256 * 256 * 4};                  //!< Max size in bytes used by `resize_image` function to store an image on the stack

///////////////////////////////////////////////////////////////////
///////////////////          Fonts              ///////////////////
///////////////////////////////////////////////////////////////////

inline const sf::Font INTER_LIGHT_18{Inter_Light_ttf, Inter_Light_ttf_len};
inline const sf::Font INTER_REGULAR_18{Inter_Regular_ttf, Inter_Regular_ttf_len};
inline const sf::Font INTER_SEMIBOLD_18{Inter_SemiBold_ttf, Inter_SemiBold_ttf_len};
}
}
#endif