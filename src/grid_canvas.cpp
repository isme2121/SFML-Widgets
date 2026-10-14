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
}

namespace sfml_widgets // LineRenderer
{
LineRenderer::LineRenderer(const std::string& line_type, std::string_view shader, Widget::W_LoadType loading_type) : m_type(line_type)
{
    if(!shader.empty())
    {
        m_shader = std::make_unique<sf::Shader>();
        if(loading_type == Widget::W_LoadType::File && m_shader->loadFromFile(shader, sf::Shader::Type::Fragment) ||
            loading_type == Widget::W_LoadType::Memory && m_shader->loadFromMemory(shader, sf::Shader::Type::Fragment))
        {
            m_states.shader = m_shader.get();
        }
        else
        {
            m_shader.reset();
        }

    }
}

LineRenderer::~LineRenderer() = default;

void LineRenderer::appendLine(float line_begin_px, float line_end_px, float fixed_px, float line_thickness,
                            float run_length_units, sf::Color line_color, bool is_vertical)
{
    const float half = line_thickness / 2.f;
    const RectRange<float> rect = is_vertical ?
            RectRange<float>{fixed_px - half, line_begin_px - half, fixed_px + half, line_end_px + half} :
            RectRange<float>{line_begin_px - half, fixed_px - half, line_end_px + half, fixed_px + half};
    append_rectangle(m_vertices, rect, line_color);   
}

void LineRenderer::draw(sf::RenderTarget& target) const
{
    if(m_vertices.getVertexCount()) target.draw(m_vertices, m_states);
}
void LineRenderer::clear()
{
    m_vertices.clear();
}

std::size_t LineRenderer::getVertexCount() const
{
    return m_vertices.getVertexCount();
}

const std::string& LineRenderer::getType() const
{
    return m_type;
}

void LineRenderer::setLineUpdate()
{
    m_requireLineUpdate = true;
}

#ifdef CANVAS_MORE_LINE_TYPES
OpenLineRenderer::OpenLineRenderer() : LineRenderer(OPEN_LINE_LABEL, OPEN_LINE_SHADER_FRAG)
{
    setUniform("join_lines", m_joinMode);
    setUniform("zero_space_width_ratio", m_doorWidthRatio);
    setUniform("one_space_width_ratio", m_stubRatio);
}

void OpenLineRenderer::appendLine(float line_begin_px, float line_end_px, float fixed_px, float line_thickness,
                                float run_length_units, sf::Color line_color, bool is_vertical)
{
    const float half = line_thickness / 2.f;
    const RectRange<float> rect = is_vertical ?
            RectRange<float>{fixed_px - half, line_begin_px - half, fixed_px + half, line_end_px + half} :
            RectRange<float>{line_begin_px - half, fixed_px - half, line_end_px + half, fixed_px + half};

    const std::size_t offset = m_vertices.getVertexCount();
    m_vertices.resize(offset + 6);
    if(!is_vertical)
    {
        m_vertices[offset+0] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, run_length_units}};
        m_vertices[offset+1] = sf::Vertex{{rect.x1, rect.y0}, line_color, {run_length_units, run_length_units}};
        m_vertices[offset+2] = sf::Vertex{{rect.x1, rect.y1}, line_color, {run_length_units, run_length_units}};
        m_vertices[offset+3] = sf::Vertex{{rect.x1, rect.y1}, line_color, {run_length_units, run_length_units}};
        m_vertices[offset+4] = sf::Vertex{{rect.x0, rect.y1}, line_color, {0.f, run_length_units}};
        m_vertices[offset+5] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, run_length_units}};
    }
    else
    {
        m_vertices[offset+0] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, run_length_units}};
        m_vertices[offset+1] = sf::Vertex{{rect.x1, rect.y0}, line_color, {0.f, run_length_units}};
        m_vertices[offset+2] = sf::Vertex{{rect.x1, rect.y1}, line_color, {run_length_units, run_length_units}};
        m_vertices[offset+3] = sf::Vertex{{rect.x1, rect.y1}, line_color, {run_length_units, run_length_units}};
        m_vertices[offset+4] = sf::Vertex{{rect.x0, rect.y1}, line_color, {run_length_units, run_length_units}};
        m_vertices[offset+5] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, run_length_units}};
    }
}

void OpenLineRenderer::setJoinMode(bool merged)
{
    m_joinMode = merged;
    setUniform("join_lines", m_joinMode);
}

void OpenLineRenderer::setDoorWidthRatio(float ratio)
{
    m_doorWidthRatio = std::clamp(ratio, 0.f, 1.f);
    setUniform("zero_space_width_ratio", m_doorWidthRatio);
}

void OpenLineRenderer::setStubRatio(float ratio)
{
    m_stubRatio = std::clamp(ratio, 0.01f, 0.49f);
    setUniform("one_space_width_ratio", m_stubRatio);
}

DottedLineRenderer::DottedLineRenderer() : LineRenderer(DOTS_LINE_LABEL, DOTTED_LINE_SHADER_FRAG)
{
    setUniform("dot_radius", m_pixelRadius);
}

void DottedLineRenderer::appendLine(float line_begin_px, float line_end_px, float fixed_px, float line_thickness,
                                float run_length_units, sf::Color line_color, bool is_vertical)
{
    const float half = (line_thickness * m_rowPixelHeight) / 2.f;
    const RectRange<float> rect = is_vertical
        ? RectRange<float>{fixed_px - half, line_begin_px, fixed_px + half, line_end_px}
        : RectRange<float>{line_begin_px, fixed_px - half, line_end_px, fixed_px + half};

    const float dot_cycles = std::max(std::round(run_length_units * m_dotSpacing), 1.f);

    const std::size_t offset = m_vertices.getVertexCount();
    m_vertices.resize(offset + 6);
    if(!is_vertical)
    {
        m_vertices[offset+0] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, 0.f}};
        m_vertices[offset+1] = sf::Vertex{{rect.x1, rect.y0}, line_color, {dot_cycles, 0.f}};
        m_vertices[offset+2] = sf::Vertex{{rect.x1, rect.y1}, line_color, {dot_cycles, line_thickness}};
        m_vertices[offset+3] = sf::Vertex{{rect.x1, rect.y1}, line_color, {dot_cycles, line_thickness}};
        m_vertices[offset+4] = sf::Vertex{{rect.x0, rect.y1}, line_color, {0.f, line_thickness}};
        m_vertices[offset+5] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, 0.f}};
    }
    else
    {
        m_vertices[offset+0] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, 0.f}};
        m_vertices[offset+1] = sf::Vertex{{rect.x1, rect.y0}, line_color, {0.f, line_thickness}};
        m_vertices[offset+2] = sf::Vertex{{rect.x1, rect.y1}, line_color, {dot_cycles, line_thickness}};
        m_vertices[offset+3] = sf::Vertex{{rect.x1, rect.y1}, line_color, {dot_cycles, line_thickness}};
        m_vertices[offset+4] = sf::Vertex{{rect.x0, rect.y1}, line_color, {dot_cycles, 0.f}};
        m_vertices[offset+5] = sf::Vertex{{rect.x0, rect.y0}, line_color, {0.f, 0.f}};
    }
}

