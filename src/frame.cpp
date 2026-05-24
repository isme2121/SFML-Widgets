#include "../include/frame.hpp"

namespace sfml_widgets // Frame
{
///////////////////////////////////////////////////////////////////
/// Constructors / Destructor
///////////////////////////////////////////////////////////////////

Frame::Frame(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size) : Widget(window, position, size),
m_window(&window), m_texture(sf::Vector2u(size)), m_sprite(m_texture.getTexture()), m_contentHeight(size.y)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    setWidgetPosition(position);
    setWidgetSize(size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);
    toggle(m_flags, FLAGS::SCROLL);
}

Frame::Frame(Frame& root, sf::Vector2f position, sf::Vector2u size) : Widget(root, position, size),
m_window(root.getWindowPtr()), m_texture(sf::Vector2u(size)), m_sprite(m_texture.getTexture()), m_contentHeight(size.y)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    setWidgetPosition(position);
    setWidgetSize(size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);
    toggle(m_flags, FLAGS::SCROLL);
}

Frame::Frame(Frame&& other) noexcept : Widget(std::move(other)), m_window(other.m_window), m_texture(std::move(other.m_texture)), 
m_sprite(std::move(other.m_sprite)), m_childWidgets(std::move(other.m_childWidgets)), m_maxScroll(other.m_maxScroll), m_scrollOffset(other.m_scrollOffset),
m_contentHeight(other.m_contentHeight), m_gridMapping(other.m_gridMapping), m_style(other.m_style)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    other.m_window = nullptr;
    m_sprite.setTexture(m_texture.getTexture(), true);

}

Frame& Frame::operator=(Frame&& other) noexcept
{
    if(this != &other)
    {
        static_cast<Widget&>(*this) = std::move(other);
        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        m_currentOutlineThicknessPtr = &m_style.outline_thickness;

        m_style = other.m_style;
        m_window = other.m_window;
        m_texture = std::move(other.m_texture);
        m_sprite = std::move(other.m_sprite);
        m_childWidgets = std::move(other.m_childWidgets);
        m_maxScroll = other.m_maxScroll;
        m_scrollOffset = other.m_scrollOffset;
        m_contentHeight = other.m_contentHeight;
        m_gridMapping = other.m_gridMapping;

        other.m_window = nullptr;
        m_sprite.setTexture(m_texture.getTexture(), true);
    }
    return *this;
}

Frame::~Frame()
{
    // assign to lower level container
    if(Frame* root = m_master.getFramePtr())
    {
        for(Widget* w : m_childWidgets)
        {
            // guarantee child widgets don't have window ref
            // replacing current frame ref with root frame
            w->m_master.setFramePtr(root); //!< updates Widget::m_master
            root->addWidget(*w); //!< adds widget to root frame buffer
        }
            
    }
    else if(m_window != nullptr)
    {
        for(Widget* w : m_childWidgets)
        {
            w->m_master.setWindowPtr(m_window);
            w->m_master.setFramePtr(nullptr);
        }
    }
}

bool Frame::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::RESTRICTED)) return false;

    if(!mouse_position) *mouse_position = getMousePosition();

    // handle widgets events
    std::vector<Widget*>::reverse_iterator it_end = m_childWidgets.rbegin();
    std::vector<Widget*>::const_reverse_iterator it_begin = m_childWidgets.crend();
    while(it_end != it_begin)
    {
        if((*it_end)->captureMouseEvents(event, mouse_position))
        {
            if(parameters::STACK_FRAME_WITH_WIDGETS) break;
            if(!parameters::STACK_MOUSE_CALLBACKS) return true;
        }
        ++it_end;
    }

    // handle frame widget events
    W_MouseEventDetails details = mouseCallBackEvent(event, *mouse_position);

    // handle frame unique events
    if(details.event_type == W_MouseEventType::Scroll && checkFlag(m_flags, FLAGS::HOVERED))
    {
        scroll(details.scroll_delta);
        details.is_used = true;
    }

    return details.is_used;
}

