#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Widget that contains and manages child widgets
///
/// @see `Widget`
///
///////////////////////////////////////////////////////////////////
class Frame : public Widget
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the frame with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    ///
    ///////////////////////////////////////////////////////////////////
    Frame(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the frame object on a root frame
    ///
    /// @param root root frame widget
    /// @param position frame's initial position
    /// @param size frame's initial size
    ///
    ///////////////////////////////////////////////////////////////////
    Frame(Frame& root, sf::Vector2f position, sf::Vector2u size);

    Frame(Frame&& other) = delete;
    Frame& operator=(Frame&& other) = delete;

    Frame(const Frame& other) = delete;
    Frame& operator=(const Frame& other) = delete;

    ~Frame() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events
    ///
    /// Called in the events loop, can recieve the current mouse position based on
    /// root window view to skip a few steps internally.
    ///
    /// This method handles the frame's child widgets' mouse events first so avoid
    /// calling this method for any child widget manually after calling it from the root
    /// frame object.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    /// @param mouse_position optional current mouse position from `sf::Mouse::getPosition`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note frame with `FLAGS::BLOCKED | FLAGS::HIDDEN | FLAGS::RESTRICTED`
    /// states cannot trigger mouse events.
    ///
    /// @see `Widget::bindMouseEvent`, `Widget::unbindMouseEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureMouseEvents(const sf::Event& _event, std::optional<sf::Vector2i> mouse_position = std::nullopt) override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events
    ///
    /// Captures keyboard events in the events loop, also handles
    /// keyboard events for child widgets, meaning that only the root
    /// frame should call this method within the loop in place of it's
    /// children widgets.
    ///
    /// @param event Event object
    ///
    /// @return true if the frame triggers a callback, otherwise false.
    ///
    /// @note frame must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, frames with `FLAGS::BLOCKED | FLAGS::RESTRICTED`
    /// cannot trigger keyboard events.
    ///
    /// @see `Widget::bindShortCut`, `Widget::unbindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& _event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Shows frame and it's drawn child widgets on the window
    ///
    /// Do not call if `Widget::draw` method is already in use as it is called
    /// automatically within that method, instead, call `Frame::display` if:
    ///
    /// 1- `Widget::draw` is not called from the frame object.
    ///
    /// 2- if child widgets are drawn manually, then we use `Frame::display`
    ///    to draw them to the screen.
    ///
    /// We can use either methods depending on what we want to archieve:
    ///
    /// - use `Widget::draw` to draw all child widgets and the frame.
    ///
    /// - use `Frame::display` to draw only the child widgets that previously
    ///   called their own draw methods within the loop.
    ///
    /// @see `Widget::draw`
    ///
    ///////////////////////////////////////////////////////////////////
    void display();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns pixel coordinates based on view
    ///
    /// Used for accurate coordinates when the frame's view is moved,
    /// it considers scrolled surface and returns the correct coordinates
    /// for the widgets but may not match the raw mouse position coordinates.
    ///
    /// @param mouse_position mouse position based on window's view from
    /// `sf::Mouse::getPosition(window)`
    ///
    /// @return float coordinates matching the frame's view
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f mapPixelToCoords(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Enables/Disables grid view for child widgets
    ///
    /// Re-orders all child widgets based on grid parameter and forces
    /// their positions, grid can be disabled by passing one of the
    /// grids as 0.
    ///
    /// @param grid_mapping number of widgets drawn per view, with
    /// Ncols X Nrows ratio (Ncols : number of w in one row, Nrows : Number
    /// of w in one column)
    ///
    ///////////////////////////////////////////////////////////////////
    void setGrid(sf::Vector2u grid_mapping);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates separator vertices between widgets
    ///
    /// Adds separator lines between widgets if they match the parameter
    /// condition.
    ///
    /// @param _t type of the separator
    /// @param length_ratio length of the separator line based on frame's size
    /// (depends on separator type)
    /// @param sep_distance condition distance between widgets to draw a
    /// separator in between
    ///
    /// @see `SeparatorType`
    ///
    ///////////////////////////////////////////////////////////////////
    void setSeparators(SeparatorType _t, float length_ratio, float sep_distance);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to root window
    ///
    /// Returns a pointer to root window even if the frame is not directly
    /// placed on top of one.
    ///
    ///////////////////////////////////////////////////////////////////
    sf::RenderWindow* getWindowPtr() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle __frame_style);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the frame and it's child widgets
    ///
    /// Calling this method renders `Frame::display` unnecessary as it draws
    /// all child widgets and frame then displays while updating state.
    ///
    /// @see `display`
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
    /// @brief matches the current angle value with the rest of the frame
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates frame's colors to match current state alongside
    /// shaders, called in `Widget::Draw`
    ///
    /// The default tested states are: `BLOCKED`, `TRANSPARENT`, `ACTIVE`
    /// `HOVERED` respectively.
    ///
    /// @note This method can be skipped if `FLAGS::PASSIVESKIP` is set
    /// internally
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
    /// @see `Widget::loadClipShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateClipShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in a shader parameters, color shader is applied
    /// on background texture that is set via `setBackGroundTexture`.
    ///
    /// @see `Widget::loadColorShader`, `updateState`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateColorShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Inforces grid ordering
    ///
    /// @see `setGrid`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateGrid();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws line separators if enabled
    ///
    /// @see `setSeparators`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawSeparators();

    ///////////////////////////////////////////////////////////////////
    /// @brief Refreshes the frame's invisible space
    ///
    /// Called after an update to the frame's height or a widget's position
    /// to limit scrolling space.
    ///
    /// @see `updateScrollView`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateFrameHeight();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the scroll offset after a height update
    ///
    /// Shrinks the scrolling space to match the new height
    ///
    /// @see `updateFrameHeight`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateScrollView();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates view center based on scroll offset
    ///
    ///////////////////////////////////////////////////////////////////
    void refreshView();

    ///////////////////////////////////////////////////////////////////
    /// @brief scroll event
    ///
    ///////////////////////////////////////////////////////////////////
    void scroll(float delta);

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws a child widget individually
    ///
    /// Called within `RenderContext` class to draw a child widget
    ///
    /// @see `RenderContext`, `Frame::draw`, `Frame::display`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawChildWidget(const sf::Drawable& __widget, const sf::RenderStates state = sf::RenderStates::Default);

    ///////////////////////////////////////////////////////////////////
    /// @brief Appends a child widget to frame, managed by base class
    ///
    /// @param __widget new widget to add
    ///
    ///////////////////////////////////////////////////////////////////
    void addWidget(Widget& __widget);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a child widget's reference from frame, managed
    /// by base class
    ///
    /// @param __widget child widget to remove
    ///
    ///////////////////////////////////////////////////////////////////
    void removeWidget(Widget& __widget);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates a reference to an existing child widget after moving,
    /// managed by base class
    ///
    /// @param __old_widget reference to previous widget
    /// @param __new_widget reference to the same widget after moving
    ///
    ///////////////////////////////////////////////////////////////////
    void updateWidget(Widget& __old_widget, Widget& __new_widget);

    ///////////////////////////////////////////////////////////////////
    /// Member Variables
    ///////////////////////////////////////////////////////////////////

    float m_scrollOffset = 0.f;                                         //!< Scrolled height offset
    float m_maxScroll = 0.f;                                            //!< Invisible height and max scroll offset value
    float m_contentHeight;                                              //!< visible + invisible frame height

    SeparatorType m_separatorType = SeparatorType::NoSeparator;         //!< Separator line between child widgets
    float m_separatorRatio = 0.f;                                       //!< Separator's length based on the frame itself
    float m_separatorDistance = 0.f;                                    //!< Required distance in pixels between child widgets to draw a separator

    sf::RenderWindow* m_window;                                         //!< Root window
    sf::RenderTexture m_texture;                                        //!< Drawing ground for widgets
    sf::Sprite m_sprite;                                                //!< Drawn to window
    std::vector<Widget*> m_childWidgets;                                //!< Active pointers to widgets
    sf::Vector2u m_gridMapping;                                         //!< 2 dimensional map for displaying widgets in a grid line
    TextureStyle m_style = Theme::texture_style::STYLE_STALE_VIOLET;    //!< Frame's default style

    friend class Widget;
    friend class RenderContext;
};

template <>
struct __WidgetStyle<Frame>
{
    using StyleType = TextureStyle;

    static const StyleType& get(const WidgetTheme& t)
    {
        return t.m_frameTheme;
    }

    static void set(WidgetTheme& t, StyleType _b)
    {
        t.m_frameTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_frameToken;
    }
};
}
#endif