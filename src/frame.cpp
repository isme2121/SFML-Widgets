#include "frame.hpp"
#include "widget.hpp"

namespace sfml_widgets // Frame
{
///////////////////////////////////////////////////////////////////
/// Constructors / Destructor
///////////////////////////////////////////////////////////////////

Frame::Frame(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size) : Widget(window, position, size),
m_window(&window), m_texture(size), m_sprite(m_texture.getTexture()), m_contentHeight(size.y)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    toggle(m_flags, FLAGS::SCROLL);
    m_skipAfterPreProcess.emplace(false);
}

Frame::Frame(Frame& root, sf::Vector2f position, sf::Vector2u size) : Widget(root, position, size),
m_window(root.getWindowPtr()), m_texture(size), m_sprite(m_texture.getTexture()), m_contentHeight(size.y)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    toggle(m_flags, FLAGS::SCROLL);
    m_skipAfterPreProcess.emplace(false);
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
    // places child widgets on the root window
    else if(m_window != nullptr)
    {
        for(Widget* w : m_childWidgets)
        {
            w->m_master.setWindowPtr(m_window);
            w->m_master.setFramePtr(nullptr);
            w->m_isRootHidden = nullptr;
        }
    }
}

void Frame::setGrid(sf::Vector2u grid_mapping)
{
    m_gridMapping = grid_mapping;
    // check if non of the grids is 0
    if(m_gridMapping.x * m_gridMapping.y) 
    {
        setFlag(m_flags, FLAGS::GRID | FLAGS::UPDATE);
        return;
    }
    resetFlag(m_flags, FLAGS::GRID);
}

void Frame::setSeparators(F_SeparatorType type, float length_ratio, float sep_distance)
{
    m_separatorType = type;
    if(type == F_SeparatorType::NoSeparator)
    {
        resetFlag(m_flags, FLAGS::SEPARATORS);
        m_separatorRatio = 0.f;
        m_separatorDistance = 0.f;
        return;
    }
    m_separatorRatio = std::clamp(length_ratio, 0.f, 1.f);
    m_separatorDistance = sep_distance;
    setFlag(m_flags, FLAGS::SEPARATORS);
}

void Frame::setProcessMode(F_AfterPreProcess f_mode)
{
    m_processHolder.emplace(f_mode);
}

void Frame::setScrollSpeed(float ratio)
{
    if(ratio < 0.01f || ratio > 1.f) return;
    m_scrollSpeedRatio = ratio;
}

void Frame::setTheme(TextureStyle s_frame)
{
    m_style = s_frame;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Frame::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<Frame>());
}

TextureStyle& Frame::style()
{
    return m_style;
}

void Frame::scroll(float delta)
{
    if(!checkFlag(m_flags, FLAGS::SCROLL)) return;

    m_scrollOffset = std::clamp(m_scrollOffset + (-delta) * (m_scrollSpeedRatio * m_size.y), 0.f, m_maxScroll);
    refreshView();
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

sf::RenderWindow* Frame::getWindowPtr() const
{
    return m_window;
}

std::size_t Frame::getChildCount() const
{
    return m_childWidgets.size();
}

float Frame::getScrollOffset() const
{
    return m_scrollOffset;
}

float Frame::getScrollSpeed() const
{
    return m_scrollSpeedRatio;
}

const sf::Vector2u& Frame::getGridMap() const
{
    return m_gridMapping;
}

Frame::F_SeparatorInfo Frame::getSeparatorInfo() const
{
    return {m_separatorType, m_separatorRatio, m_separatorDistance};
}

Frame::F_AfterPreProcess Frame::getProcessMode() const
{
    if(m_skipAfterPreProcess)
    {
        if(*m_skipAfterPreProcess) return F_AfterPreProcess::SKIP_ALL;
        else return F_AfterPreProcess::SKIP_CALLBACK;
    }
    return F_AfterPreProcess::NO_SKIP;
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

bool Frame::preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position)
{
    // handle widgets events
    std::vector<Widget*>::reverse_iterator it_end = m_childWidgets.rbegin();
    std::vector<Widget*>::const_reverse_iterator it_begin = m_childWidgets.crend();
    bool event_used{false};

    while(it_end != it_begin)
    {
        if(isVisible(*it_end) && (*it_end)->captureMouseEvents(event, mouse_position))
        {
            event_used = true;
        }
        ++it_end;
    }
    return event_used;
}

bool Frame::postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    // handle frame unique events
    if(details.event_type == W_MouseEventType::Scroll && checkFlag(m_flags, FLAGS::HOVERED))
    {
        scroll(details.scroll_delta);
        return true;
    }
    return false;
}

bool Frame::preProcessKeyEvents(const sf::Event& event)
{
    bool event_used = false;
    // handle widgets events
    std::vector<Widget*>::reverse_iterator it_end = m_childWidgets.rbegin();
    std::vector<Widget*>::const_reverse_iterator it_begin = m_childWidgets.crend();
    while(it_end != it_begin)
    {
        if((*it_end)->captureKeyEvents(event)) event_used = true;
        ++it_end;
    }
    return event_used;
}

void Frame::drawWidget()
{
   if(checkFlag(m_flags, FLAGS::UPDATE))
   {
        updateGrid();
        updateFrameHeight();
        resetFlag(m_flags, FLAGS::UPDATE);
   }

    if(m_processHolder)
    {
        if(*m_processHolder == F_AfterPreProcess::NO_SKIP) m_skipAfterPreProcess.reset();
        else if(*m_processHolder == F_AfterPreProcess::SKIP_CALLBACK) m_skipAfterPreProcess.emplace(false);
        else m_skipAfterPreProcess.emplace(true);
        m_processHolder.reset();
    }

    for(Widget* w : m_childWidgets)
    {
        if(isVisible(w)) w->draw();
    }

    display();
}

