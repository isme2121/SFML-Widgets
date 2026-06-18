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
    friend class Widget;
    friend class RenderContext;

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
    /// @brief Enables/Disables grid view for child widgets
    ///
    /// Re-orders all child widgets based on grid parameter and forces
    /// their positions, grid can be disabled by passing one of the
    /// grids as `0`.
    ///
    /// @param grid_mapping number of widgets drawn per view, with
    /// `Ncols` X `Nrows` ratio (`Ncols`: number of w in one row,
    /// `Nrows`: Number of w in one column)
    ///
    ///////////////////////////////////////////////////////////////////
    void setGrid(sf::Vector2u grid_mapping);

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates separator vertices between widgets
    ///
    /// Adds separator lines between widgets if they match the parameter
    /// condition.
    ///
    /// @param sep_type type of the separator
    /// @param length_ratio length of the separator line based on frame's size
    /// (depends on separator type)
    /// @param sep_dist condition distance between widgets to draw a
    /// separator in between (exp: dist(w1, w2) >= `sep_dist`? draw separator)
    ///
    /// @see `SeparatorType`
    ///
    ///////////////////////////////////////////////////////////////////
    void setSeparators(SeparatorType sep_type, float length_ratio, float sep_dist);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle s_frame);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns accurate coordinates for local view
    ///
    /// @param mouse_position mouse position based on window's view
    ///
    /// @return float coordinates relative to the frame's view
    ///
    ///////////////////////////////////////////////////////////////////
    sf::Vector2f mapPixelToCoords(sf::Vector2i mouse_position) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a pointer to root window
    ///
    ///////////////////////////////////////////////////////////////////
    sf::RenderWindow* getWindowPtr() const;

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for child widgets events first
    ///
    /// If a child widget consumes the event, exits `Widget::captureMouseEvents` early
    /// while returning `true` if `s_STACK_CHILD_EVENT` is disabled which it
    /// is by default.
    ///
    ///////////////////////////////////////////////////////////////////
    bool preProcessMouseEvents(const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls for scrolling event
    ///
    ///////////////////////////////////////////////////////////////////
    void postProcessMouseEvents(W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position) override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for child widgets events first
    ///
    /// If a child widget consumes the event, exits `Widget::captureMouseEvents` early
    /// while returning `true`. `s_STACK_CHILD_EVENT` does not currently
    /// affect the result.
    ///
    ///////////////////////////////////////////////////////////////////
    bool preProcessKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the frame and it's child widgets
    ///
    /// @see `display`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Allows certain toggled flags to be directly inherited within
    /// it's child widgets
    ///
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the widget,
    /// called in `setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the widget,
    /// called in `setWidgetPosition`
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
    ///////////////////////////////////////////////////////////////////
    void updateState() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs shared theme with widget
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief scroll event
    ///
    ///////////////////////////////////////////////////////////////////
    void scroll(float delta);

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
    /// @brief Updates the drawn content of the frame
    ///
    /// Should probably stick to `Widget::draw` instead.
    ///
    /// @warning this method is called within `drawWidget` override and it's
    /// responsible for concluding the drawing of the frame and it's child widgets
    /// for this loop, it should not be used while `draw` is being called from the
    /// same instance within the loop as it would. The only time `display` is preferred
    /// is when we decide to draw child widgets individually when compared to
    /// `Widget::draw` method which draws all child widgets automatically and
    /// safely handles the entire update process.
    ///
    ///////////////////////////////////////////////////////////////////
    void display();

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws a child widget individually
    ///
    /// Called within `RenderContext` class to draw a child widget
    ///
    /// @see `RenderContext`, `Frame::draw`, `Frame::display`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawChildWidget(const sf::Drawable& w, const sf::RenderStates state = sf::RenderStates::Default);

    ///////////////////////////////////////////////////////////////////
    /// @brief Appends a child widget to frame, managed by base class
    ///
    /// @param w new widget to add
    ///
    ///////////////////////////////////////////////////////////////////
    void addWidget(Widget& w);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes a child widget's reference from frame, managed
    /// by base class
    ///
    /// @param w child widget to remove
    ///
    ///////////////////////////////////////////////////////////////////
    void removeWidget(Widget& w);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates a reference to an existing child widget after moving,
    /// managed by base class
    ///
    /// @param old_widget reference to previous widget
    /// @param new_widget reference to the same widget after moving
    ///
    ///////////////////////////////////////////////////////////////////
    void updateWidget(Widget& old_widget, Widget& new_widget);

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

    ///////////////////////////////////////////////////////////////////
    /// Static Variables
    ///////////////////////////////////////////////////////////////////

    static constexpr bool s_STACK_CHILD_EVENT = false;                  //!< Allows the frame to trigger mouse & key events even after a child widget's callback
    static constexpr float s_SCROLL_RATIO = 0.05f;                      //!< Frame scrolls by 5% of it's height on each scroll event
};

template <>
struct WidgetStyle<Frame>
{
    using StyleType = TextureStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_frameTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_frame)
    {
        theme.m_frameTheme = s_frame;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_frameToken;
    }
};
}
#endif