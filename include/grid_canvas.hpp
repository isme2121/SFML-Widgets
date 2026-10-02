#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP

namespace sfml_widgets
{
using LineState = std::uint16_t;

namespace
{
constexpr std::uint16_t GENERATE_MASK_U16(unsigned int first, unsigned int last)
{
    return static_cast<sfml_widgets::LineState>(((UINT16_MAX >> (15 - last)) & (UINT16_MAX << first)));
}
}

// reserved color for line to inherit canvas outline
inline constexpr sf::Color DEFAULT_COLOR{1, 1, 1, 1};
// max number of different line colors stored in canvas
inline constexpr std::size_t MAX_COLOR_STORAGE = 14;

inline constexpr std::size_t NULL_CELL = constants::KSTALE;

/*
* flags for all available states for a line in `Canvas`
* stored in uint16 value
*/
// LINE IS DRAWN WITH DEFAULT FLAGS (ALL ZERO)
inline constexpr sfml_widgets::LineState LINE_SET = 1u;

// 0 to 7 range for pixel thickness value
inline constexpr unsigned int THICKNESS_OFFSET = 1u;
inline constexpr unsigned int THICKNESS_LAST_BIT = 3u;
inline constexpr sfml_widgets::LineState THICKNESS_MASK = GENERATE_MASK_U16(THICKNESS_OFFSET, THICKNESS_LAST_BIT);
inline constexpr unsigned int THICKNESS_OFF = UINT16_MAX ^ THICKNESS_MASK;

// 0 to 15 (actually 0 to MAX_COLOR_STORAGE)
inline constexpr unsigned int COLORS_OFFSET = 4u;
inline constexpr unsigned int COLORS_LAST_BIT = 7u;
inline constexpr sfml_widgets::LineState COLORS_MASK = GENERATE_MASK_U16(COLORS_OFFSET, COLORS_LAST_BIT);
inline constexpr unsigned int COLORS_OFF = UINT16_MAX ^ COLORS_MASK;

// 0 to 3, available line types in `LineType` class
inline constexpr unsigned int TYPE_OFFSET = 8u;
inline constexpr unsigned int TYPE_LAST_BIT = 9u;
inline constexpr sfml_widgets::LineState TYPE_MASK = GENERATE_MASK_U16(TYPE_OFFSET, TYPE_LAST_BIT);
inline constexpr unsigned int TYPE_OFF = UINT16_MAX ^ TYPE_MASK;

// Different types of lines managed by `Canvas`
enum class LineType : LineState
{
    None = 0u,
    Open = 1u,
    Dotted = 2u
};

///////////////////////////////////////////////////////////////////
/// @brief Rectangle bounds
///
/// @tparam Num numeric type
///
///////////////////////////////////////////////////////////////////
template<typename Num>
struct RectRange
{
    static_assert(std::is_arithmetic_v<Num>, "RectRange template type is not numeric.");