bool Frame::captureKeyEvents(const sf::Event& event)
{
    if(checkFlag(m_flags, FLAGS::BLOCKED | FLAGS::RESTRICTED) || !checkFlag(m_flags, FLAGS::AUTOSELECT | FLAGS::HOVERED | FLAGS::SELECTED)) return false;

    bool event_used = false;
    for(Widget* w : m_childWidgets)
    {
        if(w->captureKeyEvents(event)) event_used = true; //!< because of AUTOSELECT
    }
    if(event_used) return true;

    if(m_binds.checkEvents(event))
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

void Frame::display()
{
    if(checkFlag(m_flags, FLAGS::HIDDEN)) return;

    resetFlag(m_flags, FLAGS::DRAWING | FLAGS::MODIFIED);
    
    drawSeparators();

    m_texture.display();
    m_sprite.setTexture(m_texture.getTexture());
    m_master.draw(m_sprite, &m_clipShader);
}

sf::Vector2f Frame::mapPixelToCoords(sf::Vector2i mouse_position) const
{
    sf::Vector2f local = sf::Vector2f(mouse_position);

    const Frame* root = this;
    while(root != nullptr)
    {
        // handles world offset + transform ratio
        local = root->getInverseTransform().transformPoint(local);
        local.y += root->m_scrollOffset;
        root = root->m_master.getFramePtr();
    }

    return local;
}

void Frame::setGrid(sf::Vector2u grid_mapping)
{
    m_gridMapping = grid_mapping;
    if(!(m_gridMapping.x * m_gridMapping.y)) 
    {
        resetFlag(m_flags, FLAGS::GRID);
        return;
    }
    setFlag(m_flags, FLAGS::GRID);
    updateGrid();
}

void Frame::setSeparators(SeparatorType type, float length_ratio, float sep_distance)
{
    m_separatorType = type;
    if(type == SeparatorType::NoSeparator)
    {
        resetFlag(m_flags, FLAGS::SEPARATORS);
        return;
    }
    m_separatorRatio = std::clamp(length_ratio, 0.f, 1.f);
    m_separatorDistance = sep_distance;
    setFlag(m_flags, FLAGS::SEPARATORS);
}

sf::RenderWindow* Frame::getWindowPtr() const
{
    return m_window;
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void Frame::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::LONGSKIP)) //!< has child widgets
        for(Widget* w : m_childWidgets) w->draw();

    display();
}

void Frame::config(FLAGS mask)
{
    resetFlag(mask, FLAGS::GRID | FLAGS::INDICATOR | FLAGS::AUTOHIDE | FLAGS::AUTOSELECT | FLAGS::SCROLL | FLAGS::PAN | FLAGS::BLOCKBGIMAGE);
    for(Widget* w : m_childWidgets) w->configure(mask);
}

void Frame::updatePosition()
{
    m_sprite.setPosition(this->getPosition());

    bool grid_is_active = false; //!< memory for GRID flag
    if(checkFlag(m_flags, FLAGS::GRID))
    {
        grid_is_active = true;
        toggle(m_flags, FLAGS::GRID); //!< disable to prevent updating grid for every widget
    }

    setFlag(m_flags, FLAGS::SHORTSKIP); //!< prevent widgets from updating frame height on each iteration
    for(Widget* w : m_childWidgets) w->setWidgetPosition(w->m_position);
    resetFlag(m_flags, FLAGS::SHORTSKIP);

    if(grid_is_active) toggle(m_flags, FLAGS::GRID);

}

void Frame::updateSize()
{
    if(!m_texture.resize(m_size)) return;

    // update texture in sprite
    m_sprite.setTexture(m_texture.getTexture(), true);
    m_sprite.setOrigin(sf::Vector2f(m_size) / 2.f);

    // update widgets positions if we have grid enabled
    if(checkFlag(m_flags, FLAGS::GRID)) updateGrid();
    else updateFrameHeight();     //!< update invisible space (updateGrid does it anyways)

    // shader scales with size
    updateClipShader();
    update();
}

void Frame::updateRotation()
{
    m_sprite.setRotation(this->getRotation());
}

