#include "../include/frame.hpp"
#include "../include/grid_canvas.hpp"

namespace sfml_widgets // GridCanvas
{
GridCanvas::GridCanvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, size_t cell_count, float dead_zone, bool fill_initially) : Widget(window, position, size), 
m_drawTexture(size), m_drawSprite(m_drawTexture.getTexture()), MAX_CELL_COUNT(cell_count), m_deadZone(std::clamp(dead_zone, 0.f, std::min<float>(m_size.x, m_size.y) * s_MAX_DEADZONE_RATIO)),
m_gridMapping(get_prime_combination(cell_count))
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    m_cellMap.reserve(MAX_CELL_COUNT);
    if(fill_initially) fill();
    toggle(m_flags, FLAGS::ZOOM | FLAGS::PAN);
}

GridCanvas::GridCanvas(Frame& root, sf::Vector2f position, sf::Vector2u size, size_t cell_count, float dead_zone, bool fill_initially) : Widget(root, position, size), 
m_drawTexture(size), m_drawSprite(m_drawTexture.getTexture()), MAX_CELL_COUNT(cell_count), m_deadZone(std::clamp(dead_zone, 0.f, std::min<float>(m_size.x, m_size.y) * s_MAX_DEADZONE_RATIO)),
m_gridMapping(get_prime_combination(cell_count))
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;
    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    m_cellMap.reserve(MAX_CELL_COUNT);
    if(fill_initially) fill();
    toggle(m_flags, FLAGS::ZOOM | FLAGS::PAN);
}

Cell* GridCanvas::at(unsigned int row, unsigned int col)
{
    if(row < m_gridMapping.x || col < m_gridMapping.y)
    {
        sf::Vector2u coords{row, col};
        std::vector<Cell>::iterator it = std::find(m_cellMap.begin(), m_cellMap.end(), coords);
        if(it != m_cellMap.end()) return &*it;
    }
    return nullptr;
}

void GridCanvas::fill()
{
    for(std::size_t i{0}; i < MAX_CELL_COUNT; i++)
    {
        std::size_t current_row = i % m_gridMapping.x;
        std::size_t current_col = i / m_gridMapping.x;
        if(std::find(m_cellMap.begin(), m_cellMap.end(), sf::Vector2u(current_row, current_col)) != m_cellMap.end()) continue; //!< cell already exists here

        m_cellMap.emplace_back(*this, sf::Vector2u(current_row, current_col), m_cellSize, m_deadZone);
    }
}

bool GridCanvas::insert(sf::Vector2u cell_coordinates)
{
    // coordinates out of reach or max count reached
    if(m_cellMap.size() >= MAX_CELL_COUNT || cell_coordinates.x >= m_gridMapping.x || cell_coordinates.y >= m_gridMapping.y) return false;

    std::vector<Cell>::iterator it = std::find(m_cellMap.begin(), m_cellMap.end(), cell_coordinates);
    if(it != m_cellMap.end()) return false; //!< spot is taken

    m_cellMap.emplace_back(*this, cell_coordinates, m_cellSize, m_deadZone);
    return true;
}

void GridCanvas::remove(sf::Vector2u cell_coordinates)
{
    std::vector<Cell>::iterator it = std::find(m_cellMap.begin(), m_cellMap.end(), cell_coordinates);
    if(it == m_cellMap.end()) return; //!< not found

    it->clearImage();       //!< have to call destructor here
    m_cellMap.erase(it);    //!< why is erase like this
}

void GridCanvas::setActiveCells(std::function<bool(Cell&)> condition)
{
    for(Cell& cell : m_cellMap)
    {
        if(condition(cell))
        {
            resetFlag(cell.m_flags, FLAGS::BLOCKED);
        }
        else
        {
            setFlag(cell.m_flags, FLAGS::BLOCKED);
        }
    }
}

void GridCanvas::setDeadZone(float surface)
{
    m_deadZone = std::clamp(surface, 0.f, std::min<float>(m_size.x, m_size.y) * s_MAX_DEADZONE_RATIO);
    updateCellSize(); //!< scales with dead zone
}

void GridCanvas::setCellStyle(CellColor new_color)
{
    for(Cell& cell : m_cellMap)
    {
        cell.m_style = new_color;
        setFlag(cell.m_flags, FLAGS::MODIFIED);
    }
}

bool GridCanvas::isTaken(sf::Vector2u coordinates) const
{
    return std::find(m_cellMap.begin(), m_cellMap.end(), coordinates) != m_cellMap.end();
}

sf::Vector2i GridCanvas::getLocalPixel(sf::Vector2i mouse_position) const
{
    sf::Vector2f global_point = m_master.mapMousePosition(mouse_position);
    return sf::Vector2i(this->getInverseTransform().transformPoint(global_point));
}

