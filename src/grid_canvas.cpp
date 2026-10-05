#include "frame.hpp"
#include "grid_canvas.hpp"
#include <cmath>

namespace
{
///////////////////////////////////////////////////////////////////
/// @brief Multiplies bit mask with flags then right shifts (reduce) with offset,
/// Used to convert a flag to uint value
///
/// @param flags line state value 
/// @param mask desired range to keep
/// @param offset right shift bit offset
/// @return uint value representing flag value
///////////////////////////////////////////////////////////////////
constexpr sfml_widgets::LineState apply_mask(sfml_widgets::LineState flags, sfml_widgets::LineState mask, unsigned int offset = 0)
{
    return (flags & mask) >> offset;
}

///////////////////////////////////////////////////////////////////
/// @brief Left shifts flags value by shift bits, used to convert a
/// uint value to it's corresponding flag value
///
/// @param flags value to shift
/// @param shift number of bits to shift
/// @return shifted value
///////////////////////////////////////////////////////////////////
constexpr sfml_widgets::LineState apply_shift(sfml_widgets::LineState flags, unsigned int shift = 0)
{
    return flags << shift;
}

///////////////////////////////////////////////////////////////////
/// @brief Adds a double triangle vertex block to array representing
/// a rectangular shape
///
/// @param array array reference 
/// @param rect vertex rect position + size in pixels
/// @param color rectangle fill color
///
///////////////////////////////////////////////////////////////////
void append_rectangle(sf::VertexArray& array, sfml_widgets::RectRange<float> rect, sf::Color color = sf::Color::Black)
{
    const std::size_t offset = array.getVertexCount();
    array.resize(offset + 6);
    array[offset + 0] = sf::Vertex{{rect.x0, rect.y0}, color};
    array[offset + 1] = sf::Vertex{{rect.x1, rect.y0}, color};
    array[offset + 2] = sf::Vertex{{rect.x1, rect.y1}, color};
    array[offset + 3] = sf::Vertex{{rect.x1, rect.y1}, color};
    array[offset + 4] = sf::Vertex{{rect.x0, rect.y1}, color};
    array[offset + 5] = sf::Vertex{{rect.x0, rect.y0}, color};
}

///////////////////////////////////////////////////////////////////
/// @brief Creates a uint64 value to represents the rect's bounds
///
///////////////////////////////////////////////////////////////////
std::uint64_t pack_bounds(sfml_widgets::RectRange<unsigned int> rect)
{
    return (static_cast<std::uint64_t>(rect.x0) << 48) |
           (static_cast<std::uint64_t>(rect.y0) << 32) |
           (static_cast<std::uint64_t>(rect.x1) << 16) |
            static_cast<std::uint64_t>(rect.y1);
}

///////////////////////////////////////////////////////////////////
/// @brief iota generator
///
///////////////////////////////////////////////////////////////////
template<typename NumIterator, typename Num>
void gen_iota(NumIterator first, NumIterator last, Num val)
{
    for(;first != last; ++first)
    {
        *first = val;
        ++val;
    }
}

///////////////////////////////////////////////////////////////////
/// @brief Unique key for canvas textures
///
/// @param path image path 
/// @param size cell size in pixels
/// @return key identifier
///
///////////////////////////////////////////////////////////////////
std::string make_texture_key(const std::string& path, sf::Vector2u size)
{
    return path + '|' + std::to_string(size.x) + 'x' + std::to_string(size.y);
}
}

namespace sfml_widgets // GridCell
{
GridCell::GridCell(std::size_t id) : id(id)
{

}

GridCell::~GridCell()
{
    releaseTexture();
}

void GridCell::releaseTexture()
{
    Proot->_releaseTexture(*this);
}

bool GridCell::setCellTexture(const std::string& image_path)
{
    return Proot->setCellTexture(id, image_path);
}

bool GridCell::clearCellTexture()
{
    releaseTexture();
    return true;
}

bool GridCell::setCellColor(sf::Color color)
{
    fill_color = color;
    Proot->_refreshCellColor(id);
    return true;
}

bool GridCell::setOutlineThickness(DIRECTION _s, LineState thickness)
{
    return Proot->setLineThickness(getLineRange(_s), thickness);
}

bool GridCell::setOutlineColor(DIRECTION _s, sf::Color color)
{
    return Proot->setLineColor(getLineRange(_s), color);
}

bool GridCell::setOutlineType(DIRECTION _s, LineType line_type)
{
    return Proot->setLineType(getLineRange(_s), line_type);
}

bool GridCell::inRange(sf::Vector2u grid_coords) const
{
    return grid.inRange(grid_coords);
}

bool GridCell::inRange(sf::Vector2f pixel_coords) const
{
    RectRange<float> frange = Proot->mapGridRectToCoords(grid);
    return frange.inRange(pixel_coords);
}

std::size_t GridCell::getID() const
{
    return id;
}

bool GridCell::operator==(const GridCell& other) const
{
    return this->id == other.id;
}

bool GridCell::operator==(std::size_t id) const
{
    return this->id == id;
}

RectRange<unsigned int> GridCell::getLineRange(DIRECTION _s) const
{
    RectRange<unsigned int> rect;

    /* rect and canvas identify ranges differently where
    * rect ==> [x0, x1]
    * canvas ==> [x0, x1[
    * so +1 is added to the end range when talking to canvas
    */

    switch (_s)
    {
    case DIRECTION::TOP:
        rect.x0 = grid.x0;
        rect.x1 = grid.x0;
        rect.y0 = grid.y0;
        rect.y1 = grid.y1 + 1;
        break;

    case DIRECTION::BOTTOM:
        rect.x0 = grid.x1 + 1;
        rect.x1 = grid.x1 + 1;
        rect.y0 = grid.y0;
        rect.y1 = grid.y1 + 1;
        break;

    case DIRECTION::LEFT:
        rect.x0 = grid.x0;
        rect.x1 = grid.x1 + 1;
        rect.y0 = grid.y0;
        rect.y1 = grid.y0;
        break;

    case DIRECTION::RIGHT:
        rect.x0 = grid.x0;
        rect.x1 = grid.x1 + 1;
        rect.y0 = grid.y1 + 1;
        rect.y1 = grid.y1 + 1;
        break;
    }
    return rect;
}

void GridCell::inheritTo(GridCell& other)
{
    if(&other == this) return;

    other.fill_color = fill_color;
    other.texture = texture;
    other.texture_key = std::move(texture_key);

    texture = nullptr;
}

}

