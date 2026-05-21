#include "../include/entry.hpp"

namespace sfml_widgets // entry
{
Entry::Entry(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size) : Widget(root, position, size),
m_renderTexture(sf::Vector2u(size)), m_sprite(m_renderTexture.getTexture()), m_text(parameters::font), m_bgText(parameters::font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    m_text.setPosition({0.f, 0.f});
    m_bgText.setPosition({0.f, 0.f});
    setBLinkUpRatio(0.5f);

    setWidgetPosition(position);
    setWidgetSize(size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);

    config(FLAGS::INDICATOR);
}

Entry::Entry(Frame& root, sf::Vector2f position, sf::Vector2u size) : Widget(root, position, size),
m_renderTexture(sf::Vector2u(size)), m_sprite(m_renderTexture.getTexture()), m_text(parameters::font), m_bgText(parameters::font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_text.setPosition({0.f, 0.f});
    m_bgText.setPosition({0.f, 0.f});
    setBLinkUpRatio(0.5f);

    setWidgetPosition(position);
    setWidgetSize(size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);

    config(FLAGS::INDICATOR);
}

Entry::Entry(Entry&& other) noexcept : Widget(std::move(other)), m_renderTexture(std::move(other.m_renderTexture)), m_sprite(std::move(other.m_sprite)),
m_text(std::move(other.m_text)), m_bgText(std::move(other.m_bgText)), m_textBuffer(std::move(other.m_textBuffer)), m_cursorIndex(other.m_cursorIndex),
m_inputFilter(std::move(other.m_inputFilter)), m_inputLimit(other.m_inputLimit), m_textSizeRatio(other.m_textSizeRatio), m_cursorWidth(other.m_cursorWidth),
m_placeholderCharacter(other.m_placeholderCharacter), m_cursorOnDur(std::move(other.m_cursorOnDur)), m_cursorOffDur(std::move(other.m_cursorOffDur)),
m_blinkTimer(std::move(other.m_blinkTimer)), m_textBoxWidth(other.m_textBoxWidth), m_currentPanOffset(other.m_currentPanOffset), m_maxPanOffset(other.m_maxPanOffset)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    other.m_blinkTimer.stop();
}

Entry& Entry::operator=(Entry&& other) noexcept
{
    if(this != &other)
    {
        static_cast<Widget&>(*this) = std::move(other);
        m_renderTexture = std::move(other.m_renderTexture);
        m_sprite = std::move(other.m_sprite);
        m_text = std::move(other.m_text);
        m_bgText = std::move(other.m_bgText);
        m_textBuffer = std::move(other.m_textBuffer);
        m_cursorIndex = other.m_cursorIndex;
        m_inputFilter = std::move(other.m_inputFilter);
        m_inputLimit = other.m_inputLimit;
        m_textSizeRatio = other.m_textSizeRatio;
        m_cursorWidth = other.m_cursorWidth;
        m_placeholderCharacter = other.m_placeholderCharacter;
        m_cursorOnDur = std::move(other.m_cursorOnDur);
        m_cursorOffDur = std::move(other.m_cursorOffDur);
        m_blinkTimer = std::move(other.m_blinkTimer);
        m_textBoxWidth = other.m_textBoxWidth;
        m_currentPanOffset = other.m_currentPanOffset;
        m_maxPanOffset = other.m_maxPanOffset;

        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        other.m_blinkTimer.stop();
    }
    return *this;
}