sf::Vector2f GridCanvas::getViewPixel(sf::Vector2i mouse_position) const
{
    return m_drawTexture.mapPixelToCoords(getLocalPixel(mouse_position));
}

sf::Vector2f GridCanvas::mapLocalToView(sf::Vector2i local_pixel) const
{
    return m_drawTexture.mapPixelToCoords(local_pixel);
}

std::optional<sf::Vector2u> GridCanvas::mapCoordinates(sf::Vector2i mouse_position) const
{
    // considering zoom and pan levels
    sf::Vector2f view_pixel = getViewPixel(mouse_position);

    // removing left and top dead zones for easier management
    sf::Vector2f relative = {
        view_pixel.x - m_deadZone,
        view_pixel.y - m_deadZone
    };

    // only zone that can return coordinates
    sf::Vector2f cell_zone = {
        m_size.x - m_deadZone * 2.f,
        m_size.y - m_deadZone * 2.f
    };

    // check if we passed boundaries between 0 and coordinates zone
    if(relative.x < 0.f || relative.y < 0.f ||
       relative.x >= cell_zone.x || relative.y >= cell_zone.y)
        return std::nullopt;

    
    sf::Vector2u coordinates = {
        static_cast<unsigned int>(relative.y / m_cellSize.y),
        static_cast<unsigned int>(relative.x / m_cellSize.x)
    };

    // reject if outside grid bounds
    if(coordinates.x >= m_gridMapping.y || coordinates.y >= m_gridMapping.x)
        return std::nullopt;

    return coordinates;
}

Cell* GridCanvas::getSelectedCellPtr() 
{
    return m_selectedCell;
}

Cell* GridCanvas::getClickedCellPtr() 
{
    return m_clickedCell;
}

size_t GridCanvas::getCellsCount() const
{
    return m_cellMap.size();
}

void GridCanvas::setTheme(TextureStyle s_gridCanvas)
{
    m_style = s_gridCanvas;
    setFlag(m_flags, FLAGS::MODIFIED);
    for(Cell& cell : m_cellMap) setFlag(cell.m_flags, FLAGS::MODIFIED);
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

bool GridCanvas::preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position)
{
    sf::Vector2i local_mouse_position = getLocalPixel(mouse_position); //!< doesn't considers canvas view
    sf::Vector2f local_view_pixel = mapLocalToView(local_mouse_position); //!< considers view

    // canvas widget events
    
    // update all cells first
    bool canvas_hovered = checkFlag(m_flags, FLAGS::HOVERED); //!< block cell hovering if canvas not hovered
    bool transparent_cells = checkFlag(m_flags, FLAGS::AUTOHIDE) & !canvas_hovered; //!< make all cells transparent or not
    for(Cell& cell : m_cellMap) cell.updateState(local_view_pixel, transparent_cells, canvas_hovered);
    
    // update selected indicator if we previously had a selected cell
    if(m_selectedCell)
    {
        if(checkFlag(m_flags, FLAGS::INDICATOR) && !m_selectedCell->isFlag(FLAGS::SELECTED)) setFlag(m_selectedCell->m_flags, FLAGS::SELECTED);
        else if(!checkFlag(m_flags, FLAGS::INDICATOR) && m_selectedCell->isFlag(FLAGS::SELECTED)) resetFlag(m_selectedCell->m_flags, FLAGS::SELECTED);
    }
    return false;
}

void GridCanvas::postProcessMouseEvents(W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    sf::Vector2i local_mouse_position = getLocalPixel(mouse_position); //!< doesn't considers canvas view

    // click event
    if(details.event_type == W_MouseEventType::Press)
    {
        if(!checkFlag(m_flags, FLAGS::HOVERED)) return; //!< Not hovering

        /* currently hovering */

        if(m_selectedCell)
        {
            // reset previous
            resetFlag(m_selectedCell->m_flags, FLAGS::SELECTED);
            m_selectedCell = nullptr;
        }
        
        if(details.button != s_FOCUS_BUTTON) return;

        /* left mouse button was clicked section */

        m_memMousePos = local_mouse_position; //!< for panning

        // check if we clicked on a cell
        for(Cell& cell : m_cellMap)
        {
            if(cell.isFlag(FLAGS::HOVERED))
            {
                // first activation stage
                m_clickedCell = &cell;
                break;
            }
        }
    }

    // release event (cell selection)
    else if(details.event_type == W_MouseEventType::Release)
    {
        // selecting a cell
        if(checkFlag(m_flags, FLAGS::HOVERED) && details.button == s_FOCUS_BUTTON)
        {
            for(Cell& cell : m_cellMap)
            {
                if(cell.isFlag(FLAGS::HOVERED) && m_clickedCell== &cell)
                {
                    m_selectedCell = &cell;
                    details.is_used = true;
                }
            }
        }

        if(!m_selectedCell) m_clickedCell = nullptr; //!< failed to select
    }

    // zooming event
    else if(details.event_type == W_MouseEventType::Scroll)
    {
        if(checkFlag(m_flags, FLAGS::HOVERED)) 
        {
            zoom(details.scroll_delta, local_mouse_position);
            details.is_used = true;
        }
    }

    // moving event
    else if(details.event_type == W_MouseEventType::Hold)
    {
        if(checkFlag(m_flags, FLAGS::ACTIVE))
        {
            pan(local_mouse_position);
            details.is_used = true;
        }
    }
}