namespace sfml_widgets // cell_ptr
{
Canvas::cell_ptr::cell_ptr(std::weak_ptr<Access> access, std::size_t index, std::uint32_t generation) :
m_access(std::move(access)), m_index(index), m_generation(generation)
{

}

Canvas::cell_ptr::operator bool() const
{
    return resolve() != nullptr;
}

GridCell* Canvas::cell_ptr::operator->() const
{
    GridCell* cell = resolve();
    if(!cell)
    {
        assert(false && "GridCell used after it was invalidated by a canvas rebuild, or after its canvas was destroyed");
        std::abort();
    }
    return cell;
}

GridCell& Canvas::cell_ptr::operator*() const
{
    return *operator->();
}

GridCell* Canvas::cell_ptr::resolve() const
{
    const std::shared_ptr<Access> locked = m_access.lock();
    if(!locked || locked->generation != m_generation) return nullptr;
    return locked->canvas->getCell(m_index);
}
}

namespace sfml_widgets // Canvas
{
Canvas::Canvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid) : Widget(window, position, size),
m_texture(size), m_sprite(m_texture.getTexture()), m_MAX_CELL_COUNT(grid.x * grid.y), m_grid(grid) 
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    m_lines.assign(static_cast<std::size_t>(m_grid.x * m_grid.y * 2 + m_grid.x + m_grid.y), 0u);
    m_access = std::make_shared<Access>(Access{this, 0u});

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    m_openLineShaderPtr = std::make_unique<sf::Shader>();
    if(m_openLineShaderPtr->loadFromMemory(OPEN_LINE_SHADER_FRAG, sf::Shader::Type::Fragment))
    {
        m_openLineShaderPtr->setUniform("join_lines", m_openJoinMode ? 1 : 0);
        m_openLineShaderPtr->setUniform("zero_space_width", m_openDoorWidth);
        m_openLineShaderPtr->setUniform("one_space_width", m_openStubRatio);
    }
    else m_openLineShaderPtr.reset();

    m_dottedLineShaderPtr = std::make_unique<sf::Shader>();
    if(m_dottedLineShaderPtr->loadFromMemory(DOTTED_LINE_SHADER_FRAG, sf::Shader::Type::Fragment))
    {
        m_dottedLineShaderPtr->setUniform("u_dotSpacing", m_dotSpacing);
        m_dottedLineShaderPtr->setUniform("u_dotRadius", m_dotRadius);
    }
    else m_dottedLineShaderPtr.reset();
    toggle(m_flags, FLAGS::ZOOM | FLAGS::PAN);
}

Canvas::Canvas(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid) : Widget(root, position, size),
m_texture(size), m_sprite(m_texture.getTexture()), m_MAX_CELL_COUNT(grid.x * grid.y), m_grid(grid)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    m_lines.assign(static_cast<std::size_t>(m_grid.x * m_grid.y * 2 + m_grid.x + m_grid.y), 0u);
    m_access = std::make_shared<Access>(Access{this, 0u});

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    m_openLineShaderPtr = std::make_unique<sf::Shader>();
    if(m_openLineShaderPtr->loadFromMemory(OPEN_LINE_SHADER_FRAG, sf::Shader::Type::Fragment))
    {
        m_openLineShaderPtr->setUniform("join_lines", m_openJoinMode ? 1 : 0);
        m_openLineShaderPtr->setUniform("zero_space_width", m_openDoorWidth);
        m_openLineShaderPtr->setUniform("one_space_width", m_openStubRatio);
    }
    else m_openLineShaderPtr.reset();

    m_dottedLineShaderPtr = std::make_unique<sf::Shader>();
    if(m_dottedLineShaderPtr->loadFromMemory(DOTTED_LINE_SHADER_FRAG, sf::Shader::Type::Fragment))
    {
        m_dottedLineShaderPtr->setUniform("dot_radius", m_dotRadius);
    }
    else m_dottedLineShaderPtr.reset();
    toggle(m_flags, FLAGS::ZOOM | FLAGS::PAN);
}

bool Canvas::drawLine(RectRange<unsigned int> line_range, LineState thickness, sf::Color color, LineType state)
{
    LineState val{LINE_SET};

    if(thickness < (1 << THICKNESS_LAST_BIT))
        val |= apply_mask(apply_shift(thickness, THICKNESS_OFFSET), THICKNESS_MASK);

    if(color != DEFAULT_COLOR)
    {
        int idx = getColorIndex(color);
        // -1 ==> take canvas outline color
        // 0-13 ==> index for color
        if(idx != -1) val |= apply_mask(apply_shift(idx + 1, COLORS_OFFSET), COLORS_MASK);
    }

    if(state != LineType::None)
        val |= apply_mask(apply_shift(static_cast<LineState>(state), TYPE_OFFSET), TYPE_MASK);

    return setLine(line_range, val);
}

bool Canvas::removeLine(RectRange<unsigned int> line_range)
{
    return setLine(line_range, 0u);
}

void Canvas::clearCanvas()
{
    std::fill(m_lines.begin(), m_lines.end(), 0u);
    m_updateLines = true;
    m_updateCells = true;
}

bool Canvas::setLineThickness(RectRange<unsigned int> line_range, LineState thickness)
{
    LineState val{LINE_SET};

    if(thickness < (1 << THICKNESS_LAST_BIT))
    {
        val |= apply_mask(apply_shift(thickness, THICKNESS_OFFSET), THICKNESS_MASK);
        return _setLineValue(line_range, val, THICKNESS_OFF, false);
    }
    return false;
}

bool Canvas::setLineColor(RectRange<unsigned int> line_range, sf::Color color)
{
    LineState val{LINE_SET};

    int idx = getColorIndex(color);
    if(idx != -1) val |= apply_mask(apply_shift(idx + 1, COLORS_OFFSET), COLORS_MASK);
    return _setLineValue(line_range, val, COLORS_OFF, false);
}

bool Canvas::setLineType(RectRange<unsigned int> line_range, LineType Line_type)
{
    LineState val{LINE_SET};

    if(Line_type != LineType::None) val |= apply_mask(apply_shift(static_cast<LineState>(Line_type), TYPE_OFFSET), TYPE_MASK);
    return _setLineValue(line_range, val, TYPE_OFF, false);
}

