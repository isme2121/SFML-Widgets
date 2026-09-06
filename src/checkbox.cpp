#include <cmath>
#include "checkbox.hpp"

namespace sfml_widgets // checkbox
{
CheckBox::CheckBox(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, bool initial_state, const sf::Font& font) : Widget(root, position, size),
m_text(font, "", parameters::CHAR_SIZE)
{
    m_currentBgColorPtr = &m_style.fill_off_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentTextColorPtr = &m_style.text_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    if(initial_state) setFlag(m_flags, FLAGS::SET);
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
}

CheckBox::CheckBox(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, bool initial_state, const sf::Font& font) : Widget(root, position, size),
m_text(font, "", parameters::CHAR_SIZE)
{
    m_currentBgColorPtr = &m_style.fill_off_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentTextColorPtr = &m_style.text_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    
    if(initial_state) setFlag(m_flags, FLAGS::SET);
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
}

void CheckBox::bindToggleEvent(MouseCallback callback)
{
    m_clickCallback = callback;
}

void CheckBox::unbindToggleEvent()
{
    m_clickCallback = nullptr;
}

void CheckBox::setText(const sf::String& text)
{
    m_text.setString(text.substring(0, std::min(parameters::CHAR_LIMIT, static_cast<unsigned int>(text.getSize()))));

    if(!text.isEmpty()) setFlag(m_flags, FLAGS::TEXT);
    else resetFlag(m_flags, FLAGS::TEXT);

    setFlag(m_flags, FLAGS::UPDATE);
}

bool CheckBox::setTextSize(unsigned int char_size)
{
    m_textSizeRatio = 0.f;  //!< disables ratio updating
    m_charSize = char_size;
    setFlag(m_flags, FLAGS::UPDATE);
    return true;
}

bool CheckBox::setTextSize(float ratio)
{
    if(ratio < 0.f || ratio > 2.f) return false;
    m_textSizeRatio = ratio;
    setFlag(m_flags, FLAGS::UPDATE);
    return true;
}

bool CheckBox::setTextLimit(unsigned int max_size)
{
    if(max_size > 30) return false;
    m_charLimit = max_size;
    return true;
}

void CheckBox::setTextFont(const sf::Font& font)
{
    // outside reference of font so any internal reference gets cleared
    m_text.setFont(font);
    if(m_font) m_font.reset();

    setFlag(m_flags, FLAGS::UPDATE);

}

void CheckBox::setTextFont(const sf::Font&& font)
{
    m_font = std::make_unique<const sf::Font>(std::move(font));
    m_text.setFont(*m_font);

    setFlag(m_flags, FLAGS::UPDATE);
}

void CheckBox::setTextPosition(DIRECTION side)
{
    m_textSide = side;
    setFlag(m_flags, FLAGS::UPDATE);
}

bool CheckBox::setTextPadding(float ratio)
{
    if(ratio < 0.f || ratio > 5.f) return false;
    m_padding = ratio;
    setFlag(m_flags, FLAGS::UPDATE);
    return true;
}

void CheckBox::setTheme(CheckboxStyle s_checkbox)
{
    m_style = s_checkbox;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void CheckBox::fetchTheme()
{
    if(auto t = m_themePtr.lock())
    {
        setTheme(t->getStyle<CheckBox>());
        setFlag(m_flags, FLAGS::MODIFIED);
    }
}

CheckboxStyle& CheckBox::style()
{
    return m_style;
}

bool CheckBox::getState() const
{
    return checkFlag(m_flags, FLAGS::SET);
}

unsigned int CheckBox::getTextSize() const
{
    return m_charSize;
}

unsigned int CheckBox::getTextLimit() const
{
    return m_charLimit;
}

float CheckBox::getTextPadding() const
{
    return m_padding;
}

DIRECTION CheckBox::getTextPosition() const
{
    return m_textSide;
}

const sf::Font& CheckBox::getTextFont() const
{
    return m_text.getFont();
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

bool CheckBox::postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    // checkbox state update
    if(details.event_type == W_MouseEventType::Release && details.button == s_FOCUS_BUTTON && checkFlag(m_flags, FLAGS::SELECTED))
    {
        toggle(m_flags, FLAGS::SET);
        if(m_clickCallback) m_clickCallback(event);
        return true;
    }
    return false;
}

void CheckBox::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::TEXT))
    {
        updateText();
        m_master.draw(m_text);
    }
}

void CheckBox::updatePosition()
{
    setFlag(m_flags, FLAGS::UPDATE);
}

void CheckBox::updateSize()
{
    setFlag(m_flags, FLAGS::UPDATE);
}

void CheckBox::updateRotation()
{
    m_text.setRotation(this->getRotation());
    setFlag(m_flags, FLAGS::UPDATE);
}

