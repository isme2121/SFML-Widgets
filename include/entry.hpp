#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{

class Entry : public Widget
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button with `sf::RenderWindow`
    ///
    /// Parameters such as size and position are set to default values.
    ///
    /// @param root window or a frame widget
    /// @param position widget's initial position
    /// @param size widget's initial size
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    Entry(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::Font& font = parameters::INTER_REGULAR_18);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button on a `Frame` object
    ///
    /// @param root window or a frame widget
    /// @param position widget's initial position
    /// @param size widget's initial size
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    Entry(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Font& font = parameters::INTER_REGULAR_18);

    Entry(Entry&& other) = delete;
    Entry& operator=(Entry&& other) = delete;
    Entry(const Entry&) = delete;
    Entry& operator=(const Entry&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets text as input
    ///
    /// @see `getText`, `clearText`, `setTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const sf::String& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the background string content
    ///
    /// Limit is 256 characters.
    ///
    /// @see `getBackGroundText`
    ///
    ///////////////////////////////////////////////////////////////////
    void setBackGroundText(const sf::String& bg_text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets an input filter for entered characters
    ///
    /// Function must accept a `char32_t*` type as a parameter and return
    /// a boolean value to validate input, The filter runs on text input only.
    ///
    /// Set func to nullptr type to remove the filter.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFilter(std::function<bool(char32_t*)> func = nullptr);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a character limit for the entry, default is 
    /// `parameters::CHAR_LIMIT`
    ///
    /// The limit does not effect background text.
    ///
    /// @see `getTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextLimit(std::size_t max_chars);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates character size based on the height of the widget,
    /// default is `0.5f`
    ///
    /// @note set to `0.f` to use a direct character size value instead of ratio value
    ///
    /// @param ratio value from 0.f to 1.f
    ///
    /// @see `getTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextSize(float ratio);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates character size with the new value, default is
    /// `parameters::CHAR_SIZE`
    ///
    /// @see `getTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextSize(unsigned int char_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the input text
    ///
    /// @param font reference to an existing font
    ///
    /// @warning if the passed font is an lvalue, it must outlive the entry
    /// or get replaced before it's deleted
    ///
    /// @see `getTextFont`
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
    /// @see `getTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font&& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears the entry's input
    ///
    /// @see `setText`, `getText`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearText();

    ///////////////////////////////////////////////////////////////////
    /// @brief Adjusts the uptime for the cursor
    ///
    /// Ratio can range from `0.f` to `1.f`, `0.f` would mean the cursor is never
    /// visible while `1.f` keeps it visible at all time, anything in between
    /// takes a share of `s_BLINK_PERIOD` which is `1` second by default.
    ///
    /// @note Cursor can be disabled/enabled with `FLAGS::INDICATOR` bit.
    ///
    /// @see `getBlinkUpRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setBLinkUpRatio(float ratio = 0.5f);

    ///////////////////////////////////////////////////////////////////
    /// @brief Moves the cursor to the input index
    ///
    /// If the index is larger than the current text length then the cursor
    /// is moved to the last index.
    ///
    /// @see `getCursorIndex`
    ///
    ///////////////////////////////////////////////////////////////////
    unsigned int setCursorIndex(unsigned int idx);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a filler character for entered text
    ///
    /// @param placeholder_char any character, set to `0` to remove the 
    /// placeholder
    ///
    /// @see `getPlaceholderChar`
    ///
    ///////////////////////////////////////////////////////////////////
    void setPlaceholderChar(const char placeholder_char = 0);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the left offset for text scaling with the entry's
    /// width, default is `0.1f`
    ///
    /// @param ratio ranges from `[0.f, 1.f[`
    ///
    /// @see `getTextOffsetRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextOffsetRatio(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the cursor width
    ///
    /// @see `getCursorWidth`
    ///
    ///////////////////////////////////////////////////////////////////
    void setCursorWidth(unsigned int width);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(EntryStyle s_entry);

    ///////////////////////////////////////////////////////////////////
    /// @brief Pulls the widget's style from it's linked theme object
    /// 
    ///////////////////////////////////////////////////////////////////
    void fetchTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief R/W Reference to the current widget style
    ///
    /// @note if the widget is linked to a `WidgetTheme` source, any modifications
    /// to the returned reference do not affect the theme's style and are overridden
    /// by the theme if any updates are pushed to the widget.
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] EntryStyle& style();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current input string
    ///
    /// @see `setText`, `clearText`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::String& getText() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current background text content
    ///
    /// @see `setBackGroundText`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::String& getBackGroundText() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current character size for text
    ///
    /// @see `setTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getTextSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current input text maximum number of characters
    ///
    /// @see `getTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] std::size_t getTextLimit() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text padding ratio
    ///
    /// @see `setTextOffsetRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getTextOffsetRatio() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current font used by text
    ///
    /// @see `setTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::Font& getTextFont() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the placeholder character for input
    ///
    /// Returns 0 if no placeholder is taking place.
    ///
    /// @see `setPlaceholderChar`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] char getPlaceholderChar() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current index of the cursor
    ///
    /// @see `setCursorIndex`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getCursorIndex() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current width of the cursor
    ///
    /// @see `setCursorWidth`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getCursorWidth() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the duration in seconds for the cursor uptime
    ///
    /// If the cursor's visible period is 50% of the default complete period,
    /// then this function would return `0.5` for 1/2 seconds.
    ///
    /// @see `setBlinkUpRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getBlinkUpRatio() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current scroll offset in pixels
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getScrollOffset() const;

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Selects the entry and enables cursor
    ///
    ///////////////////////////////////////////////////////////////////
    bool postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Enters text to the entry
    ///
    ///////////////////////////////////////////////////////////////////
    bool postProcessKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget on master object
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Restarts the indicator clock if toggled
    ///
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the widget,
    /// called in `setWidgetPosition`
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
    ///////////////////////////////////////////////////////////////////
    void updateState() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs shared theme with widget
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the content visible within the entry
    ///
    ///////////////////////////////////////////////////////////////////
    void writeToEntry();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws cursor while updating it's designated clock
    ///
    ///////////////////////////////////////////////////////////////////
    void drawCursorLine();

    ///////////////////////////////////////////////////////////////////
    /// @brief Positions cursor to the closest index to x_pos
    ///
    /// x_pos should be in local view as the method considers all current 
    /// offsets.
    ///
    ///////////////////////////////////////////////////////////////////
    void moveCursorIndex(float x_pos);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current x coordinate of the cursor
    ///
    ///////////////////////////////////////////////////////////////////
    float getCursorCoordinate() const;

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
/// Member Variables
///////////////////////////////////////////////////////////////////

    // drawing
    sf::RenderTexture m_renderTexture;                                  //!< Main drawing texture
    sf::Sprite m_sprite;                                                //!< Texture's sprite
    std::unique_ptr<const sf::Font> m_font;                             //!< Optional local font
    sf::Text m_text;                                                    //!< Entry text
    sf::Text m_bgText;                                                  //!< Background text, capped at 256 chars
    
    // data
    sf::String m_textBuffer;                                            //!< Entry content
    std::size_t m_cursorIndex{0};                                       //!< Points to char in string index + 1, so m_textBuffer[0] = index 1
    std::function<bool(char32_t*)> m_inputFilter{nullptr};              //!< Active input filter for entry, this include `set_input` method as well

    // parameters
    std::size_t m_inputLimit{parameters::CHAR_LIMIT};                   //!< Max possible number of characters, default is `parameters::CHAR_LIMIT`
    float m_textSizeRatio{0.5f};                                        //!< Entry determines char size based on it's height, default is `0.5f`, `range=[0.f, 1.f[`
    float m_textOffsetRatio{0.05f};                                     //!< Left offset for text, default is `0.05f`, `range=[0.f, 1.f[`
    unsigned int m_charSize{parameters::CHAR_SIZE};                     //!< Entry's char size, default is `parameters::CHAR_SIZE`
    unsigned int m_cursorWidth{1};                                      //!< The width of the cursor, default is `1`
    char m_placeholderCharacter{0};                                     //!< The visual text is replaced by this char, `0` char disables the replacement
    sf::Time m_cursorOnDuration;                                        //!< Uptime period for cursor
    sf::Time m_cursorOffDuration;                                       //!< Downtime period for cursor

    // funcionality
    sf::Clock m_blinkTimer;                                             //!< Cursor blinking timer
    float m_textBoxWidth{0.f};                                          //!< Entry's text complete width
    float m_currentPanOffset{0.f};                                      //!< Current panned value
    float m_maxPanOffset{0.f};                                          //!< Max possible space for panning
    EntryStyle m_style{Theme::entry_style::ENTRY_STALE_VIOLET};         //!< Entry's default style

    static constexpr sf::Time s_BLINK_PERIOD = sf::milliseconds(1000);  //!< Blinking indicators period
    static constexpr float s_SCROLL_RATIO{0.3f};                        //!< Scrolls by 30% of the entry's width on each scroll event
    static constexpr std::size_t s_MAX_BG_TEXT_SIZE{256};               //!< Max length for background text
};

template <>
struct WidgetStyle<Entry>
{
    using StyleType = EntryStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_entryTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_entry)
    {
        theme.m_entryTheme = s_entry;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_entryToken;
    }
};
}
#endif