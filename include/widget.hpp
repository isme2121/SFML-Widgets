#pragma once
#include "widgets.hpp"
#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Base class for all widgets
///
/// @see `WidgetGroup`, `Button`, `Frame`, `GridCanvas`, `ImageFrame`, `CheckBox`, `Entry`
///
///////////////////////////////////////////////////////////////////
class Widget : protected sf::Shape
{
public:

    enum class W_MouseEventType : int;
    enum class W_CounterType : int;
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

    Widget(Widget&& other) noexcept;
    Widget& operator=(Widget&& other) noexcept;
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
    /// Use | operator to pass multiple flags at once.
    ///
    /// @param  flags modifiable state flags, @see `MODIFIABLE`
    ///
    ///////////////////////////////////////////////////////////////////
    void configure(FLAGS flags);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets widget's position based on it's master's view
    ///
    /// @see `getWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void setWidgetPosition(const sf::Vector2f pos);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets widget's size
    ///
    /// @see `getWidgetSize`, 
    ///
    ///////////////////////////////////////////////////////////////////
    void setWidgetSize(const sf::Vector2u size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events
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
    virtual bool captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position = std::nullopt) = 0;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    ///
    /// @warning Multiple widgets with the same sequence can all fire at once.
    ///
    /// @see `bindShortCut`, `unbindShortCut`, `resizeWidgets`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual bool captureKeyEvents(const sf::Event& event) = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a mouse event with a callback to this widget
    ///
    /// Events captured by `captureMouseEvents` such as pressing a
    /// mouse button or scrolling are assigned here.
    ///
    /// Possible active state for each callback call:
    /// `~ = not guaranteed`
    ///
    /// - PRESS: `FOCUSED`, `HOVERED`, `SELECTED`, `ACTIVE~`
    ///
    /// - RELEASE: `HOVERED`, `FOCUSED`, `SELECTED`
    ///
    /// - HOLD: `ACTIVE`, `FOCUSED`, `SELECTED~`
    ///
    /// - SCROLL: `HOVERED`
    ///
    /// - FOCUSSET: `FOCUSED`
    ///
    /// - FOCUSLOST: ``
    ///
    /// @param event_type Which event type triggers the callback
    /// @param callback function call during event match, can take `const sf::Event`
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
    /// If event type is `W_MouseEventType::Press` or `W_MouseEventType::Release`
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
    /// Takes a vector of `sf::Keyboard::Scancode` objects to define a sequence
    /// of key presses, sequence can also be a single key press.
    ///
    /// @param tag string reference to the following sequence
    /// @param key_sequence vector of `sf::Keyboard::Scancode` objects defining a sequence
    /// @param callback function that triggers upon matching the sequence, does not take 
    /// any parameters
    ///
    /// @see `captureKeyEvents`, `unbindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    void bindShortCut(const std::string& tag, Sequence key_sequence, SequenceFunction callback);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Clears callback and event key sequence from widget.
    ///
    /// @param tag string reference of te sequence
    /// 
    /// @see `captureKeyEvents`, `bindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    void unbindShortCut(const std::string& tag);

