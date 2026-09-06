#include "entry.hpp"
#include <iostream>

namespace sfml_widgets // entry
{
Entry::Entry(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::Font& font) : Widget(root, position, size),
m_renderTexture(sf::Vector2u(size)), m_sprite(m_renderTexture.getTexture()), m_text(font), m_bgText(font)
{ 
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    setBLinkUpRatio(0.5f);

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    configure(FLAGS::INDICATOR);
}

Entry::Entry(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Font& font) : Widget(root, position, size),
m_renderTexture(sf::Vector2u(size)), m_sprite(m_renderTexture.getTexture()), m_text(font), m_bgText(font)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;

    setBLinkUpRatio(0.5f);

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    configure(FLAGS::INDICATOR);
}

void Entry::setText(const sf::String& text)
{
    m_textBuffer = "";
    m_cursorIndex = 0;

    if(m_inputFilter)
    {
        // filter input
        sf::String result;
        std::size_t size = std::min(static_cast<unsigned int>(text.getSize()), m_inputLimit);
        for(std::size_t idx{0}; idx < size; idx++)
        {
            char32_t c = text[idx];
            if(m_inputFilter(&c))
            {
                ++m_cursorIndex;
                m_textBuffer += c;
            }
        }
    }
    else
    {
        // cut string if surpasses limit
        m_textBuffer = text.substring(0, std::min(static_cast<unsigned int>(text.getSize()), m_inputLimit));
    }

    writeToEntry(); //!< update front end
}

void Entry::setBackGroundText(const sf::String& bg_text)
{
    // limit to 256
    m_bgText.setString(bg_text.substring(0, std::min(bg_text.getSize(), s_MAX_BG_TEXT_SIZE)));
}

void Entry::setTextFilter(std::function<bool(char32_t*)> func)
{
    m_inputFilter = func;
}

void Entry::setTextLimit(unsigned int max_chars)
{
    m_inputLimit = max_chars;

    // remove extra chars
    if(m_textBuffer.getSize() > m_inputLimit) m_textBuffer = m_textBuffer.substring(0, m_inputLimit);
    if(m_cursorIndex > m_inputLimit) m_cursorIndex = m_inputLimit;
    writeToEntry();
}

bool Entry::setTextSize(float ratio)
{
    if(ratio < 0.f || ratio > 1.f) return false;
    m_textSizeRatio = ratio;
    updateText();
    return true;
}

bool Entry::setTextSize(unsigned int char_size)
{
    m_charSize = char_size;
    m_textSizeRatio = 0.f;
    updateText();
    return true;
}

void Entry::setTextFont(const sf::Font& font)
{
    m_text.setFont(font);
    m_bgText.setFont(font);
    if(m_font) m_font.reset();

    updateText();
}

void Entry::setTextFont(const sf::Font&& font)
{
    m_font = std::make_unique<const sf::Font>(std::move(font));
    m_text.setFont(*m_font);
    m_bgText.setFont(*m_font);

    updateText();
}

void Entry::clearText()
{
    m_textBuffer.clear();
    m_cursorIndex = 0;
    writeToEntry();
}

bool Entry::setBLinkUpRatio(float ratio)
{
    if(ratio < 0.f || ratio > 1.f) return false;
    m_cursorOnDuration = s_BLINK_PERIOD * ratio;
    m_cursorOffDuration = s_BLINK_PERIOD * (1.f - ratio);
    if(ratio == 1.f)
    {
        // skips comparing the timer and always draws cursor when focused
        setFlag(m_flags, FLAGS::SET);
        resetFlag(m_flags, FLAGS::RESET);
    }
    else if(ratio == 0.f)
    {
        // skips comparing the timer and never draws cursor
        setFlag(m_flags, FLAGS::RESET);
        resetFlag(m_flags, FLAGS::SET);
    }
    else resetFlag(m_flags, FLAGS::SET | FLAGS::RESET);
    return true;
}

unsigned int Entry::setCursorIndex(unsigned int idx)
{
    m_cursorIndex = std::clamp<unsigned int>(idx, 0, m_textBuffer.getSize());
    shiftToCursor();
    return m_cursorIndex;
}

void Entry::setPlaceholderChar(const char placeholder_char)
{
    if(placeholder_char != 0 && !(placeholder_char >= 32 && placeholder_char != 127)) return;
    m_placeholderCharacter = placeholder_char;
    writeToEntry(); //!< updates front end
}