Canvas::cell_ptr Canvas::at(sf::Vector2u grid_coords) const
{
    return makeHandle(getCellIndex(grid_coords));
}

Canvas::cell_ptr Canvas::at(sf::Vector2f pixel_coords) const
{
    return makeHandle(getCellIndex(pixel_coords));
}

Canvas::cell_ptr Canvas::at(std::size_t cell_id) const
{
    return makeHandle(cell_id < m_cells.size() ? cell_id : NULL_CELL);
}

Canvas::cell_ptr Canvas::getHoveredCell() const
{
    return m_hoveredCellID == NULL_CELL ? cell_ptr{} : at(m_hoveredCellID);
}

Canvas::cell_ptr Canvas::getSelectedCell() const
{
    return m_selectedCellID == NULL_CELL ? cell_ptr{} : at(m_selectedCellID);
}

std::size_t Canvas::getCellsCount() const
{
    return m_cells.size();
}

std::size_t Canvas::getMaxCellsCount() const
{
    return m_grid.x * m_grid.y;
}

sf::Vector2i Canvas::getLocalPixel(sf::Vector2i mouse_position) const
{
    sf::Vector2f global_point = m_master.mapMousePosition(mouse_position);
    return sf::Vector2i(this->getInverseTransform().transformPoint(global_point));
}

sf::Vector2f Canvas::getViewPixel(sf::Vector2i mouse_position) const
{
    return m_texture.mapPixelToCoords(getLocalPixel(mouse_position));
}

sf::Vector2f Canvas::mapLocalToView(sf::Vector2i local_position) const
{
    return m_texture.mapPixelToCoords(local_position);
}

std::optional<sf::Vector2u> Canvas::mapCoordsToGrid(sf::Vector2i mouse_position) const
{
    sf::Vector2f view_pixel = getViewPixel(mouse_position);

    sf::Vector2f relative =
    {
        view_pixel.x - m_deadZone,
        view_pixel.y - m_deadZone
    };

    sf::Vector2f cell_zone =
    {
        m_size.x - m_deadZone * 2.f,
        m_size.y - m_deadZone * 2.f
    };

    if(relative.x < 0.f || relative.y < 0.f ||
       relative.x >= cell_zone.x || relative.y >= cell_zone.y)
        return std::nullopt;

    sf::Vector2u coordinates =
    {
        static_cast<unsigned int>(relative.y / m_cellMinSize.y),
        static_cast<unsigned int>(relative.x / m_cellMinSize.x)
    };

    if(coordinates.x >= m_grid.y || coordinates.y >= m_grid.x)
        return std::nullopt;

    return coordinates;
}

std::optional<sf::Vector2f> Canvas::mapGridToCoords(sf::Vector2u grid_coordinates) const
{
    if(grid_coordinates.x >= m_grid.x || grid_coordinates.y >= m_grid.y) return std::nullopt;

    return sf::Vector2f
    {
        m_deadZone + static_cast<float>(grid_coordinates.y) * m_cellMinSize.x,
        m_deadZone + static_cast<float>(grid_coordinates.x) * m_cellMinSize.y
    };
}

RectRange<float> Canvas::mapGridRectToCoords(RectRange<unsigned int> grid_rect) const
{
    return
    {
        m_deadZone + static_cast<float>(grid_rect.y0) * m_cellMinSize.x,
        m_deadZone + static_cast<float>(grid_rect.x0) * m_cellMinSize.y,
        m_deadZone + static_cast<float>(grid_rect.y1 + 1) * m_cellMinSize.x,
        m_deadZone + static_cast<float>(grid_rect.x1 + 1) * m_cellMinSize.y
    };
}

RectRange<unsigned int> Canvas::mapCoordsRectToGrid(RectRange<float> coords_rect) const
{
    return
    {
        static_cast<unsigned int>((coords_rect.y0 - m_deadZone) / m_cellMinSize.x),
        static_cast<unsigned int>((coords_rect.x0 - m_deadZone) / m_cellMinSize.y),
        static_cast<unsigned int>((coords_rect.y1 - m_deadZone) / m_cellMinSize.x) + 1,
        static_cast<unsigned int>((coords_rect.x1 - m_deadZone) / m_cellMinSize.y) + 1
    };
}

void Canvas::setDeadZone(float surface)
{
    surface = std::clamp(surface, 0.f, s_MAX_DEADZONE_RATIO);
    m_deadZone = surface * std::min<float>(m_size.x, m_size.y);
    updateCellSize();
    if(checkFlag(m_flags, FLAGS::GRID)) m_updateGridLines = true;
    m_updateLines = true;
    m_updateCells = true;
}

void Canvas::setTheme(CanvasStyle s_canvas)
{
    m_style = s_canvas;
    setFlag(m_flags, FLAGS::MODIFIED);

    _refreshCellColor(m_hoveredCellID);
    _refreshCellColor(m_selectedCellID);
    m_updateGridLines = true;
    m_updateLines = true;
}

void Canvas::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<Canvas>());
}

CanvasStyle& Canvas::style()
{
    return m_style;
}

void Canvas::poke()
{
    m_flags |= FLAGS::MODIFIED;

    _refreshCellColor(m_hoveredCellID);
    _refreshCellColor(m_selectedCellID);
    m_updateGridLines = true;
    m_updateLines = true;
}

void Canvas::setOpenLineJoinMode(bool merged)
{
    m_openJoinMode = merged;
    if(m_openLineShaderPtr) m_openLineShaderPtr->setUniform("join_lines", merged ? 1 : 0);
}

void Canvas::setOpenLineDoorWidth(float units)
{
    m_openDoorWidth = std::max(units, 0.f);
    if(m_openLineShaderPtr) m_openLineShaderPtr->setUniform("zero_space_width", units);
}

void Canvas::setOpenLineStubRatio(float ratio)
{
    m_openStubRatio = std::clamp(ratio, 0.01f, 0.49f);
    if(m_openLineShaderPtr) m_openLineShaderPtr->setUniform("one_space_width", ratio);
}

void Canvas::setDottedLineSpacing(float dotsPerUnit)
{
    m_dotSpacing = std::max(dotsPerUnit, 0.1f);
    m_updateLines = true;
}

void Canvas::setDottedLineRadius(float pixels)
{
    m_dotRadius = std::max(pixels, 0.1f);
    if(m_dottedLineShaderPtr) m_dottedLineShaderPtr->setUniform("dot_radius", m_dotRadius);
}

