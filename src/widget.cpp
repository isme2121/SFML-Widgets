#include <cmath>
#include "../include/frame.hpp"

namespace sfml_widgets // Widget
{
///////////////////////////////////////////////////////////////////
/// Public
///////////////////////////////////////////////////////////////////

Widget::Widget(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size) : m_master(root), m_size(size), m_position(position),
m_originalPosition(position), m_originalSize(sf::Vector2f(size))
{
    setFlag(m_flags, FLAGS::INIT);
    s_widgets.emplace_back(this);
    m_windowSize = sf::Vector2f(root.getSize());
}

Widget::Widget(Frame& root, sf::Vector2f position, sf::Vector2u size) : m_master(root), m_size(size), m_position(position),
m_originalPosition(position), m_originalSize(sf::Vector2f(size))
{
    root.addWidget(*this);
    setFlag(m_flags, FLAGS::INIT);
    s_widgets.emplace_back(this);
    m_windowSize = sf::Vector2f(root.getWindowPtr()->getSize());
}

Widget::~Widget()
{
    resetBackgroundTexture();
    if(s_focusedWidgetPtr == this) s_focusedWidgetPtr = nullptr;
    if(s_widgetToFocusPtr == this) s_widgetToFocusPtr = nullptr;
    if(auto w_group = m_groupPtr.lock()) w_group->remove(this);
    if(Frame* root = m_master.getFramePtr()) root->removeWidget(*this);
    std::vector<Widget*>::const_iterator this_is_it = std::find(s_widgets.begin(), s_widgets.end(), this);
    if(this_is_it != s_widgets.end()) s_widgets.erase(this_is_it);
}

void Widget::draw()
{
    if(!checkFlag(m_flags, FLAGS::LONGSKIP)) //!< check if widget calls for a skip
    {
        checkQueue();   //!< updates queue clock and triggers callback
        updateTheme();  //!< syncs shared theme with widget
        updateState();  //!< updates shaders and coloring
    }
    
    updateFocus();      //!< updates focused widget and triggers focus and unfocus callback
    
    if(checkFlag(m_flags, FLAGS::HIDDEN) || checkFlag(m_flags, FLAGS::AUTOHIDE) && !checkFlag(m_flags, FLAGS::HOVERED)) return;

    if(m_bgShaderPtr) m_master.draw(*this, m_bgShaderPtr.get());
    else m_master.draw(*this);

    drawWidget();
    if(m_ToolTipPtr) m_ToolTipPtr->draw(checkFlag(m_flags, FLAGS::HOVERED));
}

void Widget::configure(FLAGS flags)
{
    // filter state flags
    FLAGS mask = flags & parameters::MODIFIABLE;
    toggle(m_flags, mask);
    setFlag(m_flags, FLAGS::MODIFIED);

    // flags that should remain in sync with root frame if there is one
    if(Frame* root = m_master.getFramePtr())
    {
        if(root->isFlag(FLAGS::HIDDEN)) mask |= FLAGS::HIDDEN;
        if(root->isFlag(FLAGS::BLOCKED)) mask |= FLAGS::BLOCKED;
    }

    if(checkFlag(m_flags, FLAGS::NOBGIMAGE)) resetBackgroundTexture();
    
    /* flags that should be immediately updated */

    // apply transparency if flag was just set
    if(checkFlag(m_flags, FLAGS::TRANSPARENT))
        setFlag(m_flags, FLAGS::ALPHAREDUCED);

    // no TRANSPARENT, remove transparency if we are hovering
    else if(checkFlag(m_flags, FLAGS::HOVERED))
        resetFlag(m_flags, FLAGS::ALPHAREDUCED);

    // no TRANSPARENT | HOVERED, remove transparency if AT was just reset
    else if(!checkFlag(m_flags, FLAGS::AUTOTRANSPARENT) && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
        resetFlag(m_flags, FLAGS::ALPHAREDUCED);

    // or the opposite of ^
    else if(checkFlag(m_flags, FLAGS::AUTOTRANSPARENT) && !checkFlag(m_flags, FLAGS::HOVERED))
        setFlag(m_flags, FLAGS::ALPHAREDUCED);

    config(mask);
}

bool Widget::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED)) return false;

    if(!mouse_position) *mouse_position = getMousePosition();    

    updateMouseState(event, *mouse_position); //!< HOVERED & ALPHAREDUCED flags are updated here

    if(preProcessMouseEvents(event, *mouse_position)) return true;

    W_MouseEventDetails event_details;

    // pressed mouse button
    if(const sf::Event::MouseButtonPressed* clicked = event.getIf<sf::Event::MouseButtonPressed>())
    {
        event_details.event_type = W_MouseEventType::Press;
        event_details.button = clicked->button;

        m_memButton.reset(); //!< new pressed button
    
        if(checkFlag(m_flags, FLAGS::HOVERED))
        {
            // only time select and focus flag are set is when clicked
            setFlag(m_flags, FLAGS::SELECTED);
            focusWidget(*this, event);

            m_memButton.emplace(clicked->button); //!< for release event
            if(clicked->button == parameters::ACTIVE_MOUSE_BUTTON) setFlag(m_flags, FLAGS::ACTIVE);
            
            const std::unordered_map<sf::Mouse::Button, MouseCallback>::iterator it = m_pressEvents.find(clicked->button);
            if(it != m_pressEvents.end()) it->second(*clicked);

            event_details.is_used = true;
        }
        else
        {
            resetFlag(m_flags, FLAGS::SELECTED | FLAGS::ACTIVE);
            unfocusWidget(*this, event); //!< only time a widget in unfocused is when clicking outside of bounds
        }
    }

    // released mouse button (after pressing it)
    else if(const sf::Event::MouseButtonReleased* released = event.getIf<sf::Event::MouseButtonReleased>())
    {
        sf::Mouse::Button released_button = released->button;

        event_details.event_type = W_MouseEventType::Release;
        event_details.button = released_button;

        if(released_button == parameters::ACTIVE_MOUSE_BUTTON) resetFlag(m_flags, FLAGS::ACTIVE);

        // if widget is selected & hovered & memory mouse button was released
        if(checkFlags(m_flags, FLAGS::SELECTED | FLAGS::HOVERED) && m_memButton && m_memButton.value() == released_button)
        {
            const std::unordered_map<sf::Mouse::Button, MouseCallback>::iterator it = m_releaseEvents.find(released_button);
            if(it != m_releaseEvents.end()) it->second(*released);
            event_details.is_used = true;
        }
        else
        {
            // widget remains selected unless if mouse was pressed or released outside of bounds
            resetFlag(m_flags, FLAGS::SELECTED);
        }
        // memory resets after releasing mouse anywhere
        m_memButton.reset();
    }

    // scrolled while hovering
    else if(const sf::Event::MouseWheelScrolled* scrolled = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        event_details.event_type = W_MouseEventType::Scroll;
        event_details.scroll_delta = scrolled->delta;

        if(checkFlag(m_flags, FLAGS::HOVERED) && m_scrollCallBack)
        {
            m_scrollCallBack(*scrolled);
            event_details.is_used = true;
        }
    }

    else if(const sf::Event::Resized* resized = event.getIf<sf::Event::Resized>())
    {
        event_details.is_used = true;
        Widget::resizeWidgets(m_master.getRootWindowPtr(), *resized);
    }
    
    // holding mouse button
    else if(checkFlag(m_flags, FLAGS::ACTIVE))
    {
        event_details.event_type = W_MouseEventType::Hold;
        event_details.button = parameters::ACTIVE_MOUSE_BUTTON;

        if(m_holdCallBack)
        {
            m_holdCallBack(event);
            event_details.is_used = true;
        }
        // prevents calling press callback after releasing hold
        if(checkFlag(m_flags, FLAGS::SELECTED) && parameters::UNSELECT_WHEN_HOLD)
        {
            resetFlag(m_flags, FLAGS::SELECTED);
        }
    }

    postProcessMouseEvents(event_details, event, *mouse_position);
    return event_details.is_used;

}
void Widget::setWidgetPosition(sf::Vector2f new_position)
{
    m_position = new_position;
    updatePosition(); //!< this could change m_position before the important process
    if(s_setOriginals) updateDimensions(); //!< update originals

    // make sure the shape remains centered while not changing in position
    this->setOrigin(m_size / 2.f);
    this->setPosition(m_position + m_size / 2.f);

    // update frame height if it exists
    if(Frame* root = m_master.getFramePtr())
    {
        root->updateGrid();
        root->updateFrameHeight();
    }
}

