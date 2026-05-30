#include "../include/checkbox.hpp"

namespace sfml_widgets // checkbox
{
CheckBox::CheckBox(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, bool initial_state) : Widget(root, position, size), m_text(parameters::font)
{
    m_currentBgColorPtr = &m_style.fill_off_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentTextColorPtr = &m_style.text_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    setFlag(m_flags, FLAGS::CLIPSHADER); //!< Allow radius update
    if(initial_state) setFlag(m_flags, FLAGS::SET);
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
}

CheckBox::CheckBox(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text, bool initial_state) : Widget(root, position, size), m_text(parameters::font)
{
    m_currentBgColorPtr = &m_style.fill_off_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentTextColorPtr = &m_style.text_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    
    setFlag(m_flags, FLAGS::CLIPSHADER); //!< Allow radius update
    if(initial_state) setFlag(m_flags, FLAGS::SET);
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    setText(text);
}

// CheckBox::CheckBox(CheckBox&& other) noexcept : Widget(std::move(other)), m_text(std::move(other.m_text)), m_style(other.m_style), m_textSide(other.m_textSide)
// {
//     m_currentBgColorPtr = &m_style.fill_off_color;
//     m_currentOutlineColorPtr = &m_style.outline_color;
//     m_currentTextColorPtr = &m_style.text_color;
//     m_currentOutlineThicknessPtr = &m_style.outline_thickness;

// }

// CheckBox& CheckBox::operator=(CheckBox&& other) noexcept
// {
//     if(this != &other)
//     {
//         static_cast<Widget&>(*this) = std::move(other);

//         m_style = other.m_style;
//         m_text = std::move(other.m_text);
//         m_textSide = other.m_textSide;
//         m_currentBgColorPtr = &m_style.fill_off_color;
//         m_currentOutlineColorPtr = &m_style.outline_color;
//         m_currentTextColorPtr = &m_style.text_color;
//         m_currentOutlineThicknessPtr = &m_style.outline_thickness;
//     }
//     return *this;
// }

bool CheckBox::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED)) return false;

    if(!mouse_position) *mouse_position = getMousePosition();
    W_MouseEventDetails details =  mouseCallBackEvent(event, *mouse_position);

    if(details.event_type == W_MouseEventType::Release && details.button == parameters::ACTIVE_MOUSE_BUTTON&& checkFlag(m_flags, FLAGS::SELECTED))
    {
        toggle(m_flags, FLAGS::SET);
        if(m_clickCallback) m_clickCallback(event);
        details.is_used = true;
    }

    return details.is_used;
}

bool CheckBox::captureKeyEvents(const sf::Event& event)
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

void CheckBox::setTextFont(sf::Font font)
{
    m_text.setFont(font);
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

void CheckBox::config(FLAGS mask)
{

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
        if(checkFlag(m_flags, FLAGS::BGIMAGE)) this->setTexture(m_bgTexturePtr.get());

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
        if(checkFlag(m_flags, FLAGS::BGIMAGE)) this->setTexture(nullptr);

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
    if(auto t = m_theme.lock())
    {
        const std::size_t synced_token = t->getToken<CheckBox>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<CheckBox>());
    }
}

void CheckBox::updateClipShader()
{
    // checkbox does not use a sprite
}

void CheckBox::updateColorShader()
{
    if(!checkFlag(m_flags, FLAGS::COLORSHADER)) return;
    bool render_texture = checkFlag(m_flags, FLAGS::BGIMAGE) && checkFlag(m_flags, FLAGS::SET); 
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

    m_bgShaderPtr->setUniform("outline_color", sf::Glsl::Vec4(*m_currentOutlineColorPtr));
    m_bgShaderPtr->setUniform("opacity", m_opacity);
    m_bgShaderPtr->setUniform("has_texture", render_texture ? 1.f : 0.f);
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