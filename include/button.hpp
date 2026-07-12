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

    enum class Side : int;

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position button's initial position
    /// @param size button's initial size
    /// @param text text displayed on the button initially
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
    /// Text's length cannot surpass parameter character limit.
    /// @see `parameters::CHAR_LIMIT`
    ///
    /// @param text string text
    ///
    /// @see `setTextSide`, `setTextSize`, `setTextFont`,
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

    void setTextLimit(unsigned int char_limit);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets the center position of the button's text
    ///
    /// Position can be either `Button::Left`, `Button::Center`, or `Button::Right`
    /// and cannot overlap with image position.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSide(Side text_side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the characters for the button's text
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(unsigned int char_size);

    ///////////////////////////////////////////////////////////////////
    /// @brief Auto updates the size of the characters for the button's text
    ///
    /// Ratio is relative to the button's height and matches characters
    /// with caps' size, can be set from `0.f` to `0.999...f`, with `0.f`
    /// disabling ratio updating.
    ///
    /// @note if ratio is set to `0.f`, manual assignment to character size
    /// would become the priority.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSize(float ratio);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets a font for the button's text and clears the stored font
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
    void setTextFont(const sf::Font&& font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets the center position of the button's icon
    ///
    /// Position can be either `Button::Left`, `Button::Center`, or `Button::Right`
    /// and cannot overlap with text position.
    ///
    ///////////////////////////////////////////////////////////////////
    void setIconSide(Side icon_side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the base size of the icon image
    ///
    /// Ratio is relative to button's width and is set to `0.33f` by default.
    ///
    /// @param ratio size ratio between `0.1f` and `0.33f`
    ///
    ///////////////////////////////////////////////////////////////////
    void setIconSizeRatio(float ratio);

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

    const sf::String& getText() const;

    std::optional<sf::Image> getIcon();

    const sf::String& getIconPath() const;

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
    // Member Variables
    ///////////////////////////////////////////////////////////////////

    sf::Text m_text;                                                    //!< Button text
    std::unique_ptr<const sf::Font> m_fontPtr;                          //!< font for the text
    std::unique_ptr<sf::Texture> m_texturePtr;                          //!< Image texture
    std::unique_ptr<sf::Sprite> m_imagePtr;                             //!< Image sprite

    sf::String m_imagePath;                                             //!< Path for used image
    float m_imageSizeRatio = 0.33f;
    float m_charSizeRatio = 0.35f;                                      //!< Button's height occupied by text, prio until it's set to 0.f
    unsigned int m_charSize = parameters::CHAR_SIZE;                    //!< Manual text size assigned by user, not used until ratio is 0.f
    unsigned int m_charLimit = parameters::CHAR_LIMIT;
    Side m_imagePosition = Side::Center;                                //!< Which side does the image occupies, can't be the same as text position (center by default, adjusted during runtime)
    Side m_textPosition = Side::Center;                                 //!< Which side does the text occupies, can't be the same as image position (center by default, adjusted during runtime)
    ButtonStyle m_style = Theme::button_style::BUTTON_STALE_VIOLET;     //!< Button's default style

    bool m_requirePosUpdate = true;                                     //!< Updates the positions of text and image if set

    static constexpr unsigned int s_MAX_CHAR_COUNT{30};
public:

enum class Side : int
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