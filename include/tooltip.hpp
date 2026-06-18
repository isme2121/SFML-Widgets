#pragma once
#include "widget_themes.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Integrated in widget, pops a text box after hovering
/// on a widget
///
/// Initiated from `Widget::EnableToolTip` and exists until it's
/// disabled or the widget is destroyed.
///
///////////////////////////////////////////////////////////////////
class ToolTip : private sf::Shape
{
    friend class Widget;

public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the hovering duration for the tooltip to appear
    ///
    /// @param delay_ms delay in milliseconds
    ///
    ///////////////////////////////////////////////////////////////////
    void setDelay(sf::Time delay_ms);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the ToolTip theme
    ///
    /// @param style style object
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(ToolTipStyle style);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the content of the text box
    ///
    /// @param text new text content
    ///
    /// @see `getText`, `clearText`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(sf::String text);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text content
    ///
    /// @see `setText`, `clearText`
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::String& getText() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Resets text content
    ///
    /// @see `setText`, `getText`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearText();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the ToolTip position relative to local top left
    /// coordinates
    ///
    /// @param position new pixel position
    ///
    ///////////////////////////////////////////////////////////////////
    void setBoxPosition(sf::Vector2f position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the shape with rounded corners based on the radius
    ///
    /// Set to `0.f` for rectangular corners, the radius limit depends on
    /// the size of the box and it is capped internally.
    ///
    /// @param radius float radius value
    ///
    ///////////////////////////////////////////////////////////////////
    void setCornerRadius(float radius);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the used font
    ///
    /// @param font existing font object
    ///
    /// @warning font object must outlive the ToolTip object itself
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the style of text
    ///
    /// @param style `sf::Text::Style` enum value
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextStyle(sf::Text::Style style);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the text
    ///
    /// This indirectly updates the surrounding shape's size as well
    ///
    /// @param char_size character size
    ///
    ///////////////////////////////////////////////////////////////////
    void setCharacterSize(unsigned int char_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the vertical space between lines that is defined
    /// by the font
    ///
    /// @param factor default is `1.f`
    ///
    ///////////////////////////////////////////////////////////////////
    void setLineSpacing(float factor);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the horizontal space between characters that is
    /// defined by the font
    ///
    /// @param factor default is `1.f`
    ///
    ///////////////////////////////////////////////////////////////////
    void setLetterSpacing(float factor);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs ToolTip with it's assigned widget's root window
    ///
    /// @param position initial position relative to 0, 0 local coords
    /// @param ms_delay hover duration to trigger, can be changed later
    /// @param style ToolTip's initial theme
    ///
    ///////////////////////////////////////////////////////////////////
    ToolTip(sf::RenderWindow& window, sf::Vector2f position, sf::Time ms_delay, ToolTipStyle style);

    ToolTip(const ToolTip&) = delete;
    ToolTip(ToolTip&&) = delete;
    ToolTip& operator=(const ToolTip&) = delete;
    ToolTip& operator=(ToolTip&&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks hover duration and draws the object
    ///
    /// @param is_hovered `HOVERED` flag in widget
    ///
    ///////////////////////////////////////////////////////////////////
    void draw(bool is_hovered);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates a slow fade effect when drawing the ToolTip, called
    /// in `draw`
    ///
    ///////////////////////////////////////////////////////////////////
    void stepTransparency();

    ///////////////////////////////////////////////////////////////////
    /// @brief Expands shape bounds to cover the entire text box
    ///
    ///////////////////////////////////////////////////////////////////
    void updateShapeBounds();

    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;

    sf::RenderWindow& m_window;                                                 //!< Caller widget's root window
    sf::Text m_text;                                                            //!< Text
    sf::Vector2f m_size;                                                        //!< Shape's dimensions
    sf::Clock m_timer;                                                          //!< Hovering timer
    sf::Time m_hoverDuration;                                                   //!< Hover duration to reach before drawing
    float m_radius = 0.f;                                                       //!< Corner radius
    std::size_t m_cornerPoints = 1;                                             //!< Number of points per corner
    unsigned int m_currentLevel = 1;                                            //!< Used when gradually stepping transparency down, up to `s_FRAMES_TO_DRAW`
    bool m_needUpdate = true;                                                   //!< Calls for a shape update when true
    ToolTipStyle m_style;                                                       //!< Theme
    
    static constexpr sf::Vector2f s_MIN_SIZE = {50.f, 20.f};                    //!< Size when no text is written
    static constexpr unsigned int s_FRAMES_TO_DRAW = 120;                       //!< Number of frames for tooltip to appear
    static constexpr unsigned int s_CORNER_POINTS = Widget::s_CORNER_POINTS;    //!< Number of vertices in a single rounded corner

};
}
#endif