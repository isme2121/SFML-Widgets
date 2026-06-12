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
    /// @brief Updates the max number of characters in the label
    ///
    /// Cuts the current string if the new limit is smaller than the current
    /// size of the string.
    ///
    /// @param char_limit new cap
    ///
    ///////////////////////////////////////////////////////////////////
    void setCharacterLimit(unsigned int char_limit);

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

    ///////////////////////////////////////////////////////////////////
    /// @brief Set character size scale based on the shape's size
    ///
    /// Text size would scale with size.y of the widget if this
    /// method is called, set from `0.f` to `1.0f` as a y axis
    /// ratio with `0.f` disablding ratio scaling.
    ///
    /// @param ratio percent value from `0.f` to `1.f`
    ///
    ///////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////
    /// @brief Shifts the label horizontally
    ///
    /// The text itself remains inside the widget box but shifts based
    /// on the padding ratio if not centered
    ///
    /// @param h alignment type
    ///
    /// @see `Label::setPaddingRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    void setHorizontalAlignment(HAlign h = HAlign::Center);

    ///////////////////////////////////////////////////////////////////
    /// @brief Shifts the label vertically
    ///
    /// The text itself remains inside the widget box but shifts based
    /// on the padding ratio if not placed in the middle
    ///
    /// @param v alignment type
    ///
    /// @see `Label::setPaddingRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    void setVerticalAlignment(VAlign v = VAlign::Middle);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the vertical space between lines that is defined
    /// by the font
    ///
    /// @param factor default is `1.f`
    ///
    ///////////////////////////////////////////////////////////////////
    void setLineSpacing(float factor);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the horizontal space between characters that is
    /// defined by the font
    ///
    /// @param factor default is `1.f`
    ///
    ///////////////////////////////////////////////////////////////////
    void setLetterSpacing(float factor);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the padding value for text alignment
    ///
    /// Ratio scales with the shape's size, If ratio is set to
    /// `0.f` then any alignment type would leave the text centered, if
    /// ratio is set to `1.f` then the text is always centered.
    ///
    /// @param ratio float value from `0.f` to `1.f`
    ///
    /// @see `Label::setHorizontalAlignment`, `Label::setVerticalAlignment`
    ///
    ///////////////////////////////////////////////////////////////////
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

    std::unique_ptr<const sf::Font> m_font;                     //!< Reference for internal font
    sf::Text m_text;                                            //!< Label's text

    std::size_t m_charLimit = parameters::CHAR_LIMIT;           //!< Max number of characters
    unsigned int m_charSize = 24;                               //!< Default character size
    float m_textSizeRatio = 0.f;                                //!< Default character size ratio (disabled initially)
    float  m_paddingRatio = 0.f;                                //!< Default padding, set from 0 to 1 and scales with alignment
    HAlign m_horizontalAlign = HAlign::Center;                  //!< Horizontal alignment, scales with padding
    VAlign m_verticalAlign = VAlign::Middle;                    //!< Vertical alignment, scales with padding

    LabelStyle m_style = Theme::label_style::LABEL_SLATE_VIOLET;//!< Label theme
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Horizontal placement of the text object
    ///
    ///////////////////////////////////////////////////////////////////
    enum class HAlign
    {
        Left,
        Center,
        Right
    };

    ///////////////////////////////////////////////////////////////////
    /// @brief Vertical placement of the text object
    ///
    ///////////////////////////////////////////////////////////////////
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