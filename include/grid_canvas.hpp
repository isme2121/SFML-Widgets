#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP

namespace sfml_widgets
{
inline constexpr sf::Color DEFAULT_COLOR{1, 1, 1, 1};
inline constexpr std::size_t MAX_COLOR_STORAGE = 14;
using LineState = std::uint16_t;

namespace
{
constexpr std::uint16_t GENERATE_MASK_U16(unsigned int first, unsigned int last)
{
    return static_cast<sfml_widgets::LineState>(((UINT16_MAX >> (15 - last)) & (UINT16_MAX << first)));
}
}

inline constexpr sfml_widgets::LineState LINE_SET = 1u;
// 0 to 7
inline constexpr unsigned int THICKNESS_OFFSET = 1u;
inline constexpr unsigned int THICKNESS_LAST_BIT = 3u;
inline constexpr sfml_widgets::LineState THICKNESS_MASK = GENERATE_MASK_U16(THICKNESS_OFFSET, THICKNESS_LAST_BIT);
// 0 to 15
inline constexpr unsigned int COLORS_OFFSET = 4u;
inline constexpr unsigned int COLORS_LAST_BIT = 7u;
inline constexpr sfml_widgets::LineState COLORS_MASK = GENERATE_MASK_U16(COLORS_OFFSET, COLORS_LAST_BIT);
// 0 to 3
inline constexpr unsigned int STATES_OFFSET = 8u;
inline constexpr unsigned int STATES_LAST_BIT = 9u;
inline constexpr sfml_widgets::LineState STATES_MASK = GENERATE_MASK_U16(STATES_OFFSET, STATES_LAST_BIT);

enum class STATES : sfml_widgets::LineState
{
    None = 0u,
    Open = 1u
};

struct CoordRange
{
    CoordRange(sf::Vector2u start, sf::Vector2u end) : start(start), end(end) {}
    sf::Vector2u start{0, 0};
    sf::Vector2u end{0, 0};
};

template<typename Num>
struct RectRange
{
    static_assert(std::is_arithmetic_v<Num>, "RectRange template type is not numeric.");
    Num x0{0}, y0{0}, x1{0}, y1{0};
    constexpr bool inRange(sf::Vector2<Num> point) const
    {
        const Num minX = std::min(x0, x1);
        const Num maxX = std::max(x0, x1);
        const Num minY = std::min(y0, y1);
        const Num maxY = std::max(y0, y1);
        return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
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

class GridCell
{
public:
    bool setOutlineThickness(DIRECTION _s, LineState thickness = 1u);
    bool setOutlineColor(DIRECTION _s, sf::Color color = DEFAULT_COLOR);
    bool setOutlineType(DIRECTION _s, STATES line_type = STATES::None);
    bool setCellColor(sf::Color color);
    std::size_t getID() const;
    bool inRange(sf::Vector2u grid_coords) const;
    bool inRange(sf::Vector2f pixel_coords) const;
    bool operator==(const GridCell& other) const;
    bool operator==(std::size_t id) const;
    GridCell(Canvas& root_canvas, std::size_t id);
    GridCell(GridCell&& other) = default;
    GridCell& operator=(GridCell&& other) = default;
private:

    GridCell(const GridCell& other) = delete;
    GridCell& operator=(const GridCell& other) = delete;

    Canvas* m_root;
    std::size_t m_id{0};
    RectRange<unsigned int> m_gridBounds{};
    sf::Color bg_color{sf::Color::Transparent};

    friend class Canvas;
};


class Canvas : public Widget
{
    struct Access;

public:
    class CellHandle
    {
    public:
        CellHandle() = default;

        explicit operator bool() const;
        GridCell* operator->() const;
        GridCell& operator*() const;

    private:
        friend class Canvas;
        CellHandle(std::weak_ptr<Access> access, std::size_t index, std::uint32_t generation);

        GridCell* resolve() const;

        std::weak_ptr<Access> m_access;
        std::size_t m_index{0};
        std::uint32_t m_generation{0};
    };

    Canvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid);
    Canvas(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid);
    ~Canvas() override;

    // lines
    bool drawLine(CoordRange line_range, LineState thickness = 1u, sf::Color color = DEFAULT_COLOR, STATES Line_type = STATES::None);
    bool removeLine(CoordRange line_range);
    void clearCanvas();
    bool setLineThickness(CoordRange line_range, LineState thickness = 1u);
    bool setLineColor(CoordRange line_range, sf::Color = DEFAULT_COLOR);
    bool setLineType(CoordRange line_range, STATES Line_type = STATES::None);

    // cells
    CellHandle at(sf::Vector2u grid_coords) const;
    CellHandle at(sf::Vector2f pixel_coords) const;
    CellHandle at(std::size_t cell_id) const;
    CellHandle getHoveredCell() const;
    CellHandle getSelectedCell() const;
    std::size_t getCellsCount() const;
    std::size_t getMaxCellsCount() const;

    // view
    sf::Vector2i getLocalPixel(sf::Vector2i mouse_position) const;
    sf::Vector2f getViewPixel(sf::Vector2i mouse_position) const;
    sf::Vector2f mapLocalToView(sf::Vector2i local_position) const;
    std::optional<sf::Vector2u> mapCoordsToGrid(sf::Vector2i mouse_position) const;
    std::optional<sf::Vector2f> mapGridToCoords(sf::Vector2u grid_coordinates) const;

    template<typename Tp>
    RectRange<float> mapGridRectToCoords(RectRange<Tp> grid_rect) const
    {
        return
        {
            m_deadZone + static_cast<float>(grid_rect.x0) * m_cellMinSize.y,
            m_deadZone + static_cast<float>(grid_rect.y0) * m_cellMinSize.x,
            m_deadZone + static_cast<float>(grid_rect.x1 + 1) * m_cellMinSize.y,
            m_deadZone + static_cast<float>(grid_rect.y1 + 1) * m_cellMinSize.x
        };

    }

    // other
    void setDeadZone(float surface);

    // theme
    void setTheme(TextureStyle s_canvas);
    void fetchTheme() override;
    TextureStyle& style();

private:
    // widget
    bool preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position) override;
    bool postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position) override;
    bool postProcessKeyEvents(const sf::Event& event) override; // arrow buttons to quickly add lines
    void drawWidget() override;
    void config(FLAGS mask) override;
    void updatePosition() override;
    void updateSize() override;
    void updateRotation() override;
    void updateState() override;
    void updateTheme() override;