void Widget::setWidgetSize(sf::Vector2f new_size)
{
    if(new_size.x < 0.f || new_size.y < 0.f) return;
    m_size = new_size;
    updateSize(); //!< this could change m_size before the important process
    if(s_setOriginals) updateDimensions(); //!< update originals

    this->setOrigin(m_size / 2.f); //!< keep widget centered for transformations

    // this prevents the widget from expanding from the center, causing it to change in position
    setWidgetPosition(m_position);

    // this denies a deforming of the shape with large radius values
    m_radius = std::min(m_radius, std::min<float>(m_size.x, m_size.y) / 2.f);

    // update frame height if it exists
    if(Frame* root = m_master.getFramePtr())
    {
        root->updateGrid();
        root->updateFrameHeight();
    }
    update();   //!< forgot that Shape overrides are in widget now
}

void Widget::bindMouseEvent(W_MouseEventType event_type, MouseCallback callback, std::optional<sf::Mouse::Button> button)
{
    switch (event_type)
    {
    case W_MouseEventType::Press:
        if(button) m_pressEvents[*button] = std::move(callback);
        break;

    case W_MouseEventType::Release:
        if(button) m_releaseEvents[*button] = std::move(callback);
        break;

    case W_MouseEventType::Hold:
        m_holdCallBack = std::move(callback);
        break;

    case W_MouseEventType::Scroll:
        m_scrollCallBack = std::move(callback);
        break;

    case W_MouseEventType::HoverSet:
        m_gainhoverCallBack = std::move(callback);
        break;

    case W_MouseEventType::HoverLost:
        m_losehoverCallBack = std::move(callback);
        break;

    case W_MouseEventType::FocusSet:
        m_gainFocusCallBack = std::move(callback);
        break;

    case W_MouseEventType::FocusLost:
        m_loseFocusCallBack = std::move(callback);
        break;

    default:
        break;
    }
}