    Num x0{0}, y0{0},
        x1{0}, y1{0};

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if point is inside the rectangle surface with bounds
    /// included
    ///
    /// @param point coord point of Num type
    ///
    ///////////////////////////////////////////////////////////////////
    constexpr bool inRange(sf::Vector2<Num> point) const
    {
        const Num minX = std::min(x0, x1);
        const Num maxX = std::max(x0, x1);
        const Num minY = std::min(y0, y1);
        const Num maxY = std::max(y0, y1);
        return (point.x >= minX) && (point.x <= maxX) && (point.y >= minY) && (point.y <= maxY);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns manhattan distance between point 0 and point 1
    /// of the rect
    ///
    ///////////////////////////////////////////////////////////////////
    constexpr sf::Vector2<Num> distance() const
    {
        return
        {
            x1 > x0 ? x1 - x0 : x0 - x1,
            y1 > y0 ? y1 - y0 : y0 - y1
        };
    }

    constexpr bool operator==(const RectRange<Num>& other)
    {
        return (x0 == other.x0) && (x1 == other.x1) &&
               (y0 == other.y0) && (y1 == other.y1);
    }

    constexpr bool operator!=(const RectRange<Num>& other)
    {
        return !(*this == other);
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Cell attributes object from `Canvas`
///
/// A cell is automatically constructed from a rectangular shape made
/// by canvas lines.
///
///////////////////////////////////////////////////////////////////
class GridCell
{
    friend class Canvas;

public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs a cell on canvas
    ///
    ///////////////////////////////////////////////////////////////////
    GridCell(Canvas& root_canvas, std::size_t id);

    GridCell(GridCell&& other) = default;
    GridCell& operator=(GridCell&& other) = default;
    GridCell(const GridCell& other) = delete;
    GridCell& operator=(const GridCell& other) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the cell's fill color
    ///
    /// @param color new fill color 
    /// @return true if process is complete
    ///
    ///////////////////////////////////////////////////////////////////
    bool setCellColor(sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief  Updates the cell's outline thickness
    ///
    /// @param _s outline side 
    /// @param thickness thickness level, from 0 to 7
    /// @return true if process was successful
    ///
    ///////////////////////////////////////////////////////////////////
    bool setOutlineThickness(DIRECTION _s, LineState thickness = 1u);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the cell's outline color 
    ///
    /// @param _s outline side 
    /// @param color new outline color, default sets it to canvas outline color
    /// @return true if process was successful
    ///
    ///////////////////////////////////////////////////////////////////
    bool setOutlineColor(DIRECTION _s, sf::Color color = DEFAULT_COLOR);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the cell's outline type
    ///
    /// @param _s outline side 
    /// @param line_type new outline type
    /// @return true if process was successful
    ///
    ///////////////////////////////////////////////////////////////////
    bool setOutlineType(DIRECTION _s, LineType line_type = LineType::None);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if unit point collides with cell rect
    ///
    /// @param grid_coords canvas coords in unit 
    /// @return true if point is inside rect
    ///
    ///////////////////////////////////////////////////////////////////
    bool inRange(sf::Vector2u grid_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if pixel point collides with cell rect
    ///
    /// @param pixel_coords canvas coords in pixels 
    /// @return true if point is inside rect
    ///
    ///////////////////////////////////////////////////////////////////
    bool inRange(sf::Vector2f pixel_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the cell's ID
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getID() const;

    bool operator==(const GridCell& other) const;
    bool operator==(std::size_t id) const;

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Returned the line coordinates for canvas to identify
    ///
    /// @param _s outline side 
    /// @return rect with start and end unit coords for line
    ///
    ///////////////////////////////////////////////////////////////////
    RectRange<unsigned int> getLineRange(DIRECTION _s) const;

    Canvas* Proot;
    std::size_t id{0};
    RectRange<unsigned int> grid{};     //!< boundaries included in range
    sf::Color fill_color{sf::Color::Transparent};
};

class Canvas : public Widget
{
    struct Access;
    friend class GridCell;

public:
    class cell_ptr
    {
    public:
        cell_ptr() = default;

        explicit operator bool() const;
        GridCell* operator->() const;
        GridCell& operator*() const;

    private:
        friend class Canvas;
        cell_ptr(std::weak_ptr<Access> access, std::size_t index, std::uint32_t generation);

        GridCell* resolve() const;

        std::weak_ptr<Access> m_access;
        std::size_t m_index{0};
        std::uint32_t m_generation{0};
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs canvas on window
    ///
    /// @param window root window 
    /// @param position position relative to window
    /// @param size canvas size
    /// @param grid grid map, must be at least {1, 1}
    ///
    ///////////////////////////////////////////////////////////////////
    Canvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs canvas on root frame
    ///
    /// @param root parent frame widget 
    /// @param position position relative to frame
    /// @param size canvas size
    /// @param grid grid units map, must be at least {1, 1}
    ///
    ///////////////////////////////////////////////////////////////////
    Canvas(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid);

    ~Canvas() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws a hortizontal/vertical line
    ///
    /// @param line_range start and end point for the line in units 
    /// @param thickness line initial thickness, from 0 to 7
    /// @param color line color, default is canvas outline color
    /// @param Line_type line type
    /// @return true if canvas successfully draws the line, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool drawLine(RectRange<unsigned int> line_range, LineState thickness = 1u, sf::Color color = DEFAULT_COLOR, LineType Line_type = LineType::None);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a line range from canvas, range must be a horizontal/vertical
    /// line
    ///
    /// @param line_range start and end points in units 
    /// @return true if the line is successfully removed, otherwise false,
    /// returns true even if there is no line in that range.
    ///
    ///////////////////////////////////////////////////////////////////
    bool removeLine(RectRange<unsigned int> line_range);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears canvas from all of it's contents
    ///
    ///////////////////////////////////////////////////////////////////
    void clearCanvas();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the line's thickness value, creates the line if
    /// it doesn't exist
    /// 
    /// @param line_range start and end points in units 
    /// @param thickness value from 0 to 7
    /// @return true if line is updated, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLineThickness(RectRange<unsigned int> line_range, LineState thickness = 1u);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the line's color, creates the line if it doesn't exist
    ///
    /// If color is `DEFAULT_COLOR` or max color count is reached then
    /// the current canvas outline color is assigned instead.
    ///
    /// @param line_range start and end points in units 
    /// @param color new color value
    /// @return true if line is updated, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLineColor(RectRange<unsigned int> line_range, sf::Color color = DEFAULT_COLOR);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the line's type, creates the line if it doesn't exist
    ///
    /// @param line_range start and end points in units 
    /// @param Line_type enum type
    /// @return true if line is updated, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLineType(RectRange<unsigned int> line_range, LineType Line_type = LineType::None);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a reference to the cell in parameter unit coords
    ///
    /// @param grid_coords coords in units 
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr at(sf::Vector2u grid_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a reference to the cell in parameter pixel coords
    /// relative to canvas
    ///
    /// @param pixel_coords coords in units 
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr at(sf::Vector2f pixel_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a reference to the cell in parameter id
    ///
    /// @param cell_id cell id
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    /// @note IDs start from 0 and increment for each new cell going from
    /// left/top unit to bottom/right
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr at(std::size_t cell_id) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current hovered cell
    ///
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr getHoveredCell() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current selected cell
    /// 
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr getSelectedCell() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current cells count
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getCellsCount() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the max number of cells possible (units.x * units.y)
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getMaxCellsCount() const;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the equivalent pixel coordinates relative
    /// to canvas' board, accurate for local surface mapping
    ///
    /// @param mouse_position root window provided mouse coordinates  
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2i getLocalPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to canvas' view,
    /// accurate for zoomed/panned canvas
    ///
    /// @param mouse_position root window provided mouse coordinates
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f getViewPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to canvas' view,
    /// accurate for zoomed/panned canvas
    ///
    /// @param local_position coordinates provided from `getLocalPixel`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f mapLocalToView(sf::Vector2i local_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns unit coordinates relative to parameter pixel coordinates
    ///
    /// @param mouse_position root window provided mouse coordinates 
    /// @return nullopt if input coords are out of canvas bounds, otherwise
    /// unit coordinates
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<sf::Vector2u> mapCoordsToGrid(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to parameter unit coordinates
    ///
    /// @param grid_coordinates coordintates in units 
    /// @return nullopt if input coords are out of canvas bounds, otherwise
    /// pixel coordinates
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<sf::Vector2f> mapGridToCoords(sf::Vector2u grid_coordinates) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to parameter unit coordinates
    ///
    /// @param grid_rect rect range of unit coordinates
    /// @return nullopt if input coords are out of canvas bounds, otherwise
    /// pixel coordinates
    ///
    ///////////////////////////////////////////////////////////////////
    RectRange<float> mapGridRectToCoords(RectRange<unsigned int> grid_rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns unit coordinates relative to parameter pixel coordinates
    ///
    /// Returns range includes any unit that sits inside the provided input.
    ///
    /// @param coords_rect rect range of pixel coordinates
    /// @return nullopt if input coords are out of canvas bounds, otherwise
    /// unit coordinates
    ///
    ///////////////////////////////////////////////////////////////////
    RectRange<unsigned int> mapCoordsRectToGrid(RectRange<float> coords_rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the extra surface out of units range, deadzone is 0
    /// by default
    ///
    /// @param surface ratio from `0` to `0.1f`
    /// 
    ///////////////////////////////////////////////////////////////////
    void setDeadZone(float surface);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(CanvasStyle s_canvas);

    ///////////////////////////////////////////////////////////////////
    /// @brief Pulls the widget's style from it's linked theme object
    /// 
    ///////////////////////////////////////////////////////////////////
    void fetchTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief R/W Reference to the current widget style
    ///
    /// @note if the widget is linked to a `WidgetTheme` source, any modifications
    /// to the returned reference do not affect the theme's style and are overridden
    /// by the theme if any updates are pushed to the widget.
    ///
    ///////////////////////////////////////////////////////////////////
    CanvasStyle& style();

    ///////////////////////////////////////////////////////////////////
    /// @brief Forces an update to canvas visual style, it also updates
    /// grid lines if enabled
    ///
    ///////////////////////////////////////////////////////////////////
    void poke() override;

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates hovered cell
    ///
    ///////////////////////////////////////////////////////////////////
    bool preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles selecting cell, zooming, and padding if enabled
    /// 
    ///////////////////////////////////////////////////////////////////
    bool postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Quick draw lines using arrow keys
    ///
    ///////////////////////////////////////////////////////////////////
    bool postProcessKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws canvas to root
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Extra configurations handling
    /// 
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates canvas position
    /// 
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates canvas size
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates canvas rotation
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates canvas visuals by state
    ///
    ///////////////////////////////////////////////////////////////////
    void updateState() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates assigned theme if any
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the contents of canvas board, this includes
    /// the grid, lines, and cells
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateBoard();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the minimum cell size in canvas which matches a
    /// single unit's size
    ///
    ///////////////////////////////////////////////////////////////////
    void updateCellSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief zoom if enabled
    ///
    ///////////////////////////////////////////////////////////////////
    void zoom(float delta, sf::Vector2i local_mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief pan if enabled
    ///
    ///////////////////////////////////////////////////////////////////
    void pan(sf::Vector2i local_mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Blocks view from going out of bounds while zooming/panning
    ///
    ///////////////////////////////////////////////////////////////////
    void limitView(sf::View& v);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates a line's value
    ///
    /// @param rect horizontal/vertical line start to end points 
    /// @param val new line value
    /// @return true if assignment is successful, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLine(RectRange<unsigned int> rect, LineState val);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates a handle of a cell
    /// @param index cell's id
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr makeHandle(std::size_t index) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns single unit line value
    ///
    /// @param row row unit coord 
    /// @param col column unit coord
    /// @param axis 0->horizontal; 1->vertical
    /// @return modifiable reference to line value
    /// @warning memory access violation if coords are out of bounds, remain
    /// controlled
    /// 
    ///////////////////////////////////////////////////////////////////
    LineState& getLine(std::size_t row, std::size_t col, std::size_t axis);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns single unit line value
    ///
    /// @param row row unit coord 
    /// @param col column unit coord
    /// @param axis 0->horizontal; 1->vertical
    /// @return read-only reference to line value
    /// @warning memory access violation if coords are out of bounds, remain
    /// controlled
    /// 
    ///////////////////////////////////////////////////////////////////
    const LineState& getLine(std::size_t row, std::size_t col, std::size_t axis) const; //!< CONST BEST FRIEND

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to existing cell
    ///
    /// @param index cell's id 
    /// @return cell's address if found, otherwise nullptr
    ///
    ///////////////////////////////////////////////////////////////////
    GridCell* getCell(std::size_t index) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns cell's id that occupies grid coords unit
    ///
    /// @param grid_coords coordinates in units 
    /// @return cell id if found, otherwise `NULL_CELL`
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getCellIndex(sf::Vector2u grid_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns cell's id that occupies pixel coord
    ///
    /// @param pixel_coords coordinates in pixels relative to canvas view 
    /// @return cell id if found, otherwise `NULL_CELL`
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getCellIndex(sf::Vector2f pixel_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns current thickness value from line state
    ///
    ///////////////////////////////////////////////////////////////////
    float LineExtractThickness(LineState line_flags);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns current color determined from line state 
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Color LineExtractColor(LineState line_flags);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns current line type from line state
    ///
    ///////////////////////////////////////////////////////////////////
    LineType LineExtractType(LineState line_flags);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if rect surface in canvas has any lines insides,
    /// rect bounds do not count, this is done to check for a valid cell
    /// surface
    ///
    /// @param rect range in units 
    /// 
    ///////////////////////////////////////////////////////////////////
    bool isRectClear(const RectRange<unsigned int>& rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if rect bounds are all constructed lines if rect
    /// is touching canvas bounds
    ///
    /// @param rect range in units 
    ///
    ///////////////////////////////////////////////////////////////////
    bool isRectSealed(const RectRange<unsigned int>& rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief The actual line value setter
    ///
    /// @param rect start to end unit points for line 
    /// @param val assigned value
    /// @param mask applied if replace is false, any bit=0 in mask resets line state bit counter part
    /// @param replace param val replaces old val if true, otherwise is added to it after applying mask
    /// @return true if line is updated, otherwise false upon fail
    ///
    ///////////////////////////////////////////////////////////////////
    bool _setLineValue(const RectRange<unsigned int>& rect, LineState val, std::uint16_t mask = UINT16_MAX, bool replace = true);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates a cell's visual fill color
    ///
    /// @param cell_id cell id 
    /// @param color fill color
    /// @return true if cell exists, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool _setCellColor(std::size_t cell_id, sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns color index from colors storage
    ///
    /// @param color searched color 
    /// @return color index, or -1 if color is not found
    ///
    ///////////////////////////////////////////////////////////////////
    int _getColorIndex(sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates cell color based on it's current state
    ///
    ///////////////////////////////////////////////////////////////////
    void _refreshCellColor(std::size_t cell_id);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns cell as hovered cell
    ///
    ///////////////////////////////////////////////////////////////////
    void _onCellHover(std::size_t hovered_cell_id);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns cell as selected cell
    /// 
    ///////////////////////////////////////////////////////////////////
    void _onCellSelect(std::size_t selected_cell_id);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates grid lines color to match canvas outline
    ///
    ///////////////////////////////////////////////////////////////////
    void _refreshGridColor();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates lines that use the default color
    ///
    ///////////////////////////////////////////////////////////////////
    void _refreshLinesColor();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates line values visually
    ///
    ///////////////////////////////////////////////////////////////////
    void _updateLines();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates cells generation, constructing/deleting cells
    /// in the process
    /// 
    ///////////////////////////////////////////////////////////////////
    void _updateCells();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws grid if enabled
    /// 
    ///////////////////////////////////////////////////////////////////
    void _updateGrid();

    static constexpr float s_MAX_ZOOM_LEVEL = 5.f;
    static constexpr float s_MIN_ZOOM_LEVEL = 1.f;
    static constexpr float s_MAX_DEADZONE_RATIO = 0.1f;

    struct Access
    {
        Canvas* canvas{nullptr};
        std::uint32_t generation{0};
    };

    sf::RenderTexture m_texture;
    sf::Sprite m_sprite;
    std::vector<LineState> m_lines;
    mutable std::vector<GridCell> m_cells;


    std::shared_ptr<Access> m_access;

    // rendered data
    sf::VertexArray m_renderedlines{sf::PrimitiveType::Triangles};
    sf::VertexArray m_renderedCells{sf::PrimitiveType::Triangles};
    sf::VertexArray m_renderedGridLines{sf::PrimitiveType::Lines};
    std::array<std::optional<sf::Color>, MAX_COLOR_STORAGE> m_sharedColors{};

    
    // properties
    sf::Vector2f m_cellMinSize{0.f, 0.f};
    sf::Vector2i m_memMousePos{0, 0};
    CanvasStyle m_style = Theme::canvas_style::CANVAS_DARK_STYLE;
    float m_zoomLevel{s_MAX_ZOOM_LEVEL};
    float m_deadZone{0.f};

    // constants
    const sf::Vector2u m_grid;
    const std::size_t m_MAX_CELL_COUNT;

    // references
    std::size_t m_hoveredCellID{NULL_CELL};
    std::size_t m_clickedCellID{NULL_CELL};
    std::size_t m_selectedCellID{NULL_CELL};

    // update
    bool m_updateLines{false};
    bool m_updateCells{false};
    bool m_updateGridLines{false};

    Canvas(Canvas&& other) = delete;
    Canvas& operator=(Canvas&& other) = delete;
    Canvas(const Canvas& other) = delete;
    Canvas& operator=(const Canvas& other) = delete;
};

template <>
struct WidgetStyle<Canvas>
{
    using StyleType = CanvasStyle;
 
    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_canvasTheme;
    }
 
    static void set(WidgetTheme& theme, StyleType s_canvas)
    {
        theme.m_canvasTheme = s_canvas;
    }
 
    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_canvasToken;
    }
};

///////////////////////////////////////////////////////////////////
/// @brief Canvas with grid for a fixed number of cells
///
/// @see `Widget`, `Cell`, `CellOutlines`
///
///////////////////////////////////////////////////////////////////
class GridCanvas : public Widget
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the canvas with `sf::RenderWindow`
    ///
    /// @param window root window
    /// @param position canvas position
    /// @param size canvas size
    /// @param max_cell_count constant maximum possible number of cells in canvas
    /// @param dead_zone initial passive surface between boundaries and cells
    /// @param fill_initially choose to construct all cells initially
    ///
    ///////////////////////////////////////////////////////////////////
    GridCanvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, std::size_t max_cell_count, float dead_zone = 0.f, bool fill_initially = false);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the canvas on a `Frame` object
    ///
    /// @param Frame root frame
    /// @param position canvas position
    /// @param size canvas size
    /// @param max_cell_count constant maximum possible number of cells in canvas
    /// @param dead_zone initial passive surface between boundaries and cells
    /// @param fill_initially choose to construct all cells initially, or leave it empty for now
    ///
    ///////////////////////////////////////////////////////////////////
    GridCanvas(Frame& root, sf::Vector2f position, sf::Vector2u size, std::size_t max_cell_count, float dead_zone = 0.f, bool fill_initially = false);

    GridCanvas(GridCanvas&& other) = delete;
    GridCanvas& operator=(GridCanvas&& other) = delete;

    GridCanvas(const GridCanvas& other) = delete;
    GridCanvas& operator=(const GridCanvas& other) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a read only pointer to the cell placed in specified 
    /// grid coordinates
    ///
    /// @return const pointer to cell, or nullptr if not found
    ///
    /// @see `fill`, `insert`, `remove`, `isTaken`
    ///
    ///////////////////////////////////////////////////////////////////
    Cell* at(unsigned int row, unsigned int col);

    ///////////////////////////////////////////////////////////////////
    /// @brief Fills all empty spots in canvas with cells, any existing cells
    /// remain unchanged
    ///
    /// @see `insert`, `remove`
    ///
    ///////////////////////////////////////////////////////////////////
    void fill();

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates a cell at specified grid coordinates
    ///
    /// If a cell is already placed in those coordinates then it fails
    /// and returns false, otherwise returns true.
    ///
    /// @param cell_coordinates row x column coordinates within canvas
    ///
    /// @see `fill`, `remove`
    ///
    ///////////////////////////////////////////////////////////////////
    bool insert(sf::Vector2u cell_coordinates);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes an exiting cell based on input grid coordinates,
    /// does nothing if no cell exists in selected coordinates
    ///
    /// @param cell_coordinates row x column coordinates within canvas
    ///
    /// @see `fill`, `insert`
    ///
    ///////////////////////////////////////////////////////////////////
    void remove(sf::Vector2u cell_coordinates);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the state of the cells that don't verify the condition
    ///
    /// Takes a filter function which verifies all cells on a certain condition,
    /// if the condition function returns false then the cell is set to `BLOCKED`
    /// state, otherwise clears the `BLOCKED` flag from the cell.
    ///
    /// @param condition function that takes a cell reference and returns bool
    ///
    /// @see `at`
    ///
    ///////////////////////////////////////////////////////////////////
    void setActiveCells(std::function<bool(Cell&)> condition);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a non functional space between cells and canvas' bounds
    ///
    /// Dead zone is the distance that separates the cells zone with the
    /// outline of the canvas widget, can be set between 0 and 10% of the
    /// cell's width or height.
    ///
    /// @param surface float distance, must be in range of 0 to 10% of canvas
    /// width or height, otherwise fails to update.
    ///
    ///////////////////////////////////////////////////////////////////
    void setDeadZone(float surface);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets each existing cell's theme to parameter color
    ///
    /// @see `setTheme`
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellStyle(CellColor new_color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if input grid coordinates are occupied by an existing
    /// cell, returns true if condition is verified
    ///
    /// @param cell_coordinates row x column coordinates in canvas
    ///
    ///////////////////////////////////////////////////////////////////
    bool isTaken(sf::Vector2u cell_coordinates) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates in canvas surface
    ///
    /// Returned coordinates do not consider canvas' zoom or pan levels
    ///
    /// @see `getViewPixel`, `mapLocalToView`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2i getLocalPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns local coordinates accurate with zoom and pan levels
    ///
    /// @see `getLocalPixel`, `mapLocalToView`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f getViewPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Converts local coordinates to view accurate coordinates
    ///
    /// Local coordinates from `getLocalPixel` can be passed to get
    /// more results if canvas is currently zoomed or panned.
    ///
    /// @see `getLocalPixel`, `getViewPixel`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f mapLocalToView(sf::Vector2i local_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns row and column coordinates of a cell based on window
    /// mouse position
    ///
    /// Takes mouse coordinates relative to the window and estimates the
    /// row x column cell coordinates, returns nullopt if the coordinates
    /// are out of cell zone's range.
    /// 
    ///////////////////////////////////////////////////////////////////
    std::optional<sf::Vector2u> mapCoordinates(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to selected cell in canvas, or null if no
    /// cell is currently selected
    ///
    /// A cell can be selected by just pressing on it then releasing.
    ///
    /// @see `getClickedCellPtr`
    ///
    ///////////////////////////////////////////////////////////////////
    Cell* getSelectedCellPtr() const; 

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to last clicked cell, or null if no
    /// cell was clicked recently
    ///
    /// Can be used in a mouse event callback where clicked cell may be
    /// different from selected cell.
    ///
    /// @see `getSelectedCellPtr`
    ///
    ///////////////////////////////////////////////////////////////////
    Cell* getClickedCellPtr() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns currently existing cells in canvas
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getCellsCount() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle s_gridCanvas);

    ///////////////////////////////////////////////////////////////////
    /// @brief Pulls the widget's style from it's linked theme object
    /// 
    ///////////////////////////////////////////////////////////////////
    void fetchTheme() override;

    TextureStyle& style();

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Processes events for cells
    ///
    ///////////////////////////////////////////////////////////////////
    bool preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Canvas related events, such as selecting, panning, and zooming
    ///
    ///////////////////////////////////////////////////////////////////
    bool postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws canvas on the screen
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Used to update child widgets for container type Widget or
    /// add specific rules after updating flags, called in `configure`
    ///
    /// @param mask bits passed in `configure`
    ///
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the widget,
    /// called in `setWidgetPosition`
    ///
    /// @see `setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the widget,
    /// called in `setWidgetPosition`
    ///
    /// @see `setWidgetSize`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief matches the current angle value with the rest of the canvas
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates widget colors to match current state alongside shaders
    ///
    /// Best called within `Draw` override before drawing, it is also
    /// the optimal method for updating available shaders.
    ///
    /// The default tested states are: `BLOCKED`, `TRANSPARENT`, `ACTIVE`
    /// `HOVERED` respectively.
    ///
    ///////////////////////////////////////////////////////////////////
    void updateState() override; 

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs shared theme with widget
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of a cell in canvas
    ///
    /// This method is required after modifying `canvas' size`, `canvas'
    /// dead zone's surface`, or `canvas' grid mapping`.
    ///
    ///////////////////////////////////////////////////////////////////
    void updateCellSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws lines of grid within canvas view, activates by setting
    /// `FLAGS::GRID` flag within `configure`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawGrid();

    ///////////////////////////////////////////////////////////////////
    /// @brief Class method for zooming within canvas
    ///
    ///////////////////////////////////////////////////////////////////
    void zoom(float delta, sf::Vector2i local_mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Class method for panning the view within canvas
    ///
    ///////////////////////////////////////////////////////////////////
    void pan(sf::Vector2i local_mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Used to prevent view from exceeding canvas boundaries
    ///
    ///////////////////////////////////////////////////////////////////
    void limitView(sf::View& v);

    static constexpr float s_MAX_ZOOM_LEVEL = 5.f;                      //!< Default zoom level for showing full view
    static constexpr float s_MIN_ZOOM_LEVEL = 1.f;                      //!< Smallest view possible while zooming
    static constexpr float s_MAX_DEADZONE_RATIO = 0.1f;                 //!< Max surface occupation percentage for deadzone
    
    ///////////////////////////////////////////////////////////////////
    /// Member variables
    ///////////////////////////////////////////////////////////////////

    sf::RenderTexture m_drawTexture;                                    //!< Texture for drawing
    sf::Sprite m_drawSprite;                                            //!< Texture's sprite   
    Cell* m_selectedCell = nullptr;                                     //!< Points to currently selected cell
    Cell* m_clickedCell = nullptr;                                      //!< Points to currently clicked cell
    std::vector<Cell> m_cellMap;                                        //!< Cells memory space
    sf::Vector2i m_memMousePos = {0, 0};                                //!< Memory state for last clicked pixel inside canvas for panning
    const sf::Vector2u m_gridMapping;                                   //!< rows, columns map for cells
    const std::size_t MAX_CELL_COUNT;                                   //!< Maximum possible number of cells in canvas object
    sf::Vector2f m_cellSize;                                            //!< Shared size within all cells
    float m_zoomLevel = s_MAX_ZOOM_LEVEL;                               //!< Current zoom value, limited between `s_MAX_ZOOM_LEVEL` and `s_MIN_ZOOM_LEVEL`
    float m_deadZone = 0.f;                                             //!< Passive surface between boundaries and cells
    TextureStyle m_style = Theme::texture_style::STYLE_STALE_VIOLET;    //!< Canvas's default style

    friend class Cell;
};

///////////////////////////////////////////////////////////////////
/// @brief Separated outline for `Cell` object
///
/// @see `Cell`, `GridCanvas`
///
///////////////////////////////////////////////////////////////////
class CellOutline
{
public:

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs within `Cell` object
    ///
    /// @param direction side of the cell
    /// @param thickness outline thickness
    /// @param cell_position root cell's pixel position within canvas
    /// @param cell_size provided root cell's size
    /// @param color outline color
    ///
    ///////////////////////////////////////////////////////////////////
    CellOutline(DIRECTION direction, float thickness, sf::Vector2f cell_position, sf::Vector2f cell_size, sf::Color color = sf::Color::Black);

    CellOutline(CellOutline&& other) noexcept;
    CellOutline& operator=(CellOutline&& other) noexcept;

    CellOutline(const CellOutline& other) = delete;
    CellOutline& operator=(const CellOutline& other) = delete;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates outline thickness
    ///
    ///////////////////////////////////////////////////////////////////
    void setThickness(float thickness, DIRECTION direction);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates outline color
    ///
    ///////////////////////////////////////////////////////////////////
    void setColor(sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates outline position based on root cell position
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellPosition(sf::Vector2f cell_position, DIRECTION direction);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates length of the outline based on root cell size
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellSize(sf::Vector2f cell_size, DIRECTION direction);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a read only pointer to vertices used to draw the outline
    /// within root cell
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::Vertex* getVertices() const;

private:

    ///////////////////////////////////////////////////////////////////
    /// @brief Called after updating any member variable for drawing
    ///
    ///////////////////////////////////////////////////////////////////
    void updateOutline(DIRECTION direction);

    ///////////////////////////////////////////////////////////////////
    /// Member variables
    ///////////////////////////////////////////////////////////////////

    std::array<sf::Vertex, 6> m_vertices;           //!< 2 triangles
    sf::Vector2f m_cellPosition, m_cellSize;        //!< Root cell's information
    float m_thickness;                              //!< Outline thickness with pixels
    sf::Color m_color;                              //!< Outline color
};

///////////////////////////////////////////////////////////////////
/// @brief Cell object for `GridCanvas`
///
/// @see `GridCanvas`, `CellOutlines`
///
///////////////////////////////////////////////////////////////////
class Cell : private sf::RectangleShape
{
public:

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs with master `GridCanvas`
    ///
    /// @param root reference to canvas
    /// @param coordinates row, column coordinates for cell
    /// @param cell_size cell size provided by root
    /// @param dead_zone root canvas dead zone value
    ///
    ///////////////////////////////////////////////////////////////////
    Cell(GridCanvas& root, sf::Vector2u coordinates, sf::Vector2f cell_size, float dead_zone);

    Cell(Cell&& other) noexcept;
    Cell& operator=(Cell&& other) noexcept;

    Cell(const Cell& other) = delete;
    Cell& operator=(const Cell& other) = delete;

    ~Cell();

    bool operator==(const sf::Vector2u other_coordinates) const;
    bool operator!=(const sf::Vector2u other_coordinates) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the state of the cell with parameter flag
    ///
    /// Can toggle all modifiable flags except `HIDDEN` which is managed
    /// by canvas.
    ///
    /// @see `MODIFIABLE`
    ///
    ///////////////////////////////////////////////////////////////////
    void configure(FLAGS flag);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates background color for cell
    ///
    /// @see `setCellOutlineColor`, `setCellOutlineThickness`
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellColor(sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the outline color of the specified side
    ///
    /// @see `setCellColor`, `setCellOutlineThickness`
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellOutlineColor(DIRECTION direction, sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the outline thickness of the specified side
    ///
    /// @see `setCellColor`, `setCellOutlineColor`
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellOutlineThickness(DIRECTION direction, float thickness);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates background texture for cell
    ///
    /// Loads an image to assign with the cell's texture, and loads the
    /// default shader if successful.
    ///
    /// @param image_path string path to the background image
    ///
    /// @return true is process is a success, otherwise false
    ///
    /// @see `clearImage`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setImage(const std::string& image_path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears background image if set
    ///
    /// Clears texture data if any and also removes the shader reference
    ///
    /// @see `setImage`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearImage();

    ///////////////////////////////////////////////////////////////////
    /// @brief Loads the shader for rendering texture colors properly
    ///
    /// @see `resetShader`
    ///
    ///////////////////////////////////////////////////////////////////
    void loadShader(const std::string_view& shader_path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates position based on canvas view
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f getCellPosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns coordinates position in canvas
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2u getCellCoordinates() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks the state of input flag(s) within cell
    ///
    ///////////////////////////////////////////////////////////////////
    bool isFlag(FLAGS flag) const;
    
private:

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws cell on the canvas
    ///
    ///////////////////////////////////////////////////////////////////
    void drawCell();
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the cell
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellSize(sf::Vector2f size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the current coordinates position of the cell
    ///
    ///////////////////////////////////////////////////////////////////
    void setCellPosition(sf::Vector2u coordinates, float dead_zone);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates `FLAGS::HOVERED` & `FLAGS::TRANSPARENT` flags
    /// based on parameters, managed by root canvas
    ///
    ///////////////////////////////////////////////////////////////////
    bool updateState(sf::Vector2f local_mouse_position, bool auto_hide, bool canvas_hovered);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates color theme based on state
    ///
    ///////////////////////////////////////////////////////////////////
    void updateCell();

    ///////////////////////////////////////////////////////////////////
    /// @brief Refreshes image size while resizing cell
    ///
    ///////////////////////////////////////////////////////////////////
    void updateImageSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by root canvas for moving
    ///
    ///////////////////////////////////////////////////////////////////
    void updateDrawer(sf::RenderTexture& new_texture);

    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by root canvas for moving
    ///
    ///////////////////////////////////////////////////////////////////
    void updateCanvasStyle(TextureStyle* stylePtr);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by root canvas for moving
    ///
    ///////////////////////////////////////////////////////////////////
    void updateShader(sf::Color bg_color, float transparent_level = 0.f);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if cell is hovered based on mouse_position
    ///
    ///////////////////////////////////////////////////////////////////
    bool isHovered(sf::Vector2f local_mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// Member variables
    ///////////////////////////////////////////////////////////////////

public:
    sf::RenderTexture* m_rootDrawerPtr;                 //!< Root canvas texture for drawing
    TextureStyle* m_rootStylePtr;                       //!< Inherited state colors from root canvas
    std::unique_ptr<sf::Shader> m_imageShaderPtr;       //!< Shader for image
    std::unique_ptr<sf::Texture> m_bgTexturePtr;        //!< Bg image holder for cell
    std::string m_bgImagePath;                          //!< Member path for reloading image
    sf::Vector2u m_coordinates;                         //!< Row, column coordinates
    CellColor m_style = Theme::LIGHT_CELL;              //!< Unique cell colors in default state
    float m_deadZone;                                   //!< Root canvas' dead zone distance value
    CellOutline m_top, m_bottom, m_left, m_right;       //!< Separated outlines
    mutable FLAGS m_flags = FLAGS::MODIFIED;            //!< State flags

    friend class GridCanvas;

};

template <>
struct WidgetStyle<GridCanvas>
{
    using StyleType = TextureStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_gridCanvasTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_grid_canvas)
    {
        theme.m_gridCanvasTheme = s_grid_canvas;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_gridCanvasToken;
    }
};
}
#endif