void CheckBox::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::SET))
    {
        // show image if assigned when checkbox = 1
        if(m_bgTexturePtr) this->setTexture(m_bgTexturePtr.get());
        if(m_shaderHolder) m_shaderHolder.swap(m_bgShaderPtr);

        m_currentBgColorPtr = &m_style.fill_on_color;

        if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
        {
            m_opacity = 0.4f;
            m_currentBgColorPtr->a = m_style.fill_blocked_color.a;
            m_currentOutlineColorPtr = &m_style.outline_blocked_color;
            m_currentTextColorPtr = &m_style.text_blocked_color;
        }
        else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
        {
            m_opacity = 0.2f;
            m_currentBgColorPtr->a = m_style.fill_trasnparent_color.a;
            m_currentOutlineColorPtr = &m_style.outline_transparent_color;
            m_currentTextColorPtr = & m_style.text_transparent_color;
        }
        else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
        {
            m_opacity = 0.9f;
            m_currentBgColorPtr->a = m_style.fill_hover_color.a;
            m_currentOutlineColorPtr = &m_style.outline_hover_color;
            m_currentTextColorPtr = & m_style.text_hover_color;
        }
        else
        {
            m_opacity = 1.f;
            m_currentOutlineColorPtr = &m_style.outline_color;
            m_currentTextColorPtr = & m_style.text_color;
        }
    }
    else
    {
        if(m_bgTexturePtr) this->setTexture(nullptr);
        if(m_bgShaderPtr) m_bgShaderPtr.swap(m_shaderHolder);

        if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
        {
            m_opacity = 0.4f;
            m_currentBgColorPtr = &m_style.fill_blocked_color;
            m_currentOutlineColorPtr = &m_style.outline_blocked_color;
            m_currentTextColorPtr = &m_style.text_blocked_color;
        }
        else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
        {
            m_opacity = 0.2f;
            m_currentBgColorPtr = &m_style.fill_trasnparent_color;
            m_currentOutlineColorPtr = &m_style.outline_transparent_color;
            m_currentTextColorPtr = &m_style.text_transparent_color;
        }
        else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
        {
            m_opacity = 0.9f;
            m_currentBgColorPtr = &m_style.fill_hover_color;
            m_currentOutlineColorPtr = &m_style.outline_hover_color;
            m_currentTextColorPtr = &m_style.text_hover_color;
        }
        else
        {
            m_currentBgColorPtr = &m_style.fill_off_color;
            m_currentOutlineColorPtr = &m_style.outline_color;
            m_currentTextColorPtr = &m_style.text_color;
        }
    }

    this->setFillColor(*m_currentBgColorPtr);
    this->setOutlineColor(*m_currentOutlineColorPtr);
    this->setOutlineThickness(*m_currentOutlineThicknessPtr);
    if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(*m_currentTextColorPtr);
    updateColorShader();
}

void CheckBox::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<CheckBox>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<CheckBox>());
    }
}

void CheckBox::updateText()
{
    if(!checkFlag(m_flags, FLAGS::UPDATE)) return;
    resetFlag(m_flags, FLAGS::UPDATE); //!< skipping until another update is queued

    // update text size first for accurate local bounds
    if(m_textSizeRatio != 0.f)
    {
        float final_height, current_height;
        final_height = m_size.y * m_textSizeRatio;
        current_height = W_CharHeight::getHeight(m_text.getFont(), m_text.getCharacterSize());

        if(current_height > 0.f)
        {
            float ratio = final_height / current_height;
            m_charSize = std::floor<unsigned int>(m_text.getCharacterSize() * ratio);
        }
    }
    m_text.setCharacterSize(m_charSize);


    const float HORIZONTAL_SPACE = m_size.x * m_padding;
    const float VERTICAL_SPACE   = m_size.y * m_padding;

    sf::FloatRect bounds = m_text.getLocalBounds();


    sf::Vector2f offset; //!< offset from 0, 0 coords of checkbox
    sf::Vector2f text_origin; //!< origin point of the text space

    switch(m_textSide)
    {
    case DIRECTION::RIGHT:
        {
            text_origin = {bounds.position.x,
                           bounds.position.y + bounds.size.y / 2.f}; //!< mid-left point

            offset = {m_size.x + HORIZONTAL_SPACE,
                      m_size.y / 2.f};
            break;
        }

    case DIRECTION::LEFT:
        {
            text_origin = {bounds.position.x + bounds.size.x,
                           bounds.position.y + bounds.size.y / 2.f}; //!< mid-right point

            offset = {-HORIZONTAL_SPACE,
                      m_size.y / 2.f};
            break;
        }

    case DIRECTION::TOP:
        {
            text_origin = {bounds.position.x + bounds.size.x / 2.f,
                           bounds.position.y + bounds.size.y}; //!< bottom-mid point

            offset = {m_size.x / 2.f,
                      -VERTICAL_SPACE};
            break;
        }

    case DIRECTION::BOTTOM:
        {
            text_origin = {bounds.position.x + bounds.size.x / 2.f,
                           bounds.position.y}; //!< top-mid point

            offset = {m_size.x / 2.f,
                      m_size.y + VERTICAL_SPACE};
            break;
        }
    }

    m_text.setOrigin(text_origin);
    m_text.setPosition(this->getTransform().transformPoint(offset));
}
}