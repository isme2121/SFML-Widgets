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
    ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& image_path = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param image `sf::Image` object reference (copy data)
    /// @param ref_path a path just for reference (does not influence results except for `ImageFrame::getImagePath()` return)
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::Image& image, const sf::String& ref_path = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `sf::RenderWindow`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param texture_ptr shared pointer to a read only `sf::Texture` object, texture memory must remain alive until no `ImageFrame` is using it
    /// @param ref_path a path just for reference (does not influence results except for `ImageFrame::getImagePath()` return)
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, std::shared_ptr<sf::Texture>& texture_ptr, const sf::String& ref_path = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `Frame`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param image_path path to the displayed image
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& image_path = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `Frame`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param image `sf::Image` object reference (copy data)
    /// @param ref_path a path just for reference (does not influence results except for `ImageFrame::getImagePath()` return)
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Image& image, const sf::String& ref_path = "");

    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs the image frame with `Frame`
    ///
    /// @param root window to draw on
    /// @param position frame's initial position
    /// @param size frame's initial size
    /// @param texture_ptr shared pointer to a read only `sf::Texture` object, texture memory must remain alive until no `ImageFrame` is using it
    /// @param ref_path a path just for reference (does not influence results except for `ImageFrame::getImagePath()` return)
    ///
    ///////////////////////////////////////////////////////////////////
    ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, std::shared_ptr<sf::Texture>& texture_ptr, const sf::String& ref_path = "");

    ImageFrame(ImageFrame&& other) = delete;
    ImageFrame& operator=(ImageFrame&& other) = delete;

    ImageFrame(const ImageFrame&) = delete;
    ImageFrame& operator=(const ImageFrame&) = delete;

    ~ImageFrame() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates displayed image on the widget
    ///
    /// Takes a path to the image and loads it from disk or cache,
    /// can also take a shader for clipping the image (already loaded by default).
    /// 
    /// @param image_path path to image
    /// @param shader path to shader or the shader content itself, or an empty string
    /// @param loading_type choose the appropriate loading type based on shader parameter
    ///
    /// @return false if process fails to load the image, otherwise returns true
    ///
    /// @see `clearImage`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setImage(const sf::String& image_path, const std::string_view& shader = "", W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates displayed image on the widget
    ///
    /// Takes a reference to an existing image and creates a resized copy for the widget,
    /// can also take a shader for clipping the image (already loaded by default).
    /// 
    /// @param image `sf::Image` object reference (copy data)
    /// @param ref_path string path for reference (does not influence results except for `ImageFrame::getImagePath()` return)
    /// @param shader path to shader or the shader content itself, or an empty string
    /// @param loading_type choose the appropriate loading type based on shader parameter
    ///
    /// @return false if process fails to load the image, otherwise returns true
    ///
    /// @see `clearImage`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setImage(const sf::Image& image, const sf::String& ref_path = "", const std::string_view& shader = "", W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates displayed image on the widget
    ///
    /// Takes a reference to a pre-loaded texture, the texture must remain valid
    /// as long as an image frame is using it.
    /// 
    /// @param ref_texture shared pointer to a read only `sf::Texture` object, texture memory must remain alive until no `ImageFrame` is using it
    /// @param ref_path string path for reference (does not influence results except for `ImageFrame::getImagePath()` return)
    /// @param shader path to shader or the shader content itself, or an empty string
    /// @param loading_type choose the appropriate loading type based on shader parameter
    ///
    /// @return false if process fails to load the image, otherwise returns true
    ///
    /// @see `clearImage`
    ///
    ///////////////////////////////////////////////////////////////////
    bool setImage(std::shared_ptr<sf::Texture>& ref_texture, const sf::String& ref_path = "", const std::string_view& shader = "", W_LoadType loading_type = W_LoadType::Memory);

    ///////////////////////////////////////////////////////////////////
    /// @brief Clears the current image data from the widget, does nothing
    /// if no image is loaded
    ///
    /// @see `setImage`
    ///
    ///////////////////////////////////////////////////////////////////
    void clearImage();

    ///////////////////////////////////////////////////////////////////
    /// @brief Creates a copy image of the current texture and returns it
    /// if available
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] std::optional<const sf::Image> getImage() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the current loaded image's path
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::String& getImagePath() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the original image dimensions
    ///
    /// This may not be related to the widget's current size as it creates
    /// a resized copy to fit into the image frame, but it keeps a reference
    /// to the loaded image's original size.
    ///
    ///////////////////////////////////////////////////////////////////
    [[nodiscard]] const sf::Vector2u& getOriginalImageSize() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the widget color theme
    ///
    /// Assigned style overrides the current style even if the widget is assigned
    /// to a shared theme object, however, an update from the said object
    /// would also override parameter style in the future.
    ///
    ///////////////////////////////////////////////////////////////////
    void setTheme(TextureStyle s_imageFrame);

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
    [[nodiscard]] TextureStyle& style();

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
    /// @brief Clears weak reference to external texture
    ///
    ///////////////////////////////////////////////////////////////////
    void resetExternal();
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Clears "No Image" text from widget
    ///
    ///////////////////////////////////////////////////////////////////
    void resetText();

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
    std::unique_ptr<sf::Sprite> m_spritePtr;                            //!< Sprite to display the texture
    
    std::unique_ptr<sf::Texture> m_imageTexturePtr;                     //!< Image texture
    std::unique_ptr<sf::Text> m_noImageTextPtr;                         //!< Text displayed when no image is selected
    
    std::mutex m_guard;                                                 //!< For multi-threading safety when loading an image from a separate thread
    std::weak_ptr<sf::Texture> m_extImagePtr;                           //!< Weak pointer to an external read only texture

    sf::String m_imagePath;                                             //!< Path to currently loaded image
    sf::Vector2u m_originalImageSize{0, 0};                             //!< Original dimensions of the loaded image
    TextureStyle m_style{Theme::texture_style::STYLE_STALE_VIOLET};     //!< Image frame's default style
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