bool Entry::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::RESTRICTED)) return false;
    if(!mouse_position) *mouse_position = getMousePosition();
    W_MouseEventDetails details = mouseCallBackEvent(event, *mouse_position);

    // clicked on entry
    if(details.event_type == W_MouseEventType::Press && details.button == parameters::ACTIVE_MOUSE_BUTTON)
    {
        sf::Vector2f local = m_master.mapMousePosition(*mouse_position); //!< relative to the master
        setCursorIndex(m_renderTexture.mapPixelToCoords(sf::Vector2i(local)).x);    //!< considering the entry's surface
        shiftToCursor();
        details.is_used = true;
    }

    // scrolling while hovered
    else if(details.event_type == W_MouseEventType::Scroll && checkFlag(m_flags, FLAGS::HOVERED) && checkFlag(m_flags, FLAGS::SCROLL))
    {
        m_currentPanOffset = std::clamp(m_currentPanOffset + details.scroll_delta * (parameters::SCROLL_RATIO * m_size.y * 5.f), 0.f, m_maxPanOffset);
        updateView();
        details.is_used = true;
    }

    return details.is_used;
}

bool Entry::captureKeyEvents(const sf::Event& event)
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

    if(!checkFlag(m_flags, FLAGS::FOCUSED)) return false;
    /* This section requires the widget to be focused */

    if(const sf::Event::TextEntered* _input = event.getIf<sf::Event::TextEntered>())
    {
        if(_input->unicode >= 32 && _input->unicode != 127 && m_textBuffer.size() <= m_inputLimit && (!m_inputFilter || m_inputFilter && m_inputFilter(_input->unicode)))
        {
            m_textBuffer.insert(m_cursorIndex, 1, static_cast<char>(_input->unicode));
            ++m_cursorIndex;
            writeToEntry();
            updateText();
            updateContentWidth();
            shiftToCursor();
        }
    }

    else if(const sf::Event::KeyPressed* _key = event.getIf<sf::Event::KeyPressed>())
    {
        switch (_key->code)
        {
            case sf::Keyboard::Key::Left: //!< left arrow (move left)
                if(_key->control) m_cursorIndex = 0;
                else if(m_cursorIndex > 0) --m_cursorIndex;
                break;

            case sf::Keyboard::Key::Right: //!< right arrow (move right)
                if(_key->control) m_cursorIndex = m_textBuffer.size();
                else if(m_cursorIndex < m_textBuffer.size()) ++m_cursorIndex;
                break;

            case sf::Keyboard::Key::X: //!< X (cut)
                if(_key->control)
                {
                    sf::Clipboard::setString(m_textBuffer);
                    clear();
                }
                break;
            
            case sf::Keyboard::Key::C: //!< C (copy)
                if(_key->control) sf::Clipboard::setString(m_textBuffer);
                break;
            
            case sf::Keyboard::Key::V: //!< V (paste)
                if(_key->control)
                {
                    sf::String pasted_str = sf::Clipboard::getString();
                    std::size_t current_size = m_textBuffer.size();
                    if(current_size > m_inputLimit) break;
                    else if(current_size + pasted_str.getSize() > m_inputLimit)
                        pasted_str = pasted_str.substring(0, m_inputLimit - current_size);
                    
                    m_textBuffer.insert(m_cursorIndex, pasted_str);
                    m_cursorIndex += pasted_str.getSize();
                    writeToEntry();
                    updateText();
                }
                break;

            case sf::Keyboard::Key::Backspace: //!< remove character(s)
                if(_key->control)
                {
                    m_textBuffer.erase(0, m_cursorIndex);
                    m_cursorIndex = 0;
                    writeToEntry();
                    updateText();
                }
                else if(m_cursorIndex > 0)
                {
                    m_textBuffer.erase(m_cursorIndex - 1, 1);
                    --m_cursorIndex;
                    writeToEntry();
                    updateText();
                }
                break;
            
            case sf::Keyboard::Key::Delete:
                clear();
                break;

            case sf::Keyboard::Key::Home:
                m_cursorIndex = 0;
                break;

            case sf::Keyboard::Key::End:
                m_cursorIndex = m_textBuffer.size();
                break;
            
            default:
                return true;
        }
        updateContentWidth();
        shiftToCursor();
    }

    return true;
}