void DottedLineRenderer::setSpacing(unsigned int dotsPerUnit)
{
    m_dotSpacing = std::max(dotsPerUnit, 1u);
    setLineUpdate();
}

void DottedLineRenderer::setRowPixelHeight(float px)
{
    m_rowPixelHeight = std::max(px, 0.1f);
    setLineUpdate();
}

void DottedLineRenderer::setPixelRadius(float px)
{
    m_pixelRadius = std::max(px, 0.1f);
    setUniform("dot_radius", m_pixelRadius);
}
#endif
}

namespace sfml_widgets // Cell
{
Cell::Cell(std::size_t id) : id(id)
{

}

Cell::~Cell()
{
    releaseTexture();
}

void Cell::releaseTexture()
{
    Proot->_releaseTexture(*this);
}

bool Cell::setCellTexture(const std::string& image_path)
{
    return Proot->setCellTexture(id, image_path);
}

bool Cell::clearCellTexture()
{
    releaseTexture();
    return true;
}

bool Cell::setCellColor(sf::Color color)
{
    fill_color = color;
    Proot->_refreshCellColor(id);
    return true;
}

bool Cell::setOutlineThickness(DIRECTION _s, LineState thickness)
{
    return Proot->setLineThickness(getLineRange(_s), thickness);
}

bool Cell::setOutlineColor(DIRECTION _s, sf::Color color)
{
    return Proot->setLineColor(getLineRange(_s), color);
}

bool Cell::setOutlineType(DIRECTION _s, const std::string& line_type)
{
    return Proot->setLineType(getLineRange(_s), line_type);
}

bool Cell::inRange(sf::Vector2u grid_coords) const
{
    return grid.inRange(grid_coords);
}

bool Cell::inRange(sf::Vector2f pixel_coords) const
{
    RectRange<float> frange = Proot->mapGridRectToCoords(grid);
    return frange.inRange(pixel_coords);
}

std::size_t Cell::getID() const
{
    return id;
}

bool Cell::operator==(const Cell& other) const
{
    return this->id == other.id;
}

bool Cell::operator==(std::size_t id) const
{
    return this->id == id;
}

RectRange<unsigned int> Cell::getLineRange(DIRECTION _s) const
{
    RectRange<unsigned int> rect;

    /* rect and GridCanvas identify ranges differently where
    * rect ==> [x0, x1]
    * GridCanvas ==> [x0, x1[
    * so +1 is added to the end range when talking to GridCanvas
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

void Cell::inheritTo(Cell& other)
{
    if(&other == this) return;

    other.fill_color = fill_color;
    other.texture = texture;
    other.texture_path = std::move(texture_path);

    texture = nullptr;
    texture_path.clear();
}

}

namespace sfml_widgets // cell_ptr
{
GridCanvas::cell_ptr::cell_ptr(std::weak_ptr<Access> access, std::size_t index, std::uint32_t generation) :
m_access(std::move(access)), m_index(index), m_generation(generation)
{

}

GridCanvas::cell_ptr::operator bool() const
{
    return resolve() != nullptr;
}

Cell* GridCanvas::cell_ptr::operator->() const
{
    Cell* cell = resolve();
    if(!cell)
    {
        assert(false && "Cell used after it was invalidated by a GridCanvas rebuild, or after its GridCanvas was destroyed");
        std::abort();
    }
    return cell;
}

Cell& GridCanvas::cell_ptr::operator*() const
{
    return *operator->();
}

Cell* GridCanvas::cell_ptr::resolve() const
{
    const std::shared_ptr<Access> locked = m_access.lock();
    if(!locked || locked->generation != m_generation) return nullptr;
    return locked->canvas->getCell(m_index);
}
}

namespace sfml_widgets // GridCanvas
{
GridCanvas::GridCanvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid) : Widget(window, position, size),
m_texture(size), m_sprite(m_texture.getTexture()), m_MAX_CELL_COUNT(grid.x * grid.y), m_GRID_MAP(grid) 
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    m_access = std::make_shared<Access>(Access{this, 0u});

    m_lines.assign(static_cast<std::size_t>(m_GRID_MAP.x * m_GRID_MAP.y * 2 + m_GRID_MAP.x + m_GRID_MAP.y), 0u);
    m_lineRenderers.reserve(TYPE_FLAGS_COUNT);

    addLineType(std::make_unique<LineRenderer>());

    #ifdef CANVAS_MORE_LINE_TYPES
    addLineType(std::make_unique<OpenLineRenderer>());
    addLineType(std::make_unique<DottedLineRenderer>());
    #endif

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    toggle(m_flags, FLAGS::ZOOM | FLAGS::PAN);
}

GridCanvas::GridCanvas(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid) : Widget(root, position, size),
m_texture(size), m_sprite(m_texture.getTexture()), m_MAX_CELL_COUNT(grid.x * grid.y), m_GRID_MAP(grid)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    m_access = std::make_shared<Access>(Access{this, 0u});

    m_lines.assign(static_cast<std::size_t>(m_GRID_MAP.x * m_GRID_MAP.y * 2 + m_GRID_MAP.x + m_GRID_MAP.y), 0u);
    m_lineRenderers.reserve(TYPE_FLAGS_COUNT);

    addLineType(std::make_unique<LineRenderer>());

    #ifdef CANVAS_MORE_LINE_TYPES
    addLineType(std::make_unique<OpenLineRenderer>());
    addLineType(std::make_unique<DottedLineRenderer>());
    #endif

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);
    

    toggle(m_flags, FLAGS::ZOOM | FLAGS::PAN);
}
    
GridCanvas::~GridCanvas() = default;

bool GridCanvas::drawLine(RectRange<unsigned int> line_range, LineState thickness, sf::Color color, const std::string& state)
{
    LineState val{LINE_SET};

    if(thickness < THICKNESS_FLAGS_COUNT)
        val |= apply_mask(apply_shift(thickness, THICKNESS_OFFSET), THICKNESS_MASK);

    if(color != DEFAULT_COLOR)
    {
        int idx = getColorIndex(color);
        // -1 ==> take GridCanvas outline color
        if(idx != -1) val |= apply_mask(apply_shift(idx + 1, COLORS_OFFSET), COLORS_MASK);
    }

    if(state != NORMAL_LINE_LABEL)
    {
        val |= apply_mask(apply_shift(static_cast<LineState>(getTypeIndex(state)), TYPE_OFFSET), TYPE_MASK);
    }

    return _setLineValue(line_range, val);
}

bool GridCanvas::removeLine(RectRange<unsigned int> line_range)
{
    return _setLineValue(line_range, 0u);
}

void GridCanvas::clearCanvas()
{
    std::fill(m_lines.begin(), m_lines.end(), 0u);
    std::fill(m_reservedLineColors.begin(), m_reservedLineColors.end(), std::nullopt);

    m_history.clear();
    m_pen.anchor.reset();
    m_historyCursor = 0;

    m_updateLines = true;
    m_updateCells = true;
}

bool GridCanvas::setLineThickness(RectRange<unsigned int> line_range, LineState thickness)
{
    LineState val{LINE_SET};

    if(thickness < THICKNESS_FLAGS_COUNT)
    {
        val |= apply_mask(apply_shift(thickness, THICKNESS_OFFSET), THICKNESS_MASK);
        return _setLineValue(line_range, val, THICKNESS_OFF, false);
    }
    return false;
}

bool GridCanvas::setLineColor(RectRange<unsigned int> line_range, sf::Color color)
{
    LineState val{LINE_SET};

    int idx = getColorIndex(color);
    if(idx != -1) val |= apply_mask(apply_shift(idx + 1, COLORS_OFFSET), COLORS_MASK);
    return _setLineValue(line_range, val, COLORS_OFF, false);
}

bool GridCanvas::setLineType(RectRange<unsigned int> line_range, const std::string& Line_type)
{
    LineState val{LINE_SET};

    val |= apply_mask(apply_shift(static_cast<LineState>(getTypeIndex(Line_type)), TYPE_OFFSET), TYPE_MASK);
    return _setLineValue(line_range, val, TYPE_OFF, false);
}

LineRenderer* GridCanvas::getLineRenderer(const std::string& line_type)
{
    const auto it = std::find_if(m_lineRenderers.begin(), m_lineRenderers.end(),
        [&line_type](const std::unique_ptr<LineRenderer>& element)
        {
            return element->getType() == line_type;
        });

    return it != m_lineRenderers.end() ? it->get() : nullptr;
}

bool GridCanvas::addLineType(std::unique_ptr<LineRenderer> new_line_type)
{
    if(!new_line_type) return false;
    if(m_lineRenderers.size() >= TYPE_FLAGS_COUNT) return false;

    const auto it = std::find_if(m_lineRenderers.begin(), m_lineRenderers.end(),
        [&new_line_type](const std::unique_ptr<LineRenderer>& element)
        {
            return element->getType() == new_line_type->getType();
        });

    if(it != m_lineRenderers.end()) return false;

    m_lineRenderers.emplace_back(std::move(new_line_type));
    return true;
}

bool GridCanvas::removeLineType(const std::string& line_type_label)
{
    const int TYPE_IDX = getTypeIndex(line_type_label);
    if(TYPE_IDX <= 0) return false;

    __clearType(TYPE_IDX);

    m_lineRenderers.erase(m_lineRenderers.begin() + TYPE_IDX);
    m_updateLines = true;
    return true;
}

std::vector<std::string> GridCanvas::listLineTypes() const
{
    std::vector<std::string> res;
    res.reserve(m_lineRenderers.size());
    for(const auto& element : m_lineRenderers) res.emplace_back(element->getType());
    return res;
}

GridCanvas::cell_ptr GridCanvas::at(sf::Vector2u grid_coords) const
{
    return makeHandle(getCellIndex(grid_coords));
}

GridCanvas::cell_ptr GridCanvas::at(sf::Vector2f pixel_coords) const
{
    return makeHandle(getCellIndex(pixel_coords));
}

GridCanvas::cell_ptr GridCanvas::at(std::size_t cell_id) const
{
    return makeHandle(cell_id < m_cells.size() ? cell_id : NULL_CELL);
}

GridCanvas::cell_ptr GridCanvas::getHoveredCell() const
{
    return m_hoveredCellID == NULL_CELL ? cell_ptr{} : at(m_hoveredCellID);
}

GridCanvas::cell_ptr GridCanvas::getSelectedCell() const
{
    return m_selectedCellID == NULL_CELL ? cell_ptr{} : at(m_selectedCellID);
}

std::size_t GridCanvas::getCellsCount() const
{
    return m_cells.size();
}

std::size_t GridCanvas::getMaxCellsCount() const
{
    return m_GRID_MAP.x * m_GRID_MAP.y;
}

void GridCanvas::setQuickDraw(bool enabled, bool shift_skip_identical_lines)
{
    m_quickDrawEnable = enabled;
    m_quickDrawSkipIdentical = shift_skip_identical_lines;
}

void GridCanvas::showPen(bool show, sf::Color color)
{
    m_pen.show = show;
    m_pen.color = color;
}

sf::Vector2i GridCanvas::getLocalPixel(sf::Vector2i mouse_position) const
{
    sf::Vector2f global_point = m_master.mapMousePosition(mouse_position);
    return sf::Vector2i(this->getInverseTransform().transformPoint(global_point));
}

sf::Vector2f GridCanvas::getViewPixel(sf::Vector2i mouse_position) const
{
    return m_texture.mapPixelToCoords(getLocalPixel(mouse_position));
}

sf::Vector2f GridCanvas::mapLocalToView(sf::Vector2i local_position) const
{
    return m_texture.mapPixelToCoords(local_position);
}

std::optional<sf::Vector2u> GridCanvas::mapCoordsToGrid(sf::Vector2i mouse_position) const
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

    if(coordinates.x >= m_GRID_MAP.x || coordinates.y >= m_GRID_MAP.y)
        return std::nullopt;

    return coordinates;
}

std::optional<sf::Vector2f> GridCanvas::mapGridToCoords(sf::Vector2u grid_coordinates) const
{
    if(grid_coordinates.x >= m_GRID_MAP.x || grid_coordinates.y >= m_GRID_MAP.y) return std::nullopt;

    return sf::Vector2f
    {
        m_deadZone + static_cast<float>(grid_coordinates.y) * m_cellMinSize.x,
        m_deadZone + static_cast<float>(grid_coordinates.x) * m_cellMinSize.y
    };
}

RectRange<float> GridCanvas::mapGridRectToCoords(RectRange<unsigned int> grid_rect) const
{
    return
    {
        m_deadZone + static_cast<float>(grid_rect.y0) * m_cellMinSize.x,
        m_deadZone + static_cast<float>(grid_rect.x0) * m_cellMinSize.y,
        m_deadZone + static_cast<float>(grid_rect.y1 + 1) * m_cellMinSize.x,
        m_deadZone + static_cast<float>(grid_rect.x1 + 1) * m_cellMinSize.y
    };
}

RectRange<unsigned int> GridCanvas::mapCoordsRectToGrid(RectRange<float> coords_rect) const
{
    return
    {
        static_cast<unsigned int>((coords_rect.y0 - m_deadZone) / m_cellMinSize.x),
        static_cast<unsigned int>((coords_rect.x0 - m_deadZone) / m_cellMinSize.y),
        static_cast<unsigned int>((coords_rect.y1 - m_deadZone) / m_cellMinSize.x) + 1,
        static_cast<unsigned int>((coords_rect.x1 - m_deadZone) / m_cellMinSize.y) + 1
    };
}

void GridCanvas::setDeadZone(float surface)
{
    surface = std::clamp(surface, 0.f, s_MAX_DEADZONE_RATIO);
    m_deadZone = surface * std::min<float>(m_size.x, m_size.y);
    updateCellSize();
    if(checkFlag(m_flags, FLAGS::GRID)) m_updateGridLines = true;
    m_updateLines = true;
    m_updateCells = true;
}

void GridCanvas::setTheme(CanvasStyle s_canvas)
{
    m_style = s_canvas;
    setFlag(m_flags, FLAGS::MODIFIED);

    _refreshCellColor(m_hoveredCellID);
    _refreshCellColor(m_selectedCellID);
    showPen(m_pen.show, m_pen.color);
    m_updateGridLines = true;
    m_updateLines = true;
}

void GridCanvas::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<GridCanvas>());
}

CanvasStyle& GridCanvas::style()
{
    return m_style;
}

void GridCanvas::poke()
{
    setFlag(m_flags, FLAGS::MODIFIED);

    _refreshCellColor(m_hoveredCellID);
    _refreshCellColor(m_selectedCellID);
    showPen(m_pen.show, m_pen.color);
    m_updateGridLines = true;
    m_updateLines = true;
}

bool GridCanvas::preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position)
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

bool GridCanvas::postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position)
{
    sf::Vector2i local_mouse_position = getLocalPixel(mouse_position); //!< doesn't considers GridCanvas view

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

bool GridCanvas::postProcessKeyEvents(const sf::Event& event)
{
    if(!m_quickDrawEnable) return false;

    const sf::Event::KeyPressed* key_press = event.getIf<sf::Event::KeyPressed>();
    if(!key_press) return false;

    if(key_press->code == sf::Keyboard::Key::LShift || key_press->code == sf::Keyboard::Key::RShift)
    {
        __drawPen();
        return true;
    }

    // edit (CTRL)
    if(key_press->control)
    {
        if(key_press->code == sf::Keyboard::Key::Z) return _undoLine();
        if(key_press->code == sf::Keyboard::Key::Y) return _redoLine();
    }

    // delta (ARROWS)
    sf::Vector2i dir;
    switch(key_press->code)
    {
        case sf::Keyboard::Key::Up:
            dir = {-1, 0};
            break;

        case sf::Keyboard::Key::Down:
            dir = {1, 0};
            break;

        case sf::Keyboard::Key::Left:
            dir = {0, -1};
            break;

        case sf::Keyboard::Key::Right:
            dir = {0, 1};
            break;

        default:
            return false;
    }

    const LineState final_value = (key_press->control && m_pen.anchor) ? m_pen.anchor->state :
                                static_cast<LineState>(LINE_SET | apply_mask(apply_shift(1u, THICKNESS_OFFSET), THICKNESS_MASK));

    // continue from last point (SHIFT)
    RectRange<unsigned int> rect;
    if(key_press->shift)
    {
        if(!m_pen.anchor) return false;

        /*
        * Problem here is that rect.p0 can be larger than rect.p1
        * so we consider:
        * - the direction we're shifting to
        * - and from which side of the line to start from
        * so we need to find p.start first
        */
        const RectRange<unsigned int>& prev = m_pen.anchor->range;
        const unsigned int ROW_LOW = std::min(prev.x0, prev.x1);
        const unsigned int ROW_HIGH = std::max(prev.x0, prev.x1);
        const unsigned int COL_LOW = std::min(prev.y0, prev.y1);
        const unsigned int COL_HIGH = std::max(prev.y0, prev.y1);
        const bool IS_HORIZONTAL = prev.x0 == prev.x1;

        sf::Vector2u start;
        // horizontal move
        if(IS_HORIZONTAL && dir.y != 0)
            start = {prev.x0, dir.y > 0 ? COL_HIGH : COL_LOW};

        // vertical move
        else if(!IS_HORIZONTAL && dir.x != 0)
            start = {dir.x > 0 ? ROW_HIGH : ROW_LOW, prev.y0};

        // taking a turn, so anchor from the pen
        else
            start = {prev.x1, prev.y1};

        // by default, this ignores identical lines
        const std::optional<RectRange<unsigned int>> next = __findNextAnchorPoint(start, {prev.x1, prev.y1}, dir, final_value);
        if(!next) return false;
        rect = *next;
    }
    else
    {
        if(!checkFlag(m_flags, FLAGS::HOVERED)) return false;
        const std::optional<sf::Vector2u> unit = mapCoordsToGrid(sf::Mouse::getPosition(*m_master.getRootWindowPtr()));
        if(!unit) return false;

        if(dir.x)
        {
            const unsigned int edge = unit->x + (dir.x > 0 ? 1u : 0u);
            rect = {edge, unit->y, edge, unit->y + 1u};
        }
        else
        {
            const unsigned int edge = unit->y + (dir.y > 0 ? 1u : 0u);
            rect = {unit->x, edge, unit->x + 1u, edge};
        }
    }

