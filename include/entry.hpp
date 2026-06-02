#pragma once
#include "widget.hpp"
#ifdef WIDGETS_HPP
namespace sfml_widgets
{

class Entry : public Widget
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Takes a master object for internal graphics handling
    ///
    /// Parameters such as size and position are set to default values.
    ///
    /// @param root window or a frame widget
    /// @param position widget's initial position
    /// @param size widget's initial size
    ///
    ///////////////////////////////////////////////////////////////////
    Entry(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::Font& font = parameters::font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Takes a master object for internal graphics handling and
    /// position + size as a FloatRect object
    ///
    /// @param root window or a frame widget
    /// @param position widget's initial position
    /// @param size widget's initial size
    ///
    ///////////////////////////////////////////////////////////////////
    Entry(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Font& font = parameters::font);

    Entry(Entry&& other) = delete;
    Entry& operator=(Entry&& other) = delete;
    Entry(const Entry&) = delete;
    Entry& operator=(const Entry&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events.
    ///
    /// Call the method in the events loop for assigned methods callbacks,
    /// can optionally pass the current raw mouse position, this makes the
    /// widget skip getting the mouse position itself and can save computing
    /// time when multiple widgets are checking for event.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    /// @param mouse_position optional current mouse position from `sf::Mouse::getPosition`
    ///
    /// @see `bindMouseEvent`, `unbindMouseEvent`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note widgets with `FLAGS::BLOCKED` or `FLAGS::HIDDEN` states cannot
    /// trigger mouse events.
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position = std::nullopt) override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events.
    ///
    /// Captures keyboard events in the events loop, can capture individual
    /// physical key presses or a sequence of key press events depending on
    /// available assigned sequences.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    ///
    /// @see `bindShortCut`, `unbindShortCut`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note widget must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, widgets with `FLAGS::BLOCKED` cannot trigger
    /// key events.
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets text as input
    ///
    /// Replaces any existing input with parameter text.
    ///
    /// @see `get`, `clear`
    ///
    ///////////////////////////////////////////////////////////////////
    void setInput(std::string text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Moves the cursor to the input index
    ///
    /// If the index is larger than the current text length then the cursor
    /// is moved to the last index.
    ///
    ///////////////////////////////////////////////////////////////////
    void setCursorIndex(unsigned int idx);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current input string
    ///
    /// @see `setInput`, `clear`
    ///
    ///////////////////////////////////////////////////////////////////
    const std::string& get() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears the entry's input
    ///
    /// @see `setInput`, `get`
    ///
    ///////////////////////////////////////////////////////////////////
    void clear();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates character size based on the height of the widget
    ///
    /// @param ratio value from 0.f to 1.f
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(float ratio);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates character size with the new value
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(unsigned int char_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the left offset for text scaling with the entry's
    /// width
    ///
    /// @param ratio set from `0.f` to 0.99..f`
    ///
    ///////////////////////////////////////////////////////////////////
    void setOffsetRatio(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the background string content
    ///
    ///////////////////////////////////////////////////////////////////
    void setBackGroundText(std::string bg_text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a filler character for entered text
    ///
    /// @param placeholder_char any character, set to 0 to remove the 
    /// placeholder
    ///
    ///////////////////////////////////////////////////////////////////
    void setPlaceholderChar(const char placeholder_char = 0);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets an input filter for entered characters
    ///
    /// Function must accept a `char32_t` type as a parameter and return
    /// a boolean value to validate input.
    ///
    /// Set func to null type to remove the filter.
    ///
    ///////////////////////////////////////////////////////////////////
    void setInputFilter(std::function<bool(char32_t)> func = nullptr);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a character limit for the entry
    ///
    /// The limit also effects background text.
    ///
    ///////////////////////////////////////////////////////////////////
    void setInputLimit(std::size_t max_chars);

    ///////////////////////////////////////////////////////////////////
    /// @brief Adjusts the uptime for the cursor
    ///
    /// Ratio can range from 0.f to 1.f, 0.f would mean the cursor is never
    /// visible while 1.f keeps it visible at all time, anything in between
    /// takes a share of `parameters::BLINK_PERIOD`.
    ///
    /// Cursor can be disabled/enabled with `FLAGS::INDICATOR` bit.
    ///
    ///////////////////////////////////////////////////////////////////
    void setBLinkUpRatio(float ratio = 0.5f);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the cursor width
    ///
    ///////////////////////////////////////////////////////////////////
    void setCursorWidth(unsigned int width);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the input text
    ///
    /// @param font reference to an existing font
    ///
    /// @warning if the passed font is an lvalue, it must outlive the entry
    /// or get replaced before it's deleted
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the input text and stores it internally
    ///
    /// @param font font instance
    ///
    /// @warning the parameter font is stored within the entry after the
    /// call to maintain it's reference for the widget's lifetime with the
    /// cost of extra memory usage
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font&& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(EntryStyle __entry_style);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget on master object
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Used to update child widgets for container type Widget or
    /// add specific rules after updating flags, called in `configure`
    ///
    /// @param mask bits passed in `configure`
    ///
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the widget,
    /// called in `setWidgetPosition`
    ///
    /// @see `setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the widget,
    /// called in `setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief matches the current angle value with the rest of the entry
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates widget colors to match current state alongside shaders
    ///
    /// Best called within `Draw` override before drawing, it is also
    /// the optimal method for updating available shaders.
    ///
    /// The default tested states are: `BLOCKED`, `TRANSPARENT`, `ACTIVE`
    /// `HOVERED` respectively.
    ///
    ///////////////////////////////////////////////////////////////////
    void updateState() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs shared theme with widget
    ///
    /// @note This method can be skipped if `FLAGS::PASSIVESKIP` is set
    /// internally
    ///
    /// @see `Widget::linkTheme`, `Widget::unlinkTheme`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for clipping shader
    ///
    /// Best called after an update in a shader parameter, clip shader handles
    /// clipping edges on sprites.
    ///
    /// @see `loadClipShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateClipShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in a shader parameters, color shader is applied
    /// on background texture that is set via `setBackGroundTexture` and is not needed
    /// if no texture is assigned.
    ///
    /// @see `loadColorShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateColorShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current x coordinate of the cursor
    ///
    ///////////////////////////////////////////////////////////////////
    float getCursorCoordinate() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Positions cursor to the closest index to x__pos
    ///
    ///////////////////////////////////////////////////////////////////
    void setCursorIndex(float x__pos);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates text size and position for both input and background
    /// strings
    ///
    ///////////////////////////////////////////////////////////////////
    void updateText();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates max panning offset based on input text width
    ///
    ///////////////////////////////////////////////////////////////////
    void updateContentWidth();

    ///////////////////////////////////////////////////////////////////
    /// @brief Moves view to show cursor
    ///
    ///////////////////////////////////////////////////////////////////
    void shiftToCursor();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates view
    ///
    ///////////////////////////////////////////////////////////////////
    void updateView();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws cursor while updating it's designated clock
    ///
    ///////////////////////////////////////////////////////////////////
    void drawCursorLine();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the content visible within the entry
    ///
    ///////////////////////////////////////////////////////////////////
    void writeToEntry();

    // drawing
    sf::RenderTexture m_renderTexture;
    sf::Sprite m_sprite;
    std::unique_ptr<const sf::Font> m_font;
    sf::Text m_text;
    sf::Text m_bgText;
    
    // data
    std::string m_textBuffer;                                       //!< Input content
    std::size_t m_cursorIndex = 0;                                  //!< Points to char in string index + 1, so m_textBuffer[0] == m_cursorIndex{0}
    std::function<bool(char32_t)> m_inputFilter = nullptr;

    // parameters
    std::size_t m_inputLimit = parameters::CHAR_LIMIT;              //!< Max possible number of characters
    float m_textSizeRatio = 0.5f;                                   //!< How much space does the text take based on entry's height
    float m_textOffsetRatio = 0.1f;
    unsigned int m_charSize = 24;
    unsigned int m_cursorWidth = 1;
    char m_placeholderCharacter = 0;                                //!< Visible character replacing the actual content
    sf::Time m_cursorOnDuration;
    sf::Time m_cursorOffDuration;

    // funcionality
    sf::Clock m_blinkTimer;
    float m_textBoxWidth;                                           //!< Entry's text width
    float m_currentPanOffset = 0.f;                                 //!< Current panned value
    float m_maxPanOffset = 0.f;                                     //!< Max possible space for panning
    EntryStyle m_style = Theme::entry_style::ENTRY_STALE_VIOLET;    //!< Entry's default style
};

template <>
struct __WidgetStyle<Entry>
{
    using StyleType = EntryStyle;

    static const StyleType& get(const WidgetTheme& t)
    {
        return t.m_entryTheme;
    }

    static void set(WidgetTheme& t, StyleType _b)
    {
        t.m_entryTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_entryToken;
    }
};
}
#endif