void Canvas::setDottedLineRowHeight(float pixelsPerRow)
{
    m_dotRowPixelHeight = std::max(pixelsPerRow, 0.1f);
    m_updateLines = true;
}

bool Canvas::preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position)
{
    sf::Vector2f local_view_pixel = mapLocalToView(getLocalPixel(mouse_position));
    std::size_t hovered_cell_id{NULL_CELL};

    if(checkFlag(m_flags, FLAGS::HOVERED))
    {
        if(const std::optional<std::size_t> idx = getCellIndex(local_view_pixel))
        {
            hovered_cell_id = *idx;
        }
    }

    if(hovered_cell_id != m_hoveredCellID)
    {
        _onCellHover(hovered_cell_id);
    }

    return false;
}

bool Canvas::postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    sf::Vector2i local_mouse_position = getLocalPixel(mouse_position); //!< doesn't considers canvas view

    bool event_used{false};

    if(details.event_type == W_MouseEventType::Press)
    {
        if(!checkFlag(m_flags, FLAGS::HOVERED)) return event_used;
        if(details.button != s_FOCUS_BUTTON) return event_used;

        m_clickedCellID = m_hoveredCellID;

        if(m_clickedCellID != m_selectedCellID)
        {
            _onCellSelect(NULL_CELL);
        }

        m_memMousePos = local_mouse_position;
    }

    else if(details.event_type == W_MouseEventType::Release)
    {
        if(checkFlag(m_flags, FLAGS::HOVERED))
        {
            if(m_clickedCellID == m_hoveredCellID && details.button == s_FOCUS_BUTTON)
            {
                _onCellSelect(m_clickedCellID);
            }

            if(m_selectedCellID == NULL_CELL) m_clickedCellID = NULL_CELL;
        }
    }

    else if(details.event_type == W_MouseEventType::Scroll)
    {
        if(checkFlag(m_flags, FLAGS::HOVERED)) 
        {
            zoom(details.scroll_delta, local_mouse_position);
            event_used = true;
        }
    }

    else if(details.event_type == W_MouseEventType::Hold)
    {
        if(checkFlag(m_flags, FLAGS::ACTIVE))
        {
            pan(local_mouse_position);
            event_used = true;
        }
    }

    return event_used;
}

bool Canvas::postProcessKeyEvents(const sf::Event& event)
{
    return false;
}

void Canvas::drawWidget()
{
    updateBoard();
    
    if(checkFlag(m_flags, FLAGS::GRID)) m_texture.draw(m_renderedGridLines);

    if(m_renderedCells.getVertexCount()) m_texture.draw(m_renderedCells);
    for(const GridCell& cell : m_cells)
    {
        if(!cell.texture) continue;
        m_texture.draw(&m_renderedCellTextures[cell.id * 6], 6, sf::PrimitiveType::Triangles, cell.texture);
    }

    if(m_renderedlines.getVertexCount()) m_texture.draw(m_renderedlines);
    if(m_renderedOpenLines.getVertexCount() && m_openLineShaderPtr)
        m_texture.draw(m_renderedOpenLines, m_openLineShaderPtr.get());

    if(m_renderedDottedLines.getVertexCount() && m_dottedLineShaderPtr)
        m_texture.draw(m_renderedDottedLines, m_dottedLineShaderPtr.get());

    m_texture.display();
    m_sprite.setTexture(m_texture.getTexture());
    if(m_clipShaderPtr) m_master.draw(m_sprite, m_clipShaderPtr.get());
    else m_master.draw(m_sprite);
}

void Canvas::config(FLAGS mask)
{
    if(checkFlag(mask, FLAGS::INDICATOR))
    {
        _onCellSelect(m_selectedCellID);
        _onCellHover(m_hoveredCellID);
    }

    if(checkFlag(mask, FLAGS::GRID))
    {
        if(checkFlag(m_flags, FLAGS::GRID)) m_updateGridLines = true;
    }
}

void Canvas::updatePosition()
{
    m_sprite.setPosition(m_position + m_size / 2.f);
}

void Canvas::updateSize()
{
    if(!m_texture.resize(sf::Vector2u(m_size))) return;

    m_sprite.setTexture(m_texture.getTexture(), true);
    m_sprite.setOrigin(m_size / 2.f);
    updateClipShader();
    updateCellSize();
    if(checkFlag(m_flags, FLAGS::GRID)) m_updateGridLines = true;   
}

void Canvas::updateRotation()
{
    m_sprite.setRotation(this->getRotation());
}

void Canvas::updateState()
{
    m_texture.clear(sf::Color::Transparent);

    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.3f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
    {
        m_opacity = 0.8f;
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

void Canvas::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<Canvas>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Canvas>());
    }
}

void Canvas::updateBoard()
{
    _updateGrid();
    _updateLines();
    _updateCells();
}

void Canvas::updateCellSize()
{
    sf::Vector2f cells_space = {m_size.x - m_deadZone * 2.f,
                                m_size.y - m_deadZone * 2.f};

    m_cellMinSize.x = cells_space.x / static_cast<float>(m_grid.y);
    m_cellMinSize.y = cells_space.y / static_cast<float>(m_grid.x);
    m_updateLines = true;
    m_updateCells = true;
}

void Canvas::zoom(float delta, sf::Vector2i local_mouse_position)
{
    if(!checkFlag(m_flags, FLAGS::ZOOM)) return;

    float factor = delta > 0 ? 0.9f : 1.1f;
    float newZoomLevel = m_zoomLevel * factor;

    // limit zoom
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
    else m_zoomLevel *= factor;

    if(factor == 1.f) return; // no point going forward

    sf::View v = m_texture.getView();
    v.zoom(factor);

    // zoom to center
    sf::Vector2f before_zoom = mapLocalToView(local_mouse_position);
    sf::Vector2f after_zoom = m_texture.mapPixelToCoords(local_mouse_position, v);
    v.move(before_zoom - after_zoom);

    limitView(v);
}

void Canvas::pan(sf::Vector2i local_mouse_position)
{
    if(!checkFlag(m_flags, FLAGS::PAN)) return;

    sf::Vector2f previous_state = mapLocalToView(m_memMousePos);
    sf::Vector2f current_state = m_texture.mapPixelToCoords(local_mouse_position);
    sf::Vector2f delta = previous_state - current_state;
    m_memMousePos = local_mouse_position;

    sf::View v = m_texture.getView();
    v.move(delta);

    limitView(v);
}