    _setLineValue(rect, final_value);
    __drawPen();
    return true;
}
void GridCanvas::drawWidget()
{
    updateBoard();
    
    if(checkFlag(m_flags, FLAGS::GRID)) m_texture.draw(m_renderedGridLines);

    if(m_renderedCells.getVertexCount()) m_texture.draw(m_renderedCells);
    for(const Cell& cell : m_cells)
    {
        if(!cell.texture) continue;
        m_texture.draw(&m_renderedCellTextures[cell.id * 6], 6, sf::PrimitiveType::Triangles, cell.texture);
    }

    for(const auto& line_renderer : m_lineRenderers)
        line_renderer->draw(m_texture);

    _drawPenIndicator();
    m_texture.display();
    m_sprite.setTexture(m_texture.getTexture());
    if(m_clipShaderPtr) m_master.draw(m_sprite, m_clipShaderPtr.get());
    else m_master.draw(m_sprite);
}

void GridCanvas::config(FLAGS mask)
{
    if(checkFlag(mask, FLAGS::INDICATOR))
    {
        _onCellSelect(m_selectedCellID);
        _onCellHover(m_hoveredCellID);
    }

    if(checkFlag(mask, FLAGS::GRID) && checkFlag(m_flags, FLAGS::GRID))
    {
        m_updateGridLines = true;
    }
}