void Frame::updateState()
{
    m_texture.clear(sf::Color::Transparent);

    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.3f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
    }
    else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
    }
    else if(checkFlag(m_flags, FLAGS::ACTIVE))
    {
        m_opacity = 0.8f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.outline_active_thickness;
    }
    else if(checkFlag(m_flags, FLAGS::HOVERED))
    {
        m_opacity = 0.9f;
        m_currentBgColorPtr = &m_style.fill_hover_color;
        m_currentOutlineColorPtr = &m_style.outline_hover_color;
        m_currentOutlineThicknessPtr = &m_style.outline_hover_thickness;
    }
    else
    {
        m_opacity = 1.f;
        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    }
    this->setFillColor(*m_currentBgColorPtr);
    this->setOutlineColor(*m_currentOutlineColorPtr);
    this->setOutlineThickness(*m_currentOutlineThicknessPtr);
    updateClipShader();
    updateColorShader();
}

void Frame::updateClipShader()
{
    if(!checkFlag(m_flags, FLAGS::CLIPSHADER)) return;

    m_clipShader.setUniform("texture",  sf::Shader::CurrentTexture);
    m_clipShader.setUniform("size",     sf::Glsl::Vec2(m_size));
    m_clipShader.setUniform("radius",   m_radius);
    m_clipShader.setUniform("opacity",   m_opacity);
}

void Frame::updateColorShader()
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

void Frame::updateGrid()
{
    if(!checkFlag(m_flags, FLAGS::GRID) || m_childWidgets.empty()) return;

    // first sort by current position (y prio)
    std::vector<Widget*> sorted_by_position;
    sorted_by_position.reserve(m_childWidgets.size());

    // skip hidden widgets
    for(Widget* w : m_childWidgets)
    {
        if(w->isFlag(FLAGS::HIDDEN)) continue;
        sorted_by_position.emplace_back(w);
    }

    std::sort(sorted_by_position.begin(), sorted_by_position.end(), [](const Widget* first, const Widget* second)
    {
        const sf::Vector2f first_pos = first->m_position;
        const sf::Vector2f second_pos = second->m_position;
        if(first_pos.y != second_pos.y) return first_pos.y < second_pos.y;
        if(first_pos.x != second_pos.x) return first_pos.x < second_pos.x;
        return false; // sort by index if coords match
    });

    resetFlag(m_flags, FLAGS::GRID);            //!< Prevent infinite recursion
    setFlag(m_flags, FLAGS::SHORTSKIP);         //!< Prevent extra height operations

    const float width_difference = static_cast<float>(m_size.x) / static_cast<float>(m_gridMapping.x);
    const float height_difference = static_cast<float>(m_size.y) / static_cast<float>(m_gridMapping.y);
    size_t size = sorted_by_position.size();

    for(std::size_t i{0}; i < size; i++)
    {
        Widget* w = sorted_by_position[i];
        
        // grid indices
        const std::size_t col = i % m_gridMapping.x;
        const std::size_t row = i / m_gridMapping.x;

        // place based on center
        const float x = col * width_difference + (width_difference - w->m_size.x) / 2.f;
        const float y = row * height_difference + (height_difference - w->m_size.y) / 2.f;

        w->setWidgetPosition({x, y});
    }
    toggle(m_flags, FLAGS::GRID | FLAGS::SHORTSKIP);
    updateFrameHeight();
}

