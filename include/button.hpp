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
    explicit Button(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the button on a `Frame` object
    ///
    /// @param root root frame widget
    /// @param position button's initial position
    /// @param size button's initial size
    /// @param text text displayed on the button initially
    ///
    ///////////////////////////////////////////////////////////////////
    explicit Button(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text = "");

    Button(Button&&) noexcept;
    Button& operator=(Button&&) noexcept;
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    ~Button() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget mouse events
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
    /// @return true if an event triggers within the widget or a child widget,
    /// otherwise false.
    ///
    /// @note button with `FLAGS::BLOCKED` or `FLAGS::HIDDEN` states cannot
    /// trigger mouse events.
    ///
    /// @see `Widget::bindMouseEvent`, `Widget::unbindMouseEvent`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position = std::nullopt) override;
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Checks for widget keyboard events
    ///
    /// Captures keyboard events in the events loop, can capture individual
    /// physical key presses or a sequence of key press events depending on
    /// available assigned sequences.
    ///
    /// @param event Event object from `sf::WindowBase::pollEvent`
    ///
    /// @return true if the button triggers a callback, otherwise false.
    ///
    /// @note Button must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, Buttons with `FLAGS::BLOCKED | FLAGS::RESTRICTED`
    /// cannot trigger keyboard events.
    ///
    /// @see `Widget::bindShortCut`, `Widget::unbindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates button's text
    ///
    /// Displays parameter text on the button's surface, text's length
    /// can go up to `15` characters max.
    ///
    /// @param text string text
    ///
    /// @see `parameters::CHAR_LIMIT`
    ///
    ///////////////////////////////////////////////////////////////////
    void setText(const std::string& text);

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
    /// @note This assignment does not take effect until the ratio is set
    /// to `0.f` which by default is `0.2f`.
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
    /// @brief Sets a font for the button's text
    ///
    ///////////////////////////////////////////////////////////////////
    void setTextFont(sf::Font font);

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
    bool setImage(const std::string& image_path);

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

    ButtonStyle m_style = themes::BUTTON_CARBON_CYAN;  //!< Button's color theme

private:

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the button on master object
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
    /// @brief Refresh for color shader
    ///
    /// Best called after an update in a shader parameters, color shader
    /// is applied on background texture that is set via
    /// `Widget::setBackGroundTexture`.
    ///
    /// @see `Widget::loadColorShader`
    ///
    ///////////////////////////////////////////////////////////////////
    void updateColorShader() override;

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
    sf::Texture m_texture;                                              //!< Image texture
    sf::Sprite m_image;                                                 //!< Image sprite
    std::string m_imagePath;                                            //!< Path for used image
    float m_charSizeRatio = 0.2f;                                       //!< Button's height occupied by text, prio until it's set to 0.f
    unsigned int m_charSize = 16;                                       //!< Manual text size assigned by user, not used until ratio is 0.f
    Side m_imagePosition = Side::Center;                                //!< Which side does the image occupies, can't be the same as text position (center by default, adjusted during runtime)
    Side m_textPosition = Side::Center;                                 //!< Which side does the text occupies, can't be the same as image position (center by default, adjusted during runtime)

public:
    enum class Side
    {
        Center,
        Left,
        Right
    };
};

}
#endif