bool Entry::setTextOffsetRatio(float ratio)
{
    if(ratio < 0.f || ratio >= 1.f) return false;
    m_textOffsetRatio = ratio;

    updateText();
    updateContentWidth();
    shiftToCursor();
    return true;
}

void Entry::setCursorWidth(unsigned int width)
{
    m_cursorWidth = width;
    shiftToCursor();
}

void Entry::setTheme(EntryStyle s_entry)
{
    m_style = s_entry;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Entry::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<Entry>());
}

EntryStyle& Entry::style()
{
    return m_style;
}

const sf::String& Entry::getText() const
{
    return m_textBuffer;
}

const sf::String& Entry::getBackGroundText() const
{
    return m_bgText.getString();
}

unsigned int Entry::getTextSize() const
{
    return m_charSize;
}

unsigned int Entry::getTextLimit() const
{
    return m_inputLimit;
}

float Entry::getTextOffsetRatio() const
{
    return m_textOffsetRatio;
}

const sf::Font& Entry::getTextFont() const
{
    return m_text.getFont();
}

char Entry::getPlaceholderChar() const
{
    return m_placeholderCharacter;
}

unsigned int Entry::getCursorIndex() const
{
    return m_cursorIndex;
}

unsigned int Entry::getCursorWidth() const
{
    return m_cursorWidth;
}

float Entry::getBlinkUpRatio() const
{
    return m_cursorOnDuration.asSeconds();
}

float Entry::getScrollOffset() const
{
    return m_currentPanOffset;
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

bool Entry::postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    bool event_used{false};

    // clicked on entry
    if(details.event_type == W_MouseEventType::Press && checkFlag(m_flags, FLAGS::HOVERED) && details.button == s_FOCUS_BUTTON)
    {
        sf::Vector2f root_view = m_master.mapMousePosition(mouse_position);                         //!< relative to the master
        sf::Vector2f transform_view = this->getInverseTransform().transformPoint(root_view);        //!< considering rotation
        sf::Vector2f local_view = m_renderTexture.mapPixelToCoords(sf::Vector2i(transform_view));   //!< considering texture view (padding)

        moveCursorIndex(local_view.x);
        shiftToCursor();
        event_used = true;
    }

    // scrolling while hovered
    else if(details.event_type == W_MouseEventType::Scroll && checkFlags(m_flags, FLAGS::HOVERED | FLAGS::SCROLL))
    {
        updateContentWidth();
        // scrolls through text space
        m_currentPanOffset = std::clamp(m_currentPanOffset + details.scroll_delta * (s_SCROLL_RATIO * m_size.y), 0.f, m_maxPanOffset);
        updateView();
        event_used = true;
    }

    return event_used;
}

bool Entry::postProcessKeyEvents(const sf::Event& event)
{
    /* This section requires the widget to be focused */
    if(!checkFlag(m_flags, FLAGS::FOCUSED)) return false;

    bool event_used = false;

    // text entered while focused
    if(const sf::Event::TextEntered* input_char = event.getIf<sf::Event::TextEntered>())
    {
        char32_t c = input_char->unicode;
        // char is not a special character + size limit check + filter input
        if(c >= 32 && c != 127 && m_textBuffer.getSize() < m_inputLimit && (!m_inputFilter || m_inputFilter && m_inputFilter(&c)))
        {
            // insert char
            m_textBuffer.insert(m_cursorIndex, c);
            ++m_cursorIndex;

            writeToEntry();         //!< updates front end
            updateContentWidth();   //!< update scroll range
            shiftToCursor();        //!< follow cursor
            event_used = true;
        }
    }

    // special characters case
    else if(const sf::Event::KeyPressed* input_key = event.getIf<sf::Event::KeyPressed>())
    {
        switch (input_key->code)
        {
            case sf::Keyboard::Key::Left: //!< shift to left
                if(input_key->control) m_cursorIndex = 0;
                else if(m_cursorIndex > 0) --m_cursorIndex;
                event_used = true;
                break;

            case sf::Keyboard::Key::Right: //!< shift to right
                if(input_key->control) m_cursorIndex = m_textBuffer.getSize();
                else if(m_cursorIndex < m_textBuffer.getSize()) ++m_cursorIndex;
                event_used = true;
                break;

            case sf::Keyboard::Key::X: //!< cut
                if(input_key->control)
                {
                    sf::Clipboard::setString(m_textBuffer);
                    clearText();
                    event_used = true;
                }
                break;
            
            case sf::Keyboard::Key::C: //!< copy
                if(input_key->control) sf::Clipboard::setString(m_textBuffer);
                event_used = true;
                break;
            
            case sf::Keyboard::Key::V: //!< paste
                if(input_key->control)
                {
                    sf::String pasted_str = sf::Clipboard::getString();
                    std::size_t current_size = m_textBuffer.getSize();
                    if(current_size > m_inputLimit) break;
                    else if(current_size + pasted_str.getSize() > m_inputLimit)
                        pasted_str = pasted_str.substring(0, m_inputLimit - current_size);
                    
                    m_textBuffer.insert(m_cursorIndex, pasted_str);
                    m_cursorIndex += pasted_str.getSize();
                    writeToEntry();
                    event_used = true;
                }
                break;

            case sf::Keyboard::Key::Backspace: //!< remove character(s)
                if(input_key->control)
                {
                    m_textBuffer.erase(0, m_cursorIndex);
                    m_cursorIndex = 0;
                    writeToEntry();
                    event_used = true;
                }
                else if(m_cursorIndex > 0)
                {
                    m_textBuffer.erase(m_cursorIndex - 1, 1);
                    --m_cursorIndex;
                    writeToEntry();
                    event_used = true;
                }
                break;
            
            case sf::Keyboard::Key::Delete: //!< clear all input
                clearText();
                event_used = true;
                break;

            case sf::Keyboard::Key::Home:   //!< back to front
                m_cursorIndex = 0;
                event_used = true;
                break;

            case sf::Keyboard::Key::End:    //!< go to last
                m_cursorIndex = m_textBuffer.getSize();
                event_used = true;
                break;
            
            default:
                break;
        }
        updateContentWidth();
        shiftToCursor();
    }

    return event_used;
}