void GridCanvas::drawWidget()
{
    // draw grid lines if GRID is set
    drawGrid();
    // draw all cells
    for(Cell& cell : m_cellMap) cell.drawCell();

    m_drawTexture.display();
    m_drawSprite.setTexture(m_drawTexture.getTexture());
    if(m_clipShaderPtr) m_master.draw(m_drawSprite, m_clipShaderPtr.get());
    else m_master.draw(m_drawSprite);
}

void GridCanvas::config(FLAGS mask)
{
    resetFlag(mask, FLAGS::AUTOHIDE | FLAGS::NOBGIMAGE | FLAGS::TRANSPARENT); //!< cells follow canvas's m_flags directly for autohide

    for(Cell& cell : m_cellMap)
    {
        toggle(cell.m_flags, mask);
        setFlag(cell.m_flags, FLAGS::MODIFIED);
    }
}

void GridCanvas::updatePosition()
{
    // m_position and shape position are already set by Widget, texture does not need position set
    m_drawSprite.setPosition(m_position + m_size / 2.f);
}

void GridCanvas::updateSize()
{
    if(!m_drawTexture.resize(sf::Vector2u(m_size))) return;

    m_drawSprite.setTexture(m_drawTexture.getTexture(), true);
    m_drawSprite.setOrigin(m_size / 2.f);
    updateClipShader();
    updateCellSize();
}

void GridCanvas::updateRotation()
{
    m_drawSprite.setRotation(this->getRotation());
    // for(Cell& c : m_cellMap) c.setRotation(this->getRotation());
}

void GridCanvas::updateState()
{
    m_drawTexture.clear(sf::Color::Transparent);

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
    else if(checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
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

void GridCanvas::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<GridCanvas>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<GridCanvas>());
    }
}

void GridCanvas::updateCellSize()
{
    // surface for drawing cells
    sf::Vector2f cells_space = {m_size.x - m_deadZone * 2.f,
                                m_size.y - m_deadZone * 2.f};

    m_cellSize.x = cells_space.x / static_cast<float>(m_gridMapping.y); //!< cell width
    m_cellSize.y = cells_space.y / static_cast<float>(m_gridMapping.x); //!< cell height

    for(Cell& cell : m_cellMap)
    {
        cell.setCellSize(m_cellSize);
        cell.setCellPosition(cell.getCellCoordinates(), m_deadZone);
    }
}

void GridCanvas::drawGrid()
{
    if(!checkFlag(m_flags, FLAGS::GRID)) return;

    // vertical lines
    for(unsigned int row{0}; row <= m_gridMapping.x ; ++row)
    {
        sf::Vertex line[2] = 
        {
            {{m_cellSize.x * row + m_deadZone, 0.f}, m_style.outline_color},
            {{m_cellSize.x * row + m_deadZone, m_size.y}, m_style.outline_color}
        };
        m_drawTexture.draw(line, 2, sf::PrimitiveType::Lines);
    }

    // horizontal lines
    for(unsigned int col{0}; col <= m_gridMapping.y ; ++col)
    {
        sf::Vertex line[2] = 
        {
            {{0.f, m_cellSize.y * col + m_deadZone}, m_style.outline_color},
            {{m_size.x, m_cellSize.y * col + m_deadZone}, m_style.outline_color}
        };
        m_drawTexture.draw(line, 2, sf::PrimitiveType::Lines);
    }

}

void GridCanvas::zoom(float delta, sf::Vector2i local_mouse_position)
{
    if(!checkFlag(m_flags, FLAGS::ZOOM)) return;

    float factor = delta > 0 ? 0.9f : 1.1f;
    float newZoomLevel = m_zoomLevel * factor;

    // limit scrolling
    if(newZoomLevel > s_MAX_ZOOM_LEVEL)
    {
        factor = s_MAX_ZOOM_LEVEL / m_zoomLevel;
        m_zoomLevel = s_MAX_ZOOM_LEVEL;
    }
    else if(newZoomLevel < s_MIN_ZOOM_LEVEL)
    {
        factor = s_MIN_ZOOM_LEVEL / m_zoomLevel;
        m_zoomLevel = s_MIN_ZOOM_LEVEL;
    }
    else m_zoomLevel *= factor; // zoom level didn't reach limit

    if(factor == 1.f) return; // no point going forward

    sf::View v = m_drawTexture.getView();
    v.zoom(factor);

    // zooming to the direction of the mouse
    sf::Vector2f before_zoom = mapLocalToView(local_mouse_position);
    sf::Vector2f after_zoom = m_drawTexture.mapPixelToCoords(local_mouse_position, v);
    // mouse position changes while zooming based on view
    // so moving view with it will create the illusion
    v.move(before_zoom - after_zoom);

    // block view from exceeding texture's bounds
    limitView(v);
}