void Entry::setInput(std::string text)
{
    if(text.size() > m_inputLimit) text = text.substr(0, m_inputLimit);

    m_textBuffer = std::move(text);
    m_cursorIndex = m_textBuffer.size();
    writeToEntry();

    focusWidget(*this, std::nullopt); //!< enables cursor
}

void Entry::setBackGroundText(std::string bg_text)
{
    if(bg_text.size() > m_inputLimit) bg_text = bg_text.substr(0, m_inputLimit);

    m_bgText.setString(bg_text);
    updateText();
}

void Entry::clear()
{
    m_textBuffer.clear();
    m_cursorIndex = 0;
    writeToEntry();
}

void Entry::setPlaceholderChar(const char placeholder_char)
{
    m_placeholderCharacter = placeholder_char;
    writeToEntry(); //!< this handles it
    updateText();
}

const std::string& Entry::get() const
{
    return m_textBuffer;
}

void Entry::setTextSizeRatio(float ratio)
{
    if(ratio <= 0.f || ratio > 1.f) return;
    m_textSizeRatio = ratio;
    updateText();
}

void Entry::setCursorWidth(unsigned int width)
{
    m_cursorWidth = width;
    shiftToCursor();
}

void Entry::setFont(const sf::Font& font)
{
    m_text.setFont(font);
    m_bgText.setFont(font);
}

void Entry::setInputLimit(std::size_t max_chars)
{
    m_inputLimit = max_chars;
    // remove extra chars
    if(m_textBuffer.size() > m_inputLimit) m_textBuffer = m_textBuffer.substr(0, m_inputLimit);
    if(m_bgText.getString().getSize() > m_inputLimit) m_bgText.setString(m_bgText.getString().substring(0, m_inputLimit));
    if(m_cursorIndex > m_inputLimit) m_cursorIndex = m_inputLimit;
    writeToEntry();
}

void Entry::setInputFilter(std::function<bool(char32_t)> func)
{
    m_inputFilter = func;
}

void Entry::setBLinkUpRatio(float ratio)
{
    if(ratio < 0.f || ratio > 1.f) return;
    m_cursorOnDur = std::chrono::duration_cast<std::chrono::milliseconds>(float_to_milli(parameters::BLINK_PERIOD) * ratio);
    m_cursorOffDur = std::chrono::duration_cast<std::chrono::milliseconds>(float_to_milli(parameters::BLINK_PERIOD) * (1.f - ratio));
    if(ratio == 1.f)
    {
        setFlag(m_flags, FLAGS::SET);
        resetFlag(m_flags, FLAGS::RESET);
    }
    else if(ratio == 0.f)
    {
        setFlag(m_flags, FLAGS::RESET);
        resetFlag(m_flags, FLAGS::SET);
    }
    else resetFlag(m_flags, FLAGS::SET | FLAGS::RESET);
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void Entry::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::HIDDEN)) return;

    if(m_textBuffer.empty() && !checkFlag(m_flags, FLAGS::FOCUSED))
        m_renderTexture.draw(m_bgText);
    else
        m_renderTexture.draw(m_text);
    
    drawCursorLine();
    m_renderTexture.display();
    m_sprite.setTexture(m_renderTexture.getTexture());
    m_master.draw(m_sprite, &m_clipShader);
}

void Entry::config(FLAGS mask)
{
    if(checkFlag(mask, FLAGS::INDICATOR))
    {
        if(checkFlag(m_flags, FLAGS::INDICATOR)) //!< cursor is now enabled
            m_blinkTimer.restart();
        else    //!< cursor is now disabled
            m_blinkTimer.reset();
    }
}

void Entry::updatePosition()
{
    m_sprite.setPosition(m_position);
}

void Entry::updateSize()
{
    if(!m_renderTexture.resize(m_size)) return;

    m_sprite.setTexture(m_renderTexture.getTexture(), true);
    updateText();
    updateClipShader();
    update();
}

void Entry::updateRotation()
{
    m_sprite.setRotation(this->getRotation());
    m_text.setRotation(this->getRotation());
    m_bgText.setRotation(this->getRotation());
}