void Widget::unbindMouseEvent(W_MouseEventType event_type, std::optional<sf::Mouse::Button> button)
{

    switch (event_type)
    {
    case W_MouseEventType::Press:
        if(button) m_pressEvents.erase(*button);
        break;

    case W_MouseEventType::Release:
        if(button) m_releaseEvents.erase(*button);
        break;

    case W_MouseEventType::Hold:
        m_holdCallBack = nullptr;
        break;

    case W_MouseEventType::Scroll:
        m_scrollCallBack = nullptr;
        break;

    case W_MouseEventType::HoverSet:
        m_gainhoverCallBack = nullptr;
        break;

    case W_MouseEventType::HoverLost:
        m_losehoverCallBack = nullptr;
        break;

    case W_MouseEventType::FocusSet:
        m_gainFocusCallBack = nullptr;
        break;

    case W_MouseEventType::FocusLost:
        m_loseFocusCallBack = nullptr;
        break;

    default:
        break;
    }
}

void Widget::bindShortCut(const std::string& tag, Sequence key_sequence, SequenceFunction callback)
{
    m_binds.addSequence(tag, key_sequence, callback);
}

void Widget::unbindShortCut(const std::string& tag)
{
    m_binds.deleteSequence(tag);
}

bool Widget::loadClipShader(const std::string_view& shader_path)
{
    if(!sf::Shader::isAvailable()) return false;
    m_clipShaderPtr= std::make_unique<sf::Shader>();
    if(m_clipShaderPtr->loadFromFile(shader_path, sf::Shader::Type::Fragment))
    {
        updateClipShader();
        return true;
    };
    m_clipShaderPtr.reset();
    return false;
}