void GridCanvas::pan(sf::Vector2i local_mous_position)
{
    if(!checkFlag(m_flags, FLAGS::PAN)) return;

    // calculating the difference in coordinates of last click with recent click
    sf::Vector2f previous_state = mapLocalToView(m_memMousePos);
    sf::Vector2f current_state = m_drawTexture.mapPixelToCoords(local_mous_position);
    sf::Vector2f delta = previous_state - current_state;
    m_memMousePos = local_mous_position;

    sf::View v = m_drawTexture.getView();
    v.move(delta);

    // block view from exceeding texture's bounds
    limitView(v);
}

void GridCanvas::limitView(sf::View& v)
{
    sf::Vector2f half_view_size = v.getSize() / 2.f;
    sf::Vector2f view_center   = v.getCenter();

    // center considers the distance between view bounds and texture bounds
    view_center.x = std::clamp(view_center.x, half_view_size.x, m_size.x - half_view_size.x);
    view_center.y = std::clamp(view_center.y, half_view_size.y, m_size.y - half_view_size.y);

    v.setCenter(view_center);
    m_drawTexture.setView(v);
}
}

namespace sfml_widgets // Cell
{
Cell::Cell(GridCanvas& root, sf::Vector2u coordinates, sf::Vector2f size, float dead_zone) : sf::RectangleShape(size),
m_rootDrawerPtr(&root.m_drawTexture), m_rootStylePtr(&root.m_style), m_bgTexturePtr(), m_coordinates(coordinates),
m_top(DIRECTION::TOP, m_style.top_outline_thickness, {dead_zone + m_coordinates.y * size.x, dead_zone + m_coordinates.x * size.y}, size, m_style.top_color),
m_bottom(DIRECTION::BOTTOM, m_style.bottom_outline_thickness, {dead_zone + m_coordinates.y * size.x, dead_zone + m_coordinates.x * size.y}, size, m_style.bottom_color),
m_left(DIRECTION::LEFT, m_style.left_outline_thickness, {dead_zone + m_coordinates.y * size.x, dead_zone + m_coordinates.x * size.y}, size, m_style.left_color),
m_right(DIRECTION::RIGHT, m_style.right_outline_thickness, {dead_zone + m_coordinates.y * size.x, dead_zone + m_coordinates.x * size.y}, size, m_style.right_color)
{
    this->setPosition({dead_zone + m_coordinates.y * size.x, dead_zone + m_coordinates.x * size.y});
    this->setOutlineThickness(0.f);
}

Cell::Cell(Cell&& other) noexcept : sf::RectangleShape(std::move(other)), m_rootDrawerPtr(other.m_rootDrawerPtr), m_bgTexturePtr(std::move(other.m_bgTexturePtr)),
m_bgImagePath(std::move(other.m_bgImagePath)), m_rootStylePtr(other.m_rootStylePtr), m_imageShaderPtr(std::move(other.m_imageShaderPtr)), 
m_deadZone(other.m_deadZone), m_coordinates(std::move(other.m_coordinates)), m_style(std::move(other.m_style)), m_flags(other.m_flags),
m_top(std::move(other.m_top)), m_bottom(std::move(other.m_bottom)), m_left(std::move(other.m_left)), m_right(std::move(other.m_right))
{
    other.m_rootDrawerPtr = nullptr;
    other.m_rootStylePtr = nullptr;
    if(checkFlag(m_flags, FLAGS::IMAGE)) this->setTexture(m_bgTexturePtr.get(), true);
}

Cell& Cell::operator=(Cell&& other) noexcept
{
    if(this != &other)
    {
        static_cast<sf::RectangleShape&>(*this) = std::move(other);
        m_rootDrawerPtr = other.m_rootDrawerPtr;
        m_rootStylePtr = other.m_rootStylePtr;
        m_imageShaderPtr = std::move(other.m_imageShaderPtr);
        m_bgTexturePtr = std::move(other.m_bgTexturePtr);
        m_bgImagePath = std::move(other.m_bgImagePath);
        m_deadZone = other.m_deadZone;
        m_coordinates = std::move(other.m_coordinates);
        m_style = std::move(other.m_style);
        m_flags = other.m_flags;
        m_top = std::move(other.m_top);
        m_bottom = std::move(other.m_bottom);
        m_left = std::move(other.m_left);
        m_right = std::move(other.m_right);
        
        if(checkFlag(m_flags, FLAGS::IMAGE)) this->setTexture(m_bgTexturePtr.get(), true);
        other.m_rootDrawerPtr = nullptr;
        other.m_rootStylePtr = nullptr;
    }
    return *this;
}

Cell::~Cell()
{
    clearImage();
}

bool Cell::operator==(const sf::Vector2u other) const
{
    return m_coordinates == other;
}

bool Cell::operator!=(const sf::Vector2u other) const
{
    return m_coordinates != other;
}

void Cell::configure(FLAGS flag)
{
    FLAGS mask = flag & parameters::MODIFIABLE;
    resetFlag(mask, FLAGS::HIDDEN);
    toggle(m_flags, mask);
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Cell::setCellColor(sf::Color color)
{
    m_style.bg_color = color;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Cell::setCellOutlineColor(DIRECTION direction, sf::Color color)
{
    switch (direction)
    {
    case DIRECTION::TOP:
        m_style.top_color = color;
        break;
    case DIRECTION::BOTTOM:
        m_style.bottom_color = color;
        break;
    case DIRECTION::LEFT:
        m_style.left_color = color;
        break;
    case DIRECTION::RIGHT:
        m_style.right_color = color;
        break;
    default:
        break;
    }
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Cell::setCellOutlineThickness(DIRECTION direction, float thickness)
{
    switch (direction)
    {
    case DIRECTION::TOP:
        m_style.top_outline_thickness = thickness;
        break;
    case DIRECTION::BOTTOM:
        m_style.bottom_outline_thickness = thickness;
        break;
    case DIRECTION::LEFT:
        m_style.left_outline_thickness = thickness;
        break;
    case DIRECTION::RIGHT:
        m_style.right_outline_thickness = thickness;
        break;
    default:
        break;
    }
    setFlag(m_flags, FLAGS::MODIFIED);
}

bool Cell::setImage(const std::string& image_path)
{
    if(checkFlag(m_flags, FLAGS::NOBGIMAGE)) return false;
    Widget::ImageCache& global_image_buffer = Widget::ImageCache::getInstance();
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(image_path);
    if(!disk_image) return false;

    std::optional<sf::Image> resized_image = resize_image(**disk_image, sf::Vector2u(this->getSize()));
    if(!resized_image)
    {
        global_image_buffer.decrementRefCounter(image_path);
        return false;
    }

    sf::Texture temp;
    if(!temp.loadFromImage(*resized_image))
    {
        global_image_buffer.decrementRefCounter(image_path);
        return false;
    }

    if(checkFlag(m_flags, FLAGS::IMAGE)) global_image_buffer.decrementRefCounter(m_bgImagePath);

    if(m_bgTexturePtr)
    {
        *m_bgTexturePtr = std::move(temp);
    }
    else
    {
        m_bgTexturePtr = std::make_unique<sf::Texture>(std::move(temp));
    }

    m_bgImagePath = std::move(image_path);
    m_bgTexturePtr->setSmooth(true);
    this->setTexture(m_bgTexturePtr.get(), true);
    if(!m_imageShaderPtr) loadShader(TEXTURE_SHADER_FRAG);
    setFlag(m_flags, FLAGS::IMAGE);

    return true;
}

void Cell::clearImage()
{
    if(!checkFlag(m_flags, FLAGS::IMAGE)) return;

    Widget::ImageCache::getInstance().decrementRefCounter(m_bgImagePath);
    this->setTexture(nullptr);
    m_imageShaderPtr.reset();
    m_bgTexturePtr.reset();
    m_bgImagePath.clear();
    resetFlag(m_flags, FLAGS::IMAGE);
}

void Cell::loadShader(const std::string_view& shader_path)
{
    if(!sf::Shader::isAvailable()) return;

    m_imageShaderPtr = std::make_unique<sf::Shader>();
    if(m_imageShaderPtr->loadFromMemory(shader_path, sf::Shader::Type::Fragment)) return;
    m_imageShaderPtr.reset();
}

sf::Vector2f Cell::getCellPosition() const
{
    return this->getPosition();
}

sf::Vector2u Cell::getCellCoordinates() const
{
    return m_coordinates;
}

bool Cell::isFlag(FLAGS flag) const
{
    return checkFlag(m_flags, flag);
}

void Cell::drawCell()
{
    updateCell();
    if(m_imageShaderPtr) m_rootDrawerPtr->draw(*this, m_imageShaderPtr.get());
    else m_rootDrawerPtr->draw(*this);

    m_rootDrawerPtr->draw(m_top.getVertices(), 6, sf::PrimitiveType::Triangles);
    m_rootDrawerPtr->draw(m_bottom.getVertices(), 6, sf::PrimitiveType::Triangles);
    m_rootDrawerPtr->draw(m_left.getVertices(), 6, sf::PrimitiveType::Triangles);
    m_rootDrawerPtr->draw(m_right.getVertices(), 6, sf::PrimitiveType::Triangles);
}

void Cell::setCellSize(sf::Vector2f size)
{
    this->setSize(size);
    m_top.setCellSize(size, DIRECTION::TOP);
    m_bottom.setCellSize(size, DIRECTION::BOTTOM);
    m_left.setCellSize(size, DIRECTION::LEFT);
    m_right.setCellSize(size, DIRECTION::RIGHT);
}

void Cell::setCellPosition(sf::Vector2u coordinates, float dead_zone)
{
    m_deadZone = dead_zone;

    sf::Vector2f pos{m_deadZone + m_coordinates.y * this->getSize().x,
                    m_deadZone + m_coordinates.x * this->getSize().y};

    this->setPosition(pos);
    m_top.setCellPosition(pos, DIRECTION::TOP);
    m_bottom.setCellPosition(pos, DIRECTION::BOTTOM);
    m_left.setCellPosition(pos, DIRECTION::LEFT);
    m_right.setCellPosition(pos, DIRECTION::RIGHT);
}

bool Cell::updateState(sf::Vector2f local_mouse_position, bool auto_hide, bool canvas_hovered)
{
    // hovering
    if(canvas_hovered && isHovered(local_mouse_position))
    {
        if(!checkFlag(m_flags, FLAGS::HOVERED)) setFlag(m_flags, FLAGS::HOVERED);
        if(checkFlag(m_flags, FLAGS::ALPHAREDUCED)) resetFlag(m_flags, FLAGS::ALPHAREDUCED);
        return true;
    }
    // not hovering
    if(checkFlag(m_flags, FLAGS::HOVERED)) resetFlag(m_flags, FLAGS::HOVERED);
    if(auto_hide || checkFlag(m_flags, FLAGS::AUTOHIDE)) setFlag(m_flags, FLAGS::ALPHAREDUCED);
    else resetFlag(m_flags, FLAGS::ALPHAREDUCED);
    return false;
}

void Cell::updateCell()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;

    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::BLOCKED))
    {
        this->setFillColor(m_rootStylePtr->fill_blocked_color);
        updateShader(m_rootStylePtr->fill_blocked_color, 0.2f);
        m_top.setColor(m_rootStylePtr->outline_blocked_color);
        m_top.setThickness(m_rootStylePtr->outline_blocked_thickness, DIRECTION::TOP);
        m_bottom.setColor(m_rootStylePtr->outline_blocked_color);
        m_bottom.setThickness(m_rootStylePtr->outline_blocked_thickness, DIRECTION::BOTTOM);
        m_left.setColor(m_rootStylePtr->outline_blocked_color);
        m_left.setThickness(m_rootStylePtr->outline_blocked_thickness, DIRECTION::LEFT);
        m_right.setColor(m_rootStylePtr->outline_blocked_color);
        m_right.setThickness(m_rootStylePtr->outline_blocked_thickness, DIRECTION::RIGHT);
    }

    else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        this->setFillColor(m_rootStylePtr->fill_transparent_color);
        updateShader(m_rootStylePtr->fill_transparent_color, 0.4f);
        m_top.setColor(m_rootStylePtr->outline_transparent_color);
        m_top.setThickness(m_rootStylePtr->outline_transparent_thickness, DIRECTION::TOP);
        m_bottom.setColor(m_rootStylePtr->outline_transparent_color);
        m_bottom.setThickness(m_rootStylePtr->outline_transparent_thickness, DIRECTION::BOTTOM);
        m_left.setColor(m_rootStylePtr->outline_transparent_color);
        m_left.setThickness(m_rootStylePtr->outline_transparent_thickness, DIRECTION::LEFT);
        m_right.setColor(m_rootStylePtr->outline_transparent_color);
        m_right.setThickness(m_rootStylePtr->outline_transparent_thickness, DIRECTION::RIGHT);
    }

