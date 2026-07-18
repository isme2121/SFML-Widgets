#pragma once
#include <deque>
#include <mutex>
#include "widget_themes.hpp"
#include "tooltip.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Base class for all widgets
///
/// @see `WidgetGroup`, `Button`, `Frame`, `Label`, `GridCanvas`, `ImageFrame`, `CheckBox`, `Entry`
///
///////////////////////////////////////////////////////////////////
class Widget : protected sf::Shape
{
    friend class Frame;
    friend class WidgetGroup;

protected:
    struct ImageCache;

public:
    enum class W_MouseEventType : int;
    enum class W_CounterType : int;
    enum class W_LoadType : int;
    struct W_MouseEventDetails;
    struct W_QueueEvent;
    struct W_RepeatEvent;

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button with `sf::RenderWindow`
    ///
    /// @param root window or a frame widget
    /// @param position widget's initial position
    /// @param size widget's initial size
    ///
    ///////////////////////////////////////////////////////////////////
    Widget(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button on a `Frame` object
    ///
    /// @param root window or a frame widget
    /// @param position widget's initial position
    /// @param size widget's initial size
    ///
    ///////////////////////////////////////////////////////////////////
    Widget(Frame& root, sf::Vector2f position, sf::Vector2u size);

    Widget(Widget&& other) = delete;
    Widget& operator=(Widget&& other) = delete;
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    virtual ~Widget();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget on master object
    ///
    ///////////////////////////////////////////////////////////////////
    void draw();

    ///////////////////////////////////////////////////////////////////
    /// @brief Toggles control flags for widget
    ///
    /// Use `|` operator to pass multiple flags at once.
    ///
    /// @note only flags stated in constant `MODIFIABLE` are accepted as parameters.
    /// @note flags like `HIDDEN | BLOCKED | RESIZED` are propagated by the root frame
    /// if this widget is constructed on one but can still be manually set here.
    /// @note flags like `NOBGIMAGE | NOCLIPSHADER | NOCOLORSHADER | TRANSPARENT | AUTOTRANSPARENT`
    /// immediately take effect.
    ///
    /// @param  flags modifiable state flags, @see `MODIFIABLE`
    ///
    ///////////////////////////////////////////////////////////////////
    void configure(FLAGS flags);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates widget's position based on it's master's view
    ///
    /// Position is relative to the widget's {0, 0} local coordinates.
    ///
    /// @see `getWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void setWidgetPosition(const sf::Vector2f pos);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates widget's size
    ///
    /// @see `getWidgetSize`
    ///
    ///////////////////////////////////////////////////////////////////
    void setWidgetSize(const sf::Vector2f size);

    ///////////////////////////////////////////////////////////////////
    /// @brief similar to `sf::Transformable::setRotation` where it overwrites
    /// the widget's rotation angle
    ///
    /// @see `rotateWidget`
    ///
    ///////////////////////////////////////////////////////////////////
    void setWidgetRotation(float new_angle);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events
    ///
    /// Checks for all kinds of mouse events related to the widget including
    /// hovering while triggering the proper callback for each event if assigned
    /// within `bindMouseEvent` method.
    ///
    /// Call this method in the events loop once per widget unless if the widget
    /// belongs to a `Frame`, then calling this from the root `Frame` widget is
    /// sufficient.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    /// @param mouse_position optional current mouse position from `sf::Mouse::getPosition`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @see `bindMouseEvent`, `unbindMouseEvent`, `resizeWidgets`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position = std::nullopt);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events
    ///
    /// Checks for keyboard events that are assigned to the widget within
    /// `bindKeyEvent` method.
    ///
    /// Call this method in the events loop once per widget unless if the widget
    /// belongs to a `Frame`, then calling this from the root `Frame` widget is
    /// sufficient.
    ///
    /// @note Multiple widgets can trigger their assigned callback if they share
    /// the same key sequence
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    ///
    /// @see `bindKeyEvent`, `unbindKeyEvent`, `resizeWidgets`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a mouse event with a callback to this widget
    ///
    /// Events captured by `captureMouseEvents` such as pressing a
    /// mouse button or scrolling are assigned here.
    ///
    /// @note Certain events like `PRESS` & `RELEASE` require specifying which
    /// button would trigger their respective bindings.
    ///
    /// @param event_type Which event type triggers the callback
    /// @param callback function call during event match, can take `sf::Event`
    /// as a parameter
    /// @param button only necessary if event type is `W_MouseEventType::Press` or
    /// `W_MouseEventType::release`
    ///
    /// @see `captureMouseEvents`, `unbindMouseEvent`, `W_MouseEventType`
    ///
    ///////////////////////////////////////////////////////////////////
    void bindMouseEvent(W_MouseEventType event_type, MouseCallback callback, std::optional<sf::Mouse::Button> button = std::nullopt);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Clears callback and event sequence from widget.
    ///
    /// @note If event type is `W_MouseEventType::Press` or `W_MouseEventType::Release`
    /// then a button parameter is required.
    ///
    /// @param event_type Which event type to delete
    /// @param button only necessary if event type is `W_MouseEventType::Press` or `W_MouseEventType::Release`
    ///
    /// @see `captureMouseEvents`, `bindMouseEvent`, `W_MouseEventType`
    ///
    ///////////////////////////////////////////////////////////////////
    void unbindMouseEvent(W_MouseEventType event_type, std::optional<sf::Mouse::Button> button = std::nullopt);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates a keyboard sequence event for widget.
    ///
    /// Takes a vector of `Key` objects to define a sequence of key presses,
    /// sequence can also be a single key press.
    ///
    /// @param tag unique string reference to the following sequence
    /// @param key_sequence vector<sfml_widgets::Key> objects defining a sequence, passing
    /// an empty sequence will simply fail to bind
    /// @param callback function that triggers upon matching the sequence, does not take 
    /// any parameters
    ///
    /// @see `captureKeyEvents`, `unbindKeyEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    void bindKeyEvent(const std::string& tag, Sequence key_sequence, SequenceFunction callback);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Clears callback and event key sequence from widget.
    ///
    /// @param tag string reference of the sequence
    /// 
    /// @see `captureKeyEvents`, `bindKeyEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    void unbindKeyEvent(const std::string& tag);

    ///////////////////////////////////////////////////////////////////
    /// @brief Loads a shader for sprites
    ///
    /// Attempts to load a fragment shader from disk or from memory while, any previously
    /// loaded clip shader is cleared even if the process fails.
    ///
    /// Clip shader is responsible for clipping sprites to match the widget's
    /// shape based on what the widget requires, not needed for widgets that
    /// don't use `sf::Sprite` object.
    ///
    /// @note this process would fail if the hardware does no support shaders or
    /// `NOCLIPSHADER` flag is set
    ///
    /// @param shader fragment file path or content from memory, takes default shader by default
    /// @param loading_type appropriate loading type based on shader parameter
    /// @return true if loading was successful, otherwise false.
    ///
    /// @see `resetClipShader`
    ///
    ///////////////////////////////////////////////////////////////////
    bool loadClipShader(const std::string_view& shader = SPRITE_SHADER_FRAG, W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////    
    /// @brief Clears the clipping shader from memory
    ///
    /// @param shader shader that replaces the previous one
    /// @param loading_type appropriate loading type based on shader parameter
    ///
    /// @see `loadClipShader`
    ///
    ///////////////////////////////////////////////////////////////////    
    void resetClipShader(const std::string_view& shader = "", W_LoadType loading_type = W_LoadType::File);

    ///////////////////////////////////////////////////////////////////
    /// @brief Loads a shader for fill color 
    ///
    /// Applied to the shape texture by default, handles colors multiplication
    /// between the shape and any loaded texture, any previously loaded color shader
    /// is cleared even if the process fails.
    ///
    /// @note this process would fail if the hardware does no support shaders or
    /// `NOCOLORSHADER` flag is set
    ///
    /// @param shader fragment file path or content from memory, takes default shader by default
    /// @param loading_type appropriate loading type based on shader parameter
    ///
    /// @return true if loading was successful, otherwise false.
    ///
    /// @see `resetColorShader`
    ///
    ///////////////////////////////////////////////////////////////////    
    bool loadColorShader(const std::string_view& shader = TEXTURE_SHADER_FRAG, W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////    
    /// @brief Clears the coloring shader from memory
    ///
    /// @param shader shader that replaces the previous one
    /// @param loading_type appropriate loading type based on shader parameter
    ///
    /// @see `loadColorShader`
    ///
    ///////////////////////////////////////////////////////////////////    
    void resetColorShader(const std::string_view& shader = "", W_LoadType loading_type = W_LoadType::File);

    ///////////////////////////////////////////////////////////////////    
    /// @brief Loads an image as a fill background
    ///
    /// Attempts to load the widget's texture from memory or from a file,
    /// texture is displayed on the widget's background and can also use a shader
    /// for better color handling.
    ///
    /// @note this process is skipped if `FLAGS::NOBGIMAGE` flag is set
    ///
    /// @param file_path path to image
    /// @param shader fragment file path or content from memory
    /// @param loading_type appropriate loading type based on shader parameter
    ///
    /// @return true if loading was successful, otherwise false.
    ///
    /// @see `loadColorShader`, `clearBackgroundTexture`, `refreshBackgroundTexture`
    ///
    ///////////////////////////////////////////////////////////////////    
    bool setBackgroundTexture(const std::string& file_path, const std::string_view& shader = TEXTURE_SHADER_FRAG, W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////    
    /// @brief Clears Background image texture from memory
    ///
    /// @see `setBackgroundTexture`, `refreshBackgroundTexture`
    ///
    ///////////////////////////////////////////////////////////////////    
    void resetBackgroundTexture();

    ///////////////////////////////////////////////////////////////////
    /// @brief Resamples the widget's texture data to fit the new size
    /// of the shape
    ///
    /// Can be used to restore image quality in case of a corruption instead
    /// of reloading the image from disk.
    ///
    /// @see `setBackgroundTexture`, `resetBackgroundTexture`.
    ///
    ///////////////////////////////////////////////////////////////////
    void refreshBackgroundTexture();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget's shape with rounded corners based
    /// on radius parameter
    ///
    /// @param radius new corner radius value, `0.f` by default
    ///
    ///////////////////////////////////////////////////////////////////
    void setCornerRadius(float radius);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Blocks visual updates of the widget while active
    ///
    /// @param is_stale stale value
    ///
    ///////////////////////////////////////////////////////////////////
    void setStale(bool is_stale);

    ///////////////////////////////////////////////////////////////////
    /// @brief similar to `sf::Transformable::rotate` where it adds to the
    /// current rotation angle of the widget
    ///
    /// @see `setWidgetRotation`
    ///
    ///////////////////////////////////////////////////////////////////
    void rotateWidget(float angle);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a callback for execution after a certain delay
    ///
    /// @note `BLOCKED` flag pauses any queued event timer but can still add or remove
    /// from the queue while in a blocked state
    ///
    /// @param after_ms delay in ms
    /// @param callback function to trigger
    /// @param is_priority takes the current queued event spot, any postponed
    /// event is only paused until this one executes
    ///
    /// @see `clearQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    void addToQueue(sf::Time after_ms, std::function<void()> callback, bool is_priority = false);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a callback for execution after a set number of frames
    /// have passed
    ///
    /// @note `BLOCKED` flag pauses any queued event timer but can still add or remove
    /// from the queue while in a blocked state
    ///
    /// @param after_frames delay in frames
    /// @param callback function to trigger
    /// @param is_priority takes the current queued event spot, any postponed
    /// event is only paused until this one executes
    ///
    /// @see `clearQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    void addToQueue(std::size_t after_frames, std::function<void()> callback, bool is_priority = false);

    ///////////////////////////////////////////////////////////////////
    /// @brief Cancels all queued events for this widget
    ///
    /// @see `addToQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearQueue();

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates and starts a repeated process until callback returns
    /// true
    ///
    /// callback is called everytime the specified period is passed,
    /// the routine terminates if callback returns true or `clearRoutine`
    /// method is called.
    ///
    /// @note creating multiple assigned routines are processed sequentially
    ///
    /// @param ms_period down time before calling callback again
    /// @param callback function to call, must return boolean value to indicate
    /// routine termination
    ///
    /// @see `clearRoutine`
    ///
    ///////////////////////////////////////////////////////////////////
    void addRoutine(sf::Time ms_period, std::function<bool()> callback);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates and starts a repeated process until it terminates
    /// itself
    ///
    /// The event begins the countdown right after assignment and keeps calling
    /// callback for each period until it returns `true` then clears the process
    /// from memory. Multiple routines can run concurrently.
    ///
    /// @note `BLOCKED` flag pauses all routines
    ///
    /// @param frames_period period relative to the number of loops
    /// @param callback function to call, must return boolean value to indicate
    /// routine termination
    ///
    /// @see `clearRoutine`
    ///
    ///////////////////////////////////////////////////////////////////
    void addRoutine(std::size_t frames_period, std::function<bool()> callback);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief clears all routines from the widget's memory
    ///
    /// @see `addRoutine`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearRoutine();

    ///////////////////////////////////////////////////////////////////
    /// @brief Appends this widget to a group of related widgets
    ///
    /// Useful for controlling the `FOCUSED` flag, @see `WidgetGroup` for more info.
    ///
    /// @param new_group shared ptr of type `WidgetGroup` created using `WidgetGroup::make_group`
    ///    
    /// @see `WidgetGroup`, `unlinkGroup`
    ///
    ///////////////////////////////////////////////////////////////////
    bool linkWidget(std::shared_ptr<WidgetGroup>& new_group);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes the widget from it's current group
    ///
    /// @see `WidgetGroup`, `linkWidget`
    ///
    ///////////////////////////////////////////////////////////////////
    bool unlinkWidget();

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns widget to a shared theme object
    ///
    /// A shared theme syncs the assigned styles of multiple types of widgets
    /// while updating them in real time.
    ///
    /// @param t shared ptr of type `WidgetTheme` created using `WidgetTheme::make_theme`
    ///
    /// @see `WidgetTheme`, `unlinkTheme`
    ///
    ///////////////////////////////////////////////////////////////////
    void linkTheme(std::shared_ptr<WidgetTheme>& t);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes syncing of theme with assigned `WidgetTheme` object
    ///
    /// Widget keeps it current style and can still be manually updated
    /// but it would no longer get updates from a shared theme.
    ///
    /// @see `WidgetTheme`, `linkTheme`
    ///
    ///////////////////////////////////////////////////////////////////
    void unlinkTheme();

    ///////////////////////////////////////////////////////////////////
    /// @brief Pulls the widget's style from it's linked theme object
    /// 
    ///////////////////////////////////////////////////////////////////
    virtual void fetchTheme() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Initiates ToolTip for this widget and returns it's pointer
    ///
    /// After hovering on this widget for `delay_ms` period of time, `ToolTip`
    /// is drawn at `abs_position` parameter relative to the main window, each
    /// parameter can be directly updated from the returned pointer alongside
    /// the text content.
    ///
    /// @param abs_position position relative to the widget's window
    /// @param delay_ms hover duration in milliseconds
    /// @param text_size character size used by text
    /// @param style ToolTip's color style
    ///
    /// @see `DisableToolTip`, `ToolTip`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] std::weak_ptr<ToolTip> EnableToolTip(sf::Vector2f abs_position,
                                         sf::Time delay_ms = s_HOVER_DURATION,
                                         unsigned int text_size = parameters::CHAR_SIZE,
                                         ToolTipStyle style = Theme::tooltip_style::TOOLTIP_DEFAULT_DARK);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears the current ToolTip instance
    ///
    /// Any weak pointer pointing at the generated ToolTip turns expired and
    /// `ToolTip` is no longer drawn after hovering, it can be enabled again but
    /// its parameters are reset back to default.
    ///
    /// @see `EnableToolTip`, `ToolTip`
    ///
    ///////////////////////////////////////////////////////////////////
    void DisableToolTip();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget position relative to it's local {0, 0} coordinates
    ///
    /// @see `setWidgetPosition`, `getAbsolutePosition`
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::Vector2f getWidgetPosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget size
    ///
    /// @see `setWidgetSize`
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::Vector2f getWidgetSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the widget's position relative to the window
    ///
    /// @see `setWidgetPosition`, `getWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::Vector2f getAbsolutePosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief returns the current rotation angle of the widget
    ///
    /// @see `setWidgetRotation`, `rotateWidget`
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Angle getWidgetRotation() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget bounds relative to root view
    ///
    /// Position in bounds would be relative to the root frame if widget
    /// is placed in one, otherwise, returns position relative to the window 
    /// view if widget is placed directly on the window.
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::FloatRect getBounds() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget bounds relative to the main window
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::FloatRect getAbsoluteBounds() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to the main window
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::RenderWindow* getRootWindow() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to the root frame if it exists
    ///
    ///////////////////////////////////////////////////////////////////
    const Frame* getRootFrame() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for a state or a flag's current condition in this widget
    ///
    /// @return true if at least 1 bit from parameter is set internally,
    /// otherwise false.
    ///
    /// @see `checkFlag`, `isFlags`
    ///
    ///////////////////////////////////////////////////////////////////
    bool isFlag(FLAGS bits) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for multiple states or flags current condition in this widget
    ///
    /// @return true if all bits from parameter are set internally,
    /// otherwise false.
    ///
    /// @see `checkFlags`, `isFlag`
    ///
    ///////////////////////////////////////////////////////////////////
    bool isFlags(FLAGS bits) const;

protected:
    ///////////////////////////////////////////////////////////////////
    /// @brief Optional pre-process for when checking for mouse events
    ///
    /// `captureMouseEvents` calls this before doing it's state update process if event is
    /// not consumed yet, if true is returned then the event is consumed, this widget can still
    /// trigger bind callbacks and postProcess afterwards but other widgets cannot call any
    /// callbacks until next loop (unless if `s_GUARD_EVENT` macro is false).
    ///
    /// @note Returning true consumes the event for this loop.
    /// @note Flag states are updated after this method is called even if true is returned.
    ///
    /// @param event reference to event object passed in `captureMouseEvents`
    /// @param mouse_position position relative to the window view
    ///
    /// @return true if the event was consumed, otherwise false
    ///
    /// @see `captureMouseEvents`, `postProcessMouseEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual bool preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Optional post-process for when checking for mouse events
    ///
    /// `captureMouseEvents` calls this after doing it's state update process,
    /// takes read only details parameter for info about the current event.
    ///
    /// @note Returning true consumes the event for this loop.
    /// @note This method will only run if the event is not consumed by other widgets
    /// or the event was just consumed by this widget (will always run if `s_GUARD_EVENT` is false).
    /// @note Flag states are updated before this method is called (except `FOCUSED` flag)
    ///
    /// @param details details object
    /// @param event reference to event object passed in `captureMouseEvents`
    /// @param mouse_position position relative to the window view
    ///
    /// @return true if the event was consumed, otherwise false
    ///
    /// @see `captureMouseEvents`, `preProcessMouseEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual bool postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Optional pre-process for when checking for key events
    ///
    /// `captureKeyEvents` calls this method before bind event, if true is returned then
    /// the event is consumed but bind callback and postProcess are still processed.
    /// 
    /// @param event reference to event object passed in `captureKeyEvents`
    ///
    /// @return true if the event was consumed, otherwise false
    ///
    /// `captureKeyEvents`, `postProcessKeyEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual bool preProcessKeyEvents(const sf::Event& event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Optional post-process for when checking for key events
    ///
    /// `captureKeyEvents` calls this method after bind callback as long as
    /// the event is not consumed by other widgets (will always run if `s_GUARD_EVENT` is false).
    ///
    /// @param event reference to event object passed in `captureKeyEvents`
    ///
    /// @return true if event is consumed, otherwise false
    ///
    /// `captureKeyEvents`, `preProcessKeyEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual bool postProcessKeyEvents(const sf::Event& event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Optional post-process for `configure` method
    ///
    /// @param mask bits passed in `configure`
    ///
    /// @see `configure`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void config(FLAGS mask);

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the shape and updates the necessary functions, called
    /// within `draw`
    ///
    /// @see `draw`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void drawWidget() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the widget,
    /// called in `setWidgetPosition`
    ///
    /// @see `setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updatePosition() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the widget,
    /// called in `setWidgetPosition`
    ///
    /// @see `setWidgetSize`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateSize() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief matches the current angle value with the rest of the widget
    ///
    /// @see `rotateWidget`, `updateWidgetRotation`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateRotation() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates widget colors and shaders depending on the current
    /// state
    ///
    /// Called in `draw` method, The default tested states are:
    /// `BLOCKED`, `ALPHAREDUCED`, `ACTIVE`, `HOVERED` respectively.
    ///
    /// @note this is the most optimal function to call any once per loop process
    /// before beginning to draw (such as clearing a rendering object) AS LONG AS
    /// `LONGSKIP` flag is not set.
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateState() = 0; 

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs shared theme with widget
    ///
    /// @note This method can be skipped if `LONGSKIP` is set
    /// by the derived class
    ///
    /// @see `linkTheme`, `unlinkTheme`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateTheme() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for clipping shader
    ///
    /// Best called after an update in a shader parameter, clip shader handles
    /// clipping edges on sprites.
    ///
    /// @see `loadClipShader`, `resetClipShader`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateClipShader();

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in any of the shader parameters.
    ///
    /// @see `loadColorShader`, `resetColorShader`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateColorShader();

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Returns current mouse position based on the window's view
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2i getMousePosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks whether the mouse is hovered on widget or not
    ///
    /// Takes the mouse's position on the window to check whether it's hovering
    /// on the widget.
    ///
    /// @param mouse_position Mouse view position relative to window
    ///
    /// @return true if position exists within the widget's bounds,
    /// otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////
    bool isHovered(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates hovered and transparent states for the widget, called
    /// while checking for mouse events
    ///
    /// @param event event reference
    /// @param mouse_position mouse coordinates relative to window
    ///
    ///////////////////////////////////////////////////////////////////
    void updateMouseState(const sf::Event& event, sf::Vector2i mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates and calls internal queues and routines, called while
    /// drawing
    ///
    /// @note this method can be skipped if `LONGSKIP` flag is set by
    /// a derived class
    ///
    /// @see `addToQueue`, `addRoutine`
    ///
    ///////////////////////////////////////////////////////////////////
    void checkQueue();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates original dimensions of the widget and the root window
    /// size
    ///
    /// Original dimensions are used for better accuracy in widget
    /// position and size when a resize event occurs.
    ///
    ///////////////////////////////////////////////////////////////////
    void updateDimensions();

    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;


///////////////////////////////////////////////////////////////////
/// Member Variables
///////////////////////////////////////////////////////////////////
protected:
    // common members
    RenderContext m_master;                                                         //!< Root object that draws the widget
    sf::Vector2f m_position, m_size;                                                //!< Current local dimensions of the widget
    sf::Vector2f m_originalPosition, m_originalSize;                                //!< User-set dimensions for the widget, used for more precise resize ratio
    sf::Vector2f m_windowSize;                                                      //!< Window construction-time size, used for more precise resize ratio
    
    // pointer members
    std::weak_ptr<WidgetGroup> m_groupPtr;                                          //!< Group that shares `FOCUSED` state
    std::weak_ptr<WidgetTheme> m_themePtr;                                          //!< Global style from external pointer
    std::unique_ptr<sf::Shader> m_bgShaderPtr;                                      //!< Shader responsible for bg image's opacity
    std::unique_ptr<sf::Shader> m_clipShaderPtr;                                    //!< Needed for widgets that use sprites
    std::unique_ptr<sf::Texture> m_bgTexturePtr;                                    //!< Background image texture, allocated in loadBgImage
    std::shared_ptr<ToolTip> m_ToolTipPtr;                                          //!< ToolTip object pointer for this widget

    // unique members and flags
    sf::String m_bgImagePath;                                                       //!< Path to bg resource on disk
    CaptureSequence m_binds;                                                        //!< Stores keyboard sequences and triggers keyboard events
    FLAGS m_flags{FLAGS::MODIFIED};                                                 //!< Stores internal and external states of the widget, start with MODIFIED for initial style assignment
    bool m_stale{false};                                                            //!< Blocks visual updates for widget while set
    
    // numeric members
    float m_radius{0.f};                                                            //!< Corner radius, `0.f` for rectangle shape
    unsigned int m_cornerPoints{1};                                                 //!< Number of points in each corner, 1 for rectangle shape (automatically updates to `Widget::s_CORNER_POINTS`)
    float m_opacity{1.f};                                                           //!< Transparency ratio of the widget, `1.f` for fully transparent
    std::size_t m_generationToken{constants::KSTALE};                               //!< Used with `m_themePtr` to check if optimal style for widget was modified

    // active style members
    sf::Color* m_currentBgColorPtr{nullptr};                                        //!< Pointer to current fill color
    sf::Color* m_currentOutlineColorPtr{nullptr};                                   //!< Pointer to current outline color
    float* m_currentOutlineThicknessPtr{nullptr};                                   //!< Pointer to current outline color

private:
    /* events */
    // queues
    std::unique_ptr<W_QueueEvent> m_restrictEvent;                                  //!< For key event restriction
    std::deque<W_QueueEvent> m_events;                                              //!< Queued events
    std::vector<W_RepeatEvent> m_repeatEvents;                                      //!< Routine events, they run concurrently

    // memory
    std::optional<sf::Mouse::Button> m_memButton{std::nullopt};                     //!< Memorizes recently clicked mouse button

    // bind callbacks
    std::unordered_map<sf::Mouse::Button, MouseCallback> m_pressEvents;             //!< Stores assigned mouse press events
    std::unordered_map<sf::Mouse::Button, MouseCallback> m_releaseEvents;           //!< Stored assigned mouse release events
    MouseCallback m_holdCallBack{nullptr};                                          //!< Called while holding `Widget::s_FOCUS_BUTTON` on widget
    MouseCallback m_scrollCallBack{nullptr};                                        //!< Called when scrolling the mouse wheel while hovering on widget
    MouseCallback m_gainhoverCallBack{nullptr};                                     //!< Called once when the widget is first hovered
    MouseCallback m_losehoverCallBack{nullptr};                                     //!< Called once when the widget is no longer hovered
    MouseCallback m_gainFocusCallBack{nullptr};                                     //!< Called automatically when widget gains focus
    MouseCallback m_loseFocusCallBack{nullptr};                                     //!< Called automatically when widget loses focus


///////////////////////////////////////////////////////////////////
// Static Methods
///////////////////////////////////////////////////////////////////
private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Handles resizing of widgets during a resize event, called
    /// by an existing widget
    ///
    /// @param win_ptr pointer to the resized window
    /// @param resize_event resize event
    ///
    ///////////////////////////////////////////////////////////////////
    static void resizeWidgets(const sf::RenderWindow* win_ptr, const sf::Event::Resized& resize_event);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a widget to receive the focus resource at the end
    /// of the loop
    ///
    /// Called when a widget is clicked with any mouse button,
    /// focus and focuslost events trigger callbacks assigned manually,
    /// widgets can be grouped together to maintain focus as one widget.
    ///
    /// Only one widget can be focused at a time, and by default, only
    /// one assignment takes place per loop unless if `Widget::s_GUARD_EVENT`
    /// is set to false then the assignment becomes more unpredictable but it would
    /// still only call one focus callback.
    ///
    /// @note This process is done automatically by the widget class.
    ///
    /// @see `unfocusWidget`, `updateFocus`, `WidgetGroup`
    ///
    /// @param w widget to focus
    /// @param event optional event to assign with the callback
    ///
    ///////////////////////////////////////////////////////////////////
    static void focusWidget(Widget& w, std::optional<sf::Event> event) noexcept;

    ///////////////////////////////////////////////////////////////////
    /// @brief Queues the widget for unfocus event if it's currently focused
    ///
    /// @see `focusWidget`, `updateFocus`, `WidgetGroup`
    ///
    /// @param w widget to unfocus
    /// @param event optional event to assign with the callback
    ///
    ///////////////////////////////////////////////////////////////////
    static void unfocusWidget(Widget& w, std::optional<sf::Event> event) noexcept;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates `FOCUSED` flag for widgets and triggers focus or
    /// focuslost callback
    ///
    /// Called in `draw` method, cannot be skipped.
    ///
    /// @see `focusWidget`, `unfocusWidget`, `WidgetGroup`
    ///
    ///////////////////////////////////////////////////////////////////
    static void updateFocus();

///////////////////////////////////////////////////////////////////
/// Static Variables
///////////////////////////////////////////////////////////////////

    inline static std::vector<Widget*> s_widgets;                                   //!< Reference to every live widget
    inline static sf::Vector2f s_windowSize{0.f, 0.f};                              //!< Prevents calling resize method multiple times for no purpose

    inline static Widget* s_focusedWidgetPtr{nullptr};                              //!< Pointer to currently focused widget
    inline static Widget* s_widgetToFocusPtr{nullptr};                              //!< Pointer to the upcoming focused widget, this only exists until focus resource is updated

    inline static std::optional<sf::Event> s_memFocusEvent{std::nullopt};           //!< Cached event for widget focus event
    inline static std::optional<sf::Event> s_memUnfocusEvent{std::nullopt};         //!< Cached event for widget unfocus event

    inline static bool s_FfoundFocusedWidget{false};                                //!< Tells widget to bother updating the focused widget
    inline static bool s_FrequestUnfocus{false};                                    //!< Requests for the focused widget to lose the resource
    inline static bool s_FrequestFocusUpdate{false};                                //!< Requests for a new widget to obtain the resource

    inline static bool s_mouseEventUsed{false};                                     //!< Indicates whether a mouse event was used in the current loop, works with `s_GUARD_EVENT`
    inline static bool s_keyEventUsed{false};                                       //!< Indicates whether a key event was used in the current loop, works with `s_GUARD_EVENT`
    inline static bool s_setOriginals{true};                                        //!< Widgets update their original dimensions if set, otherwise skips the process

public:
    static constexpr sf::Mouse::Button s_FOCUS_BUTTON{sf::Mouse::Button::Left};     //!< Mouse button that triggers focus and hold events
    static constexpr sf::Time s_RESTRICTION_DURATION{sf::Time::Zero};               //!< Duration of which a widget that triggered a key event remains limited
    static constexpr sf::Time s_HOVER_DURATION{sf::milliseconds(500)};              //!< Hovering duration before tooltip is drawn
    static constexpr unsigned int s_CORNER_POINTS{16};                              //!< Number of vertices for a single rounded corner in a widget

    static constexpr bool s_GUARD_EVENT{true};                                      //!< Blocks multiple event triggers in a single loop
    static constexpr bool s_RESTRICT_AFTER_KEYEVENT{false};                         //!< Blocks the widget from triggering any key events for `s_RESTRICTION_DURATION` period after using a key event
    static constexpr bool s_UNSELECT_WHEN_HELD{true};                               //!< Prevents a widget from triggering mouse release callback at the end of a hold event

///////////////////////////////////////////////////////////////////
/// Widget objects
///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// @brief Countdown type for queued events and routines
    ///
    /// @see `W_QueueEvent`, `W_RepeatEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    enum class W_CounterType
    {
        Time,
        Frames
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief All available types for mouse events
    ///
    /// @see `bindMouseEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    enum class W_MouseEventType
    {
        Press,
        Release,
        Hold,
        Scroll,
        HoverSet,
        HoverLost,
        FocusSet,
        FocusLost
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Which loading process should widget use when receiving
    /// a shader
    ///
    /// @see `loadClipShader`, `loadColorShader`
    ///
    ///////////////////////////////////////////////////////////////////
    enum class W_LoadType
    {
        File,
        Memory
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Used in widget class to pass event information more
    /// efficiently.
    ///
    /// @see `captureMouseEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    struct W_MouseEventDetails
    {
        std::optional<W_MouseEventType> event_type;
        std::optional<sf::Mouse::Button> button;
        float scroll_delta{0.f};
        bool is_used{false};
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Stores queued callbacks for `Widget`
    ///
    /// Stored callbacks trigger after the specified delay is reached
    /// within the widget, delay can be paused if a more priority queued
    /// event is pushed onto the stack.
    ///
    /// @see `addToQueue`, `checkQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    struct W_QueueEvent
    {
        W_QueueEvent(sf::Time ms_delay, std::function<void()> callback);
        W_QueueEvent(std::size_t frames_delay, std::function<void()> callback);

        ///////////////////////////////////////////////////////////////////
        /// @brief Updates the counter and triggers callback
        ///
        ///////////////////////////////////////////////////////////////////
        bool advance();

        ///////////////////////////////////////////////////////////////////
        /// @brief for Time based queues, pauses the internal clock
        ///
        ///////////////////////////////////////////////////////////////////
        void pause();

    private:
        W_CounterType m_type;
        sf::Clock m_timer;
        std::function<void()> m_callback;
        sf::Time m_delay{sf::Time::Zero};
        std::size_t m_framesCount{0};
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Stores a routine information
    ///
    /// Calls the designated callback for each period until the process returns
    /// true, a routine can count using time or loops and multiple routines can run
    /// concurrently in a single widget.
    ///
    /// @see `addRoutine`, `checkQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    struct W_RepeatEvent
    {
        W_RepeatEvent(sf::Time ms_period, std::function<bool()> callback);
        W_RepeatEvent(std::size_t frames_period, std::function<bool()> callback);

        ///////////////////////////////////////////////////////////////////
        /// @brief Updates the counter and triggers callback
        ///
        ///////////////////////////////////////////////////////////////////
        bool advance();

    private:
        W_CounterType m_type;
        sf::Time m_period{sf::Time::Zero};
        std::size_t m_framesPeriod{0};
        std::size_t m_framesCounter{0};
        std::function<bool()> m_callback;
        sf::Clock m_timer;
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief static class for estimating the maximum height for text
    ///
    ///////////////////////////////////////////////////////////////////
    class W_CharHeight
    {
    public:
        ///////////////////////////////////////////////////////////////////
        /// @brief returns the estimated maximum height of a `sf::Text` object
        ///
        /// @param text_font current font
        /// @param char_size text's current character size
        ///
        ///////////////////////////////////////////////////////////////////
        static float getHeight(const sf::Font& text_font, unsigned int char_size);
    private:
        static sf::Text Ag; //!< Ag
    };

protected:
///////////////////////////////////////////////////////////////////
/// @brief Static class for storing images used within widgets by paths
///
/// @see `ImageReferenceCounter`
///
///////////////////////////////////////////////////////////////////
struct ImageCache
{
    static ImageCache& getInstance();

    ///////////////////////////////////////////////////////////////////
    /// @brief Fetch image pointer from memory or loads from disk if not
    /// found
    ///
    /// @param image_path relative path to image as a look up key
    /// @param increment_ref specify whether to increment reference counter of image,
    /// set to false if widget already reference's the image.
    ///
    /// @return read only image pointer if image is found on disk or memory,
    /// otherwise nullopt
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<const sf::Image*> getImage(const std::string& image_path, bool increment_ref = true);

    ///////////////////////////////////////////////////////////////////
    /// @brief Called by widgets after clearing their texture image
    ///
    /// Decrements reference counter for a pre-loaded image, and frees
    /// the image memory if no widget is using it.
    ///
    ///////////////////////////////////////////////////////////////////
    void decrementRefCounter(const std::string& image_path);

private:
    ImageCache() = default;
    ImageCache(const ImageCache&) = delete;
    ImageCache& operator=(const ImageCache&) = delete;
    ImageCache(ImageCache&&) = delete;
    ImageCache& operator=(ImageCache&&) = delete;

    struct ImageReferenceCounter
    {
        sf::Image image;
        std::size_t ref_count;
    };

    std::mutex m_guard;
    std::unordered_map<std::string, ImageReferenceCounter> m_cachedImages;
};
};
}
#endif