bool Widget::loadColorShader(const std::string_view& shader_path)
{
    if(!sf::Shader::isAvailable()) return false;
    
    // discarding previous shader because a fail in loadFromFile may end up clearing it anyways
    m_bgShaderPtr = std::make_unique<sf::Shader>();
    if(m_bgShaderPtr->loadFromFile(shader_path, sf::Shader::Type::Fragment))
    {   
        updateColorShader();
        return true;
    };
    m_bgShaderPtr.reset();
    return false;
}

bool Widget::setBackgroundTexture(const std::string& file_path)
{
    if(checkFlag(m_flags, FLAGS::NOBGIMAGE)) return false;
    // load from disk
    ImageCache& global_image_buffer = ImageCache::getInstance(); 
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(file_path);
    if(!disk_image) return false;

    // attempt to make a resized copy
    std::optional<sf::Image> resized_image = resize_image(**disk_image, sf::Vector2u(m_size));
    if(!resized_image)
    {
        global_image_buffer.decrementRefCounter(file_path);
        return false;
    }

    // attempt to copy it
    sf::Texture temp;
    if(!temp.loadFromImage(*resized_image))
    {
        global_image_buffer.decrementRefCounter(file_path);
        return false;
    }

    if(m_bgShaderPtr)
    {
        global_image_buffer.decrementRefCounter(m_bgImagePath);
        *m_bgTexturePtr = std::move(temp); //!< use existing memory
    }
    else
    {
        // or allocate
        m_bgTexturePtr = std::make_unique<sf::Texture>(std::move(temp));
    }
    
    // loading shader
    if(!m_bgShaderPtr) loadColorShader(parameters::TEXTURE_COLOR_SHADER_PATH);

    // texture update
    m_bgTexturePtr->setSmooth(true);
    this->setTexture(m_bgTexturePtr.get(), true);
    m_bgImagePath = std::move(file_path);

    return true;
}

void Widget::resetBackgroundTexture()
{
    // BGIMAGE flag is always synced with ptr counter part
    if(!m_bgTexturePtr) return;

    ImageCache::getInstance().decrementRefCounter(m_bgImagePath);
    // clear all image data
    m_bgTexturePtr.reset();
    m_bgImagePath.clear();
    this->setTexture(nullptr);
}

void Widget::refreshBackgroundTexture()
{
    if(!m_bgShaderPtr) return;

    std::optional<sf::Image> resized_image;
    std::optional<const sf::Image*> disk_image = ImageCache::getInstance().getImage(m_bgImagePath, false);

    if(disk_image) resized_image = resize_image(**disk_image, sf::Vector2u(m_size));
    else resized_image = resize_image(m_bgTexturePtr->copyToImage(), sf::Vector2u(m_size));

    if(resized_image)
    {
        sf::Texture temp;
        if(temp.loadFromImage(*resized_image))
        {
            *m_bgTexturePtr = std::move(temp);
            m_bgTexturePtr->setSmooth(true);
            this->setTexture(m_bgTexturePtr.get(), true);
        }
    }
}

void Widget::setCornerRadius(float radius)
{
    m_radius = std::max(0.f, std::min(radius, std::min<float>(m_size.x, m_size.y) / 2.f));

    // settle with rect if radius is 0 or radius is set but shader is not available
    if(m_radius == 0.f)
    {
        m_radius = 0.f;
        m_cornerPoints = 1;
    }
    else
    {
        m_cornerPoints = parameters::CORNER_POINTS;
    }
    update();
    updateClipShader();
}