void Entry::drawWidget()
{
    if(m_textBuffer.isEmpty() && !checkFlag(m_flags, FLAGS::FOCUSED))
        m_renderTexture.draw(m_bgText);
    else
        m_renderTexture.draw(m_text);
    
    drawCursorLine();
    m_renderTexture.display();
    m_sprite.setTexture(m_renderTexture.getTexture());
    if(m_clipShaderPtr) m_master.draw(m_sprite, m_clipShaderPtr.get());
    else m_master.draw(m_sprite);
}

void Entry::config(FLAGS mask)
{
    if(!checkFlag(mask, FLAGS::INDICATOR)) return;

    // if indicator is set that means it was recently enabled
    // otherwise it was disabled
    if(checkFlag(m_flags, FLAGS::INDICATOR)) m_blinkTimer.restart();
    else m_blinkTimer.reset();
}

void Entry::updatePosition()
{
    /*
    m_position is relative to the top left corner of the shape,
    we do this since m_sprite's origin is it's center, just
    like the base widget shape
    */
    m_sprite.setPosition(m_position + m_size / 2.f);
}

void Entry::updateSize()
{
    if(!m_renderTexture.resize(sf::Vector2u(m_size))) return;

    m_sprite.setTexture(m_renderTexture.getTexture(), true);
    m_sprite.setOrigin(m_size / 2.f);
    updateText();
    updateClipShader();
}

void Entry::updateRotation()
{
    // render texture handles the rotation of it's drawables (text and cursor)
    // and we already know it's origin is the center of the shape
    m_sprite.setRotation(this->getRotation());
}