    else if(checkFlag(m_flags, FLAGS::SELECTED))
    {
        this->setFillColor(m_rootStylePtr->fill_active_color);
        updateShader(m_rootStylePtr->fill_active_color, 0.8f);
        m_top.setColor(m_rootStylePtr->outline_active_color);
        m_top.setThickness(m_rootStylePtr->outline_active_thickness, DIRECTION::TOP);
        m_bottom.setColor(m_rootStylePtr->outline_active_color);
        m_bottom.setThickness(m_rootStylePtr->outline_active_thickness, DIRECTION::BOTTOM);
        m_left.setColor(m_rootStylePtr->outline_active_color);
        m_left.setThickness(m_rootStylePtr->outline_active_thickness, DIRECTION::LEFT);
        m_right.setColor(m_rootStylePtr->outline_active_color);
        m_right.setThickness(m_rootStylePtr->outline_active_thickness, DIRECTION::RIGHT);
    }

    else if(checkFlag(m_flags, FLAGS::HOVERED))
    {
        this->setFillColor(m_rootStylePtr->fill_hover_color);
        updateShader(m_rootStylePtr->fill_hover_color, 0.9f);
        m_top.setColor(m_rootStylePtr->outline_hover_color);
        m_top.setThickness(m_rootStylePtr->outline_hover_thickness, DIRECTION::TOP);
        m_bottom.setColor(m_rootStylePtr->outline_hover_color);
        m_bottom.setThickness(m_rootStylePtr->outline_hover_thickness, DIRECTION::BOTTOM);
        m_left.setColor(m_rootStylePtr->outline_hover_color);
        m_left.setThickness(m_rootStylePtr->outline_hover_thickness, DIRECTION::LEFT);
        m_right.setColor(m_rootStylePtr->outline_hover_color);
        m_right.setThickness(m_rootStylePtr->outline_hover_thickness, DIRECTION::RIGHT);
    }
    else
    {
        this->setFillColor(m_style.bg_color);
        updateShader(m_style.bg_color, 1.f);
        m_top.setColor(m_style.top_color);
        m_top.setThickness(m_style.top_outline_thickness, DIRECTION::TOP);
        m_bottom.setColor(m_style.bottom_color);
        m_bottom.setThickness(m_style.bottom_outline_thickness, DIRECTION::BOTTOM);
        m_left.setColor(m_style.left_color);
        m_left.setThickness(m_style.left_outline_thickness, DIRECTION::LEFT);
        m_right.setColor(m_style.right_color);
        m_right.setThickness(m_style.right_outline_thickness, DIRECTION::RIGHT);
    }
}

