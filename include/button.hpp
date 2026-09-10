#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Basic button widget class
///
/// @see `Widget`
///
///////////////////////////////////////////////////////////////////
class Button : public Widget
{
public:
    enum class SIDE : int;

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position button's initial position
    /// @param size button's initial size
    /// @param text text displayed on the button initially
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    explicit Button(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text = "", const sf::Font& font = parameters::INTER_SEMIBOLD_18);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button on a `Frame` object
    ///
    /// @param root root frame widget
    /// @param position button's initial position
    /// @param size button's initial size
    /// @param text text displayed on the button initially
    /// @param font text's font
    ///
    ///////////////////////////////////////////////////////////////////
    explicit Button(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text = "", const sf::Font& font = parameters::INTER_SEMIBOLD_18);

    Button(Button&&) = delete;
    Button& operator=(Button&&) = delete;
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    ~Button() override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Updates button's text
    ///
    /// Displays parameter text on the button's surface.
    ///
    /// Text's length cannot surpass parameter character limit, default is
    /// `parameters::CHAR_LIMIT`. @see `setTextLimit`
    ///
    /// @param text string text
    ///
    /// @see `setTextSide`, `setTextSize`, `getText`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const sf::String& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates displayed icon on button widget
    ///
    /// Takes a string path to the image and loads it from disk or cache,
    /// can also take a shader for clipping the image (already loaded by default).
    /// 
    /// @param image_path path to image
    /// @param shader path to shader or the shader content itself, or an empty string
    /// @param loading_type choose the appropriate loading type based on shader parameter
    ///
    /// @return false if process fails to load the image, otherwise returns true
    ///
    /// @see `setIconSide`, `clearIcon`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setIcon(const sf::String& image_path, const std::string_view& shader = "", W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the maximum number of text characters in this button
    ///
    /// `range=[0, s_MAX_CHAR_COUNT]`
    ///
    /// @return true if the new value is set, otherwise false
    ///
    /// @see `getTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextLimit(unsigned int char_limit);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets the center position of the button's text
    ///
    /// Position can be either `Button::Left`, `Button::Center`, or `Button::Right`
    /// and cannot overlap with image position.
    ///
    /// @see `getTextPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextPosition(SIDE text_side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the characters for the button's text
    ///
    /// @return true if the new value is set, otherwise false
    ///
    /// @see `getTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextSize(unsigned int char_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Auto updates the size of the characters for the button's text
    ///
    /// Ratio is relative to the button's height and matches characters
    /// with caps' size, can be set from `0.f` to `0.999...f`, with `0.f`
    /// disabling ratio updating, default is `0.35f`.
    ///
    /// @note if ratio is set to `0.f`, manual assignment to character size
    /// would become the priority.
    ///
    /// @return true if the new value is set, otherwise false
    ///
    /// @see `getTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setTextSize(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the button's text and clears the stored font
    /// if available
    ///
    /// @param font reference to an existing font
    ///
    /// @warning parameter value must outlive the widget to prevent memory errors,
    /// to store the font in the widget pass an rvalue instead.
    ///
    /// @see `getTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the button's text and stores it internally
    ///
    /// @param font font
    ///
    /// @see `getTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(const sf::Font&& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets the center position of the button's icon
    ///
    /// Position can be either `Button::Left`, `Button::Center`, or `Button::Right`
    /// and cannot overlap with text position.
    ///
    /// @see `getIconPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void setIconPosition(SIDE icon_side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the base size of the icon image
    ///
    /// Ratio is relative to button's width and is set to `0.33f` by default.
    ///
    /// @param ratio size ratio between `0.1f` and `0.33f`
    ///
    /// @see `getIconSizeRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setIconSizeRatio(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears loaded image from widget
    ///
    /// @see `setIcon`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearIcon();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(ButtonStyle s_button);

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
    [[nodiscard]] ButtonStyle& style();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a read only reference of the current displayed text
    /// on the button
    ///
    /// @see `setText`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::String& getText() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a copy image of the current used icon on the button
    /// if icon is set
    ///
    /// @see `setIcon`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] std::optional<sf::Image> getIcon();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the image icon path
    ///
    /// @see `setIcon`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::String& getIconPath() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current character size for text
    ///
    /// @see `setTextSize`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getTextSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current icon size ratio
    ///
    /// @see `setIconSizeRatio`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] float getIconSizeRatio() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current maximum number of characters in text
    ///
    /// @see `setTextLimit`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getTextLimit() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current icon position relative to button surface
    ///
    /// @see `setIconPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] SIDE getIconPosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current text position relative to button surface
    ///
    /// @see `setTextPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] SIDE getTextPosition() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns text font
    ///
    /// @see `setTextFont`
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::Font& getTextFont() const;
    
private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the button on master object
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the button,
    /// called in `Widget::setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the widget,
    /// called in `setWidgetSize`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateSize() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief matches the current angle value with the rest of the button
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates button colors to match current state alongside
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
    /// @brief Refresh for clipping shader
    ///
    ///////////////////////////////////////////////////////////////////
    void updateClipShader() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates text position
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTextPosition();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates text size
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTextSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates image position if image is available
    ///
    ///////////////////////////////////////////////////////////////////
    void updateImagePosition();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates image size if image is available
    ///
    ///////////////////////////////////////////////////////////////////
    bool updateImageSize();

///////////////////////////////////////////////////////////////////
/// Member Variables
///////////////////////////////////////////////////////////////////

    sf::Text m_text;                                                    //!< Button text
    std::unique_ptr<const sf::Font> m_fontPtr;                          //!< font for the text
    std::unique_ptr<sf::Texture> m_texturePtr;                          //!< Image texture
    std::unique_ptr<sf::Sprite> m_imagePtr;                             //!< Image sprite
    sf::String m_imagePath;                                             //!< Path for used image
    sf::Vector2f m_imageCenter;

    float m_imageSizeRatio{0.33f};                                      //!< Icon size relative to the button's width, `range=[0.1f, 0.33f]`
    float m_charSizeRatio{0.35f};                                       //!< Button's height occupied by text, prio until it's set to 0.f, `range=[0.f, 1.f[`
    unsigned int m_charSize{parameters::CHAR_SIZE};                     //!< Manual text size assigned by user, not used until ratio is 0.f, default is `parameters::CHAR_SIZE`
    unsigned int m_charLimit{parameters::CHAR_LIMIT};                   //!< Maximum number of text characters in button, `range=[0, s_MAX_CHAR_COUNT]`, default is `parameters::CHAR_LIMIT` 
    SIDE m_imagePosition{SIDE::Center};                                 //!< Which side does the image occupies, can't be the same as text position (center by default, adjusted during runtime)
    SIDE m_textPosition{SIDE::Center};                                  //!< Which side does the text occupies, can't be the same as image position (center by default, adjusted during runtime)
    ButtonStyle m_style{Theme::button_style::BUTTON_STALE_VIOLET};      //!< Button's default style

    static constexpr unsigned int s_MAX_CHAR_COUNT{30};
public:

enum class SIDE : int
    {
        Center,
        Left,
        Right
    };
};

template <>
struct WidgetStyle<Button>
{
    using StyleType = ButtonStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_buttonTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_button)
    {
        theme.m_buttonTheme = s_button;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_buttonToken;
    }
};
}
#endif