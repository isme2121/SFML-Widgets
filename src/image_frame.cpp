#include <cmath>
#include "../include/image_frame.hpp"

namespace sfml_widgets
{
ImageFrame::ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& image_path) : Widget(root, position, size), m_imagePath(image_path)
{
    // this guarantees sprite allocation with the correct texture
    if(!(!m_imagePath.empty() && setImage(m_imagePath))) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& image_path) : Widget(root, position, size), m_imagePath(image_path)
{
    // this guarantees sprite allocation with the correct texture
    if(!(!m_imagePath.empty() && setImage(m_imagePath))) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::~ImageFrame()
{
    clearImage();
}

bool ImageFrame::captureKeyEvents(const sf::Event& event)
{
    if(checkFlag(m_flags, FLAGS::BLOCKED | FLAGS::RESTRICTED) || !checkFlag(m_flags, FLAGS::AUTOSELECT | FLAGS::SELECTED)) return false;

    if(m_binds.checkEvents(event)) //!< triggers callback
    {
        if(parameters::QUEUE_DELAY != sf::Time::Zero)
        {
            // rate limit
            setFlag(m_flags, FLAGS::RESTRICTED);
            addToQueue(parameters::QUEUE_DELAY, [this]()
            {
                resetFlag(m_flags, FLAGS::RESTRICTED);
            }, 
            true);
        }
        return true;
    }
    return false;
}

bool ImageFrame::setImage(const std::string& image_path)
{
    // load from disk
    ImageCache& global_image_buffer = ImageCache::getInstance(); 
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(image_path);
    if(!disk_image) return false;

    // attempt to make a resized copy
    std::optional<sf::Image> resized_image = resize_image(**disk_image, sf::Vector2u(m_size));
    if(!resized_image)
    {
        global_image_buffer.decrementRefCounter(image_path);
        return false;
    }

    // attempt to copy it
    sf::Texture temp;
    if(!temp.loadFromImage(*resized_image))
    {
        global_image_buffer.decrementRefCounter(image_path);
        return false;
    }

    if(checkFlag(m_flags, FLAGS::IMAGE)) global_image_buffer.decrementRefCounter(m_imagePath);

    // free text memory
    if(m_textTexturePtr)
    {
        m_textTexturePtr->clear(sf::Color::Transparent);
        m_textTexturePtr.reset();
        m_noImageTextPtr.reset();
        resetFlag(m_flags, FLAGS::TEXT);
    }

    // texture update
    m_imageTexturePtr = std::make_unique<sf::Texture>(std::move(temp));
    m_imagePath = std::move(image_path);
    m_imageTexturePtr->setSmooth(true);

    // allocate sprite if needed
    if(!m_imageSpritePtr) m_imageSpritePtr = std::make_unique<sf::Sprite>(*m_imageTexturePtr);
    else m_imageSpritePtr->setTexture(*m_imageTexturePtr, true);

    // just for isFlag
    setFlag(m_flags, FLAGS::IMAGE);

    return true;
}

void ImageFrame::clearImage()
{
    if(!m_imageTexturePtr) return; // no image exists already

    ImageCache::getInstance().decrementRefCounter(m_imagePath);
    resetFlag(m_flags, FLAGS::IMAGE);
    m_imageTexturePtr.reset();
    m_imagePath.clear();
    writeText(); //!< replace with text
}

std::optional<const sf::Image> ImageFrame::getImage() const
{
    if(m_imageTexturePtr) return m_imageTexturePtr->copyToImage();
    return std::nullopt;
}

const std::string& ImageFrame::getImagePath() const
{
    return m_imagePath;
}

void ImageFrame::setTheme(TextureStyle imageframe_style)
{
    m_style = imageframe_style;
    setFlag(m_flags, FLAGS::MODIFIED);
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void ImageFrame::drawWidget()
{
    // check if we should draw text
    if(m_textTexturePtr && m_noImageTextPtr)
    {
        // render txr is already cleared in widget
        m_textTexturePtr->draw(*m_noImageTextPtr);
        m_textTexturePtr->display();
        m_imageSpritePtr->setTexture(m_textTexturePtr->getTexture(), true);
    }

    // if we have an image, it would already be assigned to sprite
    // so no need for setTexture call
    if(m_clipShaderPtr) m_master.draw(*m_imageSpritePtr, m_clipShaderPtr.get());
    else m_master.draw(*m_imageSpritePtr);
}

void ImageFrame::config(FLAGS mask)
{
    // keep bg image blocked
    if(!checkFlag(m_flags, FLAGS::NOBGIMAGE)) setFlag(m_flags, FLAGS::NOBGIMAGE);
}

void ImageFrame::updatePosition()
{
    /*
    m_position is relative to the top left corner of the shape,
    we do this since sprite's origin is it's center, just
    like the base widget shape
    */
    m_imageSpritePtr->setPosition(m_position + m_size / 2.f);
}

void ImageFrame::updateSize()
{
    // diplaying image
    if(m_imageTexturePtr && m_imageTexturePtr->resize(sf::Vector2u(m_size)))
        updateImageSize();

    // diplaying text
    else if(m_textTexturePtr && m_textTexturePtr->resize(sf::Vector2u(m_size)))
        updateText();

    m_imageSpritePtr->setOrigin(m_size / 2.f);

    updateClipShader();
}

void ImageFrame::updateRotation()
{
    // handles both cases
    m_imageSpritePtr->setRotation(this->getRotation());
}

void ImageFrame::updateState()
{
    // here render txr clears
    if(m_textTexturePtr) m_textTexturePtr->clear(sf::Color::Transparent);

    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.2f;
        this->setFillColor(m_style.fill_blocked_color);
        this->setOutlineColor(m_style.outline_blocked_color);  
        this->setOutlineThickness(m_style.outline_blocked_thickness); 
    }
    else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.1f;
        this->setFillColor(m_style.fill_transparent_color);
        this->setOutlineColor(m_style.outline_transparent_color);
        this->setOutlineThickness(m_style.outline_transparent_thickness);
    }
    else if(checkFlag(m_flags, FLAGS::ACTIVE))
    {
        m_opacity = 0.8f;
        this->setFillColor(m_style.fill_active_color);
        this->setOutlineColor(m_style.outline_active_color);
        this->setOutlineThickness(m_style.outline_active_thickness);
    }
    else if(checkFlag(m_flags, FLAGS::HOVERED))
    {
        m_opacity = 0.9f;
        this->setFillColor(m_style.fill_hover_color);
        this->setOutlineColor(m_style.outline_hover_color);  
        this->setOutlineThickness(m_style.outline_hover_thickness); 
    }
    else
    {
        m_opacity = 1.f;
        this->setFillColor(m_style.fill_color);
        this->setOutlineColor(m_style.outline_color);
        this->setOutlineThickness(m_style.outline_thickness);
    }
    updateClipShader();
}