void Cell::updateImageSize()
{
    if(!checkFlag(m_flags, FLAGS::IMAGE)) return;

    std::optional<const sf::Image*> disk_image = Widget::ImageCache::getInstance().getImage(m_bgImagePath, false);

    std::optional<sf::Image> resized_image;
    if(disk_image) resized_image = resize_image(**disk_image, sf::Vector2u(this->getSize()));
    else resized_image = resize_image(m_bgTexturePtr->copyToImage(), sf::Vector2u(this->getSize()));
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

void Cell::updateDrawer(sf::RenderTexture& new_texture)
{
    m_rootDrawerPtr = &new_texture;
}

void Cell::updateCanvasStyle(TextureStyle* stylePtr)
{
    m_rootStylePtr = stylePtr;
}

void Cell::updateShader(sf::Color bg_color, float transparent_level)
{
    if(!m_imageShaderPtr) return;
    bool render_texture = m_bgTexturePtr.get(); 

    // for background image texture
    m_imageShaderPtr->setUniform("texture", sf::Shader::CurrentTexture);
    if(render_texture)
    {
        m_imageShaderPtr->setUniform("bg_color",
            sf::Glsl::Vec4(bg_color.r / 255.f,
                           bg_color.g / 255.f,
                           bg_color.b / 255.f,
                           bg_color.a / 255.f));
    }
    else
    {
        m_imageShaderPtr->setUniform("bg_color", sf::Glsl::Vec4(bg_color));
    }
    m_imageShaderPtr->setUniform("image_opacity", transparent_level);
    m_imageShaderPtr->setUniform("has_texture", render_texture ? 1.f : 0.f);
}

bool Cell::isHovered(sf::Vector2f local_mouse_position) const
{
    return this->getGlobalBounds().contains(local_mouse_position);
}
}

