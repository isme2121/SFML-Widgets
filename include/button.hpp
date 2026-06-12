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
    explicit Button(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text = "", const sf::Font& font = parameters::font);

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button on a `Frame` object
    ///
    /// @param root root frame widget
    /// @param position button's initial position
    /// @param size button's initial size
    /// @param text text displayed on the button initially
    ///
    ///////////////////////////////////////////////////////////////////
    explicit Button(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text = "", const sf::Font& font = parameters::font);

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
    ///
    /// @param text string text
    ///
    /// @see `parameters::CHAR_LIMIT`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const sf::String& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets the center position of the button's text
    ///
    /// Position can be either `Button::Left`, `Button::Center`, or `Button::Right`
    /// and cannot overlap with image position, if it does then image position is
    /// moved by the function.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextSide(Side text_side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the size of the characters for the button's text
    ///
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
    /// @brief Updates displayed image on button widget
    ///
    /// Takes a string path to the image and loads it from disk or cache,
    /// image display position depends on the text position in the widget.
    /// 
    /// @param image_path string image_path to image
    ///
    /// @return false if process fails to load the image, otherwise returns true
    ///
    /// @see `clearImage`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setImage(const sf::String& image_path);

    ///////////////////////////////////////////////////////////////////
    /// @brief Sets the center position of the button's image
    ///
    /// Position can be either `Button::Left`, `Button::Center`, or `Button::Right`
    /// and cannot overlap with text position, if it does then text position is
    /// moved by the function.
    ///
    ///////////////////////////////////////////////////////////////////
    void setImageSide(Side image_side);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears loaded image from widget
    ///
    /// @see `setImage`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearImage();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(ButtonStyle button_style);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the button on master object
    ///
    /// @see `Widget::draw`
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the button,
    /// called in `Widget::setWidgetPosition`
    ///
    /// @see `Widget::setWidgetPosition`
    ///
    ///////////////////////////////////////////////////////////////////
    void updatePosition() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' sizes within the widget,
    /// called in `setWidgetSize`
    ///
    /// @see `Widget::setWidgetSize`
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
    /// @see `Widget::loadClipShader`
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
    /// Refreshes characters' size based on button's width and alligns 
    /// the text in the center, can move image position if they overlap.
    ///
    /// @see `setText`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateTextSize();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates image position if image is available
    ///
    /// Re-positions the image to the left corner of the button, if image 
    /// is not provided then returns without changes.
    ///
    /// @see `setImage`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateImagePosition();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates image size if image is available
    ///
    /// Considers button's dimensions to determine a new size for icon image,
    /// only called during resizing operations.
    ///
    /// @see `setImage`
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
    float m_charSizeRatio = 0.4f;                                       //!< Button's height occupied by text, prio until it's set to 0.f
    unsigned int m_charSize = parameters::CHAR_SIZE;                    //!< Manual text size assigned by user, not used until ratio is 0.f

    Side m_imagePosition = Side::Center;                                //!< Which side does the image occupies, can't be the same as text position (center by default, adjusted during runtime)
    Side m_textPosition = Side::Center;                                 //!< Which side does the text occupies, can't be the same as image position (center by default, adjusted during runtime)
    ButtonStyle m_style = Theme::button_style::BUTTON_STALE_VIOLET;     //!< Button's default style

public:

enum class Side : int
    {
        Center,
        Left,
        Right
    };
};

template <>
struct __WidgetStyle<Button>
{
    using StyleType = ButtonStyle;

    static const StyleType& get(const WidgetTheme& t)
    {
        return t.m_buttonTheme;
    }

    static void set(WidgetTheme& t, StyleType _b)
    {
        t.m_buttonTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_buttonToken;
    }
};
}
#endif