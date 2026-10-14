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

constexpr unsigned int GET_FLAGS_COUNT(unsigned int low_bit, unsigned int high_bit)
{
    if(low_bit > high_bit)
    {
        unsigned int tmp = high_bit;
        high_bit = low_bit;
        low_bit = tmp;
    }
    return 1 << (1 + high_bit - low_bit);
}
}

/*
* flags for all available states for a line in `GridCanvas`
* stored in uint16 value
*/
inline constexpr std::size_t NULL_CELL = constants::KSTALE;
// LINE IS DRAWN WITH DEFAULT FLAGS (ALL ZERO)
inline constexpr sfml_widgets::LineState LINE_SET = 1u;

// 8 slots
inline constexpr unsigned int THICKNESS_OFFSET = 1u;
inline constexpr unsigned int THICKNESS_LAST_BIT = 3u;
inline constexpr unsigned int THICKNESS_FLAGS_COUNT = GET_FLAGS_COUNT(THICKNESS_OFFSET, THICKNESS_LAST_BIT);
inline constexpr sfml_widgets::LineState THICKNESS_MASK = GENERATE_MASK_U16(THICKNESS_OFFSET, THICKNESS_LAST_BIT);
inline constexpr unsigned int THICKNESS_OFF = UINT16_MAX ^ THICKNESS_MASK;

// 31 slots (index 0 reserved)
inline constexpr unsigned int COLORS_OFFSET = 4u;
inline constexpr unsigned int COLORS_LAST_BIT = 8u;
inline constexpr unsigned int COLORS_FLAGS_COUNT = GET_FLAGS_COUNT(COLORS_OFFSET, COLORS_LAST_BIT);
inline constexpr sfml_widgets::LineState COLORS_MASK = GENERATE_MASK_U16(COLORS_OFFSET, COLORS_LAST_BIT);
inline constexpr unsigned int COLORS_OFF = UINT16_MAX ^ COLORS_MASK;
// reserved color for line to inherit GridCanvas outline
inline constexpr sf::Color DEFAULT_COLOR{1, 1, 1, 1};
// max number of different line colors stored in GridCanvas
inline constexpr std::size_t MAX_COLOR_STORAGE = COLORS_FLAGS_COUNT - 1;

// 16 slots, available line types in `LineType` class
inline constexpr unsigned int TYPE_OFFSET = 8u;
inline constexpr unsigned int TYPE_LAST_BIT = 11u;
inline constexpr unsigned int TYPE_FLAGS_COUNT = GET_FLAGS_COUNT(TYPE_OFFSET, TYPE_LAST_BIT);
inline constexpr sfml_widgets::LineState TYPE_MASK = GENERATE_MASK_U16(TYPE_OFFSET, TYPE_LAST_BIT);
inline constexpr unsigned int TYPE_OFF = UINT16_MAX ^ TYPE_MASK;

inline constexpr const char* NORMAL_LINE_LABEL = "normal";
#ifdef CANVAS_MORE_LINE_TYPES
    inline constexpr const char* OPEN_LINE_LABEL = "open";
    inline constexpr const char* DOTS_LINE_LABEL = "dots";
#endif

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
/// @brief Base line renderer class, renders regular lines by default
/// in `GridCanvas` class
///
/// Derived renderer class can represent a different line type which
/// is explicitly added to a `GridCanvas` instance.
///
/// @see `OpenLineRenderer`, `DottedLineRenderer` 
///
///////////////////////////////////////////////////////////////////
class LineRenderer
{
    friend class GridCanvas;

public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs line renderer
    /// 
    /// @param line_type line type 
    /// @param shader shader string or path, relative to `loading_type`
    /// @param loading_type choose to load shader from memory or a file
    ///
    /// @note if the shader fails to load, the program will still run, but
    /// the created instance cannot assign a shader again, use `hasShader` method
    /// to check if the shader loaded successfully
    ///
    ///////////////////////////////////////////////////////////////////
    explicit LineRenderer(const std::string& line_type = NORMAL_LINE_LABEL, std::string_view shader = "", Widget::W_LoadType loading_type = Widget::W_LoadType::Memory);

    virtual ~LineRenderer();