void GridCanvas::updatePosition()
{
    m_sprite.setPosition(m_position + m_size / 2.f);
}

void GridCanvas::updateSize()
{
    if(!m_texture.resize(sf::Vector2u(m_size))) return;

    m_sprite.setTexture(m_texture.getTexture(), true);
    m_sprite.setOrigin(m_size / 2.f);
    updateClipShader();
    updateCellSize();
    if(checkFlag(m_flags, FLAGS::GRID)) m_updateGridLines = true;   
}

void GridCanvas::updateRotation()
{
    m_sprite.setRotation(this->getRotation());
}

void GridCanvas::updateState()
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

void GridCanvas::updateBoard()
{
    _updateGrid();
    _updateLines();
    _updateCells();
}

void GridCanvas::updateCellSize()
{
    sf::Vector2f cells_space = {m_size.x - m_deadZone * 2.f,
                                m_size.y - m_deadZone * 2.f};

    m_cellMinSize.x = cells_space.x / static_cast<float>(m_GRID_MAP.y);
    m_cellMinSize.y = cells_space.y / static_cast<float>(m_GRID_MAP.x);
    m_updateLines = true;
    m_updateCells = true;
}

void GridCanvas::zoom(float delta, sf::Vector2i local_mouse_position)
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

void GridCanvas::pan(sf::Vector2i local_mouse_position)
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

