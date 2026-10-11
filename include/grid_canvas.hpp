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
* flags for all available states for a line in `Canvas`
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
// reserved color for line to inherit canvas outline
inline constexpr sf::Color DEFAULT_COLOR{1, 1, 1, 1};
// max number of different line colors stored in canvas
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
/// in `Canvas` class
///
/// Derived renderer class can represent a different line type which
/// is explicitly added to a `Canvas` instance.
///
/// @see `OpenLineRenderer`, `DottedLineRenderer` 
///
///////////////////////////////////////////////////////////////////
class LineRenderer
{
    friend class Canvas;

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
    /// or leave it as it is for a default line generation on CPU. Managed by `Canvas`
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
    /// @brief Flags `Canvas` instance to update line data, this is used
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
/// @brief Renders open line type, added to every `Canvas` instance
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
    /// unit in `Canvas`, default is `false`.
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
/// @brief Renderers dotted lines, added to every `Canvas` instance
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
    /// @brief Updates the number of dots in each unit in `Canvas`, default
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
    GridCell(std::size_t id);

    GridCell(GridCell&& other) = delete;
    GridCell& operator=(GridCell&& other) = delete;
    GridCell(const GridCell& other) = delete;
    GridCell& operator=(const GridCell& other) = delete;

    ~GridCell();

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
    bool setOutlineType(DIRECTION _s, const std::string& line_type = NORMAL_LINE_LABEL);

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
    /// @brief Returned the line coordinates for canvas to identify
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
    void inheritTo(GridCell& other);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears texture from cell while updating canvas reference count
    ///
    ///////////////////////////////////////////////////////////////////
    void releaseTexture();

    Canvas* Proot;
    std::size_t id{0};
    RectRange<unsigned int> grid{};     //!< boundaries included in range
    sf::Color fill_color{sf::Color::Transparent};
    const sf::Texture* texture{nullptr};
    std::string texture_path;
};

///////////////////////////////////////////////////////////////////
/// @brief Grid Canvas widget for drawing lines and creating interractive
/// objects
///
///////////////////////////////////////////////////////////////////
class Canvas : public Widget
{
    struct Access;
    struct EditAnchor;
    struct EditAction;
    struct UnitEditDetails;

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

    Canvas(Canvas&& other) = delete;
    Canvas& operator=(Canvas&& other) = delete;
    Canvas(const Canvas& other) = delete;
    Canvas& operator=(const Canvas& other) = delete;

    ~Canvas() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws a horizontal/vertical line
    ///
    /// @param line_range start and end point for the line in units 
    /// @param thickness line initial thickness, range `[0, THICKNESS_FLAGS_COUNT[`
    /// @param color line color, default is canvas outline color
    /// @param Line_type line type, @see  `addLineType`, `removeLineType`, `listLineTypes`
    /// @return true if canvas successfully draws the line, otherwise false
    ///
    /// @see `removeLine`, `clearCanvas`, `setLineThickness`, `setLineColor`, `setLineType`
    ///
    ///////////////////////////////////////////////////////////////////
    bool drawLine(RectRange<unsigned int> line_range, LineState thickness = 1u, sf::Color color = DEFAULT_COLOR, const std::string& Line_type = NORMAL_LINE_LABEL);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a line range from canvas, range must be a horizontal/vertical
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
    /// @brief Clears canvas from all of it's contents (lines, cells, colors, textures)
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
    /// the current canvas outline color is assigned instead.
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
    /// canvas
    ///
    /// @note use `static_cast` or `dynamic_cast` to access the expected derived
    /// type.
    ///
    /// @see `LineRenderer`, `addLineType`, `removeLineType`, `listLineTypes`
    ///
    ///////////////////////////////////////////////////////////////////
    LineRenderer* getLineRenderer(const std::string& type);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a new line type to this canvas instance
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
    /// relative to canvas
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
    /// @brief Returns the equivalent pixel coordinates relative
    /// to canvas' board, accurate for local surface mapping
    ///
    /// @param mouse_position root window provided mouse coordinates  
    ///
    /// @see `getViewPixel`, `mapLocalToView`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2i getLocalPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to canvas' view,
    /// accurate for zoomed/panned canvas
    ///
    /// @param mouse_position root window provided mouse coordinates
    ///
    /// @see `getLocalPixel`, `mapLocalToView`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f getViewPixel(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates relative to canvas' view,
    /// accurate for zoomed/panned canvas
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
    /// @return nullopt if input coords are out of canvas bounds, otherwise
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
    /// @return nullopt if input coords are out of canvas bounds, otherwise
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
    /// canvas bounds, out-of-range input produces an out-of-range result
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
    /// canvas bounds, out-of-range input produces an out-of-range result
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
    /// @brief Forces an update to canvas visual style, it also updates
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
    GridCell* getCell(std::size_t index);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a read-only pointer to existing cell
    ///
    /// @param index cell's id 
    /// @return cell's address if found, otherwise nullptr
    ///
    ///////////////////////////////////////////////////////////////////
    const GridCell* getCell(std::size_t index) const;

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
    const std::string& LineExtractType(LineState line_flags);

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
    /// @brief Records a line edit in history for redo/undo actions
    ///
    /// @param action line edit
    ///
    /// @see `_undoLine`, `_redoLine`, `s_MAX_HISTORY`
    ///
    ///////////////////////////////////////////////////////////////////
    void pushEditHistory(EditAction&& action);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the index of the specific line type
    ///
    /// @param line_type line type
    /// @return index to renderer if found, otherwise defaults to 
    ///////////////////////////////////////////////////////////////////
    int getTypeIndex(const std::string& line_type) const;

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
    /// @brief Draws the anchor to canvas
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
    /// @brief Loads texture content from disk or from canvas cache if the
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
    std::pair<std::string, const sf::Texture*> _getTexture(const std::string& path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Decrements the texture's reference count, clears it from
    /// memory if it reaches 0
    ///
    /// @param cell desired cell
    /// 
    ///////////////////////////////////////////////////////////////////
    void _releaseTexture(GridCell& cell);

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
    std::optional<RectRange<unsigned int>> _findNextAnchorPoint(sf::Vector2u point, sf::Vector2u pen_point, sf::Vector2i delta, LineState line_value) const;

    static constexpr float s_MAX_ZOOM_LEVEL = 5.f;
    static constexpr float s_MIN_ZOOM_LEVEL = 1.f;
    static constexpr float s_MAX_DEADZONE_RATIO = 0.1f;
    static constexpr float s_PEN_INDICATOR_SIZE = 6.f;
    static constexpr std::size_t s_MAX_HISTORY = 128;

    // Responsible for invalidating external cell pointers after a new gen
    struct Access
    {
        Canvas* canvas{nullptr};
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
    std::deque<GridCell> m_cells;       //!< cells data only
    std::deque<EditAction> m_history;   //!< edit history

    // references
    std::shared_ptr<Access> m_access;   //!< every external cell_ptr counts on this
    std::optional<EditAnchor> m_pen;    //!< cached last added line in history for quick edit
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