void Widget::rotateWidget(sf::Angle angle)
{
    this->rotate(angle);
    updateRotation();
}

void Widget::setWidgetRotation(sf::Angle new_angle)
{
    this->setRotation(new_angle);
    updateRotation();
}

void Widget::addToQueue(sf::Time after_ms, std::function<void()> callback, bool is_priority)
{
    if(is_priority && !m_events.empty())
    {
        m_events.front().pause();
        m_events.emplace_front(std::move(after_ms), std::move(callback));
    }
    else m_events.emplace_back(std::move(after_ms), std::move(callback));
    checkQueue();
}

void Widget::addToQueue(std::size_t after_frames, std::function<void()> callback, bool is_priority)
{
    if(is_priority && !m_events.empty())
    {
        m_events.front().pause();
        m_events.emplace_front(std::move(after_frames), std::move(callback));
    }
    else m_events.emplace_back(std::move(after_frames), std::move(callback));

    checkQueue();
}

void Widget::addRoutine(sf::Time ms_period, std::function<bool()> callback)
{
    m_repeatEvents.emplace_back(ms_period, std::move(callback));
}

void Widget::addRoutine(std::size_t frames_period, std::function<bool()> callback)
{
    m_repeatEvents.emplace_back(frames_period, std::move(callback));
}

void Widget::clearRoutine()
{
    m_repeatEvents.clear();
}

void Widget::linkWidget(std::shared_ptr<WidgetGroup>& new_group)
{
    if(!new_group) return;

    std::shared_ptr<WidgetGroup> resource = m_groupPtr.lock();
    if(resource == new_group) return;
    
    new_group->add(this);
    if(resource) resource->remove(this);
    m_groupPtr = new_group;
}

void Widget::unlinkWidget()
{
    if(std::shared_ptr<WidgetGroup> resource = m_groupPtr.lock())
    {
        resource->remove(this);
        m_groupPtr.reset();
    }
}

void Widget::linkTheme(std::shared_ptr<WidgetTheme>& t)
{
    m_themePtr = t;
    m_generationToken = constants::KSTALE;
}

void Widget::unlinkTheme()
{
    m_themePtr.reset();
}

const sf::Vector2f Widget::getWidgetPosition() const
{
    return m_position;
}

const sf::Vector2f Widget::getWidgetSize() const
{
    return m_size;
}

const sf::Vector2f Widget::getAbsolutePosition() const
{
    sf::Vector2f results = m_position;
    const Frame* root = m_master.getFramePtr();
    while(root) //!< considering all layers
    {
        results.x += root->m_position.x;
        results.y += root->m_position.y;
        results.y -= root->m_scrollOffset;
        root = root->m_master.getFramePtr();
    }
    return results;
}

sf::Angle Widget::getWidgetRotation() const
{
    return this->getRotation();
}

const sf::FloatRect Widget::getBounds() const
{
    return this->getGlobalBounds();
}

const sf::FloatRect Widget::getAbsoluteBounds() const
{
    return sf::FloatRect(getAbsolutePosition(), m_size);
}

sf::Vector2i Widget::getMousePosition() const
{
    if(const sf::RenderWindow* root = m_master.getWindowPtr())  return sf::Mouse::getPosition(*root);
    else if(Frame* root = m_master.getFramePtr())  return sf::Mouse::getPosition(*root->getWindowPtr());
    return {0, 0}; //!< shouldn't reach here but here we are
}

bool Widget::isHovered(sf::Vector2i mouse_position) const 
{
    return this->getGlobalBounds().contains(m_master.mapMousePosition(mouse_position));
}

bool Widget::isFlag(FLAGS bits) const
{
    return checkFlag(m_flags, bits);
}

bool Widget::isFlags(FLAGS bits) const
{
    return checkFlags(m_flags, bits);
}