void GridCanvas::limitView(sf::View& v)
{
    sf::Vector2f half_view_size = v.getSize() / 2.f;
    sf::Vector2f view_center   = v.getCenter();

    view_center.x = std::clamp(view_center.x, half_view_size.x, m_size.x - half_view_size.x);
    view_center.y = std::clamp(view_center.y, half_view_size.y, m_size.y - half_view_size.y);

    v.setCenter(view_center);
    m_texture.setView(v);
}

GridCanvas::cell_ptr GridCanvas::makeHandle(std::size_t index) const
{
    if(index == NULL_CELL) return cell_ptr{};
    return cell_ptr(m_access, index, m_access->generation);
}

LineState& GridCanvas::getLine(std::size_t row, std::size_t col, std::size_t axis)
{
    return m_lines[(1 - axis) * (row * m_GRID_MAP.y + col) +
           axis * ((m_GRID_MAP.x + 1) * m_GRID_MAP.y + row * (m_GRID_MAP.y + 1) + col)];
}

const LineState& GridCanvas::getLine(std::size_t row, std::size_t col, std::size_t axis) const
{
    return m_lines[(1 - axis) * (row * m_GRID_MAP.y + col) +
           axis * ((m_GRID_MAP.x + 1) * m_GRID_MAP.y + row * (m_GRID_MAP.y + 1) + col)];
}

Cell* GridCanvas::getCell(std::size_t index)
{
    return index < m_cells.size() ? &m_cells[index] : nullptr;
}

const Cell* GridCanvas::getCell(std::size_t index) const
{
    return index < m_cells.size() ? &m_cells[index] : nullptr;
}

std::size_t GridCanvas::getCellIndex(sf::Vector2u grid_coords) const
{
    for(std::size_t i{0}; i < m_cells.size(); ++i)
    {
        if(m_cells[i].inRange(grid_coords)) return i;
    }
    return NULL_CELL;
}

std::size_t GridCanvas::getCellIndex(sf::Vector2f pixel_coords) const
{
    for(std::size_t i{0}; i < m_cells.size(); ++i)
    {
        if(m_cells[i].inRange(pixel_coords)) return i;
    }
    return NULL_CELL;
}

float GridCanvas::LineExtractThickness(LineState line_flags)
{
    return static_cast<float>(apply_mask(line_flags, THICKNESS_MASK, THICKNESS_OFFSET));
}

sf::Color GridCanvas::LineExtractColor(LineState line_flags)
{
    LineState idx = apply_mask(line_flags, COLORS_MASK, COLORS_OFFSET);
    if(idx && m_reservedLineColors.at(idx - 1u).has_value())
        return *m_reservedLineColors.at(idx - 1u);
    else
        return m_style.outline_color;
}

const std::string& GridCanvas::LineExtractType(LineState line_flags)
{
    return m_lineRenderers[apply_mask(line_flags, TYPE_MASK, TYPE_OFFSET)]->getType();
}

bool GridCanvas::isRectClear(const RectRange<unsigned int>& rect) const
{
    for(unsigned int row{rect.x0 + 1}; row <= rect.x1; ++row)
        for(unsigned int col{rect.y0}; col <= rect.y1; ++col)
            if(getLine(row, col, 0)) return false;
 
    for(unsigned int row{rect.x0}; row <= rect.x1; ++row)
        for(unsigned int col{rect.y0 + 1}; col <= rect.y1; ++col)
            if(getLine(row, col, 1)) return false;
 
    return true;
}

bool GridCanvas::isRectSealed(const RectRange<unsigned int>& rect) const
{
    if(rect.x0 == 0)
        for(unsigned int col{rect.y0}; col <= rect.y1; ++col)
            if(!getLine(0, col, 0)) return false;
 
    if(rect.x1 == m_GRID_MAP.x - 1)
        for(unsigned int col{rect.y0}; col <= rect.y1; ++col)
            if(!getLine(m_GRID_MAP.x, col, 0)) return false;
 
    if(rect.y0 == 0)
        for(unsigned int row{rect.x0}; row <= rect.x1; ++row)
            if(!getLine(row, 0, 1)) return false;
 
    if(rect.y1 == m_GRID_MAP.y - 1)
        for(unsigned int row{rect.x0}; row <= rect.x1; ++row)
            if(!getLine(row, m_GRID_MAP.y, 1)) return false;
 
    return true;
}