void Entry::updateState()
{
    m_renderTexture.clear(sf::Color::Transparent);

    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.3f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
        m_text.setFillColor(m_style.text_blocked_color);
        m_bgText.setFillColor(m_style.bg_text_blocked_color);
    }
    else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
        m_text.setFillColor(m_style.text_transparent_color);    
        m_bgText.setFillColor(m_style.bg_text_transparent_color);
    }
    else if(checkFlag(m_flags, FLAGS::ACTIVE))
    {
        m_opacity = 0.8f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.outline_active_thickness;
        m_text.setFillColor(m_style.text_active_color);
        m_bgText.setFillColor(m_style.bg_text_active_color);
    }
    else if(checkFlag(m_flags, FLAGS::HOVERED))
    {
        m_opacity = 0.9f;
        m_currentBgColorPtr = &m_style.fill_hover_color;
        m_currentOutlineColorPtr = &m_style.outline_hover_color;
        m_currentOutlineThicknessPtr = &m_style.outline_hover_thickness;
        m_text.setFillColor(m_style.text_hover_color);
        m_bgText.setFillColor(m_style.bg_text_hover_color);
    }
    else
    {
        m_opacity = 1.f;
        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        m_currentOutlineThicknessPtr = &m_style.outline_thickness;
        m_text.setFillColor(m_style.text_color);
        m_bgText.setFillColor(m_style.bg_text_color);
    }
    this->setFillColor(*m_currentBgColorPtr);
    this->setOutlineColor(*m_currentOutlineColorPtr);
    this->setOutlineThickness(*m_currentOutlineThicknessPtr);
    updateClipShader();
    updateColorShader();
}

void Entry::updateClipShader()
{
    if(!checkFlag(m_flags, FLAGS::CLIPSHADER)) return;

    m_clipShader.setUniform("texture",  sf::Shader::CurrentTexture);
    m_clipShader.setUniform("size",     sf::Glsl::Vec2(m_size));
    m_clipShader.setUniform("radius",   m_radius);
    m_clipShader.setUniform("opacity",   m_opacity);
}