std::weak_ptr<ToolTip> Widget::EnableToolTip(sf::Vector2f abs_position, sf::Time delay_ms, unsigned int text_size, ToolTipStyle style)
{
    if(m_ToolTipPtr) return m_ToolTipPtr;

    m_ToolTipPtr = std::shared_ptr<ToolTip>(new ToolTip(*m_master.getRootWindowPtr(), abs_position, delay_ms, style));
    if(m_ToolTipPtr) m_ToolTipPtr->setCharacterSize(text_size);
    return m_ToolTipPtr;
}

void Widget::DisableToolTip()
{
    m_ToolTipPtr.reset();
}

///////////////////////////////////////////////////////////////////
/// Protected
///////////////////////////////////////////////////////////////////

bool Widget::preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position)
{
    return false;
}

void Widget::postProcessMouseEvents(W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    
}

void Widget::config(FLAGS mask)
{

}

void Widget::updateMouseState(const sf::Event& event, sf::Vector2i mouse_position)
{
    // hovering event
    if(isHovered(mouse_position))
    {
        if(!checkFlag(m_flags, FLAGS::HOVERED))
        {
            setFlag(m_flags, FLAGS::HOVERED);
            if(m_gainhoverCallBack) m_gainhoverCallBack(event);
        }
        if(!checkFlag(m_flags, FLAGS::TRANSPARENT)) resetFlag(m_flags, FLAGS::ALPHAREDUCED);
    }
    else
    {
        if(checkFlag(m_flags, FLAGS::HOVERED))
        {
            resetFlag(m_flags,  FLAGS::HOVERED);
            if(m_losehoverCallBack) m_losehoverCallBack(event);
        }

        bool root_hidden = false;
        if(Frame* root = m_master.getFramePtr()) root_hidden = root->isFlag(FLAGS::ALPHAREDUCED);

        if(checkFlags(m_flags, FLAGS::AUTOTRANSPARENT | FLAGS::TRANSPARENT) || root_hidden) setFlag(m_flags, FLAGS::ALPHAREDUCED);
        else resetFlag(m_flags, FLAGS::ALPHAREDUCED);
    }
}

void Widget::checkQueue()
{
    if(checkFlag(m_flags, FLAGS::BLOCKED)) return;

    // queue
    if(!m_events.empty())
    {
        if(m_events.front().advance()) m_events.pop_front();
    }

    // routines, all checked at once
    m_repeatEvents.erase(std::remove_if(m_repeatEvents.begin(), m_repeatEvents.end(),
                            [](W_RepeatEvent& e)
                            {
                                return e.advance();
                            }),
                            m_repeatEvents.end()
    );
}