void Canvas::limitView(sf::View& v)
{
    sf::Vector2f half_view_size = v.getSize() / 2.f;
    sf::Vector2f view_center   = v.getCenter();

    view_center.x = std::clamp(view_center.x, half_view_size.x, m_size.x - half_view_size.x);
    view_center.y = std::clamp(view_center.y, half_view_size.y, m_size.y - half_view_size.y);

    v.setCenter(view_center);
    m_texture.setView(v);
}

bool Canvas::setLine(RectRange<unsigned int> rect, LineState val)
{
    return _setLineValue(rect, val);
}

Canvas::cell_ptr Canvas::makeHandle(std::size_t index) const
{
    if(index == NULL_CELL) return cell_ptr{};
    return cell_ptr(m_access, index, m_access->generation);
}

LineState& Canvas::getLine(std::size_t row, std::size_t col, std::size_t axis)
{
    return m_lines[(1 - axis) * (row * m_grid.y + col) +
           axis * ((m_grid.x + 1) * m_grid.y + row * (m_grid.y + 1) + col)];
}

const LineState& Canvas::getLine(std::size_t row, std::size_t col, std::size_t axis) const
{
    return m_lines[(1 - axis) * (row * m_grid.y + col) +
           axis * ((m_grid.x + 1) * m_grid.y + row * (m_grid.y + 1) + col)];
}

GridCell* Canvas::getCell(std::size_t index) const
{
    return index < m_cells.size() ? &m_cells[index] : nullptr;
}

std::size_t Canvas::getCellIndex(sf::Vector2u grid_coords) const
{
    for(std::size_t i{0}; i < m_cells.size(); ++i)
    {
        if(m_cells[i].inRange(grid_coords)) return i;
    }
    return NULL_CELL;
}

std::size_t Canvas::getCellIndex(sf::Vector2f pixel_coords) const
{
    for(std::size_t i{0}; i < m_cells.size(); ++i)
    {
        if(m_cells[i].inRange(pixel_coords)) return i;
    }
    return NULL_CELL;
}

float Canvas::LineExtractThickness(LineState line_flags)
{
    return static_cast<float>(apply_mask(line_flags, THICKNESS_MASK, THICKNESS_OFFSET));
}

sf::Color Canvas::LineExtractColor(LineState line_flags)
{
    LineState idx = apply_mask(line_flags, COLORS_MASK, COLORS_OFFSET);
    if(idx && m_sharedColors[idx - 1u].has_value())
        return *m_sharedColors[idx - 1u];
    else
        return m_style.outline_color;
}

LineType Canvas::LineExtractType(LineState line_flags)
{
    return static_cast<LineType>(apply_mask(line_flags, TYPE_MASK, TYPE_OFFSET));
}

bool Canvas::isRectClear(const RectRange<unsigned int>& rect) const
{
    for(unsigned int row{rect.x0 + 1}; row <= rect.x1; ++row)
        for(unsigned int col{rect.y0}; col <= rect.y1; ++col)
            if(getLine(row, col, 0)) return false;
 
    for(unsigned int row{rect.x0}; row <= rect.x1; ++row)
        for(unsigned int col{rect.y0 + 1}; col <= rect.y1; ++col)
            if(getLine(row, col, 1)) return false;
 
    return true;
}

bool Canvas::isRectSealed(const RectRange<unsigned int>& rect) const
{
    if(rect.x0 == 0)
        for(unsigned int col{rect.y0}; col <= rect.y1; ++col)
            if(!getLine(0, col, 0)) return false;
 
    if(rect.x1 == m_grid.x - 1)
        for(unsigned int col{rect.y0}; col <= rect.y1; ++col)
            if(!getLine(m_grid.x, col, 0)) return false;
 
    if(rect.y0 == 0)
        for(unsigned int row{rect.x0}; row <= rect.x1; ++row)
            if(!getLine(row, 0, 1)) return false;
 
    if(rect.y1 == m_grid.y - 1)
        for(unsigned int row{rect.x0}; row <= rect.x1; ++row)
            if(!getLine(row, m_grid.y, 1)) return false;
 
    return true;
}

bool Canvas::_setLineValue(const RectRange<unsigned int>& rect, LineState val, std::uint16_t mask, bool replace)
{
    if(rect.x0 > m_grid.x || rect.x1 > m_grid.x || rect.y0 > m_grid.y || rect.y1 > m_grid.y) return false;
    if(rect.x0 != rect.x1 && rect.y0 != rect.y1) return false;
    if(rect.distance().x == 0 && rect.distance().y == 0) return false;

    bool board_updated{false};
    const std::size_t AXIS = rect.x0 == rect.x1 ? 0u : 1u;
    const std::size_t FIXED_POINT = AXIS ? rect.y0 : rect.x0;
    const std::size_t START_POINT = AXIS ? rect.x0 : rect.y0;
    const std::size_t END_POINT = AXIS ? rect.x1 : rect.y1;
    const std::size_t START_LINE = std::min(START_POINT, END_POINT);
    const std::size_t END_LINE = std::max(START_POINT, END_POINT);

    for(std::size_t i{START_LINE}; i < END_LINE; ++i)
    {
        LineState& line = AXIS ? getLine(i, FIXED_POINT, AXIS) : getLine(FIXED_POINT, i, AXIS);
        if(line != val)
        {
            if((line ^ val) & LINE_SET)
                m_updateCells = true;

            if(replace)
                line = val;
            else
            {
                line &= mask;
                line |= val;
            }

            board_updated = true;
        }
    }

    if(board_updated) m_updateLines = true;
    return board_updated;
}

bool Canvas::setCellColor(std::size_t cell_id, sf::Color color)
{
    GridCell* Pcell = getCell(cell_id);
    if(Pcell)
    {
        m_renderedCells[cell_id * 6].color = color;
        m_renderedCells[cell_id * 6 + 1].color = color;
        m_renderedCells[cell_id * 6 + 2].color = color;
        m_renderedCells[cell_id * 6 + 3].color = color;
        m_renderedCells[cell_id * 6 + 4].color = color;
        m_renderedCells[cell_id * 6 + 5].color = color;
        return true;
    }
    return false;
}

