#include <cmath>
#include "label.hpp"

namespace sfml_widgets
{
Label::Label(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text, const sf::Font& font) : Widget(root, position, size), 
m_text(font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.text_outline_color;
    m_currentOutlineThicknessPtr = &m_style.text_outline_thickness;

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
    this->setOutlineThickness(0.f);
}

Label::Label(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text, const sf::Font& font) : Widget(root, position, size),
m_text(font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.text_outline_color;
    m_currentOutlineThicknessPtr = &m_style.text_outline_thickness;

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
    this->setOutlineThickness(0.f);
}

void Label::setText(const sf::String& text)
{
    m_text.setString(text.substring(0, std::min(m_charLimit, text.getSize())));

    if(!m_text.getString().isEmpty()) setFlag(m_flags, FLAGS::TEXT);
    else resetFlag(m_flags, FLAGS::TEXT);

    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setTextLimit(unsigned int char_limit)
{
    m_charLimit = char_limit;
    m_text.setString(m_text.getString().substring(0, std::min(m_charLimit, m_text.getString().getSize())));
}


void Label::setTextSize(unsigned int char_size)
{
    m_textSizeRatio = 0.f;
    m_charSize = char_size;

    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setTextSize(float ratio)
{
    if(ratio <= 0.f || ratio >= 1.f) return;
    m_textSizeRatio = ratio;
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setTextFont(const sf::Font& font)
{
    m_text.setFont(font);
    if(m_font) m_font.reset();
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setTextFont(sf::Font&& font)
{
    m_font = std::make_unique<const sf::Font>(std::move(font));
    m_text.setFont(*m_font);
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setTextStyle(sf::Text::Style style)
{
    m_text.setStyle(style);
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setHorizontalAlignment(HAlign h)
{
    m_horizontalAlign = h;
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setVerticalAlignment(VAlign v)
{
    m_verticalAlign = v;
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setLineSpacing(float factor)
{
    m_text.setLineSpacing(factor);
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setLetterSpacing(float factor)
{
    m_text.setLetterSpacing(factor);
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setPaddingRatio(float ratio)
{
    if(ratio < 0.f || ratio > 1.f) return;
    m_paddingRatio = ratio;
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::setTheme(LabelStyle s_label)
{
    m_style = s_label;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Label::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<Label>());
}

LabelStyle& Label::style()
{
    return m_style;
}

const sf::String& Label::getText() const
{
    return m_text.getString();
}

std::size_t Label::getTextLimit() const
{
    return m_charLimit;
}

unsigned int Label::getTextSize() const
{
    return m_charSize;
}

const sf::Font& Label::getTextFont() const
{
    return m_text.getFont();
}

sf::Text::Style Label::getTextStyle() const
{
    return static_cast<sf::Text::Style>(m_text.getStyle());
}

Label::HAlign Label::getHorizontalAlignment() const
{
    return m_horizontalAlign;
}

Label::VAlign Label::getVerticalAlignment() const
{
    return m_verticalAlign;
}

float Label::getLineSpacing() const
{
    return m_text.getLineSpacing();
}

float Label::getLetterSpacing() const
{
    return m_text.getLetterSpacing();
}

float Label::getPaddingRatio() const
{
    return m_paddingRatio;
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void Label::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::UPDATE))
    {
        resetFlag(m_flags, FLAGS::UPDATE);
        updateTextSize();
        updateTextPosition();
    }

    if(checkFlag(m_flags, FLAGS::TEXT)) m_master.draw(m_text);
}

void Label::updatePosition()
{
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::updateSize()
{
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::updateRotation()
{
    m_text.setRotation(this->getRotation());
    setFlag(m_flags, FLAGS::UPDATE);
}

void Label::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.5f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.text_outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_blocked_thickness;
        m_text.setFillColor(m_style.text_blocked_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.25f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.text_outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_transparent_thickness;
        m_text.setFillColor(m_style.text_transparent_color);
    }
    else if(!m_stale && checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.text_outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_active_thickness;
        m_text.setFillColor(m_style.text_active_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
    {
        m_opacity = 0.9f;
        m_currentBgColorPtr = &m_style.fill_hover_color;
        m_currentOutlineColorPtr = &m_style.text_outline_hover_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_hover_thickness;
        m_text.setFillColor(m_style.text_hover_color);
    }
    else
    {
        m_opacity = 1.f;
        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.text_outline_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_thickness;
        m_text.setFillColor(m_style.text_color);
    }
    this->setFillColor(*m_currentBgColorPtr);
    this->setOutlineColor(*m_currentOutlineColorPtr);
    this->setOutlineThickness(*m_currentOutlineThicknessPtr);
    updateColorShader();
}

void Label::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<Label>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Label>());
    }
}

void Label::updateTextSize()
{
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    if(m_textSizeRatio > 0.f)
    {
        float target_height  = m_size.y * m_textSizeRatio;
        // single line scale
        float current_height = W_CharHeight::getHeight(m_text.getFont(),
                                                      m_text.getCharacterSize());
        if(current_height > 0.f)
        {
            float ratio = target_height / current_height;
            m_charSize  = std::floor<unsigned int>(m_text.getCharacterSize() * ratio);
        }
    }

    m_text.setCharacterSize(m_charSize);
    updateShapeBounds();
}

void Label::updateTextPosition()
{
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    sf::FloatRect bounds = m_text.getLocalBounds();
    m_text.setOrigin(bounds.position + bounds.size / 2.f);

    // all local constants
    const float SHAPE_CENTER = m_size.x / 2.f;      //!< shape horizontal center
    const float SHAPE_MIDDLE = m_size.y / 2.f;      //!< shape vertical center
    const float TEXT_CENTER = bounds.size.x / 2.f;  //!< text horizontal center
    const float TEXT_MIDDLE = bounds.size.y / 2.f;  //!< text vertical center
    const float DIFF = 1.f - m_paddingRatio;        //!< reverse padding value, makes sure that text is centered if padding = 1.f

    sf::Vector2f local;

    switch(m_horizontalAlign)
    {
    case HAlign::Left:
        // DIFF = 0 --> considers shape center only (CENTERED)
        // DIFF = 1 --> shifts text by half of it's size to keep it inside the shape (MAX LEFT)
        local.x = TEXT_CENTER * DIFF + SHAPE_CENTER * m_paddingRatio;
        break;
    case HAlign::Center:
        // placed in the center of the shape
        local.x = SHAPE_CENTER;
        break;
    case HAlign::Right:
        // DIFF = 0 --> considers shape center only (CENTERED)
        // DIFF = 1 --> shifts text by shape's size.x value then sub 1/2 * text_size.x to keep it inside the shape (MAX RIGHT)
        local.x = (m_size.x - TEXT_CENTER) * DIFF + SHAPE_CENTER * m_paddingRatio;
        break;
    }

    switch(m_verticalAlign)
    {
    case VAlign::Top:
        local.y = TEXT_MIDDLE * DIFF + SHAPE_MIDDLE * m_paddingRatio;
        break;
    case VAlign::Middle:
        local.y = SHAPE_MIDDLE;
        break;
    case VAlign::Bottom:
        local.y = (m_size.y - TEXT_MIDDLE) * DIFF + SHAPE_MIDDLE * m_paddingRatio;
        break;
    }

    // scales with shape's local coordinates and considers rotation value
    m_text.setPosition(this->getTransform().transformPoint(local));
}

void Label::updateShapeBounds()
{
    const sf::FloatRect TEXT_BOUNDS = m_text.getLocalBounds();
    const sf::String& TEXT = m_text.getString();

    // single line's pixel height
    const float LINE_HEIGHT = m_text.getFont().getLineSpacing(m_text.getCharacterSize());

    // count the number of lines in label
    const int N_LINES = 1 + static_cast<int>
    (
        std::count(TEXT.begin(), TEXT.end(), static_cast<uint32_t>('\n'))
    );

    // considers the full horizontal space of text
    const float WIDTH = TEXT_BOUNDS.position.x + TEXT_BOUNDS.size.x;
    const float HEIGHT = N_LINES * LINE_HEIGHT;

    // minimum size of the shape considers padding ratio
    m_size.x = std::max(m_size.x, WIDTH + 0.5f * m_size.x * m_paddingRatio);
    m_size.y = std::max(m_size.y, HEIGHT + 0.5f * m_size.y * m_paddingRatio);

    m_radius = std::max(0.f, std::min(m_radius, std::min<float>(m_size.x, m_size.y) / 2.f));
    update();
}
}