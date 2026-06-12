#include <cmath>
#include "../include/checkbox.hpp"

namespace sfml_widgets // checkbox
{
CheckBox::CheckBox(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, const sf::Font& font, bool initial_state) : Widget(root, position, size), m_text(font)
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

CheckBox::CheckBox(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, const sf::Font& font, bool initial_state) : Widget(root, position, size), m_text(font)
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

void CheckBox::postProcessMouseEvents(W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    if(details.event_type == W_MouseEventType::Release && details.button == parameters::ACTIVE_MOUSE_BUTTON&& checkFlag(m_flags, FLAGS::SELECTED))
    {
        toggle(m_flags, FLAGS::SET);
        if(m_clickCallback) m_clickCallback(event);
        details.is_used = true;
    }
}

void CheckBox::setText(const std::string& text)
{
    if(text.length() >= parameters::CHAR_LIMIT) return;

    if(!text.empty()) setFlag(m_flags, FLAGS::TEXT);
    else resetFlag(m_flags, FLAGS::TEXT);

    m_text.setString(text);
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::setTextSize(unsigned int char_size)
{
    m_textSizeRatio = 0.f;
    m_charSize = char_size;
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::setTextSize(float ratio)
{
    if(ratio < 0.f || ratio > 2.f) return;
    m_textSizeRatio = ratio;
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::setTextFont(const sf::Font& font)
{
    // outside reference of font so any internal reference gets cleared
    m_text.setFont(font);
    if(m_font) m_font.reset();

    resetFlag(m_flags, FLAGS::SHORTSKIP);

}

void CheckBox::setTextFont(const sf::Font&& font)
{
    m_font = std::make_unique<const sf::Font>(std::move(font));
    m_text.setFont(*m_font);

    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::bindToggleEvent(MouseCallback callback)
{
    m_clickCallback = callback;
}

void CheckBox::unbindToggleEvent()
{
    m_clickCallback = nullptr;
}

void CheckBox::setSpace(float ratio)
{
    if(ratio < 0.f || ratio > 1.f) return;
    m_deadSpace = ratio;
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

bool CheckBox::getState()
{
    return checkFlag(m_flags, FLAGS::SET);
}

void CheckBox::setTextSide(DIRECTION side)
{
    m_textSide = side;
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::setTheme(CheckboxStyle __checkbox_style)
{
    m_style = __checkbox_style;
    setFlag(m_flags, FLAGS::MODIFIED);
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

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
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::updateSize()
{
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::updateRotation()
{
    m_text.setRotation(this->getRotation());
    resetFlag(m_flags, FLAGS::SHORTSKIP);
}

void CheckBox::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::SET))
    {
        if(m_bgTexturePtr) this->setTexture(m_bgTexturePtr.get());

        m_currentBgColorPtr = &m_style.fill_on_color;

        if(checkFlag(m_flags, FLAGS::BLOCKED))
        {
            m_opacity = 0.2f;
            m_currentBgColorPtr->a = m_style.fill_blocked_color.a;
            m_currentOutlineColorPtr = &m_style.outline_blocked_color;
            m_currentTextColorPtr = &m_style.text_blocked_color;
        }
        else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
        {
            m_opacity = 0.1f;
            m_currentBgColorPtr->a = m_style.fill_trasnparent_color.a;
            m_currentOutlineColorPtr = &m_style.outline_transparent_color;
            m_currentTextColorPtr = & m_style.text_transparent_color;
        }
        else if(checkFlag(m_flags, FLAGS::HOVERED))
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

        if(checkFlag(m_flags, FLAGS::BLOCKED))
        {
            m_opacity = 0.2f;
            m_currentBgColorPtr = &m_style.fill_blocked_color;
            m_currentOutlineColorPtr = &m_style.outline_blocked_color;
            m_currentTextColorPtr = &m_style.text_blocked_color;
        }
        else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
        {
            m_opacity = 0.1f;
            m_currentBgColorPtr = &m_style.fill_trasnparent_color;
            m_currentOutlineColorPtr = &m_style.outline_transparent_color;
            m_currentTextColorPtr = &m_style.text_transparent_color;
        }
        else if(checkFlag(m_flags, FLAGS::HOVERED))
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
    if(checkFlag(m_flags, FLAGS::SHORTSKIP)) return;
    setFlag(m_flags, FLAGS::SHORTSKIP); //!< skipping until another update is queued

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


    const float HORIZONTAL_SPACE = m_size.x * m_deadSpace;
    const float VERTICAL_SPACE   = m_size.y * m_deadSpace;

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