int Canvas::getColorIndex(sf::Color color)
{
    if(color == DEFAULT_COLOR) return -1;

    for(std::size_t i{0}; i < MAX_COLOR_STORAGE; ++i)
    {
        if(!m_sharedColors[i].has_value())
        {
            m_sharedColors[i].emplace(color);
            return i;
        }

        if(m_sharedColors[i].has_value() && *m_sharedColors[i] == color)
            return i;
    }
    return -1;
}

void Canvas::_refreshCellColor(std::size_t cell_id)
{
    const GridCell* cell = getCell(cell_id);
    if(!cell) return;

    sf::Color color = cell->fill_color;
    if(cell_id == m_selectedCellID && checkFlag(m_flags, FLAGS::INDICATOR)) color = m_style.selected_cell_color;
    else if(cell_id == m_hoveredCellID) color = m_style.hovered_cell_color;

    setCellColor(cell_id, color);
}

void Canvas::_onCellHover(std::size_t hovered_cell_id)
{
    const std::size_t previous = m_hoveredCellID;
    m_hoveredCellID = hovered_cell_id;
    _refreshCellColor(previous);
    _refreshCellColor(hovered_cell_id);
}

void Canvas::_onCellSelect(std::size_t selected_cell_id)
{
    const std::size_t previous = m_selectedCellID;
    m_selectedCellID = selected_cell_id;
    _refreshCellColor(previous);
    _refreshCellColor(selected_cell_id);
}

void Canvas::_updateLines()
{
    if(!m_updateLines) return;
    m_updateLines = false;

    m_renderedlines.clear();
    m_renderedOpenLines.clear();
    m_renderedDottedLines.clear();

    for(std::size_t row{0}; row <= m_grid.x; ++row)
    {
        unsigned int col{0};
        while(col < m_grid.y)
        {
            if(LineState& line_flags = getLine(row, col, 0))
            {
                const std::size_t line_start{col};
                while(col < m_grid.y && getLine(row, col, 0) == line_flags) ++col;

                const float thickness = LineExtractThickness(line_flags);
                const sf::Color color = LineExtractColor(line_flags);
                const LineType type = LineExtractType(line_flags);
                const float run_length = static_cast<float>(col - line_start);

                const float x0 = m_deadZone + static_cast<float>(line_start) * m_cellMinSize.x;
                const float x1 = m_deadZone + static_cast<float>(col) * m_cellMinSize.x;
                const float y = m_deadZone + static_cast<float>(row) * m_cellMinSize.y;

                if(type == LineType::Dotted)
                {
                    const float half = (thickness * m_dotRowPixelHeight) / 2.f;
                    RectRange<float> rect{x0, y - half, x1, y + half};
                    const float dot_cycles = std::max(std::round(run_length * m_dotSpacing), 1.f);
                    _appendDottedLineQuad(m_renderedDottedLines, rect, color, dot_cycles, thickness, false);
                }
                else
                {
                    const float half = thickness / 2.f;
                    RectRange<float> rect{x0 - half, y - half, x1 + half, y + half};

                    if(type == LineType::Open) _appendOpenLineQuad(m_renderedOpenLines, rect, color, run_length, false);
                    else append_rectangle(m_renderedlines, rect, color);
                }
            }
            else ++col;
        }
    }

    for(std::size_t col{0}; col <= m_grid.y; ++col)
    {
        unsigned int row{0};
        while(row < m_grid.x)
        {
            if(LineState& line_flags = getLine(row, col, 1))
            {
                const std::size_t line_start{row};
                while(row < m_grid.x && getLine(row, col, 1) == line_flags) ++row;

                const float thickness = LineExtractThickness(line_flags);
                const sf::Color color = LineExtractColor(line_flags);
                const LineType type = LineExtractType(line_flags);
                const float run_length = static_cast<float>(row - line_start);

                const float y0 = m_deadZone + static_cast<float>(line_start) * m_cellMinSize.y;
                const float y1 = m_deadZone + static_cast<float>(row) * m_cellMinSize.y;
                const float x = m_deadZone + static_cast<float>(col) * m_cellMinSize.x;

                if(type == LineType::Dotted)
                {
                    const float half = (thickness * m_dotRowPixelHeight) / 2.f;
                    RectRange<float> rect{x - half, y0, x + half, y1};
                    const float dot_cycles = std::max(std::round(run_length * m_dotSpacing), 1.f);
                    _appendDottedLineQuad(m_renderedDottedLines, rect, color, dot_cycles, thickness, true);
                }
                else
                {
                    const float half = thickness / 2.f;
                    RectRange<float> rect{x - half, y0 - half, x + half, y1 + half};

                    if(type == LineType::Open) _appendOpenLineQuad(m_renderedOpenLines, rect, color, run_length, true);
                    else append_rectangle(m_renderedlines, rect, color);
                }
            }
            else ++row;
        }
    }
}