    // helpers
    LineState& getLine(std::size_t row, std::size_t col, std::size_t axis); //!< BEST FRIEND
    const LineState& getLine(std::size_t row, std::size_t col, std::size_t axis) const; //!< CONST BEST FRIEND
    bool setLine(sf::Vector2u start, sf::Vector2u end, LineState val);
    bool isClear(const RectRange<unsigned int>& rect) const;
    bool isBoundarySealed(const RectRange<unsigned int>& rect) const;
    float getThickness(LineState line_flags);
    sf::Color getColor(LineState line_flags);
    STATES getState(LineState line_flags);
    int getColorIndex(sf::Color color);

    std::optional<std::size_t> findIndex(sf::Vector2u grid_coords) const;
    std::optional<std::size_t> findIndex(sf::Vector2f pixel_coords) const;
    GridCell* getCell(std::size_t index) const;
    CellHandle makeHandle(std::optional<std::size_t> index) const;

    // update
    void updateCellSize();
    void updateBoard();
    void _updateLines();
    void _updateCells();
    void _updateGrid();

    // view handle
    void zoom(float delta, sf::Vector2i local_mouse_position);
    void pan(sf::Vector2i local_mouse_position);
    void limitView(sf::View& v);

    static constexpr float s_MAX_ZOOM_LEVEL = 5.f;
    static constexpr float s_MIN_ZOOM_LEVEL = 1.f;
    static constexpr float s_MAX_DEADZONE_RATIO = 0.1f;

    // drawing
    sf::RenderTexture m_texture;
    sf::Sprite m_sprite;

    // data
    std::vector<LineState> m_lines;
    mutable std::vector<GridCell> m_cells;

    ///////////////////////////////////////////////////////////////////
    /// Kept alive only by this shared_ptr. CellHandle instances hold a
    /// weak_ptr to it: once Canvas is destroyed this block goes with it,
    /// so every outstanding handle's lock() starts failing on its own,
    /// with no cooperation needed from Canvas at destruction time.
    /// `generation` bumps once per `_updateCells()` pass; a handle whose
    /// snapshot doesn't match the current generation is stale even if
    /// Canvas is still alive, since its index may now name a different
    /// cell or none at all.
    ///////////////////////////////////////////////////////////////////
    struct Access
    {
        Canvas* canvas{nullptr};
        std::uint32_t generation{0};
    };
    std::shared_ptr<Access> m_access;

    // rendered data
    sf::VertexArray m_renderedlines{sf::PrimitiveType::Triangles};
    sf::VertexArray m_renderedCells{sf::PrimitiveType::Triangles};
    sf::VertexArray m_renderedGridLines{sf::PrimitiveType::Lines};
    std::array<std::optional<sf::Color>, MAX_COLOR_STORAGE> m_sharedColors{};

    
    // properties
    sf::Vector2f m_cellMinSize{0.f, 0.f};
    sf::Vector2i m_memMousePos{0, 0};
    TextureStyle m_style = Theme::texture_style::CANVAS_DARK_STYLE;
    float m_zoomLevel{s_MAX_ZOOM_LEVEL};
    float m_deadZone{0.f};

    // constants
    const sf::Vector2u m_grid;
    const std::size_t MAX_CELL_COUNT;

    // reference - these are slot indices, valid only within the generation
    // they were captured in; a rebuild resets all three, since a remembered
    // slot may now belong to a different cell or none
    std::size_t m_hoveredCellID{constants::KSTALE};
    std::size_t m_clickedCellID{constants::KSTALE};
    std::size_t m_selectedCellID{constants::KSTALE};

    // update
    bool m_updateLines{false};
    bool m_updateGridLines{false};

    Canvas(Canvas&& other) = delete;
    Canvas& operator=(Canvas&& other) = delete;
    Canvas(const Canvas& other) = delete;
    Canvas& operator=(const Canvas& other) = delete;

    friend class GridCell;
};

template <>
struct WidgetStyle<Canvas>
{
    using StyleType = TextureStyle;
 
    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_gridCanvasTheme;
    }
 
    static void set(WidgetTheme& theme, StyleType s_canvas)
    {
        theme.m_gridCanvasTheme = s_canvas;
    }
 
    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_gridCanvasToken;
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