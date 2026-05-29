#pragma once
#include "widget.hpp"
#ifdef WIDGETS_HPP
namespace sfml_widgets
{
class CheckBox : public Widget
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the checkbox with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position checkbox's initial position
    /// @param size checkbox's initial size
    /// @param text initial text next to the checkbox
    /// @param initial_state initial state value for checkbox
    ///
    ///////////////////////////////////////////////////////////////////
    CheckBox(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "", bool initial_state = false);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the checkbox on a `Frame` object
    ///
    /// @param root root frame widget
    /// @param position checkbox's initial position
    /// @param size checkbox's initial size
    /// @param text initial text next to the checkbox
    /// @param initial_state initial state value for checkbox
    ///
    ///////////////////////////////////////////////////////////////////
    CheckBox(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "", bool initial_state = false);

    CheckBox(CheckBox&& other) noexcept;
    CheckBox& operator=(CheckBox&& other) noexcept;
    CheckBox(const CheckBox&) = delete;
    CheckBox& operator=(const CheckBox&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events.
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
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note widgets with `FLAGS::BLOCKED` or `FLAGS::HIDDEN` states cannot
    /// trigger mouse events.
    ///
    /// @see `Widget::bindMouseEvent`, `Widget::unbindMouseEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position = std::nullopt) override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events.
    ///
    /// Captures keyboard events in the events loop, can capture individual
    /// physical key presses or a sequence of key press events depending on
    /// available assigned sequences.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note checkbox must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, checkboxs with `FLAGS::BLOCKED | FLAGS::RESTRICTED`
    /// cannot trigger keyboard events.
    ///
    /// @see `Widget::bindShortCut`, `Widget::unbindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the checkbox's text
    ///
    /// Displays parameter text on the checkbox's surface, text's length
    /// can go up to `15` characters max.
    ///
    /// @param text string text
    ///
    /// @see `parameters::CHAR_LIMIT`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const std::string& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the characters for the checkbox's text
    ///
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(unsigned int char_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Auto updates the size of the characters for the checkbox's text
    ///
    /// Ratio is relative to the checkbox's height and matches characters
    /// with caps' size, can be set from `0.f` to `2.f`, with `0.f`
    /// disabling ratio updating, default is `0.5f`.
    ///
    /// @note if ratio is set to `0.f`, manual assignment to character size
    /// would become the priority.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the checkbox's text
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(sf::Font font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a function call to state update event
    ///
    /// Callback is triggered whenever the checkbox updates in state,
    /// the state can be checked using `CheckBox::getState` or 
    /// `Widget::isFlag(FLAGS::SET)` which returns true if the checkbox is set.
    ///
    /// @see `unbindToggleEvent`, `Widget::isFlag`
    ///
    ///////////////////////////////////////////////////////////////////
    void bindToggleEvent(MouseCallback callback);

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes assigned callback for checkbox's state update
    ///
    /// @see `bindToggleEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    void unbindToggleEvent();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates empty space between text and the checkbox based
    /// on the corresponding direction's checkbox diameter
    ///
    /// Scales with the size of the checkbox itself, can be set from `0.f`
    /// to `1.f` as a maximum distance, default is `0.3f`.
    ///
    ///////////////////////////////////////////////////////////////////
    void setSpace(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current state of the checkbox widget
    ///
    /// Identical to `Widget::isFlag(FLAGS::SET)` which checks for the
    /// boolean state of the widget.
    ///
    /// @returns true if the checkbox is set, otherwise false
    ///
    /// @see `Widget::isFlag`
    ///
    ///////////////////////////////////////////////////////////////////
    bool getState();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the position of the checkbox's text
    ///
    /// @param side which side to position the text based on the checkbox
    ///
    /// @see `DIRECTION`, `setText`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSide(DIRECTION side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(CheckboxStyle __checkbox_style);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the checkbox on master object
    ///
    /// @see `Widget::draw`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Used to update child widgets for container type Widget or
    /// add specific rules after updating flags, called in `Widget::configure`
    ///
    /// @param mask bits passed by `Widget::configure`
    ///
    /// @see `Widget::configure`, `parameters::MODIFIABLE`
    ///
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the checkbox,
    /// called in `Widget::setWidgetPosition`
    ///
    /// @see `Widget::setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the checkbox,
    /// called in `setWidgetSize`
    ///
    /// @see `Widget::setWidgetSize`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief matches the current angle value with the rest of the checkbox
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates checkbox colors to match current state alongside
    /// shaders, called in `Widget::Draw`
    ///
    /// The default tested states are: `BLOCKED`, `TRANSPARENT`, `HOVERED`
    /// respectively.
    ///
    /// @note This method can be skipped if `FLAGS::PASSIVESKIP` is set
    /// internally
    ///
    /// @see `Widget:draw`
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
    /// @brief Refresh for clipping shader, Not needed for checkbox class
    ///
    /// @see `Widget::loadClipShader`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateClipShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in a shader parameters, color shader
    /// is applied on background texture that is set via
    /// `Widget::setBackGroundTexture` and is not needed if no texture
    /// is assigned.
    ///
    /// @see `Widget::loadColorShader`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateColorShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates text size and position if provided
    ///
    /// Refreshes characters' size based on checkbox's size and updates
    /// it's position.
    ///
    /// @see `setText`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateText();

    sf::Text m_text;                                        //!< Checkbox's text
    MouseCallback m_clickCallback = nullptr;                //!< State update callback
    sf::Color* m_currentTextColorPtr = nullptr;             //!< Pointer to the current text color
    DIRECTION m_textSide = DIRECTION::LEFT;                 //!< Text's position based on the checkbox
    float m_textSizeRatio = 0.5f;                           //!< Text size scale based on check box size and direction
    unsigned int m_charSize = 24;                           //!< Actual text size, can be set manually or updated with ratio
    float m_deadSpace = 0.3f;                               //!< Space between text and the checkbox
    CheckboxStyle m_style = themes::CHECKBOX_STALE_VIOLET;  //!< Checkbox's default style
};

template <>
struct __WidgetStyle<CheckBox>
{
    static const CheckboxStyle& get(const WidgetTheme& t)
    {
        return t.m_checkboxTheme;
    }

    static void set(WidgetTheme& t, CheckboxStyle _b)
    {
        t.m_checkboxTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_checkboxToken;
    }
};
}
#endif