bool GridCanvas::setCellColor(std::size_t cell_id, sf::Color color)
{
    Cell* Pcell = getCell(cell_id);
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

bool GridCanvas::setCellTexture(std::size_t cell_id, const std::string& image_path)
{
    Cell* cell = getCell(cell_id);
    if(!cell) return false;

    if(cell->texture_path == image_path) return false;

    const RectRange<float> PIXELS_RECT = mapGridRectToCoords(cell->grid);
    const sf::Vector2f CELL_SIZE = PIXELS_RECT.distance();
    if(CELL_SIZE.x < 1.f || CELL_SIZE.y < 1.f) return false;

    auto texture_pair = __getTexture(image_path);
    if(!texture_pair.second) return false;

    cell->releaseTexture();
    cell->texture_path = texture_pair.first;
    cell->texture = texture_pair.second;

    _emplaceCellTexture(cell_id);
    return true;
}

int GridCanvas::getColorIndex(sf::Color color)
{
    if(color == DEFAULT_COLOR) return -1;

    for(std::size_t i{0}; i < MAX_COLOR_STORAGE; ++i)
    {
        if(!m_reservedLineColors[i].has_value())
        {
            m_reservedLineColors[i].emplace(color);
            return i;
        }

        if(m_reservedLineColors[i].has_value() && *m_reservedLineColors[i] == color)
            return i;
    }
    return -1;
}

int GridCanvas::getTypeIndex(const std::string& line_type) const
{
    const std::size_t current_size = m_lineRenderers.size();
    for(std::size_t i{0}; i < current_size; ++i)
    {
        if(m_lineRenderers[i]->getType() == line_type) return i;
    }
    return 0;
}

void GridCanvas::_pushEditHistory(EditAction&& action)
{
    m_history.erase(m_history.begin() + static_cast<std::ptrdiff_t>(m_historyCursor), m_history.end());
    m_history.push_back(std::move(action));
    if(m_history.size() > s_MAX_HISTORY) m_history.pop_front();
    m_historyCursor = m_history.size();
}

bool GridCanvas::_setLineValue(const RectRange<unsigned int>& rect, LineState val, std::uint16_t mask, bool replace)
{
    if(rect.x0 > m_GRID_MAP.x || rect.x1 > m_GRID_MAP.x || rect.y0 > m_GRID_MAP.y || rect.y1 > m_GRID_MAP.y) return false;
    if(rect.x0 != rect.x1 && rect.y0 != rect.y1) return false;
    if(rect.distance().x == 0 && rect.distance().y == 0) return false;

    EditAction action;
    const std::size_t AXIS = rect.x0 == rect.x1 ? 0u : 1u;
    const std::size_t FIXED_POINT = AXIS ? rect.y0 : rect.x0;
    const std::size_t START_POINT = AXIS ? rect.x0 : rect.y0;
    const std::size_t END_POINT = AXIS ? rect.x1 : rect.y1;
    const std::size_t START_LINE = std::min(START_POINT, END_POINT);
    const std::size_t END_LINE = std::max(START_POINT, END_POINT);

    for(std::size_t i{START_LINE}; i < END_LINE; ++i)
    {
        LineState& line = AXIS ? getLine(i, FIXED_POINT, AXIS) : getLine(FIXED_POINT, i, AXIS);
        const LineState PREV_VAL = line;

        if(replace)
        {
            line = val;
        }
        else
        {
            line &= mask;
            line |= val;
        }

        if(line == PREV_VAL) continue;

        if((line ^ PREV_VAL) & LINE_SET) m_updateCells = true;
        action.changes.push_back({static_cast<std::uint32_t>(&line - m_lines.data()), PREV_VAL, line});
    }

    /*
    * why only replace gets a cached state? cuz i'm an idiot
    */
    if(m_quickDrawEnable && replace && (val & LINE_SET)) m_pen.anchor = EditAnchor{rect, val};

    if(action.changes.empty()) return false;

    action.pen = m_pen.anchor;
    m_updateLines = true;
    _pushEditHistory(std::move(action));
    return true;
}

void GridCanvas::_updateLines()
{
    for(auto& line_renderer : m_lineRenderers)
    {
        if(line_renderer->m_requireLineUpdate)
        {
            line_renderer->m_requireLineUpdate = false;
            m_updateLines = true;
        }
    }

    if(!m_updateLines) return;
    m_updateLines = false;

    for(auto& line_renderer : m_lineRenderers) line_renderer->clear();

    for(std::size_t row{0}; row <= m_GRID_MAP.x; ++row)
    {
        unsigned int col{0};
        while(col < m_GRID_MAP.y)
        {
            if(LineState& line_flags = getLine(row, col, 0))
            {
                const std::size_t line_start{col};
                while(col < m_GRID_MAP.y && getLine(row, col, 0) == line_flags) ++col;

                const float thickness = LineExtractThickness(line_flags);
                const sf::Color color = LineExtractColor(line_flags);
                const std::string& line_type = LineExtractType(line_flags);
                const float run_length = static_cast<float>(col - line_start);

                const float x0 = m_deadZone + static_cast<float>(line_start) * m_cellMinSize.x;
                const float x1 = m_deadZone + static_cast<float>(col) * m_cellMinSize.x;
                const float y = m_deadZone + static_cast<float>(row) * m_cellMinSize.y;

                _appendLine(line_type, x0, x1, y, thickness, run_length, color, false);
            }
            else ++col;
        }
    }

    for(std::size_t col{0}; col <= m_GRID_MAP.y; ++col)
    {
        unsigned int row{0};
        while(row < m_GRID_MAP.x)
        {
            if(LineState& line_flags = getLine(row, col, 1))
            {
                const std::size_t line_start{row};
                while(row < m_GRID_MAP.x && getLine(row, col, 1) == line_flags) ++row;

                const float thickness = LineExtractThickness(line_flags);
                const sf::Color color = LineExtractColor(line_flags);
                const std::string& line_type = LineExtractType(line_flags);
                const float run_length = static_cast<float>(row - line_start);

                const float y0 = m_deadZone + static_cast<float>(line_start) * m_cellMinSize.y;
                const float y1 = m_deadZone + static_cast<float>(row) * m_cellMinSize.y;
                const float x = m_deadZone + static_cast<float>(col) * m_cellMinSize.x;

                _appendLine(line_type, y0, y1, x, thickness, run_length, color, true);
            }
            else ++row;
        }
    }
}

void GridCanvas::_updateCells()
{
    if(!m_updateCells) return;
    m_updateCells = false;

    struct Bounds
    {
        RectRange<unsigned int> rect;
        std::uint32_t count{0};
    };

    std::vector<std::uint32_t> dsu(m_MAX_CELL_COUNT);
    std::vector<Bounds> bounds(m_MAX_CELL_COUNT);
    std::deque<Cell> cached_cells = std::move(m_cells);
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
    for(std::size_t row{0}; row < m_GRID_MAP.x; ++row)
    {
        for(std::size_t col{0}; col < m_GRID_MAP.y; ++col)
        {
            const std::uint32_t idx = row * m_GRID_MAP.y + col;
            if(col + 1 < m_GRID_MAP.y && !getLine(row, col + 1, 1))
                dsu[find_dsu(idx + 1)] = find_dsu(idx);

            if(row + 1 < m_GRID_MAP.x && !getLine(row + 1, col, 0))
                dsu[find_dsu(idx + m_GRID_MAP.y)] = find_dsu(idx);
        }
    }

    // identify rect surface for each group
    for(unsigned int row{0}; row < m_GRID_MAP.x; ++row)
    {
        for(unsigned int col{0}; col < m_GRID_MAP.y; ++col)
        {
            Bounds& b = bounds[find_dsu(row * m_GRID_MAP.y + col)];
            if(b.count == 0)
            {
                b.rect.x0 = b.rect.x1 = row;
                b.rect.y0 = b.rect.y1 = col;
            }
            else
            {
                b.rect.x0 = std::min(b.rect.x0, row);
                b.rect.x1 = std::max(b.rect.x1, row);
                b.rect.y0 = std::min(b.rect.y0, col);
                b.rect.y1 = std::max(b.rect.y1, col);
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
        if(bound.count != (bound.rect.x1 - bound.rect.x0 + 1) * (bound.rect.y1 - bound.rect.y0 + 1)) continue;
        // internal line(s)
        if(!isRectClear(bound.rect)) continue;
        // not sealed (on GridCanvas edges)
        if(!isRectSealed(bound.rect)) continue;

        Cell& cell = m_cells.emplace_back(m_cells.size());

        cell.Proot = this;

        cell.grid.x0 = bound.rect.x0;
        cell.grid.x1 = bound.rect.x1;
        cell.grid.y0 = bound.rect.y0;
        cell.grid.y1 = bound.rect.y1;

        // pass old attributes to the same cell
        const std::uint64_t key = pack_bounds(bound.rect);
        auto it = hashed_cells.find(key);
        if(it != hashed_cells.end())
        {
            if(hovered_id == it->second) m_hoveredCellID = cell.id;
            if(clicked_id == it->second) m_clickedCellID = cell.id;
            if(selected_id == it->second) m_selectedCellID = cell.id;
            cached_cells[it->second].inheritTo(cell);
        }

        RectRange<float> rect;
        rect.x0 = m_deadZone + static_cast<float>(bound.rect.y0) * m_cellMinSize.x;
        rect.y0 = m_deadZone + static_cast<float>(bound.rect.x0) * m_cellMinSize.y;
        rect.x1 = m_deadZone + static_cast<float>(bound.rect.y1 + 1) * m_cellMinSize.x;
        rect.y1 = m_deadZone + static_cast<float>(bound.rect.x1 + 1) * m_cellMinSize.y;
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
    for(const Cell& cell : m_cells)
    {
        if(cell.texture) _emplaceCellTexture(cell.id);
    }
    ++m_access->generation;
}

void GridCanvas::_updateGrid()
{
    if(!m_updateGridLines || !checkFlag(m_flags, FLAGS::GRID)) return;
    m_renderedGridLines.clear();
    const sf::Color color = m_style.outline_color;
    m_renderedGridLines.resize((m_GRID_MAP.x + 1) * 2 + (m_GRID_MAP.y + 1) * 2);
    std::size_t offset{0};
    for(unsigned int r{0}; r <= m_GRID_MAP.x ; ++r)
    {
        const float y = m_deadZone + static_cast<float>(r) * m_cellMinSize.y;
        m_renderedGridLines[offset++] = sf::Vertex{{m_deadZone, y}, color};
        m_renderedGridLines[offset++] = sf::Vertex{{m_size.x - m_deadZone, y}, color};
    }
    for(unsigned int c{0}; c <= m_GRID_MAP.y; ++c)
    {
        const float x = m_deadZone + static_cast<float>(c) * m_cellMinSize.x;
        m_renderedGridLines[offset++] = sf::Vertex{{x, m_deadZone}, color};
        m_renderedGridLines[offset++] = sf::Vertex{{x, m_size.y - m_deadZone}, color};
    }
 
    m_updateGridLines = false;
}

void GridCanvas::_drawPenIndicator()
{
    if(!m_quickDrawEnable || !m_pen.anchor || !m_pen.show) return;

    if(m_pen.timer.getElapsedTime() >= s_PEN_DURATION) return;

    const float x = m_deadZone + static_cast<float>(m_pen.anchor->range.y1) * m_cellMinSize.x;
    const float y = m_deadZone + static_cast<float>(m_pen.anchor->range.x1) * m_cellMinSize.y;
    const float half = s_PEN_WIDTH / 2.f;

    const sf::Color pen_color = m_pen.color == DEFAULT_COLOR ? m_style.outline_color : m_pen.color;

    const sf::Vertex quad[6] =
    {
        sf::Vertex{{x - half, y - half}, pen_color},
        sf::Vertex{{x + half, y - half}, pen_color},
        sf::Vertex{{x + half, y + half}, pen_color},
        sf::Vertex{{x + half, y + half}, pen_color},
        sf::Vertex{{x - half, y + half}, pen_color},
        sf::Vertex{{x - half, y - half}, pen_color}
    };
    m_texture.draw(quad, 6, sf::PrimitiveType::Triangles);
}

void GridCanvas::_emplaceCellTexture(std::size_t cell_id)
{
    const Cell* cell = getCell(cell_id);
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

void GridCanvas::_releaseTexture(Cell& cell)
{
    if(cell.texture_path.empty()) return;

    auto it = m_textureCache.find(cell.texture_path);
    if(it != m_textureCache.end() && --it->second.ref_count == 0) m_textureCache.erase(it);
    cell.texture = nullptr;
    cell.texture_path.clear();
}

void GridCanvas::_appendLine(const std::string& line_type, float span_start_px, float span_end_px, float fixed_px,
                             float thickness, float run_length_units, sf::Color color, bool is_vertical)
{
    auto it = std::find_if(m_lineRenderers.begin(), m_lineRenderers.end(),
        [&line_type](const std::unique_ptr<LineRenderer>& element)
        {
            return element->getType() == line_type;
        });

    if(it != m_lineRenderers.end())
    {
        (*it)->appendLine(span_start_px, span_end_px, fixed_px, thickness, run_length_units, color, is_vertical);
    }
}

void GridCanvas::_refreshCellColor(std::size_t cell_id)
{
    const Cell* cell = getCell(cell_id);
    if(!cell) return;

    sf::Color color = cell->fill_color;
    if(cell_id == m_selectedCellID && checkFlag(m_flags, FLAGS::INDICATOR)) color = m_style.selected_cell_color;
    else if(cell_id == m_hoveredCellID) color = m_style.hovered_cell_color;

    setCellColor(cell_id, color);
}

void GridCanvas::_onCellHover(std::size_t hovered_cell_id)
{
    const std::size_t previous = m_hoveredCellID;
    m_hoveredCellID = hovered_cell_id;
    _refreshCellColor(previous);
    _refreshCellColor(hovered_cell_id);
}

void GridCanvas::_onCellSelect(std::size_t selected_cell_id)
{
    const std::size_t previous = m_selectedCellID;
    m_selectedCellID = selected_cell_id;
    _refreshCellColor(previous);
    _refreshCellColor(selected_cell_id);
}

bool GridCanvas::_undoLine()
{
    if(m_historyCursor == 0) return false;

    __applyAction(m_history[--m_historyCursor], true);
    m_pen.anchor = m_historyCursor ? m_history[m_historyCursor - 1].pen : std::nullopt;
    return true;
}

bool GridCanvas::_redoLine()
{
    if(m_historyCursor >= m_history.size()) return false;

    __applyAction(m_history[m_historyCursor], false);
    m_pen.anchor = m_history[m_historyCursor].pen;
    ++m_historyCursor;
    return true;
}

void GridCanvas::__applyAction(const EditAction& action, bool undo)
{
    for(const UnitEditDetails& change : action.changes)
    {
        const LineState TARGET = undo ? change.prev_val : change.new_val;
        LineState& line = m_lines[change.line_index];

        if((line ^ TARGET) & LINE_SET) m_updateCells = true;
        line = TARGET;
    }
    m_updateLines = true;
}

std::pair<std::string, const sf::Texture*> GridCanvas::__getTexture(const std::string& path)
{
    auto it = m_textureCache.find(path);
    if(it != m_textureCache.end())
    {
        ++it->second.ref_count;
        return {path, &it->second.texture};
    }

    sfml_widgets::Widget::ImageCache& image_cache = sfml_widgets::Widget::ImageCache::getInstance();
    std::optional<const sf::Image*> source = image_cache.getImage(path);
    if(!source) return {"", nullptr};
    
    // this is fine since local cache is checked first
    // so disk wouldn't be accessed if another cell requests the texture
    image_cache.decrementRefCounter(path);

    sf::Texture texture;
    if(!texture.loadFromImage(**source)) return {"", nullptr};
    
    texture.setSmooth(true);
    TexSource& entry = m_textureCache[path];
    entry.texture = std::move(texture);
    entry.ref_count = 1;
    return {path, &entry.texture};
}

std::optional<RectRange<unsigned int>> GridCanvas::__findNextAnchorPoint(sf::Vector2u point, sf::Vector2u pen_point, sf::Vector2i delta, LineState line_value) const
{
    const std::size_t AXIS = delta.x != 0 ? 1u : 0u;    // 0u->horizontal; 1u->vertical

    const auto out_of_board = [&](sf::Vector2u p) -> bool
    {
        return AXIS ? (delta.x > 0 ? p.x >= m_GRID_MAP.x : p.x == 0)
                    : (delta.y > 0 ? p.y >= m_GRID_MAP.y : p.y == 0);
    };

    const auto step_range = [&](sf::Vector2u p) -> RectRange<unsigned int>
    {
        return RectRange<unsigned int>{
            p.x, p.y,
            static_cast<unsigned int>(static_cast<int>(p.x) + delta.x),
            static_cast<unsigned int>(static_cast<int>(p.y) + delta.y)
        };
    };

    std::optional<RectRange<unsigned int>> one_unit_step;
    if(!out_of_board(pen_point)) one_unit_step = step_range(pen_point);
    if(!m_quickDrawSkipIdentical) return one_unit_step;

    while(true)
    {
        if(out_of_board(point)) return one_unit_step;

        const std::size_t ROW = AXIS ? (delta.x > 0 ? point.x : point.x - 1) : point.x;
        const std::size_t COL = AXIS ? point.y : (delta.y > 0 ? point.y : point.y - 1);
        const RectRange<unsigned int> step = step_range(point);

        if(getLine(ROW, COL, AXIS) != line_value) return step;
        point = {step.x1, step.y1};
    }
}

void GridCanvas::__clearType(int type_idx)
{
    const unsigned int REMOVED_IDX = static_cast<unsigned int>(type_idx);

    const auto re_assign = [REMOVED_IDX](LineState state) -> LineState
    {
        const unsigned int idx = apply_mask(state, TYPE_MASK, TYPE_OFFSET);
        if(idx == REMOVED_IDX) // reset to normal line
        {
            return static_cast<LineState>(state & TYPE_OFF);
        }
        if(idx > REMOVED_IDX) // decrement type index
        {
            return static_cast<LineState>((state & TYPE_OFF) |
                   apply_shift(static_cast<LineState>(idx - 1u), TYPE_OFFSET));
        }
        return state;
    };

    for(LineState& line : m_lines) line = re_assign(line);

    for(EditAction& action : m_history)
    {
        for(UnitEditDetails& change : action.changes)
        {
            change.prev_val = re_assign(change.prev_val);
            change.new_val = re_assign(change.new_val);
        }
        if(action.pen) action.pen->state = re_assign(action.pen->state);
    }
    if(m_pen.anchor) m_pen.anchor->state = re_assign(m_pen.anchor->state);
}

void GridCanvas::__drawPen()
{
    if(!m_quickDrawEnable) return;
    m_pen.timer.restart();
}
}