void Entry::updateColorShader()
{
    if(!checkFlag(m_flags, FLAGS::COLORSHADER)) return;
    bool render_texture = checkFlag(m_flags, FLAGS::BGIMAGE); 
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

void Entry::drawCursorLine()
{
    if(!(checkFlag(m_flags, FLAGS::INDICATOR) && checkFlag(m_flags, FLAGS::FOCUSED) && !checkFlag(m_flags, FLAGS::BLOCKED))) return;

    if(checkFlag(m_flags, FLAGS::RESET)) return; //!< 0.f ratio
    else if(checkFlag(m_flags, FLAGS::SET)) resetFlag(m_flags, FLAGS::SHORTSKIP); //!< 1.0 ratio
    else //!< balanced ratio
    {
        bool is_active = !checkFlag(m_flags, FLAGS::SHORTSKIP);
        sf::Time t_dur_ms = m_blinkTimer.getElapsedTime();
        if(is_active && t_dur_ms >= m_cursorOnDur || !is_active && t_dur_ms >= m_cursorOffDur)
        {
            m_blinkTimer.restart();
            toggle(m_flags, FLAGS::SHORTSKIP);
        }
    }
    if(checkFlag(m_flags, FLAGS::SHORTSKIP)) return;

    float x0 = getCursorCoordinate();
    float x1 = x0 + m_cursorWidth;
    float y0 = m_size.y * ((1.f - m_textSizeRatio) / 2.f);
    float y1 = m_size.y * (m_textSizeRatio + (1.f - m_textSizeRatio) / 2.f);
    sf::Vertex cursor_line[6];
    cursor_line[0] = {{x0, y0}, *m_currentOutlineColorPtr};
    cursor_line[1] = {{x0, y1}, *m_currentOutlineColorPtr};
    cursor_line[2] = {{x1, y0}, *m_currentOutlineColorPtr};
    cursor_line[3] = {{x1, y1}, *m_currentOutlineColorPtr};
    cursor_line[4] = {{x1, y0}, *m_currentOutlineColorPtr};
    cursor_line[5] = {{x0, y1}, *m_currentOutlineColorPtr};

    m_renderTexture.draw(cursor_line, 6, sf::PrimitiveType::Triangles);
}

void Entry::setCursorIndex(float x__pos)
{
    float x = (x__pos - m_position.x) + m_currentPanOffset;
    float smallest_distance  = std::abs(m_text.findCharacterPos(0).x - x); //!< begin with the first char
    
    std::size_t text_length = m_textBuffer.size();
    for(std::size_t idx{1}; idx <= text_length; idx++)
    {
        float dist = std::abs(m_text.findCharacterPos(idx).x - x);
        if(dist < smallest_distance)
        {
            smallest_distance  = dist;
            m_cursorIndex = idx;
        }
    }
}

void Entry::updateText()
{
    float final_height = m_size.y * m_textSizeRatio;

    if(!m_text.getString().isEmpty())
    {
        float current_height = m_text.getLocalBounds().size.y;
        if(current_height > 0.f)
        {
            float ratio   = final_height / current_height;
            unsigned int final_char_size = static_cast<unsigned int>(m_text.getCharacterSize() * ratio);
            m_text.setCharacterSize(final_char_size);
            float lineSpacing = m_text.getFont().getLineSpacing(final_char_size);
            float y_offset     = (m_size.y - lineSpacing) / 2.f;
            m_text.setPosition( {0.f,   y_offset});
        }
    }

    if(!m_bgText.getString().isEmpty())
    {
        float current_height = m_bgText.getLocalBounds().size.y;
        if(current_height > 0.f)
        {
            float ratio   = final_height / current_height;
            unsigned int final_char_size = static_cast<unsigned int>(m_bgText.getCharacterSize() * ratio);
            m_bgText.setCharacterSize(final_char_size);
            float lineSpacing = m_bgText.getFont().getLineSpacing(final_char_size);
            float y_offset     = (m_size.y - lineSpacing) / 2.f;
            m_bgText.setPosition( {0.f,   y_offset});
        }
    }
}

float Entry::getCursorCoordinate() const
{
    return m_text.findCharacterPos(m_cursorIndex).x;
}

void Entry::updateContentWidth()
{
    m_textBoxWidth = m_text.findCharacterPos(m_textBuffer.size()).x;
    m_maxPanOffset    = std::max(0.f, (m_textBoxWidth + m_cursorWidth) - m_size.x);
    m_currentPanOffset = std::clamp(m_currentPanOffset, 0.f, m_maxPanOffset);

    updateView();
}

void Entry::shiftToCursor()
{
    float x_position = getCursorCoordinate();
    // check if cursor passed entry view from the right
    if((x_position + m_cursorWidth) - m_currentPanOffset > m_size.x)
        // move the offset to show the cursor
        m_currentPanOffset = (x_position + m_cursorWidth) - m_size.x;

    // check if cursor passed entry view from the left
    if(x_position - m_currentPanOffset < 0.f)
        m_currentPanOffset = x_position;

    m_currentPanOffset = std::clamp(m_currentPanOffset, 0.f, m_maxPanOffset);
    updateView();
}
void Entry::updateView()
{
    sf::View v = m_renderTexture.getView();
    v.setCenter({static_cast<float>(m_size.x) / 2.f + m_currentPanOffset,
                 static_cast<float>(m_size.y) / 2.f});

    m_renderTexture.setView(v);
}

void Entry::writeToEntry()
{
    if(m_placeholderCharacter == 0)
    {
        m_text.setString(m_textBuffer);
        return;
    }
    m_text.setString(std::string(m_textBuffer.size(), m_placeholderCharacter));
}
}