void ImageFrame::updateTheme()
{
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<ImageFrame>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<ImageFrame>());
    }
}

void ImageFrame::updateClipShader()
{
    if(!m_clipShaderPtr) return;

    m_clipShaderPtr->setUniform("texture", sf::Shader::CurrentTexture);
    m_clipShaderPtr->setUniform("size", sf::Glsl::Vec2(m_size));
    m_clipShaderPtr->setUniform("radius", m_radius);
    m_clipShaderPtr->setUniform("opacity", m_opacity);
}

void ImageFrame::updateColorShader()
{
    // no bg image
}

void ImageFrame::writeText()
{
    if(m_imageTexturePtr) return; //!< we have an image loaded

    m_textTexturePtr = std::make_unique<sf::RenderTexture>(sf::Vector2u(m_size));
    m_noImageTextPtr = std::make_unique<sf::Text>(parameters::font, "\t No\nImage");

    // allocate sprite here if needed
    if(!m_imageSpritePtr) m_imageSpritePtr = std::make_unique<sf::Sprite>(m_textTexturePtr->getTexture());

    setFlag(m_flags, FLAGS::TEXT);
    updateText();
}

void ImageFrame::updateText()
{
    if(!m_noImageTextPtr) return; //!< no text allocated

    float target_height = m_size.y * 0.2f;  //!< one line takes 30% of the shape's height
    float current_height = W_CharHeight::getHeight(m_noImageTextPtr->getFont(),
                                                   m_noImageTextPtr->getCharacterSize());

    float ratio = target_height / current_height;
    m_noImageTextPtr->setCharacterSize(std::floor<unsigned int>(m_noImageTextPtr->getCharacterSize() * ratio));

    // placing text on the center of the widget
    sf::FloatRect bounds = m_noImageTextPtr->getLocalBounds();
    m_noImageTextPtr->setOrigin(bounds.position + bounds.size / 2.f);
    m_noImageTextPtr->setPosition(m_size / 2.f);
}

void ImageFrame::updateImageSize()
{
    if(!m_imageTexturePtr) return; //!< no image
    std::optional<sf::Image> resized_image;

    // this loads an existing image
    std::optional<const sf::Image*> disk_image = ImageCache::getInstance().getImage(m_imagePath, false);
    
    // resize from original image to preserve as much quality as possible
    if(disk_image) resized_image = resize_image(**disk_image, sf::Vector2u(m_size));

    // otherwise resize from used image if the original is somehow not in cache
    else resized_image = resize_image(m_imageTexturePtr->copyToImage(), sf::Vector2u(m_size));

    if(resized_image)
    {
        sf::Texture temp;
        if(temp.loadFromImage(*resized_image))
        {
            m_imageTexturePtr = std::make_unique<sf::Texture>(std::move(temp));
            m_imageTexturePtr->setSmooth(true);
            m_imageSpritePtr->setTexture(*m_imageTexturePtr, true);
        }
    }
}
}