void Canvas::_updateCells()
{
    if(!m_updateCells) return;
    m_updateCells = false;

    struct Bounds
    {
        RectRange<unsigned int> box;
        std::uint32_t count{0};
    };

    std::vector<std::uint32_t> dsu(m_MAX_CELL_COUNT);
    std::vector<Bounds> bounds(m_MAX_CELL_COUNT);
    std::deque<GridCell> cached_cells = std::move(m_cells);
    std::unordered_map<std::uint64_t, std::size_t> hashed_cells;
    hashed_cells.reserve(cached_cells.size());
    m_renderedCells.clear();
    m_renderedCellTextures.clear();
    m_cells.clear();

    std::size_t hovered_id = m_hoveredCellID;
    std::size_t clicked_id = m_clickedCellID;
    std::size_t selected_id = m_selectedCellID;
    m_hoveredCellID = m_selectedCellID = m_clickedCellID = NULL_CELL;

    gen_iota(dsu.begin(), dsu.end(), 0u);

    auto find_dsu = [&dsu](std::uint32_t idx) -> std::uint32_t
    {
        while(dsu[idx] != idx)
        {
            dsu[idx] = dsu[dsu[idx]];
            idx = dsu[idx];
        }
        return idx; 
    };

    // group units
    for(std::size_t row{0}; row < m_grid.x; ++row)
    {
        for(std::size_t col{0}; col < m_grid.y; ++col)
        {
            const std::uint32_t idx = row * m_grid.y + col;
            if(col + 1 < m_grid.y && !getLine(row, col + 1, 1))
                dsu[find_dsu(idx + 1)] = find_dsu(idx);

            if(row + 1 < m_grid.x && !getLine(row + 1, col, 0))
                dsu[find_dsu(idx + m_grid.y)] = find_dsu(idx);
        }
    }

    // identify rect surface for each group
    for(unsigned int row{0}; row < m_grid.x; ++row)
    {
        for(unsigned int col{0}; col < m_grid.y; ++col)
        {
            Bounds& b = bounds[find_dsu(row * m_grid.y + col)];
            if(b.count == 0)
            {
                b.box.x0 = b.box.x1 = row;
                b.box.y0 = b.box.y1 = col;
            }
            else
            {
                b.box.x0 = std::min(b.box.x0, row);
                b.box.x1 = std::max(b.box.x1, row);
                b.box.y0 = std::min(b.box.y0, col);
                b.box.y1 = std::max(b.box.y1, col);
            }
            ++b.count;
        }
    }

    // get previous cells' rect hashes
    for(std::size_t i{0}; i < cached_cells.size(); ++i)
    {
        hashed_cells.emplace(pack_bounds(cached_cells[i].grid), i);
    }

    // create/inherit new gen cells
    for(std::size_t unit_idx{0}; unit_idx < m_MAX_CELL_COUNT; ++unit_idx)
    {
        const Bounds& bound = bounds[unit_idx];

        // non-owning space
        if(bound.count == 0) continue;
        // bad shape
        if(bound.count != (bound.box.x1 - bound.box.x0 + 1) * (bound.box.y1 - bound.box.y0 + 1)) continue;
        // internal line(s)
        if(!isRectClear(bound.box)) continue;
        // not sealed (on canvas edges)
        if(!isRectSealed(bound.box)) continue;

        GridCell& cell = m_cells.emplace_back(m_cells.size());

        cell.Proot = this;

        cell.grid.x0 = bound.box.x0;
        cell.grid.x1 = bound.box.x1;
        cell.grid.y0 = bound.box.y0;
        cell.grid.y1 = bound.box.y1;

        // pass old attributes to the same cell
        const std::uint64_t key = pack_bounds(bound.box);
        auto it = hashed_cells.find(key);
        if(it != hashed_cells.end())
        {
            if(hovered_id == it->second) m_hoveredCellID = cell.id;
            if(clicked_id == it->second) m_clickedCellID = cell.id;
            if(selected_id == it->second) m_selectedCellID = cell.id;
            cached_cells[it->second].inheritTo(cell);
        }

        RectRange<float> rect;
        rect.x0 = m_deadZone + static_cast<float>(bound.box.y0) * m_cellMinSize.x;
        rect.y0 = m_deadZone + static_cast<float>(bound.box.x0) * m_cellMinSize.y;
        rect.x1 = m_deadZone + static_cast<float>(bound.box.y1 + 1) * m_cellMinSize.x;
        rect.y1 = m_deadZone + static_cast<float>(bound.box.x1 + 1) * m_cellMinSize.y;
        append_rectangle(m_renderedCells, rect, cell.fill_color);
    }
    
    _refreshCellColor(m_hoveredCellID);
    _refreshCellColor(m_selectedCellID);

    /*
    * assuming every cell can have a texture at anytime
    * memory is allocated either way since it's cheap
    * or do we ask for 120 bytes every time like broke lil bitches?
    */
    m_renderedCellTextures.resize(m_cells.size() * 6);
    for(const GridCell& cell : m_cells)
    {
        if(cell.texture) _emplaceCellTexture(cell.id);
    }
    ++m_access->generation;
}

void Canvas::_updateGrid()
{
    if(!m_updateGridLines || !checkFlag(m_flags, FLAGS::GRID)) return;
    m_renderedGridLines.clear();
    const sf::Color color = m_style.outline_color;
    m_renderedGridLines.resize((m_grid.x + 1) * 2 + (m_grid.y + 1) * 2);
    std::size_t offset{0};
    for(unsigned int r{0}; r <= m_grid.x ; ++r)
    {
        const float y = m_deadZone + static_cast<float>(r) * m_cellMinSize.y;
        m_renderedGridLines[offset++] = sf::Vertex{{m_deadZone, y}, color};
        m_renderedGridLines[offset++] = sf::Vertex{{m_size.x - m_deadZone, y}, color};
    }
    for(unsigned int c{0}; c <= m_grid.y; ++c)
    {
        const float x = m_deadZone + static_cast<float>(c) * m_cellMinSize.x;
        m_renderedGridLines[offset++] = sf::Vertex{{x, m_deadZone}, color};
        m_renderedGridLines[offset++] = sf::Vertex{{x, m_size.y - m_deadZone}, color};
    }
 
    m_updateGridLines = false;
}

bool Canvas::setCellTexture(std::size_t cell_id, const std::string& image_path)
{
    GridCell* cell = getCell(cell_id);
    if(!cell) return false;

    const RectRange<float> PIXELS_RECT = mapGridRectToCoords(cell->grid);
    const sf::Vector2f CELL_SIZE = PIXELS_RECT.distance();
    if(CELL_SIZE.x < 1.f || CELL_SIZE.y < 1.f) return false;

    const sf::Vector2u target_size{static_cast<unsigned int>(CELL_SIZE.x), static_cast<unsigned int>(CELL_SIZE.y)};

    auto texture_pair = _getTexture(image_path, target_size);
    if(!texture_pair.second) return false;

    cell->releaseTexture();
    cell->texture_key = texture_pair.first;
    cell->texture = texture_pair.second;

    _emplaceCellTexture(cell_id);
    return true;
}

void Canvas::_emplaceCellTexture(std::size_t cell_id)
{
    const GridCell* cell = getCell(cell_id);
    if(!cell || !cell->texture) return;

    const RectRange<float> rect = mapGridRectToCoords(cell->grid);
    const sf::Vector2f size(cell->texture->getSize());
    const std::size_t base = cell_id * 6;

    m_renderedCellTextures[base + 0] = sf::Vertex{{rect.x0, rect.y0}, sf::Color::White, {0.f, 0.f}};
    m_renderedCellTextures[base + 1] = sf::Vertex{{rect.x1, rect.y0}, sf::Color::White, {size.x, 0.f}};
    m_renderedCellTextures[base + 2] = sf::Vertex{{rect.x1, rect.y1}, sf::Color::White, {size.x, size.y}};
    m_renderedCellTextures[base + 3] = sf::Vertex{{rect.x1, rect.y1}, sf::Color::White, {size.x, size.y}};
    m_renderedCellTextures[base + 4] = sf::Vertex{{rect.x0, rect.y1}, sf::Color::White, {0.f, size.y}};
    m_renderedCellTextures[base + 5] = sf::Vertex{{rect.x0, rect.y0}, sf::Color::White, {0.f, 0.f}};
}


