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
    Label(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text = "", const sf::Font& font = parameters::INTER_REGULAR_18);

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
    Label(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text = "", const sf::Font& font = parameters::INTER_REGULAR_18);

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
    /// @see `setTextLimit`, `setTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const sf::String& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the max number of characters in the label
    ///
    /// Cuts the current string if the new limit is smaller than the current
    /// size of the string, default is `parameters::CHAR_LIMIT`.
    ///
    /// @param char_limit new maximum number of characters for this label
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextLimit(unsigned int char_limit);

    ///////////////////////////////////////////////////////////////////
    /// @brief Manually set the character size for the label
    ///
    /// Label is set with a constant character size, to add a variant
    /// text size, use the float overload instead, default is `parameters::CHAR_SIZE`
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
    /// ratio with `0.f` disabling ratio scaling, default is `0.f`.
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
    /// @see `setPaddingRatio`
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
    /// @see `setPaddingRatio`
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
    /// @see `setHorizontalAlignment`, `setVerticalAlignment`
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
    void setTheme(LabelStyle s_label);

    ///////////////////////////////////////////////////////////////////
    /// @brief Pulls the widget's style from it's linked theme object
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
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] LabelStyle& style();
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current label content
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::String& getText() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current maximum number of characters in label
    ///
    /// default is `parameters::CHAR_LIMIT`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] std::size_t getTextLimit() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text size
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getTextSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text font
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::Font& getTextFont() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text style
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Text::Style getTextStyle() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current horizontal alignment value
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] HAlign getHorizontalAlignment() const;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current vertical alignment value
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] VAlign getVerticalAlignment() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current line spacing value
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getLineSpacing() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current letter spacing value
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getLetterSpacing() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current alignment padding ratio value
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getPaddingRatio() const;

    
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

    std::size_t m_charLimit{parameters::CHAR_LIMIT};            //!< Max number of characters
    unsigned int m_charSize{parameters::CHAR_SIZE};             //!< Default character size
    float m_textSizeRatio{0.f};                                 //!< Default character size ratio (disabled initially)
    float m_paddingRatio{0.f};                                  //!< Default padding, set from 0 to 1 and scales with alignment
    HAlign m_horizontalAlign{HAlign::Center};                   //!< Horizontal alignment, scales with padding
    VAlign m_verticalAlign{VAlign::Middle};                     //!< Vertical alignment, scales with padding

    LabelStyle m_style{Theme::label_style::LABEL_SLATE_VIOLET}; //!< Label theme
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
struct WidgetStyle<Label>
{
    using StyleType = LabelStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_labelTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_label)
    {
        theme.m_labelTheme = s_label;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_labelToken;
    }
};
}
#endif