    ///////////////////////////////////////////////////////////////////
    /// @brief Loads a shader for handling sprites
    ///
    /// Takes the string path to a .frag file to reload shader settings
    /// for image display, required shader is loaded by default for the
    /// proper widget types.
    ///
    /// @param shader_path fragment file path
    ///
    /// @return true if loading was successful, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////
    bool loadClipShader(const std::string& shader_path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Loads shader parameters from fragment file for background
    /// texture
    ///
    /// Applied to the shape texture, handles transparency and color multiplication
    /// for the background texture.
    ///
    /// @see `loadClipShader`, `setBackGroundTexture`, `clearBackGroundTexture`,
    /// `updateColorShader`
    ///
    /// @param shader_path fragment file path
    ///
    /// @return true if loading was successful, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////    
    bool loadColorShader(const std::string& shader_path);

    ///////////////////////////////////////////////////////////////////    
    /// @brief Clears color shader if available
    ///
    /// @see `loadColorShader`
    ///
    ///////////////////////////////////////////////////////////////////    
    void resetColorShader();

    ///////////////////////////////////////////////////////////////////    
    /// @brief Assigns an image data to the widget and draws it in the background
    ///
    /// Attempts to load the image data as a fill texture for the widget,
    /// it also can initiate a coloring shader if a path is provided.
    ///
    /// The shader handles transparency and color multiplication falses.
    ///
    /// @see `loadColorShader`, `clearBackGroundTexture`, `TEXTURE_COLOR_SHADER_PATH`
    ///
    /// @param file_path string path leading to an image on disk
    /// @param shader_path path to fragment file
    ///
    /// @return true if loading was successful, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////    
    bool setBackGroundTexture(const std::string& file_path, const std::string& shader_path = "");

    ///////////////////////////////////////////////////////////////////    
    /// @brief Clears background texture data alongside the color shader
    ///
    /// @param clear_color_shader choose to reset color shader as well with true,
    /// otherwise the shader remains applied while drawing.
    ///
    /// @see `loadColorShader`, `setBackGroundTexture`,
    /// `clearColorShader`, `TEXTURE_COLOR_SHADER_PATH`
    ///
    ///////////////////////////////////////////////////////////////////    
    void resetBackGroundTexture(bool clear_color_shader = false);

    ///////////////////////////////////////////////////////////////////
    /// @brief Resamples the widget's texture data to fit the new size
    /// of the shape
    ///
    /// @see `setBackGroundTexture`, `clearBackGroundTexture`.
    ///
    ///////////////////////////////////////////////////////////////////
    void updateBackGroundSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the shape of the button with rounded corners
    ///
    /// @param radius new corner radius value, radius is 0 by default
    ///
    ///////////////////////////////////////////////////////////////////
    void setCornerRadius(float radius);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief similar to `sf::Transformable::rotate` where it adds to the
    /// current rotation angle of the widget
    ///
    ///////////////////////////////////////////////////////////////////
    void rotateWidget(sf::Angle angle);

    ///////////////////////////////////////////////////////////////////
    /// @brief similar to `sf::Transformable::setRotation` where it overwrites
    /// the widget's rotation angle
    ///
    ///////////////////////////////////////////////////////////////////
    void setWidgetRotation(sf::Angle new_angle);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a callback for execution after a certain delay
    ///
    /// @param after_ms delay in ms
    /// @param callback function to trigger
    /// @param is_priority pauses any on going queues and begins counting
    /// for this callback
    ///
    /// @see `checkQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    void addToQueue(sf::Time after_ms, std::function<void()> callback, bool is_priority = false);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a callback for execution after a set number of frames
    /// have passed
    ///
    /// @param after_frames delay in frames
    /// @param callback function to trigger
    /// @param is_priority pauses any on going queues and begins counting
    /// for this callback
    ///
    /// @see `checkQueue`
    ///
    ///////////////////////////////////////////////////////////////////
    void addToQueue(std::size_t after_frames, std::function<void()> callback, bool is_priority = false);

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
    ///////////////////////////////////////////////////////////////////
    void addRoutine(sf::Time ms_period, std::function<bool()> callback);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates and starts a repeated process until callback returns
    /// true
    ///
    /// callback is called after a set number of frames have passed,
    /// the routine terminates if callback returns true or `clearRoutine`
    /// method is called.
    ///
    /// @note creating multiple assigned routines are processed sequentially
    ///
    /// @param frames_period number of frames to wait before calling callback again
    /// @param callback function to call, must return boolean value to indicate
    /// routine termination
    ///
    ///////////////////////////////////////////////////////////////////
    void addRoutine(std::size_t frames_period, std::function<bool()> callback);
    
    ///////////////////////////////////////////////////////////////////
    /// @brief clears all routines from the widget's memory
    ///
    ///////////////////////////////////////////////////////////////////
    void clearRoutine();

    ///////////////////////////////////////////////////////////////////
    /// @brief Appends this widget to a group of related widgets
    ///
    /// Widgets of the same group avoid calling unfocus callbacks
    /// as long as they still own `FOCUSED` resource but the focus callback
    /// still gets called for the newly focused widget (depends on `WidgetGroup`
    /// object's public flags).
    ///
    /// @see `WidgetGroup`, `unlinkGroup`, `focusWidget`, `unfocusWidget`
    ///
    /// @param new_group pointer to `WidgetGroup` object
    ///
    ///////////////////////////////////////////////////////////////////
    void linkWidget(std::shared_ptr<WidgetGroup>& new_group);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes the widget from it's current group
    ///
    /// @see `WidgetGroup`, `linkWidget`
    ///
    ///////////////////////////////////////////////////////////////////
    void unlinkWidget();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget position
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
    const sf::Vector2u getWidgetSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget screen position
    ///
    /// Returns the widget's position on the main window.
    ///
    /// @see `getWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::Vector2f getAbsolutePosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief returns the current rotation angle of the widget
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Angle getWidgetRotation() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget bounds based on local view
    ///
    /// Position in bounds would be relative to the root frame if widget
    /// is placed in one, otherwise, returns position relative to the window 
    /// view if widget is placed on the window.
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::FloatRect getBounds() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns widget bounds relative to the window
    ///
    ///////////////////////////////////////////////////////////////////
    const sf::FloatRect getAbsoluteBounds() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns current mouse position based on the window's view
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2i getMousePosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks whether mouse is hovered on widget or not
    ///
    /// Takes the mouse's position on the window to check whether it's hovering
    /// on the widget.
    ///
    /// @param mouse_position Mouse view position based on window
    ///
    /// @return true if position exists within the widget's bounds,
    /// otherwise false.
    ///
    /// @see `isFlag`
    ///
    ///////////////////////////////////////////////////////////////////
    bool isHovered(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Verifies if input flag is set within the widget, can
    /// be used to check for all available flags
    ///
    /// @param bits bit or multiple bits to check
    ///
    /// @return true if at least 1 bit from parameter is set internally,
    /// otherwise false.
    ///
    /// @see `checkFlag`, `isHovered`
    ///
    ///////////////////////////////////////////////////////////////////
    bool isFlag(FLAGS bits) const;

protected:
    ///////////////////////////////////////////////////////////////////
    /// @brief Main method for triggering mouse callbacks from a widget
    ///
    /// This method is designed to be called in `Widget::captureMouseEvents`
    /// override.
    ///
    /// Updates internal state while triggering the proper callback if the
    /// event condition is met.
    ///
    /// @param event event object
    /// @param mouse_position current mouse position based on window view
    ///
    /// @return `W_MouseEventDetails` object for callback information
    ///
    /// @see `captureMouseEvents`, `updateMouseState`
    ///
    ///////////////////////////////////////////////////////////////////
    W_MouseEventDetails mouseCallBackEvent(const sf::Event& event, const sf::Vector2i& mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Called in `mouseCallBackEvent` to update hovering state
    ///
    /// Updates HOVERED and TRANSPARENT states.
    ///
    /// @see `mouseCallBackEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateMouseState(const sf::Event& event, sf::Vector2i mouse_position);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates internal events' periods such as queues and routines
    /// and calling their callbacks
    ///
    /// @see `addToQueue`, `addRoutine`
    ///
    ///////////////////////////////////////////////////////////////////
    void checkQueue();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates `FOCUSED` flag for widgets and triggers focus or
    /// focuslost callback
    ///
    /// Called in `draw` method.
    ///
    /// @see `focusWidget`, `unfocusWidget`, `WidgetGroup`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateFocus();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the shape and updates the necessary functions, called
    /// within `draw`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void drawWidget() = 0;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Used to update child widgets for container type Widget or
    /// add specific rules after updating flags, called in `configure`
    ///
    /// @param mask bits passed in `configure`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void config(FLAGS mask) = 0;

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
    ///////////////////////////////////////////////////////////////////
    virtual void updateRotation() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates widget colors and shaders depending on the current
    /// state
    ///
    /// Called in `draw` method, The default tested states are:
    /// `BLOCKED`, `TRANSPARENT`, `ACTIVE`, `HOVERED` respectively.
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateState() = 0; 

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for clipping shader
    ///
    /// Best called after an update in a shader parameter, clip shader handles
    /// clipping edges on sprites.
    ///
    /// @see `loadClipShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateClipShader() = 0;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in a shader parameters.
    ///
    /// @see `loadColorShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    virtual void updateColorShader() = 0;

    ///////////////////////////////////////////////////////////////////
    // Protected Member Variables
    ///////////////////////////////////////////////////////////////////

    std::weak_ptr<WidgetGroup> m_group;                                             //!< Group that shares `FOCUSED` state
    RenderContext m_master;                                                         //!< Root object that draws the widget
    sf::Shader m_clipShader;                                                        //!< for sprite edge clipping

    std::unique_ptr<sf::Shader> m_bgShaderPtr;                                      //!< Shader responsible for bg image's opacity
    std::unique_ptr<sf::Texture> m_bgTexturePtr;                                    //!< Background image texture, allocated in loadBgImage
    std::string m_bgTexturePath;                                                    //!< Path to bg resource on disk

    sf::Vector2u m_size;
    sf::Vector2f m_position;
    FLAGS m_flags = FLAGS::MODIFIED;                                                //!< Stores internal and external states of the widget
    CaptureSequence m_binds;                                                        //!< Stores keyboard sequences
    float m_radius = 0.f;                                                           //!< Corner radius
    unsigned int m_cornerPoints = 1;                                                //!< Number of points in each corner
    float m_opacity = 1.f;                                                          //!< Transparency ratio of the widget
    sf::Color* m_currentBgColorPtr = nullptr;                                       //!< Pointer to current fill color
    sf::Color* m_currentOutlineColorPtr = nullptr;                                  //!< Pointer to current outline color
    float* m_currentOutlineThicknessPtr = nullptr;                                  //!< Pointer to current outline color

private:
    ///////////////////////////////////////////////////////////////////
    /// Static Variables
    ///////////////////////////////////////////////////////////////////

    inline static std::vector<Widget*> s_widgets;                                   //!< Reference to every live widget

    inline static Widget* s_focusedWidgetPtr = nullptr;                             //!< Currently focused widget
    inline static Widget* s_widgetToFocusPtr = nullptr;                             //!< Alive for one frame as widget is focused at the end of the loop
    inline static std::optional<const sf::Event> s_memFocusEvent = std::nullopt;    //!< Cached event for widget focus event
    inline static std::optional<const sf::Event> s_memUnfocusEvent = std::nullopt;  //!< Cached event for widget unfocus event
    inline static bool s_FrequestUnfocus = false;                                   //!< Calling for currently focused widget to lose focus
    inline static bool s_FfoundFocusedWidget = false;                               //!< Informs the class that a focused widget is already found for this loop
    inline static bool s_FrequestFocusUpdate = false;                               //!< Calls for an update on focus status for widgets

    ///////////////////////////////////////////////////////////////////
    /// Private Member Variables
    ///////////////////////////////////////////////////////////////////

    std::deque<W_QueueEvent> m_events;                                              //!< Queued events
    std::vector<W_RepeatEvent> m_repeatEvents;
    std::optional<sf::Mouse::Button> m_memButton = std::nullopt;                    //!< Memorizes recently clicked mouse button
    std::unordered_map<sf::Mouse::Button, MouseCallback> m_pressEvents;             //!< For mouse press events
    std::unordered_map<sf::Mouse::Button, MouseCallback> m_releaseEvents;           //!< For mouse release events
    MouseCallback m_holdCallBack = nullptr;                                         //!< Called while holding `ACTIVE_MOUSE_BUTTON` on widget
    MouseCallback m_scrollCallBack = nullptr;                                       //!< Called when scrolling the mouse wheel while hovering on widget
    MouseCallback m_gainhoverCallBack = nullptr;                                    //!< Called once when the widget is first hovered
    MouseCallback m_losehoverCallBack = nullptr;                                    //!< Called once when the widget is no longer hovered
    MouseCallback m_gainFocusCallBack = nullptr;                                    //!< Called automatically when widget gains focus
    MouseCallback m_loseFocusCallBack = nullptr;                                    //!< Called automatically when widget loses focus

    friend class Frame;
    friend class WidgetGroup;

    ///////////////////////////////////////////////////////////////////
    // SFML's drawing methods
    ///////////////////////////////////////////////////////////////////

    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;

public:
    ///////////////////////////////////////////////////////////////////
    // Public Static Methods
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a widget to receive the focus resource at the end
    /// of the loop
    ///
    /// Focus and focuslost events trigger callbacks assigned by the user,
    /// widgets can be grouped together to maintain focus as one widget.
    ///
    /// This process is done automatically by the widget class but it's
    /// valid to use outside the class.
    ///
    /// @see `unfocusWidget`, `updateFocus`, `WidgetGroup`
    ///
    /// @param w widget to focus
    /// @param _e optional event to assign with the callback
    ///
    ///////////////////////////////////////////////////////////////////
    static void focusWidget(Widget& w, std::optional<const sf::Event> _e) noexcept;

    ///////////////////////////////////////////////////////////////////
    /// @brief Queues the widget for unfocus event if it's currently focused
    ///
    /// @see `focusWidget`, `updateFocus`, `WidgetGroup`
    ///
    /// @param w widget to unfocus
    /// @param _e optional event to assign with the callback
    ///
    ///////////////////////////////////////////////////////////////////
    static void unfocusWidget(Widget& w, std::optional<const sf::Event> _e) noexcept;

    ///////////////////////////////////////////////////////////////////
    // Public Widget objects
    ///////////////////////////////////////////////////////////////////
    
    enum class W_CounterType
    {
        Time,
        Frames
    };

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
    /// @brief Used in widget class to pass event information more
    /// efficiently.
    ///
    /// @see `Widget::captureMouseEvents`
    ///
    ///////////////////////////////////////////////////////////////////
    struct W_MouseEventDetails
    {
        std::optional<W_MouseEventType> event_type;
        std::optional<sf::Mouse::Button> button;
        float scroll_delta = 0.f;
        bool is_used = false;
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Stores queued callbacks for `Widget`
    ///
    /// Stored callbacks trigger after the specified delay is reached
    /// within the widget, delay can be paused if a more priority queued
    /// event is pushed onto the stack.
    ///
    /// @see `Widget::addToQueue`, `Widget::checkQueue`
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
        sf::Time m_delay = sf::Time::Zero;
        std::size_t m_framesCount{0};
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Stores a routine information
    ///
    /// Calls te designated callback for every period until the process returns
    /// true, a routine can count using time or frames and multiple routines can run
    /// sequentially in a single widget.
    ///
    /// @see `Widget::addRoutine`, `Widget::checkQueue`, `Widget::clearRoutine`
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
        sf::Time m_period = sf::Time::Zero;
        std::size_t m_framesPeriod{0};
        std::size_t m_framesCounter{0};
        std::function<bool()> m_callback;
        sf::Clock m_timer;
    };

    class W_CharHeight
    {
    public:
        static float getHeight(sf::Font text_font, unsigned int char_size);
    private:
        static sf::Text Ag;
    };
};
}
#endif