namespace sfml_widgets // CellOutline
{
CellOutline::CellOutline(DIRECTION direction, float thickness, sf::Vector2f cell_position, sf::Vector2f cell_size, sf::Color color) : m_cellPosition(cell_position),
m_cellSize(cell_size), m_thickness(thickness), m_color(color)
{
    float x0 = m_cellPosition.x;
    float x1 = m_cellPosition.x + m_cellSize.x;
    float y0 = m_cellPosition.y;
    float y1 = m_cellPosition.y + m_cellSize.y;
    switch (direction)
    {
    case DIRECTION::TOP:
        m_vertices.at(0) = {{x0, y0 + m_thickness}, m_color};
        m_vertices.at(1) = {{x1, y0 + m_thickness}, m_color};
        m_vertices.at(2) = {{x1, y0}, m_color};
        m_vertices.at(3) = {{x1, y0}, m_color};
        m_vertices.at(4) = {{x0, y0}, m_color};
        m_vertices.at(5) = {{x0, y0 + m_thickness}, m_color};
        break;
    
    case DIRECTION::BOTTOM:
        m_vertices.at(0) = {{x0, y1}, m_color };
        m_vertices.at(1) = {{x1, y1}, m_color};
        m_vertices.at(2) = {{x1, y1 - m_thickness}, m_color};
        m_vertices.at(3) = {{x1, y1 - m_thickness}, m_color};
        m_vertices.at(4) = {{x0, y1 - m_thickness}, m_color};
        m_vertices.at(5) = {{x0, y1}, m_color};
        break;
    
    case DIRECTION::LEFT:
        m_vertices.at(0) = {{x0 + m_thickness, y0}, m_color};
        m_vertices.at(1) = {{x0, y0}, m_color};
        m_vertices.at(2) = {{x0, y1}, m_color};
        m_vertices.at(3) = {{x0, y1}, m_color};
        m_vertices.at(4) = {{x0 + m_thickness, y1}, m_color};
        m_vertices.at(5) = {{x0 + m_thickness, y0}, m_color};
        break;

    case DIRECTION::RIGHT:
        m_vertices.at(0)  = {{x1, y0}, m_color};
        m_vertices.at(1)  = {{x1 - m_thickness, y0}, m_color};
        m_vertices.at(2)  = {{x1 - m_thickness, y1}, m_color};
        m_vertices.at(3)  = {{x1 - m_thickness, y1}, m_color};
        m_vertices.at(4) = {{x1, y1}, m_color};
        m_vertices.at(5) = {{x1, y0}, m_color};
        break;
    }
}

CellOutline::CellOutline(CellOutline&& other) noexcept : m_vertices(std::move(other.m_vertices)), m_cellPosition(other.m_cellPosition), m_cellSize(other.m_cellSize),
m_thickness(other.m_thickness), m_color(std::move(other.m_color))
{

}

CellOutline& CellOutline::operator=(CellOutline&& other) noexcept
{
    if(this != &other)
    {
        m_vertices = std::move(other.m_vertices);
        m_cellPosition = other.m_cellPosition;
        m_cellSize = other.m_cellSize;
        m_color = std::move(other.m_color);
        m_thickness = other.m_thickness;
    }
    return *this;
}

void CellOutline::setThickness(float thickness, DIRECTION direction)
{
    m_thickness = thickness;
    updateOutline(direction);
}

void CellOutline::setColor(sf::Color _color)
{
    m_color = _color;
    m_vertices.at(0).color = m_color;
    m_vertices.at(1).color = m_color;
    m_vertices.at(2).color = m_color;
    m_vertices.at(3).color = m_color;
    m_vertices.at(4).color = m_color;
    m_vertices.at(5).color = m_color;
}

void CellOutline::setCellPosition(sf::Vector2f cell_position, DIRECTION direction)
{
    m_cellPosition = cell_position;
    updateOutline(direction);
}

void CellOutline::setCellSize(sf::Vector2f cell_size, DIRECTION direction)
{
    m_cellSize = cell_size;
    updateOutline(direction);
}

const sf::Vertex* CellOutline::getVertices() const
{
    return m_vertices.data();
}

void CellOutline::updateOutline(DIRECTION direction)
{
    float x0 = m_cellPosition.x;
    float x1 = m_cellPosition.x + m_cellSize.x;
    float y0 = m_cellPosition.y;
    float y1 = m_cellPosition.y + m_cellSize.y;
    switch (direction)
    {
    case DIRECTION::TOP:
        m_vertices.at(0).position = {x0, y0 + m_thickness};
        m_vertices.at(1).position = {x1, y0 + m_thickness};
        m_vertices.at(2).position = {x1, y0};
        m_vertices.at(3).position = {x1, y0};
        m_vertices.at(4).position = {x0, y0};
        m_vertices.at(5).position = {x0, y0 + m_thickness};
        break;
    
    case DIRECTION::BOTTOM:
        m_vertices.at(0).position = {x0, y1};
        m_vertices.at(1).position = {x1, y1};
        m_vertices.at(2).position = {x1, y1 - m_thickness};
        m_vertices.at(3).position = {x1, y1 - m_thickness};
        m_vertices.at(4).position = {x0, y1 - m_thickness};
        m_vertices.at(5).position = {x0, y1};
        break;
    
    case DIRECTION::LEFT:
        m_vertices.at(0).position = {x0 + m_thickness, y0};
        m_vertices.at(1).position = {x0, y0};
        m_vertices.at(2).position = {x0, y1};
        m_vertices.at(3).position = {x0, y1};
        m_vertices.at(4).position = {x0 + m_thickness, y1};
        m_vertices.at(5).position = {x0 + m_thickness, y0};
        break;

    case DIRECTION::RIGHT:
        m_vertices.at(0).position = {x1, y0};
        m_vertices.at(1).position = {x1 - m_thickness, y0};
        m_vertices.at(2).position = {x1 - m_thickness, y1};
        m_vertices.at(3).position = {x1 - m_thickness, y1};
        m_vertices.at(4).position = {x1, y1};
        m_vertices.at(5).position = {x1, y0};
        break;
    }
}
}