void Frame::config(FLAGS mask)
{
    mask &= (FLAGS::BLOCKED | FLAGS::RESIZE);
    for(Widget* w : m_childWidgets) w->configure(mask);
}

void Frame::updatePosition()
{
    /*
    m_position is relative to the top left corner of the shape,
    we do this since m_sprite's origin is it's center, just
    like the base widget
    */
    m_sprite.setPosition(m_position + m_size / 2.f);
}

void Frame::updateSize()
{
    sf::Vector2u temp = sf::Vector2u(m_size);
    if(!m_texture.resize(temp)) return;

    // update texture in sprite
    m_sprite.setTexture(m_texture.getTexture(), true);
    m_sprite.setOrigin(m_size / 2.f);

    // update grid & max view height
    setFlag(m_flags, FLAGS::UPDATE);

    // shader scales with size
    updateClipShader();
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

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.5f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.25f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ACTIVE))
    {
        m_opacity = 1.f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.outline_active_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
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

void Frame::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<Frame>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Frame>());
    }
}

void Frame::checkQueue()
{
    Widget::checkQueue();
    for(Widget* w : m_childWidgets)
    {
        w->checkQueue();
        if(m_isHidden) w->m_loopGuard = false;
    }
}

bool Frame::isVisible(const Widget* w)
{
    sf::FloatRect view_bounds
    {
        sf::Vector2f{0.f, m_scrollOffset},
        m_size
    };
    return view_bounds.findIntersection(w->getBounds()).has_value();

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
        if(w->isHidden()) continue;
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

    const float width_difference = m_size.x / static_cast<float>(m_gridMapping.x);
    const float height_difference = m_size.y / static_cast<float>(m_gridMapping.y);
    std::size_t size = sorted_by_position.size();

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
}

void Frame::drawSeparators()
{
    if(!checkFlag(m_flags, FLAGS::SEPARATORS) || m_childWidgets.size() < 2) return;

    sf::Color separator_color = *m_currentOutlineColorPtr;
    std::vector<Widget*> sorted_by_position;
    sorted_by_position.reserve(m_childWidgets.size());

    for(Widget* w : m_childWidgets)
    {
        if(w->isHidden()) continue;
        sorted_by_position.emplace_back(w);
    }

    // sort by y, smallest y first
    if(m_separatorType == F_SeparatorType::Horizontal)
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

    std::size_t n_widgets = sorted_by_position.size();
    // we don't want to draw a line after the last widget
    for(std::size_t i{0}; i < n_widgets - 1; ++i)
    {
        sf::Vertex line[2];
        Widget* current_widget = sorted_by_position[i];
        Widget* new_widget = sorted_by_position[i + 1];

        if(m_separatorType == F_SeparatorType::Horizontal)
        {
            // gap between bottom of current and top of next
            float current_bottom_y = current_widget->m_position.y + current_widget->m_size.y;
            float distance = new_widget->m_position.y - current_bottom_y;

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
            float current_right_x = current_widget->m_position.x + current_widget->m_size.x;
            float distance = new_widget->m_position.x - current_right_x;

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

void Frame::display()
{
    if(m_isHidden) return;

    /* 
    * if LONGSKIP is set --> this process was skipped initially, this could happen if
    * child widgets were manually drawn without calling draw from root Frame object
    * 
    * if LONGSKIP is reset --> this process was not skipped initially, this happens
    * since this frame has no child widgets currently and is drawn as a widget instead
    * of a container
    */
    drawSeparators();

    m_texture.display();
    m_sprite.setTexture(m_texture.getTexture());
    if(m_clipShaderPtr) m_master.draw(m_sprite, m_clipShaderPtr.get());
    else m_master.draw(m_sprite);
}

void Frame::drawChildWidget(const sf::Drawable& w, const sf::RenderStates state)
{
    if(!checkFlag(m_flags, FLAGS::DRAWING))
    {
        setFlag(m_flags, FLAGS::DRAWING);
        checkQueue();
        updateTheme();
        updateState();
    }

    m_texture.draw(w, state);
}

void Frame::addWidget(Widget& w)
{
    m_childWidgets.emplace_back(&w);
    w.m_isRootHidden = &m_isHidden;
    setFlag(m_flags, FLAGS::UPDATE); //!< skip initial drawing part (child widget handles it) and call for view update

    /*
    * At this point, w only ran it's Widget constructor so the V table
    * is not mapped yet therefore any virtual method call will crash the program.
    * 
    * Removed updateGrid from here because it called setWidgetPosition which
    * internally calls the virtual updatePosition.
    */
}

void Frame::removeWidget(Widget& w)
{
    std::vector<Widget*>::iterator it = std::find(m_childWidgets.begin(), m_childWidgets.end(), &w);
    if(it != m_childWidgets.end()) 
    {
        m_childWidgets.erase(it);
        setFlag(m_flags, FLAGS::UPDATE); //!< call for view update
    }
}

void Frame::updateWidget(Widget& old_widget, Widget& new_widget)
{
    std::vector<Widget*>::iterator it = std::find(m_childWidgets.begin(), m_childWidgets.end(), &old_widget);
    if(it != m_childWidgets.end())
    {
        m_childWidgets.erase(it);
        m_childWidgets.emplace_back(&new_widget);
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
    v.setCenter({m_size.x / 2.f,
                 m_size.y / 2.f + m_scrollOffset});
    m_texture.setView(v);
}
}