void Widget::updateFocus()
{
    if(!s_FrequestFocusUpdate) return; //!< no changes? just return

    if(s_widgetToFocusPtr) //!< check if a widget is demanding the resource
    {
        if(s_focusedWidgetPtr != s_widgetToFocusPtr) //!< check if old is null or the same widget
        {
            /* here we know there's a new widget to focus */
            std::shared_ptr<WidgetGroup> new_w_group = s_widgetToFocusPtr->m_groupPtr.lock();
            bool same_group = false; //!< assuming for now they don't share the same group

            if(s_focusedWidgetPtr) //!< old is not null?
            {
                // previous focused widget loses the resource even if they share the same group
                // but only calls the callback if they don't share the same group
                resetFlag(s_focusedWidgetPtr->m_flags, FLAGS::FOCUSED);
                std::shared_ptr<WidgetGroup> old_w_group = s_focusedWidgetPtr->m_groupPtr.lock();
                same_group = old_w_group && new_w_group && old_w_group == new_w_group;

                if(!same_group)
                {
                    /*
                    we know they don't share the same group
                    so prev will call focusOff callback here if it has
                    */
                    if(old_w_group && old_w_group->groupFocuslossCallback) //!< old has a group and it's not the same as new and allows a group callback
                        old_w_group->focusOff(s_memUnfocusEvent);
                    else if(s_focusedWidgetPtr->m_loseFocusCallBack)
                        s_focusedWidgetPtr->m_loseFocusCallBack(s_memUnfocusEvent);
                }
            }

            /* here we assign the new focused widget */
            setFlag(s_widgetToFocusPtr->m_flags, FLAGS::FOCUSED);

            if(new_w_group && !same_group && new_w_group->groupFocusCallback)
                new_w_group->focusOn(s_memFocusEvent); //!< new group, never called if they share the same group
            else if(s_widgetToFocusPtr->m_gainFocusCallBack)
                s_widgetToFocusPtr->m_gainFocusCallBack(s_memFocusEvent); //!< always called as long as old widget != new widget

            s_focusedWidgetPtr = s_widgetToFocusPtr;
        }

    }

    else if(s_FrequestUnfocus && s_focusedWidgetPtr) //!< unfocus event
    {
        /* prev lose focus and no new widget demands the resource (clicked on non-widget space)*/
        
        resetFlag(s_focusedWidgetPtr->m_flags, FLAGS::FOCUSED);
        std::shared_ptr<WidgetGroup> old_w_group = s_focusedWidgetPtr->m_groupPtr.lock();

        if(old_w_group && old_w_group->groupFocuslossCallback)
            old_w_group->focusOff(s_memUnfocusEvent);
        else if(s_focusedWidgetPtr->m_loseFocusCallBack)
            s_focusedWidgetPtr->m_loseFocusCallBack(s_memUnfocusEvent);

        s_focusedWidgetPtr = nullptr;

    }
    s_widgetToFocusPtr = nullptr;
    s_memFocusEvent.reset();
    s_memUnfocusEvent.reset();
    s_FrequestUnfocus = false;
    s_FfoundFocusedWidget = false;
    s_FrequestFocusUpdate = false;
}

void Widget::updateDimensions()
{
    m_originalPosition = m_position;
    m_originalSize = m_size;
    if(const sf::RenderWindow* pw = m_master.getRootWindowPtr()) m_windowSize = sf::Vector2f(pw->getSize());
}

std::size_t Widget::getPointCount() const
{
    return m_cornerPoints * 4;
}

sf::Vector2f Widget::getPoint(std::size_t index) const {
    std::size_t corner = index / m_cornerPoints;
    float angle = static_cast<float>(index) / m_cornerPoints * constants::DEGREE90;
    float rad = angle * constants::PI / constants::DEGREE180;

    // Center of each corner's arc circle
    sf::Vector2f centers[4] = {
        { m_radius, m_radius },
        { m_size.x - m_radius, m_radius },
        { m_size.x - m_radius, m_size.y - m_radius },
        { m_radius, m_size.y - m_radius }
    };

    float a = constants::START_ANGLES[corner] + static_cast<float>(index % m_cornerPoints) / m_cornerPoints * (constants::PI / 2.f);

    return {
        centers[corner].x + std::cos(a) * m_radius,
        centers[corner].y + std::sin(a) * m_radius
    };
}
///////////////////////////////////////////////////////////////////
/// Static methods
///////////////////////////////////////////////////////////////////

void Widget::focusWidget(Widget& w, std::optional<sf::Event> event) noexcept
{
    if(s_FfoundFocusedWidget && !parameters::STACK_MOUSE_CALLBACKS) return;

    s_widgetToFocusPtr = &w;
    s_memFocusEvent.emplace(*event);
    s_FfoundFocusedWidget = true; //!< no more checking unless macro is set
    s_FrequestFocusUpdate = true;
}

void Widget::unfocusWidget(Widget& w, std::optional<sf::Event> event) noexcept
{
    if(s_focusedWidgetPtr && s_focusedWidgetPtr == &w)
    {
        s_FrequestUnfocus = true;
        s_memUnfocusEvent.emplace(*event);
        s_FrequestFocusUpdate = true;
    }
}

