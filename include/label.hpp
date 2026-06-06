#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
class Label : public Widget
{
public:
    enum class HAlign : int;
    enum class VAlign : int;

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the label with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position label's initial position
    /// @param size label's initial size
    /// @param text initial text for label
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    Label(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "", const sf::Font& font = parameters::font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the label with `Frame`
    ///
    /// @param root window to draw on
    /// @param position label's initial position
    /// @param size label's initial size
    /// @param text initial text for label
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    Label(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "", const sf::Font& font = parameters::font);

    Label(Label&& other) = delete;
    Label& operator=(Label&& other) = delete;
    Label(const Label&) = delete;
    Label& operator=(const Label&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events.
    ///
    /// Call the method in the events loop for assigned methods callbacks,
    /// can optionally pass the current raw mouse position, this makes the
    /// widget skip getting the mouse position itself and can save computing
    /// time when multiple widgets are checking for event.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    /// @param mouse_position optional current mouse position from `sf::Mouse::getPosition`
    ///
    /// @see `bindMouseEvent`, `unbindMouseEvent`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note widgets with `FLAGS::BLOCKED` or `FLAGS::HIDDEN` states cannot
    /// trigger mouse events.
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
    /// @see `bindShortCut`, `unbindShortCut`
    ///
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note widget must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, widgets with `FLAGS::BLOCKED` cannot trigger
    /// key events.
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the text content of the label
    ///
    /// Input is displayed on the label, if it exceeds the size limit then
    /// the string is cut first, default limit is `parameters::CHAR_LIMIT`.
    ///
    /// @param text string object
    ///
    /// @see `Label::setTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(sf::String text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Manually set the character size for the label
    ///
    /// Label is set with a constant character size, to add a variant
    /// text size, use the float overload instead.
    ///
    /// @param char_size character size
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(unsigned int char_size);

    ////////////////////////////////////////////////////////////
    /// @brief Set character size scale based on the shape's size
    ///
    /// Text size would scale with size.y of the widget if this
    /// method is called, set from `0.f` to `1.0f` as a y axis
    /// ratio with `0.f` disablding ratio scaling.
    ///
    /// @param ratio percent value from `0.f` to `1.f`
    ///
    ////////////////////////////////////////////////////////////
    void setTextSize(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the label's text and clears the stored font
    /// if available
    ///
    /// @param font reference to an existing font
    ///
    /// @warning parameter value must outlive the widget to prevent memory errors,
    /// to store the font in the widget pass an rvalue instead.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the button's text and stores it internally
    ///
    /// @param font font instance
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(sf::Font&& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief sf::Text rendering style
    ///
    /// @param style `sf::Text::Style` value
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextStyle(sf::Text::Style style);

    ////////////////////////////////////////////////////////////
    /// @brief Shifts the label horizontally
    ///
    /// The text itself remains inside the widget box but shifts based
    /// on the padding ratio if not centered
    ///
    /// @param h alignment type
    ///
    /// @see `Label::setPaddingRatio`
    ///
    ////////////////////////////////////////////////////////////
    void setHorizontalAlignment(HAlign h = HAlign::Center);

    ////////////////////////////////////////////////////////////
    /// @brief Shifts the label vertically
    ///
    /// The text itself remains inside the widget box but shifts based
    /// on the padding ratio if not placed in the middle
    ///
    /// @param v alignment type
    ///
    /// @see `Label::setPaddingRatio`
    ///
    ////////////////////////////////////////////////////////////
    void setVerticalAlignment(VAlign v = VAlign::Middle);

    ////////////////////////////////////////////////////////////
    /// @brief Updates the spacing between lines
    ///
    /// @param factor spacing ratio
    ///
    /// @see `Label::setLetterSpacing`
    ///
    ////////////////////////////////////////////////////////////
    void setLineSpacing(float factor);

    ////////////////////////////////////////////////////////////
    /// @brief Updates the spacing between characters
    ///
    /// @param factor spacing ratio
    ///
    /// @see `Label::setLineSpacing`
    ///
    ////////////////////////////////////////////////////////////
    void setLetterSpacing(float factor);

    ////////////////////////////////////////////////////////////
    /// @brief Updates the padding value for text alignment
    ///
    /// Ratio scales with the shape's size, If ratio is set to
    /// `0.f` then any alignment type would leave the text centered.
    ///
    /// @param ratio float value from `0.f` to `0.5f`
    ///
    /// @see `Label::setHorizontalAlignment`, `Label::setVerticalAlignment`
    ///
    ////////////////////////////////////////////////////////////
    void setPaddingRatio(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(LabelStyle label_style);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget on master object
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
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief matches the current angle value with the rest of the entry
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
    /// @brief Updates the character size of text
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTextSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the text's position
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTextPosition();

    ///////////////////////////////////////////////////////////////////
    /// @brief Keeps the shape wrapped around the text
    ///
    ///////////////////////////////////////////////////////////////////
    void updateShapeBounds();

    std::unique_ptr<const sf::Font> m_font;
    sf::Text m_text;

    std::size_t m_charLimit = parameters::CHAR_LIMIT;
    unsigned int m_charSize = 24;
    float m_textSizeRatio = 0.f;
    float  m_paddingRatio = 0.25f;
    HAlign m_horizontalAlign = HAlign::Center;
    VAlign m_verticalAlign = VAlign::Middle;

    LabelStyle m_style = Theme::label_style::LABEL_SLATE_VIOLET;
public:
    ////////////////////////////////////////////////////////////
    /// @brief Horizontal placement of the text object
    ///
    ////////////////////////////////////////////////////////////
    enum class HAlign
    {
        Left,
        Center,
        Right
    };

    ////////////////////////////////////////////////////////////
    /// @brief Vertical placement of the text object
    ///
    ////////////////////////////////////////////////////////////
    enum class VAlign
    {
        Top,
        Middle,
        Bottom
    };

};

template <>
struct __WidgetStyle<Label>
{
    using StyleType = LabelStyle;

    static const StyleType& get(const WidgetTheme& t)
    {
        return t.m_labelTheme;
    }

    static void set(WidgetTheme& t, StyleType _b)
    {
        t.m_labelTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_labelToken;
    }
};
}
#endif