void Frame::drawSeparators()
{
    if(!checkFlag(m_flags, FLAGS::SEPARATORS) || m_childWidgets.size() < 2) return;

    sf::Color separator_color = *m_currentOutlineColorPtr;
    std::vector<Widget*> sorted_by_position;
    sorted_by_position.reserve(m_childWidgets.size());

    for(Widget* w : m_childWidgets)
    {
        if(w->isFlag(FLAGS::HIDDEN)) continue;
        sorted_by_position.emplace_back(w);
    }

    // sort by y, smallest y first
    if(m_separatorType == SeparatorType::Horizontal)
    {
        std::sort(sorted_by_position.begin(), sorted_by_position.end(), [](const Widget* a, const Widget* b)
        {
            return a->m_position.y < b->m_position.y;
        });
    }
    // or sort by x, smallest x first
    else
    {
        std::sort(sorted_by_position.begin(), sorted_by_position.end(), [](const Widget* a, const Widget* b)
        {
            return a->m_position.x < b->m_position.x;
        });
    }

    size_t size = sorted_by_position.size();
    // we don't want to draw a line after the last widget
    for(size_t i{0}; i < size - 1; ++i)
    {
        Widget* Pcurrent = sorted_by_position[i];
        Widget* Pnext    = sorted_by_position[i + 1];
        sf::Vertex line[2];

        if(m_separatorType == SeparatorType::Horizontal)
        {
            // gap between bottom of current and top of next
            float current_bottom_y = Pcurrent->m_position.y + Pcurrent->m_size.y;
            float distance = Pnext->m_position.y - current_bottom_y;

            if(distance < m_separatorDistance) continue;

            // center line in the gap
            float line_length = m_size.x * m_separatorRatio;
            float line_position_x = (m_size.x - line_length) / 2.f; // centered horizontally
            float line_position_y = current_bottom_y + distance / 2.f;

            line[0].position = {line_position_x, line_position_y};
            line[0].color = separator_color;
            line[1].position = {line_position_x + line_length, line_position_y};
            line[1].color = separator_color;
        }
        else
        {
            // gap between right of current and left of next
            float current_right_x = Pcurrent->m_position.x + Pcurrent->m_size.x;
            float distance = Pnext->m_position.x - current_right_x;

            if(distance < m_separatorDistance) continue;

            // center line in the gap
            float line_length = m_size.y * m_separatorRatio;
            float line_position_x = current_right_x + distance / 2.f;
            float line_position_y = (m_size.y - line_length) / 2.f; // centered vertically

            line[0].position = {line_position_x, line_position_y};
            line[0].color = separator_color;
            line[1].position = {line_position_x, line_position_y + line_length};
            line[1].color = separator_color;
        }

        m_texture.draw(line, 2, sf::PrimitiveType::Lines);
    }
}

void Frame::updateFrameHeight()
{
    if(checkFlag(m_flags, FLAGS::SHORTSKIP)) return; //!< prevent calling for every widget
    float maxBottom = m_size.y;
    for(Widget* w : m_childWidgets)
    {
        float bottom = w->m_position.y + w->m_size.y;
        if(bottom > maxBottom)
            maxBottom = bottom;
    }

    m_contentHeight = maxBottom;
    updateScrollView();
}

void Frame::updateScrollView()
{
    m_maxScroll = std::max(0.f, m_contentHeight - m_size.y);

    // just in case we lost widget content
    m_scrollOffset = std::clamp(m_scrollOffset, 0.f, m_maxScroll);
    refreshView();
}

void Frame::refreshView()
{
    sf::View v = m_texture.getView();
    v.setCenter({static_cast<float>(m_size.x) / 2.f,
                 static_cast<float>(m_size.y) / 2.f + m_scrollOffset});
    m_texture.setView(v);
}

void Frame::scroll(float delta)
{
    if(!checkFlag(m_flags, FLAGS::SCROLL)) return;

    m_scrollOffset = std::clamp(m_scrollOffset + delta * (parameters::SCROLL_RATIO * m_size.y), 0.f, m_maxScroll);
    refreshView();
}

void Frame::drawChildWidget(const sf::Drawable& __widget, const sf::RenderStates state)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN)) return;

    // prevent refreshing at each call
    if(!checkFlag(m_flags, FLAGS::DRAWING))
    {
        setFlag(m_flags, FLAGS::DRAWING);
        checkQueue();
        updateState();
    }

    m_texture.draw(__widget, state);
}

void Frame::addWidget(Widget& __widget)
{
    m_childWidgets.emplace_back(&__widget);
    updateGrid();
    updateFrameHeight();
    setFlag(m_flags, FLAGS::LONGSKIP); //!< skip initial drawing part (child widget handles it)
}

void Frame::removeWidget(Widget& __widget)
{
    std::vector<Widget*>::iterator it = std::find(m_childWidgets.begin(), m_childWidgets.end(), &__widget);
    if(it != m_childWidgets.end()) 
    {
        m_childWidgets.erase(it);
        updateGrid();
        updateFrameHeight();
        if(m_childWidgets.empty()) resetFlag(m_flags, FLAGS::LONGSKIP); //!< not skipping essential updates as no child widget handles it
    }
}

void Frame::updateWidget(Widget& __old_widget, Widget& __new_widget)
{
    std::vector<Widget*>::iterator it = std::find(m_childWidgets.begin(), m_childWidgets.end(), &__old_widget);
    if(it != m_childWidgets.end())
    {
        m_childWidgets.erase(it);
        m_childWidgets.emplace_back(&__new_widget);
    }
}
}