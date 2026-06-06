#pragma once
#include "widget_themes.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Integrated in widget, shows a text screen after hovering
/// on a widget
///
/// 1. a ToolTip is initiated after calling `Widget::addToolTip` which
///    returns a weak_ptr of that internal object
///
/// 2. user can then add text to that object which is displayed once
///    the ToolTip appears
///
/// 3. ToolTip saves it's style and content until the widget is destroyed
///    or calls `Widget::removeToolTip`
///
/// 4. position is configured by the user however, it is relative to
///    the root window so for accurate representation, use the absolute
///    position instead.
///
///////////////////////////////////////////////////////////////////
class ToolTip : private sf::Shape
{
public:
    void setDelay(sf::Time delay_ms);
    void setStyle(ToolTipStyle style);
    void setText(sf::String text);
    const sf::String& getText() const;
    void clearText();

    void setBoxPosition(sf::Vector2f position);
    void setCornerRadius(float radius);

    void setTextFont(const sf::Font& font);
    void setTextStyle(sf::Text::Style style);
    void setCharacterSize(unsigned int char_size);
    void setLineSpacing(float factor);
    void setLetterSpacing(float factor);

private:
    ToolTip(sf::RenderWindow& window, sf::Vector2f position, sf::Time ms_delay, ToolTipStyle style);
    ToolTip(const ToolTip&) = delete;
    ToolTip(ToolTip&&) = delete;
    ToolTip& operator=(const ToolTip&) = delete;
    ToolTip& operator=(ToolTip&&) = delete;

    void draw(bool is_hovered);
    void stepTransparency();
    void updateShapeBounds();

    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;

    sf::RenderWindow& m_window;
    sf::Text m_text;
    ToolTipStyle m_style;
    bool m_needUpdate = true;

    sf::Vector2f m_size;
    float m_radius = 0.f;
    std::size_t m_cornerPoints = 16;

    sf::Clock m_timer;
    sf::Time m_hoverDuration = parameters::HOVER_DURATION;

    unsigned int m_currentLoop = 1;

    friend class Widget;
};
}
#endif