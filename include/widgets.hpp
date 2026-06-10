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
template <typename DERIVED_TYPE> struct __WidgetStyle;
class CaptureSequence;
struct Key;
class RenderContext;
class ImageCache;
class CellOutline;
class Cell;

///////////////////////////////////////////////////////////////////
///////////////////         Types               ///////////////////
///////////////////////////////////////////////////////////////////

using Sequence = std::vector<Key>;                                          //!< Represents a sequence of key presses
using SequenceFunction = std::function<void()>;                             //!< Required function structure for key press events
using MouseCallback = std::function<void(std::optional<const sf::Event>)>;  //!< Required function structure for mouse press events

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
    FOCUSED = 1 << 3,               //!< State: `widget is clicked, only one widget can be focused at a time`

    HOVERED = 1 << 4,               //!< State: `mouse hovered within widget bounds`
    SELECTED = 1 << 5,              //!< State: `widget is not yet active but can trigger callbacks and set active`
    RESTRICTED = 1 << 6,            //!< State: `certain functions are limited temporarily`
    ACTIVE = 1 << 7,                //!< State: `active widget state as ACTIVE_BUTTON is pressed`
    SET = 1 << 8,                   //!< State: `1 state, used in checkbox and entry`
    RESET = 1 << 9,                 //!< State: `0 state, used in entry`
    TRANSPARENT = 1 << 10,          //!< State: `transparent state, set by AUTOHIDE when not hovering`
    DRAWING = 1 << 11,              //!< State: `drawing state for containers`
    SHORTSKIP = 1 << 12,            //!< State: `skips a small process`
    LONGSKIP = 1 << 13,             //!< State: `skips a process until a condition is satisfied, used by Widget`

    INDICATOR = 1 << 14,            //!< Configured flag: `unique widget indicator`
    HIDDEN = 1 << 15,               //!< Configured flag: `invisible widget, does not trigger mouse events`
    BLOCKED = 1 << 16,              //!< Configured flag: `widget becomes non-interactive`
    AUTOSELECT = 1 << 17,           //!< Configured flag: `widget is always selected for keyboard events`
    AUTOHIDE = 1 << 18,             //!< Configured flag: `widget is transparent when not hovered`
    
    CLIPSHADER = 1 << 19,           //!< Flag: `shader for sprites is loaded`
    COLORSHADER = 1 << 20,          //!< Flag: `shader for fill texture is loaded`

    RESIZE = 1 << 21,               //!< Configured Flag: `allows widget to scale with window size`
    TEXT = 1 << 22,                 //!< Flag: `widget is currently displaying text`
    IMAGE = 1 << 23,                //!< Flag: `widget is currently displaying an image`
    BGIMAGE = 1 << 24,              //!< Flag: `widget has fill texture loaded`
    DEFAULTIMAGE = 1 << 25,         //!< Flag: `default image currently used`
    
    GRID = 1 << 26,                 //!< Configured Flag: `grid is enabled for container`
    SCROLL = 1 << 27,               //!< Configured Flag: `scrolling is enabled for scrollable widgets like frame`
    SEPARATORS = 1 << 28,           //!< Flag: `line separators in frame is enabled`
    ZOOM = 1 << 29,                 //!< Configured Flag: `zooming is enabled for canvas`
    PAN = 1 << 30,                  //!< Configured Flag: `panning is enabled for canvas`
    BLOCKBGIMAGE = 1 << 31,          //!< Configured Flag: `blocks loading fill texture, clears it with shader if enabled`
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
/// @brief Static class for storing images used within widgets by paths
///
/// @see `ImageReferenceCounter`
///
///////////////////////////////////////////////////////////////////
class ImageCache
{
public:
    static ImageCache& getInstance();

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
    ImageCache() = default;
    ImageCache(const ImageCache&) = delete;
    ImageCache& operator=(const ImageCache&) = delete;
    ImageCache(ImageCache&&) = delete;
    ImageCache& operator=(ImageCache&&) = delete;

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
    /// @brief Updates root window's view dimensions after a resize event
    ///
    /// If a window pointer is not available then it updates the frame's
    /// root window view.
    ///
    ///////////////////////////////////////////////////////////////////
    void setWindowView(const sf::View& __v);

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
    ///////////////////////////////////////////////////////////////////
    void add(Widget* w);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a widget from the group, does nothing if the widget
    /// does not belong in the first place
    ///
    ///////////////////////////////////////////////////////////////////
    void remove(Widget* w);

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
// indicator | hidden | blocked | autohide | autoselect | grid | scroll | pan | block bg image | resize
inline const FLAGS MODIFIABLE = FLAGS::INDICATOR | FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::AUTOHIDE |
                                FLAGS::AUTOSELECT | FLAGS::GRID | FLAGS::SCROLL | FLAGS::ZOOM |
                                FLAGS::PAN | FLAGS::BLOCKBGIMAGE | FLAGS::RESIZE;


constexpr bool UNSELECT_WHEN_HOLD = false;                                  //!< Should holding a widget unselect it? true by default
constexpr bool STACK_MOUSE_CALLBACKS = false;                               //!< Allows mouse events to trigger on multiple widgets inside a frame (stacked widgets)
constexpr bool STACK_FRAME_WITH_WIDGETS = false;                            //!< Allows the frame callback to trigger after a child callback

constexpr sf::Mouse::Button ACTIVE_MOUSE_BUTTON = sf::Mouse::Button::Left;  //!< Mouse button for event triggering
constexpr sf::Time QUEUE_DELAY = sf::milliseconds(0);                       //!< Delay between keyboard events
constexpr sf::Time HOVER_DURATION = sf::milliseconds(500);                  //!< Hovering duration before tooltip is drawn
constexpr sf::Time BLINK_PERIOD = sf::milliseconds(1000);                   //!< Blinking indicators period

constexpr unsigned int N_FADING_FRAMES = 120;                               //!< Number of frames for tooltip to appear
constexpr std::size_t CHAR_LIMIT = 15;                                      //!< Limit of characters in widgets with text
constexpr unsigned int CHAR_SIZE = 12;                                      //!< Default character size for text used by widgets
constexpr unsigned int CORNER_POINTS = 16;                                  //!< Number of vertices in a rounded widget corner 
constexpr float SCROLL_RATIO = 0.05f;                                       //!< Frame scrolls by 5% of it's height on each scroll
constexpr float MAX_ZOOM_LEVEL = 5.f;                                       //!< Default zoom level for `GridCanvas` showing full view
constexpr float MIN_ZOOM_LEVEL = 1.f;                                       //!< Smallest view possible while zooming in `GridCanvas`
constexpr float MAX_DEADZONE_RATIO = 0.1f;                                  //!< Max surface occupation percentage for deadzone in `GridCanvas`

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