void Entry::updateState()
{
    m_renderTexture.clear(sf::Color::Transparent);

    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.4f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
        m_text.setFillColor(m_style.text_blocked_color);
        m_bgText.setFillColor(m_style.bg_text_blocked_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
    {
        m_opacity = 0.25f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
        m_text.setFillColor(m_style.text_transparent_color);    
        m_bgText.setFillColor(m_style.bg_text_transparent_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ACTIVE))
    {
        m_opacity = 0.8f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.outline_active_thickness;
        m_text.setFillColor(m_style.text_active_color);
        m_bgText.setFillColor(m_style.bg_text_active_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
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

void Entry::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<Entry>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Entry>());
    }
}

void Entry::writeToEntry()
{
    if(m_placeholderCharacter == 0)
    {
        m_text.setString(m_textBuffer);
    }
    else
    {
        m_text.setString(std::string(m_textBuffer.getSize(), m_placeholderCharacter));
    }
}

void Entry::drawCursorLine()
{
    if(!checkFlag(m_flags, FLAGS::INDICATOR) || !checkFlag(m_flags, FLAGS::FOCUSED) || checkFlag(m_flags, FLAGS::BLOCKED) || m_cursorWidth == 0) return;
    // entry is not blocked + focused + enabled cursor + cursor width is > 0

    if(checkFlag(m_flags, FLAGS::RESET)) return; //!< 0.f ratio
    else if(checkFlag(m_flags, FLAGS::SET)) resetFlag(m_flags, FLAGS::SHORTSKIP); //!< 1.0 ratio
    else //!< balanced ratio
    {
        bool is_active = !checkFlag(m_flags, FLAGS::SHORTSKIP);
        sf::Time current_dur_ms = m_blinkTimer.getElapsedTime();
        if(is_active && current_dur_ms >= m_cursorOnDuration || !is_active && current_dur_ms >= m_cursorOffDuration)
        {
            m_blinkTimer.restart();
            toggle(m_flags, FLAGS::SHORTSKIP);
        }
    }
    if(checkFlag(m_flags, FLAGS::SHORTSKIP)) return;
    
    // default cursor height takes 80% of entry height
    float height_ratio = m_textSizeRatio == 0.f? 0.8f : m_textSizeRatio;

    float x0 = getCursorCoordinate();
    float y0 = m_size.y * ((1.f - height_ratio) / 2.f);
    float y1 = m_size.y * (height_ratio + (1.f - height_ratio) / 2.f);

    if(m_cursorWidth == 1)
    {
        sf::Vertex cursor_line[2];
        cursor_line[0] = {{x0, y0}, *m_currentOutlineColorPtr};
        cursor_line[1] = {{x0, y1}, *m_currentOutlineColorPtr};
        m_renderTexture.draw(cursor_line, 2, sf::PrimitiveType::Lines);
        return;
    }

    float x1 = x0 + m_cursorWidth;
    sf::Vertex cursor_line[6];
    cursor_line[0] = {{x0, y0}, *m_currentOutlineColorPtr};
    cursor_line[1] = {{x0, y1}, *m_currentOutlineColorPtr};
    cursor_line[2] = {{x1, y0}, *m_currentOutlineColorPtr};
    cursor_line[3] = {{x1, y1}, *m_currentOutlineColorPtr};
    cursor_line[4] = {{x1, y0}, *m_currentOutlineColorPtr};
    cursor_line[5] = {{x0, y1}, *m_currentOutlineColorPtr};

    m_renderTexture.draw(cursor_line, 6, sf::PrimitiveType::Triangles);
}

void Entry::moveCursorIndex(float x_pos)
{
    float smallest_distance = 10000.f;
    std::size_t text_length = m_textBuffer.getSize();
    for(std::size_t idx{0}; idx <= text_length; idx++)
    {
        // finding the smallest diff, starting with index 0
        float dist = std::abs(m_text.findCharacterPos(idx).x - x_pos);
        if(dist < smallest_distance)
        {
            smallest_distance  = dist;
            m_cursorIndex = idx;
        }
    }
}

float Entry::getCursorCoordinate() const
{
    return m_text.findCharacterPos(m_cursorIndex).x; //!< this returns the position at the right of the char
}

void Entry::updateText()
{
    // desired height
    float final_height = m_size.y * m_textSizeRatio;

    if(m_textSizeRatio != 0.f)
    {
        // estimated maximum possible height
        float current_height = W_CharHeight::getHeight(m_text.getFont(), m_text.getCharacterSize());
        float ratio = final_height / current_height;
        m_charSize = static_cast<unsigned int>(m_text.getCharacterSize() * ratio);
    }
    // line up text vertically
    float line_spacing = m_text.getFont().getLineSpacing(m_charSize);
    float y_offset = (m_size.y - line_spacing) / 2.f;

    m_text.setPosition({m_textOffsetRatio * m_size.x, y_offset});
    m_bgText.setPosition({m_textOffsetRatio * m_size.x, y_offset});

    m_text.setCharacterSize(m_charSize);
    m_bgText.setCharacterSize(m_charSize);
}


void Entry::updateContentWidth()
{
    m_textBoxWidth = m_text.findCharacterPos(m_textBuffer.getSize()).x + m_textOffsetRatio * m_size.y;
    m_maxPanOffset = std::max(0.f, (m_textBoxWidth + m_cursorWidth) - m_size.x);
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
    v.setCenter({m_size.x / 2.f + m_currentPanOffset,
                 m_size.y / 2.f});

    m_renderTexture.setView(v);
}
}