void Widget::resizeWidgets(const sf::RenderWindow* win_ptr, const sf::Event::Resized& event)
{
    sf::Vector2f win_size = sf::Vector2f(event.size);
    if(win_size.x * win_size.y == 0.f)
        return;

    bool updated_view = false; //!< updating the resized window's view once

    for(Widget* w : s_widgets)
    {
        const sf::RenderWindow* root = w->m_master.getRootWindowPtr();

        // root is the resized window?
        if((!updated_view) && root && win_ptr == root)
        {        
            // manually update view of the window to the new size
            sf::FloatRect view_size({0.f, 0.f}, win_size);
            w->m_master.setWindowView(sf::View(view_size));
            updated_view = true;
        }

        if(!checkFlag(w->m_flags, FLAGS::RESIZE)) continue;
        
        // ratio based on the win size we had before any resize event
        // to try to match the original window as much as possible
        sf::Vector2f ratio = 
        {
            win_size.x / w->m_windowSize.x,
            win_size.y / w->m_windowSize.y
        };

        s_setOriginals = false; //!< preventing original dimensions update from a resize
        w->setWidgetPosition(
        {
            w->m_originalPosition.x * ratio.x,
            w->m_originalPosition.y * ratio.y
        });

        w->setWidgetSize(
        {
            w->m_originalSize.x * ratio.x,
            w->m_originalSize.y * ratio.y
        });
        s_setOriginals = true;
    }
}
}

namespace sfml_widgets // W_QueueEvent
{
Widget::W_QueueEvent::W_QueueEvent(sf::Time ms_delay, std::function<void()> callback): m_type(W_CounterType::Time)
, m_delay(ms_delay), m_callback(std::move(callback))
{
    m_timer.reset();
}

Widget::W_QueueEvent::W_QueueEvent(std::size_t frames_delay, std::function<void()> callback) : m_type(W_CounterType::Frames)
, m_framesCount(frames_delay), m_callback(std::move(callback))
{

}

bool Widget::W_QueueEvent::advance()
{
    if(m_type == W_CounterType::Time)
    {
        if(!m_timer.isRunning()) m_timer.start();
        if(m_timer.getElapsedTime() >= m_delay)
        {
            m_callback();
            return true;
        }
    }
    else
    {
        --m_framesCount;
        if(m_framesCount == 0)
        {
            m_callback();
            return true;
        }
    }
    return false;
}

void Widget::W_QueueEvent::pause()
{
    if(m_type == W_CounterType::Time && m_timer.isRunning()) m_timer.stop();
}
}

namespace sfml_widgets // W_RepeatEvent
{
Widget::W_RepeatEvent::W_RepeatEvent(sf::Time ms_period, std::function<bool()> callback) : m_type(W_CounterType::Time)
, m_period(ms_period), m_callback(std::move(callback))
{
    m_timer.restart();
}

Widget::W_RepeatEvent::W_RepeatEvent(std::size_t frames_period, std::function<bool()> callback) : m_type(W_CounterType::Frames)
, m_period(sf::Time::Zero), m_framesPeriod(frames_period), m_callback(std::move(callback))
{

}

bool Widget::W_RepeatEvent::advance()
{
    if(m_type == W_CounterType::Time)
    {
        if(!m_timer.isRunning()) m_timer.start();
        if(m_timer.getElapsedTime() >= m_period)
        {
            m_timer.restart();
            return m_callback();
        }
    }
    else
    {
        ++m_framesCounter;
        if(m_framesCounter >= m_framesPeriod)
        {
            m_framesCounter = 0;
            return m_callback();
        }
    }
    return false;
}
}

namespace sfml_widgets // W_CharHeight
{
sf::Text Widget::W_CharHeight::Ag(parameters::font, "Ag");

float Widget::W_CharHeight::getHeight(sf::Font text_font, unsigned int char_size)
{
    Ag.setFont(text_font);
    Ag.setCharacterSize(char_size);
    return Ag.getLocalBounds().size.y;
}
}