std::pair<std::string, const sf::Texture*> Canvas::_getTexture(const std::string& path, sf::Vector2u target_size)
{
    const std::string key = make_texture_key(path, target_size);
    std::unordered_map<std::string, TexSource>::iterator it = m_cache.find(key);
    if(it != m_cache.end())
    {
        ++it->second.ref_count;
        return {key, &it->second.texture};
    }

    sfml_widgets::Widget::ImageCache& image_cache = sfml_widgets::Widget::ImageCache::getInstance();
    std::optional<const sf::Image*> source = image_cache.getImage(path);
    if(!source) return {"", nullptr};

    std::optional<sf::Image> resized = sfml_widgets::resize_image(**source, target_size);
    image_cache.decrementRefCounter(path);
    if(!resized) return {"", nullptr};

    sf::Texture texture;
    if(!texture.loadFromImage(*resized)) return {"", nullptr};
    texture.setSmooth(true);

    TexSource& entry = m_cache[key];
    entry.texture = std::move(texture);
    entry.ref_count = 1;
    return {key, &entry.texture};
}

void Canvas::_releaseTexture(GridCell& cell)
{
    if(cell.texture_key.empty()) return;

    std::unordered_map<std::string, TexSource>::iterator it = m_cache.find(cell.texture_key);
    if(it != m_cache.end() && --it->second.ref_count == 0) m_cache.erase(it);
    cell.texture = nullptr;
    cell.texture_key.clear();
    _refreshCellColor(cell.id);
}

void Canvas::_appendOpenLineQuad(sf::VertexArray& target, RectRange<float> rect, sf::Color color, float run_length, bool vertical)
{
    const std::size_t offset = target.getVertexCount();
    target.resize(offset + 6);

    if(!vertical)
    {
        target[offset + 0] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,         run_length}};
        target[offset + 1] = sf::Vertex{{rect.x1, rect.y0}, color, {run_length,  run_length}};
        target[offset + 2] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length,  run_length}};
        target[offset + 3] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length,  run_length}};
        target[offset + 4] = sf::Vertex{{rect.x0, rect.y1}, color, {0.f,         run_length}};
        target[offset + 5] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,         run_length}};
    }
    else
    {
        target[offset + 0] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,         run_length}};
        target[offset + 1] = sf::Vertex{{rect.x1, rect.y0}, color, {0.f,         run_length}};
        target[offset + 2] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length,  run_length}};
        target[offset + 3] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length,  run_length}};
        target[offset + 4] = sf::Vertex{{rect.x0, rect.y1}, color, {run_length,  run_length}};
        target[offset + 5] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,         run_length}};
    }
}

void Canvas::_appendDottedLineQuad(sf::VertexArray& target, RectRange<float> rect, sf::Color color, float run_length, float row_count, bool vertical)
{
    const std::size_t offset = target.getVertexCount();
    target.resize(offset + 6);

    if(!vertical)
    {
        target[offset + 0] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,        0.f}};
        target[offset + 1] = sf::Vertex{{rect.x1, rect.y0}, color, {run_length, 0.f}};
        target[offset + 2] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length, row_count}};
        target[offset + 3] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length, row_count}};
        target[offset + 4] = sf::Vertex{{rect.x0, rect.y1}, color, {0.f,        row_count}};
        target[offset + 5] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,        0.f}};
    }
    else
    {
        target[offset + 0] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,        0.f}};
        target[offset + 1] = sf::Vertex{{rect.x1, rect.y0}, color, {0.f,        row_count}};
        target[offset + 2] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length, row_count}};
        target[offset + 3] = sf::Vertex{{rect.x1, rect.y1}, color, {run_length, row_count}};
        target[offset + 4] = sf::Vertex{{rect.x0, rect.y1}, color, {run_length, 0.f}};
        target[offset + 5] = sf::Vertex{{rect.x0, rect.y0}, color, {0.f,        0.f}};
    }
}
}



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
    surface = std::clamp(surface, 0.f, s_MAX_DEADZONE_RATIO);
    m_deadZone = surface * std::min<float>(m_size.x, m_size.y);
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

Cell* GridCanvas::getSelectedCellPtr() const
{
    return m_selectedCell;
}

Cell* GridCanvas::getClickedCellPtr() const
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

void GridCanvas::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<GridCanvas>());
}

TextureStyle& GridCanvas::style()
{
    return m_style;
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

bool GridCanvas::postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    sf::Vector2i local_mouse_position = getLocalPixel(mouse_position); //!< doesn't considers canvas view

    bool event_used{false};

    // click event
    if(details.event_type == W_MouseEventType::Press)
    {
        if(!checkFlag(m_flags, FLAGS::HOVERED)) return event_used; //!< Not hovering

        /* currently hovering */

    
        if(m_selectedCell)
        {
            // reset previous
            resetFlag(m_selectedCell->m_flags, FLAGS::SELECTED);
            m_selectedCell = nullptr;
        }
        
        if(details.button != s_FOCUS_BUTTON) return event_used;

        /* left mouse button was clicked section */

        m_memMousePos = local_mouse_position; //!< for panning

        // check if we clicked on a cell
        for(Cell& cell : m_cellMap)
        {
            if(cell.isFlag(FLAGS::HOVERED))
            {
                // first activation stage
                m_clickedCell = &cell;
                event_used = true;
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
                    event_used = true;
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
            event_used = true;
        }
    }

    // moving event
    else if(details.event_type == W_MouseEventType::Hold)
    {
        if(checkFlag(m_flags, FLAGS::ACTIVE))
        {
            pan(local_mouse_position);
            event_used = true;
        }
    }

    return event_used;
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

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.3f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
    }
    else if(!m_stale && checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
    {
        m_opacity = 0.8f;
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
    sf::Vector2f cells_space = {m_size.x - m_deadZone * 2.f,
                                m_size.y - m_deadZone * 2.f};

    m_cellSize.x = cells_space.x / static_cast<float>(m_gridMapping.y);
    m_cellSize.y = cells_space.y / static_cast<float>(m_gridMapping.x);

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
