#include "../include/tooltip.hpp"

namespace sfml_widgets
{
ToolTip::ToolTip(sf::RenderWindow& window, sf::Vector2f position, sf::Time ms_delay, ToolTipStyle style) : m_window(window),
m_text(parameters::font, "", parameters::CHAR_SIZE), m_hoverDuration(ms_delay)
{
    setBoxPosition(position);
    setTheme(style);
    m_timer.reset();
}
void ToolTip::setDelay(sf::Time delay_ms)
{
    m_hoverDuration = delay_ms;
}

void ToolTip::setTheme(ToolTipStyle style)
{
    m_style = style;
    this->setFillColor(m_style.fill_color);
    this->setOutlineColor(m_style.outline_color);
    this->setOutlineThickness(m_style.outline_thickness);
    m_text.setFillColor(m_style.text_color);
    m_text.setOutlineColor(m_style.text_outline_color);
    m_text.setOutlineThickness(m_style.text_outline_thickness);
}

void ToolTip::setText(sf::String text)
{
    m_text.setString(text);
    m_needUpdate = true;
}

const sf::String& ToolTip::getText() const
{
    return m_text.getString();
}

void ToolTip::clearText()
{
    m_text.setString("");
    m_needUpdate = true;
}

void ToolTip::setCornerRadius(float radius)
{
    m_radius = radius;
    m_needUpdate = true;
}

void ToolTip::setTextFont(const sf::Font& font)
{
    m_text.setFont(font);
    m_needUpdate = true;
}

void ToolTip::setTextStyle(sf::Text::Style style)
{
    m_text.setStyle(style);
    m_needUpdate = true;
}

void ToolTip::setCharacterSize(unsigned int char_size)
{
    m_text.setCharacterSize(char_size);
    m_needUpdate = true;
}

void ToolTip::setLineSpacing(float factor)
{
    m_text.setLineSpacing(factor);
    m_needUpdate = true;
}

void ToolTip::setLetterSpacing(float factor)
{
    m_text.setLetterSpacing(factor);
    m_needUpdate = true;
}

void ToolTip::setBoxPosition(sf::Vector2f position)
{
    this->setPosition(position);
    m_text.setPosition(position);
}

void ToolTip::draw(bool is_hovered)
{
    if(is_hovered)
    {
        if(!m_timer.isRunning()) m_timer.restart();
        if(m_timer.getElapsedTime() >= m_hoverDuration)
        {
            if(m_needUpdate) updateShapeBounds();
            stepTransparency();
            m_window.draw(*this);
            m_window.draw(m_text);
        }
    }
    else
    {
        m_currentLevel = 1;
        m_timer.reset();
    }
}

void ToolTip::stepTransparency()
{
    if(m_currentLevel > parameters::N_FADING_FRAMES) return;

    const float T_LEVEL = static_cast<float>(m_currentLevel) / static_cast<float>(parameters::N_FADING_FRAMES);

    std::function<sf::Color(sf::Color)> scaled = [T_LEVEL](sf::Color c)
    {
        c.a = std::round<uint8_t>(static_cast<float>(c.a) * T_LEVEL);
        return c;
    };
    this->setFillColor(scaled(m_style.fill_color));
    this->setOutlineColor(scaled(m_style.outline_color));
    m_text.setFillColor(scaled(m_style.text_color));
    m_text.setOutlineColor(scaled(m_style.text_outline_color));
    ++m_currentLevel;
}

void ToolTip::updateShapeBounds()
{
    m_needUpdate = false;

    const sf::FloatRect bounds = m_text.getLocalBounds();
    const sf::String& text = m_text.getString();
    const float line_height = m_text.getFont().getLineSpacing(m_text.getCharacterSize());

    const int n_lines = 1 + static_cast<int>
    (
        std::count(text.begin(), text.end(), static_cast<uint32_t>('\n'))
    );

    const float width = bounds.position.x + bounds.size.x;

    const float height = n_lines * line_height;

    m_size.x = std::max(s_MIN_SIZE.x, width);
    m_size.y = std::max(s_MIN_SIZE.y, height);
    m_radius = std::max(0.f, std::min(m_radius, std::min<float>(m_size.x, m_size.y) / 2.f));
    if(m_radius == 0.f)
    {
        m_radius = 0.f;
        m_cornerPoints = 1;
    }
    else
    {
        m_cornerPoints = parameters::CORNER_POINTS;
    }
    update();
}

std::size_t ToolTip::getPointCount() const
{
    return m_cornerPoints * 4;
}

sf::Vector2f ToolTip::getPoint(std::size_t index) const {
    std::size_t corner = index / m_cornerPoints;
    float angle = static_cast<float>(index) / m_cornerPoints * constants::DEGREE90;
    float rad = angle * constants::PI / constants::DEGREE180;

    // Center of each corner's arc circle
    sf::Vector2f centers[4] = {
        { m_radius, m_radius },
        { m_size.x - m_radius, m_radius },
        { m_size.x - m_radius, m_size.y - m_radius },
        { m_radius, m_size.y - m_radius }
    };

    float a = constants::START_ANGLES[corner] + static_cast<float>(index % m_cornerPoints) / m_cornerPoints * (constants::PI / 2.f);

    return {
        centers[corner].x + std::cos(a) * m_radius,
        centers[corner].y + std::sin(a) * m_radius
    };
}
}