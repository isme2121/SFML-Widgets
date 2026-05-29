#pragma once
#include "frame.hpp"
#ifdef WIDGETS_HPP
namespace sfml_widgets
{
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

    GridCanvas(GridCanvas&& other) noexcept;
    GridCanvas& operator=(GridCanvas&& other) noexcept;

    GridCanvas(const GridCanvas& other) = delete;
    GridCanvas& operator=(const GridCanvas& other) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events
    ///
    /// Called in the events loop, can recieve the current mouse position based on
    /// root window view to skip a few steps internally.
    ///
    /// Mouse events such as pressing, scrolling, holding are triggered within the
    /// method while also updating the widget's status such as focus, select, hover
    /// , and transparent, then trigger a callback if assigned to an event by the user.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    /// @param mouse_position optional current mouse position from `sf::Mouse::getPosition`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note canvas with `FLAGS::BLOCKED` or `FLAGS::HIDDEN` states cannot
    /// trigger mouse events.
    ///
    /// @see `Widget::bindMouseEvent`, `Widget::unbindMouseEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureMouseEvents(const sf::Event& _event, std::optional<sf::Vector2i> mouse_position = std::nullopt) override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events
    ///
    /// Captures keyboard events in the events loop, can capture individual
    /// physical key presses or a sequence of key press events depending on
    /// available assigned sequences.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    ///
    /// @return true if the canvas triggers a callback, otherwise false.
    ///
    /// @note canvas must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, canvas' with `FLAGS::BLOCKED | FLAGS::RESTRICTED`
    /// cannot trigger keyboard events.
    ///
    /// @see `Widget::bindShortCut`, `Widget::unbindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& _event) override;

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
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle __gridcanvas_style);

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
    Cell* getSelectedCellPtr(); 

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
    Cell* getClickedCellPtr();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns currently existing cells in canvas
    ///
    ///////////////////////////////////////////////////////////////////
    std::size_t getCellsCount() const;

    const std::size_t MAX_CELL_COUNT;        //!< Maximum possible number of cells in canvas object
private:

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
    /// @note This method can be skipped if `FLAGS::PASSIVESKIP` is set
    /// internally
    ///
    /// @see `Widget::linkTheme`, `Widget::unlinkTheme`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for clipping shader
    ///
    /// Best called after an update in a shader parameter, clip shader handles
    /// clipping edges on sprites.
    ///
    /// @see `loadClipShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateClipShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in a shader parameters, color shader is applied
    /// on background texture that is set via `setBackGroundTexture` and is not needed
    /// if no texture is assigned.
    ///
    /// @see `loadColorShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateColorShader() override;

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

    ///////////////////////////////////////////////////////////////////
    /// Member variables
    ///////////////////////////////////////////////////////////////////

    sf::RenderTexture m_drawTexture;                    //!< Texture for drawing
    sf::Sprite m_drawSprite;                            //!< Texture's sprite   
    Cell* m_selectedCell = nullptr;                     //!< Points to currently selected cell
    Cell* m_clickedCell = nullptr;                      //!< Points to currently clicked cell
    std::vector<Cell> m_cellMap;                        //!< Cells memory space
    sf::Vector2i m_memMousePos = {0, 0};                //!< Memory state for last clicked pixel inside canvas for panning
    const sf::Vector2u m_gridMapping;                   //!< rows, columns map for cells
    sf::Vector2f m_cellSize;                            //!< Shared size within all cells
    float m_zoomLevel = parameters::MAX_ZOOM_LEVEL;     //!< Current zoom value, limited between `MAX_ZOOM_LEVEL` and `MIN_ZOOM_LEVEL`
    float m_deadZone = 0.f;                             //!< Passive surface between boundaries and cells
    TextureStyle m_style = themes::STYLE_STALE_VIOLET;  //!< Canvas's default style

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
    void loadShader(const std::string& shader_path);

    ///////////////////////////////////////////////////////////////////
    /// @ Clears loaded color shader from cell
    ///
    /// @see `loadShader`
    ///
    ///////////////////////////////////////////////////////////////////
    void resetShader();

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
    CellColor m_style = themes::LIGHT_CELL;             //!< Unique cell colors in default state
    float m_deadZone;                                   //!< Root canvas' dead zone distance value
    CellOutline m_top, m_bottom, m_left, m_right;       //!< Separated outlines
    mutable FLAGS m_flags = FLAGS::MODIFIED;            //!< State flags

    friend class GridCanvas;

};

template <>
struct __WidgetStyle<GridCanvas>
{
    static const TextureStyle& get(const WidgetTheme& t)
    {
        return t.m_gridCanvasTheme;
    }

    static void set(WidgetTheme& t, TextureStyle _b)
    {
        t.m_gridCanvasTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_gridCanvasToken;
    }
};
}
#endif