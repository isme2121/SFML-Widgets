#include "../include/label.hpp"

namespace sfml_widgets
{
Label::Label(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, const sf::Font& font) : Widget(root, position, size), 
m_text(font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.text_outline_color;
    m_currentOutlineThicknessPtr = &m_style.text_outline_thickness;

    setFlag(m_flags, FLAGS::CLIPSHADER); //!< Allow radius update
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
    this->setOutlineThickness(0.f);
}

Label::Label(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, const sf::Font& font) : Widget(root, position, size),
m_text(font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.text_outline_color;
    m_currentOutlineThicknessPtr = &m_style.text_outline_thickness;

    setFlag(m_flags, FLAGS::CLIPSHADER); //!< Allow radius update
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
    this->setOutlineThickness(0.f);
}

bool Label::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED)) return false;

    if(!mouse_position) *mouse_position = getMousePosition();
    return mouseCallBackEvent(event, *mouse_position).is_used;
}

bool Label::captureKeyEvents(const sf::Event& event)
{
    if(checkFlag(m_flags, FLAGS::BLOCKED | FLAGS::RESTRICTED) || !checkFlag(m_flags, FLAGS::AUTOSELECT | FLAGS::SELECTED)) return false;

    if(m_binds.checkEvents(event)) //!< triggers callback
    {
        if(parameters::QUEUE_DELAY != sf::Time::Zero)
        {
            // rate limit
            setFlag(m_flags, FLAGS::RESTRICTED);
            addToQueue(parameters::QUEUE_DELAY, [this]()
            {
                resetFlag(m_flags, FLAGS::RESTRICTED);
            }, 
            true);
        }
        return true;
    }
    return false;
}

void Label::setTheme(LabelStyle label_style)
{
    m_style = label_style;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Label::setText(sf::String text)
{
    text = text.substring(0, std::min(m_charLimit, text.getSize()));

    if(!text.isEmpty()) setFlag(m_flags, FLAGS::TEXT);
    else resetFlag(m_flags, FLAGS::TEXT);

    m_text.setString(text);
    updateTextSize();
    updateTextPosition();
}

void Label::setTextSize(unsigned int char_size)
{
    m_textSizeRatio = 0.f;
    m_charSize = char_size;

    updateTextSize();
    updateTextPosition();
}

void Label::setTextSize(float ratio)
{
    if(ratio <= 0.f || ratio >= 1.f) return;
    m_textSizeRatio = ratio;
    updateTextSize();
    updateTextPosition();
}

void Label::setTextFont(const sf::Font& font)
{
    m_text.setFont(font);
    if(m_font) m_font.reset();
    updateTextSize();
    updateTextPosition();
}

void Label::setTextFont(sf::Font&& font)
{
    m_font = std::make_unique<const sf::Font>(std::move(font));
    m_text.setFont(*m_font);
    updateTextSize();
    updateTextPosition();
}

void Label::setTextStyle(sf::Text::Style style)
{
    m_text.setStyle(style);
    updateTextSize();
    updateTextPosition();
}

void Label::setHorizontalAlignment(HAlign h)
{
    m_horizontalAlign = h;
    updateTextPosition();
}

void Label::setVerticalAlignment(VAlign v)
{
    m_verticalAlign = v;
    updateTextPosition();
}

void Label::setLineSpacing(float factor)
{
    m_text.setLineSpacing(factor);
    updateTextSize();
    updateTextPosition();
}

void Label::setLetterSpacing(float factor)
{
    m_text.setLetterSpacing(factor);
    updateTextSize();
    updateTextPosition();
}

void Label::setPaddingRatio(float ratio)
{
    if(ratio < 0.f || ratio > 0.5f) return;
    m_paddingRatio = ratio;
    updateTextSize();
    updateTextPosition();
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void Label::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::TEXT)) m_master.draw(m_text);
}

void Label::config(FLAGS mask)
{

}

void Label::updatePosition()
{
    updateTextPosition();
}

void Label::updateSize()
{
    updateTextSize();
    updateTextPosition(); //!< keep text aligned
    update();
}

void Label::updateRotation()
{
    m_text.setRotation(this->getRotation());
    updateTextPosition();
}

void Label::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.2f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.text_outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_blocked_thickness;
        m_text.setFillColor(m_style.text_blocked_color);
    }
    else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.text_outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_transparent_thickness;
        m_text.setFillColor(m_style.text_transparent_color);
    }
    else if(checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
    {
        m_opacity = 0.8f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.text_outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.text_outline_active_thickness;
        m_text.setFillColor(m_style.text_active_color);
    }
    else if(checkFlag(m_flags, FLAGS::HOVERED))
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
    if(auto t = m_theme.lock())
    {
        const std::size_t synced_token = t->getToken<Label>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Label>());
    }
}

void Label::updateClipShader()
{
    // no clip shader
}

void Label::updateColorShader()
{
    if(!checkFlag(m_flags, FLAGS::COLORSHADER)) return;
    bool render_texture = checkFlag(m_flags, FLAGS::BGIMAGE); 

    // for background image texture
    m_bgShaderPtr->setUniform("texture", sf::Shader::CurrentTexture);
    if(render_texture)
    {
        m_bgShaderPtr->setUniform("bg_color",
            sf::Glsl::Vec4(m_currentBgColorPtr->r / 255.f,
                           m_currentBgColorPtr->g / 255.f,
                           m_currentBgColorPtr->b / 255.f,
                           m_currentBgColorPtr->a / 255.f));
    }
    else
    {
        m_bgShaderPtr->setUniform("bg_color", sf::Glsl::Vec4(*m_currentBgColorPtr));
    }
    m_bgShaderPtr->setUniform("opacity", m_opacity);
    m_bgShaderPtr->setUniform("outline_color", sf::Glsl::Vec4(*m_currentOutlineColorPtr));
    m_bgShaderPtr->setUniform("has_texture", render_texture ? 1.f : 0.f);
}

void Label::updateTextSize()
{
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    if(m_textSizeRatio > 0.f)
    {
        float targetHeight  = m_size.y * m_textSizeRatio;
        float currentHeight = W_CharHeight::getHeight(m_text.getFont(),
                                                      m_text.getCharacterSize());
        if(currentHeight > 0.f)
        {
            float ratio = targetHeight / currentHeight;
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

    const float padX = m_size.x * m_paddingRatio;
    const float padY = m_size.y * m_paddingRatio;

    sf::Vector2f local;

    switch(m_horizontalAlign)
    {
    case HAlign::Left:
        local.x = padX + bounds.size.x / 2.f;
        break;
    case HAlign::Center:
        local.x = m_size.x / 2.f;
        break;
    case HAlign::Right:
        local.x = m_size.x - padX - bounds.size.x / 2.f;
        break;
    }

    switch(m_verticalAlign)
    {
    case VAlign::Top:
        local.y = padY + bounds.size.y / 2.f;
        break;
    case VAlign::Middle:
        local.y = m_size.y / 2.f;
        break;
    case VAlign::Bottom:
        local.y = m_size.y - padY - bounds.size.y / 2.f;
        break;
    }

    m_text.setPosition(this->getTransform().transformPoint(local));
}

void Label::updateShapeBounds()
{
    sf::Vector2f size = m_text.getLocalBounds().size;
    m_size.x = std::max(m_size.x, size.x + 2.f * m_size.x * m_paddingRatio);
    m_size.y = std::max(m_size.y, size.y + 2.f * m_size.y * m_paddingRatio);
}
}