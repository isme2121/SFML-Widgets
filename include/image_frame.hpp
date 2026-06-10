#pragma once
#include "widget.hpp"

#ifdef WIDGETS_HPP
namespace sfml_widgets
{
class ImageFrame : public Widget
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param image_path path to the displayed image
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& image_path = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `Frame`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param image_path path to the displayed image
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& image_path = "");

    ImageFrame(ImageFrame&& other) = delete;
    ImageFrame& operator=(ImageFrame&& other) = delete;

    ImageFrame(const ImageFrame&) = delete;
    ImageFrame& operator=(const ImageFrame&) = delete;

    ~ImageFrame() override;

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
    /// @note image frame with `FLAGS::BLOCKED` or `FLAGS::HIDDEN` states cannot
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
    /// @return true if the frame triggers a callback, otherwise false.
    ///
    /// @note The frame must be `FLAGS::SELECTED` or `FLAGS::AUTOSELECT` to
    /// trigger key event, widgets with `FLAGS::BLOCKED | FLAGS::RESTRICTED`
    /// cannot trigger keyboard events.
    ///
    /// @see `Widget::bindShortCut`, `Widget::unbindShortCut`
    ///
    ///////////////////////////////////////////////////////////////////
    bool captureKeyEvents(const sf::Event& event) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates displayed image on the widget
    ///
    /// Takes the path of the image to display on the frame, 
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
    /// @brief Clears the current image data from the widget, does nothing
    /// if no image is loaded
    ///
    /// @see `setImage`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearImage();

    ///////////////////////////////////////////////////////////////////
    /// @brief creates a copy image of the current texture and returns it
    /// if available
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<const sf::Image> getImage() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief returns the current loaded image's path
    ///
    ///////////////////////////////////////////////////////////////////
    const std::string& getImagePath() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle imageframe_style);

private:

    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget on master object
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
    /// @brief Handles any extra drawables' positions within the image frame,
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
    /// @brief matches the current angle value with the rest of the frame
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget colors to match current state alongside
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
    /// @brief Writes "No Image" text on widget box if no image is loaded
    ///
    ///////////////////////////////////////////////////////////////////
    void writeText();

    ///////////////////////////////////////////////////////////////////
    /// @brief Update "No Image" text's size and position, does nothing
    /// if an image is currently loaded
    ///
    ///////////////////////////////////////////////////////////////////
    void updateText();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the loaded image size
    ///
    ///////////////////////////////////////////////////////////////////
    void updateImageSize();

    std::unique_ptr<sf::RenderTexture> m_textTexturePtr;                //!< Text texture
    std::unique_ptr<sf::Texture> m_imageTexturePtr;                     //!< Image texture
    std::unique_ptr<sf::Text> m_noImageTextPtr;                         //!< Text displayed when no image is selected

    std::unique_ptr<sf::Sprite> m_imageSpritePtr;                       //!< Sprite to display the texture

    std::string m_imagePath;                                            //!< Path to currently loaded image
    TextureStyle m_style = Theme::texture_style::STYLE_STALE_VIOLET;    //!< Image frame's default style
};

template <>
struct __WidgetStyle<ImageFrame>
{
    using StyleType = TextureStyle;

    static const StyleType& get(const WidgetTheme& t)
    {
        return t.m_imageFrameTheme;
    }

    static void set(WidgetTheme& t, StyleType _b)
    {
        t.m_imageFrameTheme = _b;
    }

    static std::size_t& token(WidgetTheme& t)
    {
        return t.m_imageFrameToken;
    }
};
}
#endif