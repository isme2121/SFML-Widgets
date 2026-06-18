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
    /// @brief Updates displayed image on the widget
    ///
    /// Takes the path of the image to display on the frame, can also load
    /// a shader for the image frame if no empty string was provided, defaults
    /// to parameters shader.
    /// 
    /// @param image_path path to image
    /// @param shader_path path to clipping shader
    ///
    /// @return false if process fails to load the image, otherwise returns true
    ///
    /// @see `clearImage`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setImage(const sf::String& image_path, const std::string_view& shader_path = parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);

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
    const sf::String& getImagePath() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle s_imageFrame);

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Draws the widget on master object
    ///
    ///////////////////////////////////////////////////////////////////
    void drawWidget() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Keeps background image disabled
    ///
    ///////////////////////////////////////////////////////////////////
    void config(FLAGS mask) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Handles any extra drawables' positions within the image frame,
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
    /// @brief matches the current angle value with the rest of the frame
    ///
    ///////////////////////////////////////////////////////////////////
    void updateRotation() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget colors to match current state alongside
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
    std::unique_ptr<sf::Sprite> m_imageSpritePtr;                       //!< Sprite to display the texture

    std::unique_ptr<sf::Texture> m_imageTexturePtr;                     //!< Image texture
    std::unique_ptr<sf::Text> m_noImageTextPtr;                         //!< Text displayed when no image is selected

    sf::String m_imagePath;                                             //!< Path to currently loaded image
    TextureStyle m_style = Theme::texture_style::STYLE_STALE_VIOLET;    //!< Image frame's default style
};

template <>
struct WidgetStyle<ImageFrame>
{
    using StyleType = TextureStyle;

    static const StyleType& get(const WidgetTheme& theme)
    {
        return theme.m_imageFrameTheme;
    }

    static void set(WidgetTheme& theme, StyleType s_image_frame)
    {
        theme.m_imageFrameTheme = s_image_frame;
    }

    static std::size_t& token(WidgetTheme& theme)
    {
        return theme.m_imageFrameToken;
    }
};
}
#endif