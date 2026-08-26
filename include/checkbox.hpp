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
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    CheckBox(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "", bool initial_state = false, const sf::Font& font = parameters::INTER_REGULAR_18);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the checkbox on a `Frame` object
    ///
    /// @param root root frame widget
    /// @param position checkbox's initial position
    /// @param size checkbox's initial size
    /// @param text initial text next to the checkbox
    /// @param initial_state initial state value for checkbox
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    CheckBox(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "", bool initial_state = false, const sf::Font& font = parameters::INTER_REGULAR_18);

    CheckBox(CheckBox&& other) = delete;
    CheckBox& operator=(CheckBox&& other) = delete;
    CheckBox(const CheckBox&) = delete;
    CheckBox& operator=(const CheckBox&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @brief Assigns a function call to state update event
    ///
    /// Callback is triggered whenever the checkbox updates in state,
    /// the state can be checked using `CheckBox::getState` or 
    /// `Widget::isFlag(FLAGS::SET)` which returns true if the checkbox is set.
    ///
    /// @see `unbindToggleEvent`
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
    /// @brief Updates the checkbox's text
    ///
    /// Displays parameter text on the checkbox's surface, text's length
    /// can go up to `parameters::CHAR_LIMIT` characters max.
    ///
    /// @param text string text
    ///
    /// @see `setTextSize`, `setTextLimit`, `getText`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const sf::String& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the characters for the checkbox's text
    ///
    /// @see `getTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextSize(unsigned int char_size);

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
    /// @see `getTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextSize(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the maximum number of text characters in this button
    ///
    /// `range=[0, 30]`
    ///
    /// @return true if the new value is set, otherwise false
    ///
    /// @see `getTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextLimit(unsigned int max_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the checkbox's text
    ///
    /// @param font reference to an existing font
    ///
    /// @warning if the passed font is an lvalue, it must outline the checkbox
    /// itself or get replaced before it's deleted
    ///
    /// @see `getTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the checkbox's text and stores it internally
    ///
    /// @param font font instance
    ///
    /// @see `getTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font&& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the position of the checkbox's text, default is
    /// `DIRECTION::LEFT`
    ///
    /// @param side which side to position the text based on the checkbox
    ///
    /// @see `DIRECTION`, `setText`
    ///
    /// @see `getTextPosition`
    ///
    ///////////////////////////////////////////////////////////////////

    void setTextPosition(DIRECTION side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the space between text and the checkbox
    ///
    /// Ratio is relative to the checkbox's size and scales based on which
    /// direction the text is positioned (for example, text is placed on the
    /// left: ratio set to `0.5f` == space is 0.5 * m_size.x in length), default
    /// is `0.2f`, range is `[0.f, 5.f]`.
    ///
    /// @see `getTextPadding`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextPadding(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(CheckboxStyle s_checkbox);

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
    [[nodiscard]] CheckboxStyle& style();

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
    [[nodiscard]] bool getState() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current character size in text
    ///
    /// @see `setTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getTextSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current maximum number of characters for text
    ///
    /// @see `setTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getTextLimit() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current padding for text
    ///
    /// @see `setTextPadding`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getTextPadding() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text position relative to the checkbox
    ///
    /// @see `setTextPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] DIRECTION getTextPosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text font
    ///
    /// @see `setTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::Font& getTextFont() const;

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the checkbox toggle state callback
    ///
    ///////////////////////////////////////////////////////////////////
    bool postProcessMouseEvents(const W_MouseEventDetails& details, const sf::Event& event, const sf::Vector2i& mouse_position) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the checkbox on master object
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the checkbox
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the checkbox.
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
    ///////////////////////////////////////////////////////////////////
    void updateState() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs shared theme with widget
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTheme() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates text size and position if provided
    ///
    ///////////////////////////////////////////////////////////////////
    void updateText();

///////////////////////////////////////////////////////////////////
/// Member Variables
///////////////////////////////////////////////////////////////////

    std::unique_ptr<const sf::Font> m_font;                                 //!< Locally stored font
    std::unique_ptr<sf::Shader> m_shaderHolder;                             //!< Holds the shader resource temporarily to disable it's effects in `Widget`
    sf::Text m_text;                                                        //!< Checkbox's text
    MouseCallback m_clickCallback{nullptr};                                 //!< State update callback

    float m_textSizeRatio{0.5f};                                            //!< Text size scale based on check box size and direction, `range=[0.f, 2.f]`
    float m_padding{0.2f};                                                  //!< Space between text and the checkbox relatve to size, `range=[0.f, 5.f]`
    unsigned int m_charSize{parameters::CHAR_SIZE};                         //!< Actual text size, can be set manually or updated with ratio, default is `parameters::CHAR_SIZE`
    unsigned int m_charLimit{parameters::CHAR_LIMIT};                       //!< Maximum number of text characters in button, `range=[0, 30]`, default is `parameters::CHAR_LIMIT` 

    sf::Color* m_currentTextColorPtr{nullptr};                              //!< Pointer to the current text color
    DIRECTION m_textSide{DIRECTION::LEFT};                                  //!< Text's position based on the checkbox
    CheckboxStyle m_style{Theme::checkbox_style::CHECKBOX_STALE_VIOLET};    //!< Checkbox's default style
};

template <>
struct WidgetStyle<CheckBox>
{
    using StyleType = CheckboxStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_checkboxTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_checkbox)
    {
        theme.m_checkboxTheme = s_checkbox;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_checkboxToken;
    }
};
}
#endif