    LineRenderer(LineRenderer&&) = delete;
    LineRenderer& operator=(LineRenderer&&) = delete;
    LineRenderer(const LineRenderer&) = delete;
    LineRenderer& operator=(const LineRenderer&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates a uniform value in assigned shader
    /// 
    /// @tparam UniformT value type 
    /// @param uniform_label uniform name in shader
    /// @param val value
    /// 
    ///////////////////////////////////////////////////////////////////
    template<typename UniformT>
    void setUniform(const std::string& uniform_label, UniformT val)
    {
        if(m_shader) m_shader->setUniform(uniform_label, val);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current vertex count
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getVertexCount() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns assigned line type 
    /// 
    ///////////////////////////////////////////////////////////////////
    const std::string& getType() const;

protected:
    ///////////////////////////////////////////////////////////////////
    /// @brief interprets how a line is drawn on the CPU side, a derived
    /// class can override this function for a different line management process,
    /// or leave it as it is for a default line generation on CPU. Managed by `GridCanvas`
    /// instance. 
    ///
    /// @param line_start_px starting pixel coord
    /// @param line_end_px ending pixel coord
    /// @param fixed_px fixed pixel, on either x or y axis
    /// @param line_thickness line thickness
    /// @param run_length_units line length in units
    /// @param line_color line color
    /// @param is_vertical is this line vertical?
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void appendLine(float line_begin_px, float line_end_px, float fixed_px, float line_thickness, float run_length_units, sf::Color line_color, bool is_vertical);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws vertex array against target
    ///
    ///////////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& target) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Resets rendered vertex content
    ///
    ///////////////////////////////////////////////////////////////////
    void clear();

    ///////////////////////////////////////////////////////////////////
    /// @brief Flags `GridCanvas` instance to update line data, this is used
    /// when a derived class attribute (which is used to influence vertex data
    /// in overriden `appendLine` method) has it's value updated. 
    ///
    ///////////////////////////////////////////////////////////////////
    void setLineUpdate();

    sf::VertexArray m_vertices{sf::PrimitiveType::Triangles}; //!< line data

private:
    sf::RenderStates m_states;
    std::unique_ptr<sf::Shader> m_shader;
    const std::string m_type;
    bool m_requireLineUpdate{false};
};

#ifdef CANVAS_MORE_LINE_TYPES
///////////////////////////////////////////////////////////////////
/// @brief Renders open line type, added to every `GridCanvas` instance
/// by default (for now)
///
///////////////////////////////////////////////////////////////////
class OpenLineRenderer : public LineRenderer
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs with built-in shader 
    ///
    ///////////////////////////////////////////////////////////////////
    explicit OpenLineRenderer();
    
    ///////////////////////////////////////////////////////////////////
    /// @brief When join is set, multiple open type lines that are joined
    /// would have a single large open space, rather than an open space per
    /// unit in `GridCanvas`, default is `false`.
    /// 
    /// @param merged new mode
    /// 
    ///////////////////////////////////////////////////////////////////
    void setJoinMode(bool merged);

    ///////////////////////////////////////////////////////////////////
    /// @brief The width of the open space in ratio of the line's total length
    ///
    /// This value is limited by the stub ratio current value, which logically
    /// adds up to: `1.f = open_ratio + stub_ratio * 2.f;`.
    /// 
    /// Default is `0.4f`.
    ///
    /// @param ratio ratio from `0.f` to `1.f`
    /// 
    ///////////////////////////////////////////////////////////////////
    void setDoorWidthRatio(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief the width of each side's stub in ratio of the line's total length
    ///
    /// Total stub width is line length * 2 and it occupies the line's start
    /// and end sides to form an open door line.
    ///
    /// Default is `0.3f`.
    ///
    /// @param ratio ratio from `0.01f` to `0.49f`
    /// 
    ///////////////////////////////////////////////////////////////////
    void setStubRatio(float ratio);
    
private:
    void appendLine(float line_begin_px, float line_end_px, float fixed_px, float line_thickness,
                            float run_length_units, sf::Color line_color, bool is_vertical) override;

    float m_doorWidthRatio{0.4f};
    float m_stubRatio{0.3f};
    bool m_joinMode{false};
};

///////////////////////////////////////////////////////////////////
/// @brief Renderers dotted lines, added to every `GridCanvas` instance
/// by default (for now)
///
///////////////////////////////////////////////////////////////////
class DottedLineRenderer : public LineRenderer
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs with built-in shader
    ///
    ///////////////////////////////////////////////////////////////////
    explicit DottedLineRenderer();
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the number of dots in each unit in `GridCanvas`, default
    /// is `4u`
    ///
    /// @param dots_per_unit number of dots from `1u` to +inf
    /// 
    ///////////////////////////////////////////////////////////////////
    void setSpacing(unsigned int dots_per_unit);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the thickness scaling of dotted lines, default
    /// is `2.5f`
    ///
    /// Example: pixel_height=`2.f` ==> thickness is twice as effective for
    /// dotted line
    ///
    /// @param scale from `0.1f` to +inf
    /// 
    ///////////////////////////////////////////////////////////////////
    void setRowPixelHeight(float scale);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the total radius of each dot, default is `1.5f`
    /// 
    /// @param radius new radius, from `0.1f` to +inf
    /// 
    ///////////////////////////////////////////////////////////////////
    void setPixelRadius(float radius);

private:
    void appendLine(float line_begin_px, float line_end_px, float fixed_px, float line_thickness,
                            float run_length_units, sf::Color line_color, bool is_vertical) override;

    unsigned int m_dotSpacing{4u};
    float m_rowPixelHeight{2.5f};
    float m_pixelRadius{1.5f};
};
#endif

///////////////////////////////////////////////////////////////////
/// @brief Cell attributes object from `GridCanvas`
///
/// A cell is automatically constructed from a rectangular shape made
/// by GridCanvas lines.
///
///////////////////////////////////////////////////////////////////
class Cell
{
    friend class GridCanvas;

public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs a cell on GridCanvas
    ///
    ///////////////////////////////////////////////////////////////////
    Cell(std::size_t id);

