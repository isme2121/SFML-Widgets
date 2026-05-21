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
    setWidgetPosition(position);
    setWidgetSize(size);
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
    setWidgetPosition(position);
    setWidgetSize(size);
    setText(text);
}

CheckBox::CheckBox(CheckBox&& other) noexcept : Widget(std::move(other)), m_text(std::move(other.m_text)), m_style(other.m_style), m_textSide(other.m_textSide)
{
    m_currentBgColorPtr = &m_style.fill_off_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentTextColorPtr = &m_style.text_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

}

CheckBox& CheckBox::operator=(CheckBox&& other) noexcept
{
    if(this != &other)
    {
        static_cast<Widget&>(*this) = std::move(other);

        m_style = other.m_style;
        m_text = std::move(other.m_text);
        m_textSide = other.m_textSide;
        m_currentBgColorPtr = &m_style.fill_off_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        m_currentTextColorPtr = &m_style.text_color;
        m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    }
    return *this;
}

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
        if(parameters::QUEUE_DELAY != 0ms)
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
    updateText();
}

void CheckBox::bindCheckCallback(MouseCallback callback)
{
    m_clickCallback = callback;
}

void CheckBox::unbindCheckCallback()
{
    m_clickCallback = nullptr;
}

bool CheckBox::getState()
{
    return checkFlag(m_flags, FLAGS::SET);
}

void CheckBox::setTextSide(DIRECTION side)
{
    m_textSide = side;
    updateText();
}

void CheckBox::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::TEXT)) m_master.draw(m_text);
}

void CheckBox::config(FLAGS mask)
{

}

void CheckBox::updatePosition()
{
    updateText();
}

void CheckBox::updateSize()
{
    updateText();
}

void CheckBox::updateRotation()
{
    m_text.setRotation(this->getRotation());
}

void CheckBox::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::SET))
    {
        if(checkFlag(m_flags, FLAGS::BGIMAGE)) this->setTexture(m_bgTexturePtr.get());

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
            m_currentBgColorPtr = &m_style.fill_on_color;
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
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    sf::Vector2f shape_center = m_position + sf::Vector2f(m_size) / 2.f;
    const float  HORIZONTAL_SPACE = m_size.x * 0.2f;
    const float  VERTICAL_SPACE = m_size.y * 0.2f;

    // update text size first for accurate local bounds
    switch(m_textSide)
    {
        case DIRECTION::RIGHT:
        case DIRECTION::LEFT:
            m_text.setCharacterSize(m_size.x / 3);
            break;
        case DIRECTION::TOP:
        case DIRECTION::BOTTOM:
            m_text.setCharacterSize(m_size.y / 3);
            break;
    }

    sf::FloatRect bounds = m_text.getLocalBounds();

    switch(m_textSide)
    {
        case DIRECTION::RIGHT:
        {
            m_text.setOrigin({
                bounds.position.x,
                bounds.position.y + bounds.size.y / 2.f
            });
            m_text.setPosition({
                m_position.x + m_size.x + HORIZONTAL_SPACE,
                shape_center.y
            });
            break;
        }
        case DIRECTION::LEFT:
        {
            m_text.setOrigin({
                bounds.position.x + bounds.size.x,
                bounds.position.y + bounds.size.y / 2.f
            });
            m_text.setPosition({
                m_position.x - HORIZONTAL_SPACE,
                shape_center.y
            });
            break;
        }
        case DIRECTION::TOP:
        {
            m_text.setOrigin({
                bounds.position.x + bounds.size.x / 2.f,
                bounds.position.y + bounds.size.y
            });
            m_text.setPosition({
                shape_center.x,
                m_position.y - VERTICAL_SPACE
            });
            break;
        }
        case DIRECTION::BOTTOM:
        {
            m_text.setOrigin({
                bounds.position.x + bounds.size.x / 2.f,
                bounds.position.y
            });
            m_text.setPosition({
                shape_center.x,
                m_position.y + m_size.y + VERTICAL_SPACE
            });
            break;
        }
    }
}
}