    Cell(Cell&& other) = delete;
    Cell& operator=(Cell&& other) = delete;
    Cell(const Cell& other) = delete;
    Cell& operator=(const Cell& other) = delete;

    ~Cell();

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
    /// @param color new outline color, default sets it to GridCanvas outline color
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
    bool setOutlineType(DIRECTION _s, const std::string& line_type = NORMAL_LINE_LABEL);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if unit point collides with cell rect
    ///
    /// @param grid_coords GridCanvas coords in unit 
    /// @return true if point is inside rect
    ///
    ///////////////////////////////////////////////////////////////////
    bool inRange(sf::Vector2u grid_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if pixel point collides with cell rect
    ///
    /// @param pixel_coords GridCanvas coords in pixels 
    /// @return true if point is inside rect
    ///
    ///////////////////////////////////////////////////////////////////
    bool inRange(sf::Vector2f pixel_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the cell's ID
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getID() const;

    bool operator==(const Cell& other) const;

    bool operator==(std::size_t id) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a texture to cell
    ///
    /// @param image_path path to image 
    /// @return true if process is successful, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool setCellTexture(const std::string& image_path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears texture from this cell 
    ///
    ///////////////////////////////////////////////////////////////////
    bool clearCellTexture();

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Returned the line coordinates for GridCanvas to identify
    ///
    /// @param _s outline side 
    /// @return rect with start and end unit coords for line
    ///
    ///////////////////////////////////////////////////////////////////
    RectRange<unsigned int> getLineRange(DIRECTION _s) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief partial move between different cell generations and moves
    /// texture membership to other
    /// 
    /// @param other inheriting cell 
    ///
    ///////////////////////////////////////////////////////////////////
    void inheritTo(Cell& other);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears texture from cell while updating GridCanvas reference count
    ///
    ///////////////////////////////////////////////////////////////////
    void releaseTexture();

    GridCanvas* Proot;
    std::size_t id{0};
    RectRange<unsigned int> grid{};     //!< boundaries included in range
    sf::Color fill_color{sf::Color::Transparent};
    const sf::Texture* texture{nullptr};
    std::string texture_path;
};

///////////////////////////////////////////////////////////////////
/// @brief Grid GridCanvas widget for drawing lines and creating interractive
/// objects
///
///////////////////////////////////////////////////////////////////
class GridCanvas : public Widget
{
    struct Access;
    struct EditAnchor;
    struct EditAction;
    struct UnitEditDetails;

    friend class Cell;

public:
    class cell_ptr
    {
    public:
        cell_ptr() = default;

        explicit operator bool() const;
        Cell* operator->() const;
        Cell& operator*() const;

    private:
        friend class GridCanvas;
        cell_ptr(std::weak_ptr<Access> access, std::size_t index, std::uint32_t generation);

        Cell* resolve() const;

        std::weak_ptr<Access> m_access;
        std::size_t m_index{0};
        std::uint32_t m_generation{0};
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs GridCanvas on window
    ///
    /// @param window root window 
    /// @param position position relative to window
    /// @param size GridCanvas size
    /// @param grid grid map, must be at least {1, 1}
    ///
    ///////////////////////////////////////////////////////////////////
    GridCanvas(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs GridCanvas on root frame
    ///
    /// @param root parent frame widget 
    /// @param position position relative to frame
    /// @param size GridCanvas size
    /// @param grid grid units map, must be at least {1, 1}
    ///
    ///////////////////////////////////////////////////////////////////
    GridCanvas(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Vector2u& grid);

    GridCanvas(GridCanvas&& other) = delete;
    GridCanvas& operator=(GridCanvas&& other) = delete;
    GridCanvas(const GridCanvas& other) = delete;
    GridCanvas& operator=(const GridCanvas& other) = delete;

    ~GridCanvas() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws a horizontal/vertical line
    ///
    /// @param line_range start and end point for the line in units 
    /// @param thickness line initial thickness, range `[0, THICKNESS_FLAGS_COUNT[`
    /// @param color line color, default is GridCanvas outline color
    /// @param Line_type line type, @see  `addLineType`, `removeLineType`, `listLineTypes`
    /// @return true if GridCanvas successfully draws the line, otherwise false
    ///
    /// @see `removeLine`, `clearCanvas`, `setLineThickness`, `setLineColor`, `setLineType`
    ///
    ///////////////////////////////////////////////////////////////////
    bool drawLine(RectRange<unsigned int> line_range, LineState thickness = 1u, sf::Color color = DEFAULT_COLOR, const std::string& Line_type = NORMAL_LINE_LABEL);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a line range from GridCanvas, range must be a horizontal/vertical
    /// line
    ///
    /// @param line_range start and end points in units 
    /// @return true if at least one segment in range changed, otherwise false -
    /// including when the range had no line to begin with
    ///
    /// @see `drawLine`, `clearCanvas`, `setLineThickness`, `setLineColor`, `setLineType`
    ///
    ///////////////////////////////////////////////////////////////////
    bool removeLine(RectRange<unsigned int> line_range);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears GridCanvas from all of it's contents (lines, cells, colors, textures)
    ///
    /// @see `drawLine`, `removeLine`, `setLineThickness`, `setLineColor`, `setLineType`
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
    /// @see `setLineColor`, `setLineType`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLineThickness(RectRange<unsigned int> line_range, LineState thickness = 1u);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the line's color, creates the line if it doesn't exist
    ///
    /// If color is `DEFAULT_COLOR` or max color count is reached then
    /// the current GridCanvas outline color is assigned instead.
    ///
    /// @param line_range start and end points in units 
    /// @param color new color value
    /// @return true if line is updated, otherwise false
    ///
    /// @see `setLineThickness`, `setLineType`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLineColor(RectRange<unsigned int> line_range, sf::Color color = DEFAULT_COLOR);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the line's type, creates the line if it doesn't exist
    ///
    /// An unrecognized type name leaves the line's type unchanged (falls
    /// back to whatever was already set, `"normal"` for a new line) rather
    /// than failing the call.
    ///
    /// @param line_range start and end points in units 
    /// @param Line_type registered line type name
    /// @return true if line is updated, otherwise false
    ///
    /// @see `setLineThickness`, `setLineColor`, `addLineType`, `removeLineType`, `listLineTypes`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setLineType(RectRange<unsigned int> line_range, const std::string& Line_type = NORMAL_LINE_LABEL);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the assigned line renderer for corresponding type
    ///
    /// @param type renderer type 
    /// @return pointer to renderer type, nullptr if type is not assigned to
    /// GridCanvas
    ///
    /// @note use `static_cast` or `dynamic_cast` to access the expected derived
    /// type.
    ///
    /// @see `LineRenderer`, `addLineType`, `removeLineType`, `listLineTypes`
    ///
    ///////////////////////////////////////////////////////////////////
    LineRenderer* getLineRenderer(const std::string& type);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a new line type to this GridCanvas instance
    ///
    /// If successfully added, line type can be referenced in `drawLine`, `setLineType`,
    /// and `getLineRenderer`.
    ///
    /// @param new_line_type pointer to a `LineRenderer` derived class instance
    /// @return true if type is added, false if the max number of types is
    /// already reached (@see `TYPE_FLAGS_COUNT`), or if that type name is
    /// already assigned
    ///
    /// @see `getLineRenderer`, `removeLineType`, `listLineTypes`
    ///
    ///////////////////////////////////////////////////////////////////
    bool addLineType(std::unique_ptr<LineRenderer> new_line_type);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes an assigned line renderer for this instance
    ///
    /// @param line_type_label existing line tyoe 
    /// @return true if successfully removed, otherwise false
    /// if type is the default line type or not found
    ///
    /// @see  `addLineType`, `getLineRenderer`, `listLineTypes`
    ///
    ///////////////////////////////////////////////////////////////////
    bool removeLineType(const std::string& line_type_label);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns an array of available line types for this instance
    ///
    /// @see  `addLineType`, `removeLineType`, `getLineRenderer`
    ///
    ///////////////////////////////////////////////////////////////////
    std::vector<std::string> listLineTypes() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a reference to the cell in parameter unit coords
    ///
    /// @param grid_coords coords in units 
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    /// @see `getHoveredCell`, `getSelectedCell`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr at(sf::Vector2u grid_coords) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a reference to the cell in parameter pixel coords
    /// relative to GridCanvas
    ///
    /// @param pixel_coords coords in pixels
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    /// @see `getHoveredCell`, `getSelectedCell`
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
    /// @see `getHoveredCell`, `getSelectedCell`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr at(std::size_t cell_id) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current hovered cell
    ///
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    /// @see `at`, `getSelectedCell`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr getHoveredCell() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current selected cell
    /// 
    /// @return valid `cell_ptr` if cell is found, otherwise a null `cell_ptr`
    ///
    /// @see `at`, `getHoveredCell`
    ///
    ///////////////////////////////////////////////////////////////////
    cell_ptr getSelectedCell() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current cells count
    ///
    /// @see `getMaxCellsCount`
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getCellsCount() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the max number of cells possible (units.x * units.y)
    ///
    /// @see `getCellsCount`
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getMaxCellsCount() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Enables or disables quick drawing, enabled by default
    ///
    /// When enabled, lines can be inserted after selecting+hovering on
    /// the widget and pressing any `ARROW_KEY`, can also use `ARROW_KEY+CTRL`
    /// to copy the previous line's value, or `ARROW_KEY+SHIFT` to continue
    /// from last line, or both `ARROW_KEY+CTRL+SHIFT`. `CTRL+Z` & `CTRL+Y`
    /// are also enabled. 
    ///
    /// @param enabled new state
    /// @param shift_skip_identical_lines if set, `ARROW+SHIFT` leaps over lines
    /// with equal value, false by default
    ///
    /// @see `showPen`
    ///
    ///////////////////////////////////////////////////////////////////
    void setQuickDraw(bool enabled, bool shift_skip_identical_lines = false);

    ///////////////////////////////////////////////////////////////////
    /// @brief Toggles the pen, enabled by default
    ///
    /// A visual for displaying where the quick draw pen is currently
    /// positioned, drawn on every quick draw event.
    ///
    /// @param show choose to draw the visual
    /// @param color visual color, `DEFAULT_COLOR` inherits GridCanvas outline color
    ///
    /// @see `setQuickDraw`
    ///
    ///////////////////////////////////////////////////////////////////
    void showPen(bool show, sf::Color color = DEFAULT_COLOR);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the equivalent pixel coordinates relative
    /// to GridCanvas' board, accurate for local surface mapping
    ///
    /// @param mouse_position root window provided mouse coordinates  
    ///
    /// @see `getViewPixel`, `mapLocalToView`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2i getLocalPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to GridCanvas' view,
    /// accurate for zoomed/panned GridCanvas
    ///
    /// @param mouse_position root window provided mouse coordinates
    ///
    /// @see `getLocalPixel`, `mapLocalToView`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f getViewPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to GridCanvas' view,
    /// accurate for zoomed/panned GridCanvas
    ///
    /// @param local_position coordinates provided from `getLocalPixel`
    ///
    /// @see `getLocalPixel`, `getViewPixel`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f mapLocalToView(sf::Vector2i local_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns unit coordinates relative to parameter pixel coordinates
    ///
    /// @param mouse_position root window provided mouse coordinates 
    /// @return nullopt if input coords are out of GridCanvas bounds, otherwise
    /// unit coordinates
    ///
    /// @see `mapGridToCoords`, `mapGridRectToCoords`, `mapCoordsRectToGrid`
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<sf::Vector2u> mapCoordsToGrid(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to parameter unit coordinates
    ///
    /// @param grid_coordinates coordinates in units 
    /// @return nullopt if input coords are out of GridCanvas bounds, otherwise
    /// pixel coordinates
    ///
    /// @see `mapCoordsToGrid`, `mapGridRectToCoords`, `mapCoordsRectToGrid`
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<sf::Vector2f> mapGridToCoords(sf::Vector2u grid_coordinates) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to parameter unit rect
    ///
    /// Unlike `mapGridToCoords`, this does not validate the input against
    /// GridCanvas bounds, out-of-range input produces an out-of-range result
    /// rather than failing.
    ///
    /// @param grid_rect rect range of unit coordinates
    /// @return pixel coordinates rect
    ///
    /// @see `mapCoordsRectToGrid`, `mapGridToCoords`, `mapCoordsToGrid`
    ///
    ///////////////////////////////////////////////////////////////////
    RectRange<float> mapGridRectToCoords(RectRange<unsigned int> grid_rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns unit coordinates relative to parameter pixel rect
    ///
    /// Returns range includes any unit that sits inside the provided input.
    /// Unlike `mapCoordsToGrid`, this does not validate the input against
    /// GridCanvas bounds, out-of-range input produces an out-of-range result
    /// rather than failing.
    ///
    /// @param coords_rect rect range of pixel coordinates
    /// @return unit coordinates rect
    ///
    /// @see `mapGridRectToCoords`, `mapCoordsToGrid`, `mapGridToCoords`
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
    /// @see `linkTheme`, `fetchTheme`, `style`, `poke`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(CanvasStyle s_canvas);

    ///////////////////////////////////////////////////////////////////
    /// @brief Pulls the widget's style from it's linked theme object
    /// 
    /// @see `linkTheme`, `setTheme`, `style`, `poke`
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
    /// @see `linkTheme`, `setTheme`, `fetchTheme`, `poke`
    ///
    ///////////////////////////////////////////////////////////////////
    CanvasStyle& style();

    ///////////////////////////////////////////////////////////////////
    /// @brief Forces an update to GridCanvas visual style, it also updates
    /// grid lines if enabled
    ///
    /// @see `linkTheme`, `setTheme`, `fetchTheme`, `style`
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
    /// @brief Draws GridCanvas to root
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Extra configurations handling
    /// 
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates GridCanvas position
    /// 
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates GridCanvas size
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates GridCanvas rotation
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates GridCanvas visuals by state
    ///
    ///////////////////////////////////////////////////////////////////
    void updateState() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates assigned theme if any
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the contents of GridCanvas board, this includes
    /// the grid, lines, and cells
    /// 
    ///////////////////////////////////////////////////////////////////
    void updateBoard();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the minimum cell size in GridCanvas which matches a
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
    const LineState& getLine(std::size_t row, std::size_t col, std::size_t axis) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to existing cell
    ///
    /// @param index cell's id 
    /// @return cell's address if found, otherwise nullptr
    ///
    ///////////////////////////////////////////////////////////////////
    Cell* getCell(std::size_t index);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a read-only pointer to existing cell
    ///
    /// @param index cell's id 
    /// @return cell's address if found, otherwise nullptr
    ///
    ///////////////////////////////////////////////////////////////////
    const Cell* getCell(std::size_t index) const;

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
    /// @param pixel_coords coordinates in pixels relative to GridCanvas view 
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
    const std::string& LineExtractType(LineState line_flags);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if rect surface in GridCanvas has any lines insides,
    /// rect bounds do not count, this is done to check for a valid cell
    /// surface
    ///
    /// @param rect range in units 
    /// 
    ///////////////////////////////////////////////////////////////////
    bool isRectClear(const RectRange<unsigned int>& rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks if rect bounds are all constructed lines if rect
    /// is touching GridCanvas bounds
    ///
    /// @param rect range in units 
    ///
    ///////////////////////////////////////////////////////////////////
    bool isRectSealed(const RectRange<unsigned int>& rect) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates a cell's visual fill color
    ///
    /// @param cell_id cell id 
    /// @param color fill color
    /// @return true if cell exists, otherwise false
    ///
    ///////////////////////////////////////////////////////////////////
    bool setCellColor(std::size_t cell_id, sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Fetches and assigns a texture to the cell with cell_id
    ///  
    /// @param cell_id cell id 
    /// @param image_path path to image
    /// @return true if process is successful, otherwise false
    /// 
    ///////////////////////////////////////////////////////////////////
    bool setCellTexture(std::size_t cell_id, const std::string& image_path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns color index from colors storage
    ///
    /// @param color searched color 
    /// @return color index, or -1 if color is not found
    ///
    ///////////////////////////////////////////////////////////////////
    int getColorIndex(sf::Color color);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the index of the specific line type
    ///
    /// @param line_type line type
    /// @return index to renderer if found, otherwise defaults to 
    ///////////////////////////////////////////////////////////////////
    int getTypeIndex(const std::string& line_type) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief The actual line value setter
    ///
    /// @param rect start to end unit points for line 
    /// @param val assigned value
    /// @param mask applied if replace is false, any `0` bit in mask resets line state bit counter part
    /// @param replace param val replaces old val if true, otherwise is added to it after applying mask
    /// @return true if line is updated, otherwise false upon fail
    ///
    ///////////////////////////////////////////////////////////////////
    bool _setLineValue(const RectRange<unsigned int>& rect, LineState val, std::uint16_t mask = UINT16_MAX, bool replace = true);

    ///////////////////////////////////////////////////////////////////
    /// @brief Records a line edit in history for redo/undo actions
    ///
    /// @param action line edit
    ///
    /// @see `_undoLine`, `_redoLine`, `s_MAX_HISTORY`
    ///
    ///////////////////////////////////////////////////////////////////
    void _pushEditHistory(EditAction&& action);

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

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the anchor to GridCanvas
    ///
    ///////////////////////////////////////////////////////////////////
    void _drawPenIndicator();

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates texture layer for cell, fails if cell has no texture,
    /// managed internally
    ///
    /// @param cell_id cell id
    ///
    ///////////////////////////////////////////////////////////////////
    void _emplaceCellTexture(std::size_t cell_id);

    ///////////////////////////////////////////////////////////////////
    /// @brief Decrements the texture's reference count, clears it from
    /// memory if it reaches 0
    ///
    /// @param cell desired cell
    /// 
    ///////////////////////////////////////////////////////////////////
    void _releaseTexture(Cell& cell);

    ///////////////////////////////////////////////////////////////////
    /// @brief Adds a vertex quad to rendered memory
    ///
    /// @param type line type 
    /// @param span_start_px starting pixel coord
    /// @param span_end_px ending pixel coord
    /// @param fixed_px fixed pixel, on either x or y axis
    /// @param thickness line thickness
    /// @param run_length_units line length in units
    /// @param color line color
    /// @param is_vertical is this line vertical?
    /// 
    ///////////////////////////////////////////////////////////////////
    void _appendLine(const std::string& type, float span_start_px, float span_end_px, float fixed_px,
                     float thickness, float run_length_units, sf::Color color, bool is_vertical);

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
    /// @brief undo history edit with CTRL+Z
    ///
    /// @see `_redoLine`, `pushEditHistory`
    ///
    ///////////////////////////////////////////////////////////////////
    bool _undoLine();

    ///////////////////////////////////////////////////////////////////
    /// @brief redo history edit with CTRL+Y
    ///
    /// @see `_undoLine`, `pushEditHistory`
    ///
    ///////////////////////////////////////////////////////////////////
    bool _redoLine();

    ///////////////////////////////////////////////////////////////////
    /// @brief Applies a history edit action
    ///
    /// @param action edit to apply
    /// @param undo desired action
    ///
    /// @see `_undoLine`, `_redoLine`
    ///
    ///////////////////////////////////////////////////////////////////
    void __applyAction(const EditAction& action, bool undo);

    ///////////////////////////////////////////////////////////////////
    /// @brief Loads texture content from disk or from GridCanvas cache if the
    /// same image with the same size is actively used
    ///
    /// Loaded texture is placed in cache for cells to reference.
    ///
    /// @param key  
    /// @param path 
    /// @param target_size 
    /// @return 
    ///
    ///////////////////////////////////////////////////////////////////
    std::pair<std::string, const sf::Texture*> __getTexture(const std::string& path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the next anchor point relative to delta, managed
    /// in `postProcessKeyEvents` overload
    ///
    /// @param point line starting point relative to delta
    /// @param pen_point current anchor point
    /// @param delta change in direction
    /// @param line_value current line value
    /// @return new line range
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<RectRange<unsigned int>> __findNextAnchorPoint(sf::Vector2u point, sf::Vector2u pen_point, sf::Vector2i delta, LineState line_value) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Safely clears the line type from every existing line
    ///
    ///////////////////////////////////////////////////////////////////
    void __clearType(int type_idx);

    ///////////////////////////////////////////////////////////////////
    /// @brief Starts the pen's visible window, does nothing unless quick
    /// draw and the pen are both enabled
    ///
    ///////////////////////////////////////////////////////////////////
    void __drawPen();

    // Responsible for invalidating external cell pointers after a new gen
    struct Access
    {
        GridCanvas* canvas{nullptr};
        std::uint32_t generation{0};
    };

    // source texture for cells
    struct TexSource
    {
        sf::Texture texture;
        std::size_t ref_count{0};
    };

    // single unit line edit information
    struct UnitEditDetails
    {
        std::uint32_t line_index;
        LineState prev_val;
        LineState new_val;
    };

    // for SHIFT/CTRL+quick draw events
    struct EditAnchor
    {
        RectRange<unsigned int> range;  //!< prev line's range (SHIFT)
        LineState state;                //!< prev line's value (CTRL)
    };

    // single line edit action, this is what gets undone/redone
    struct EditAction
    {
        std::optional<EditAnchor> pen;          //!< cache
        std::vector<UnitEditDetails> changes;   //!< all unit changes from this edit
    };

    // the pen
    struct Pen
    {
        std::optional<EditAnchor> anchor;
        sf::Clock timer;
        sf::Color color{sf::Color::Red};
        bool show{true};
    };
    
    static constexpr float s_MAX_ZOOM_LEVEL = 5.f;
    static constexpr float s_MIN_ZOOM_LEVEL = 1.f;
    static constexpr float s_MAX_DEADZONE_RATIO = 0.1f;
    static constexpr float s_PEN_WIDTH = 6.f;
    static constexpr std::size_t s_MAX_HISTORY = 128;
    static constexpr sf::Time s_PEN_DURATION = sf::seconds(1.5f);

    // board
    sf::RenderTexture m_texture;
    sf::Sprite m_sprite;

    // rendered data
    sf::VertexArray m_renderedCells{sf::PrimitiveType::Triangles};          //!< cell bodies
    sf::VertexArray m_renderedCellTextures{sf::PrimitiveType::Triangles};   //!< cell texture bodies
    sf::VertexArray m_renderedGridLines{sf::PrimitiveType::Lines};          //!< for grid
    std::vector<std::unique_ptr<LineRenderer>> m_lineRenderers;             //!< stores and draws lines
    std::unordered_map<std::string, TexSource> m_textureCache;

    // data
    std::vector<LineState> m_lines;     //!< all lines data
    std::deque<Cell> m_cells;       //!< cells data only
    std::deque<EditAction> m_history;   //!< edit history

    // references
    Pen m_pen;                          //!< cached last added line in history for quick edit
    std::shared_ptr<Access> m_access;   //!< every external `cell_ptr` counts on this
    std::array<std::optional<sf::Color>, MAX_COLOR_STORAGE> m_reservedLineColors{};
    std::size_t m_hoveredCellID{NULL_CELL};
    std::size_t m_clickedCellID{NULL_CELL};
    std::size_t m_selectedCellID{NULL_CELL};
    std::size_t m_historyCursor{0};     //!< points at edits
    
    // properties
    sf::Vector2f m_cellMinSize{0.f, 0.f};
    sf::Vector2i m_memMousePos{0, 0};
    CanvasStyle m_style = Theme::canvas_style::CANVAS_DARK_STYLE;
    float m_zoomLevel{s_MAX_ZOOM_LEVEL};
    float m_deadZone{0.f};

    // constants
    const sf::Vector2u m_GRID_MAP;
    const std::size_t m_MAX_CELL_COUNT;

    // update
    bool m_updateLines{false};
    bool m_updateCells{false};
    bool m_updateGridLines{false};

    // flags
    bool m_quickDrawEnable{true};
    bool m_quickDrawSkipIdentical{false};
};

template <>
struct WidgetStyle<GridCanvas>
{
    using StyleType = CanvasStyle;
 
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
}
#endif