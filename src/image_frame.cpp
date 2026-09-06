#include <cmath>
#include "image_frame.hpp"

namespace sfml_widgets
{
ImageFrame::ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& image_path) : Widget(root, position, size)
{
    // this guarantees sprite allocation with the correct texture
    if(image_path.isEmpty() || !setImage(image_path)) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);

}

ImageFrame::ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::Image& image, const sf::String& image_path) : Widget(root, position, size)
{
    // this guarantees sprite allocation with the correct texture
    if(!setImage(image, image_path)) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, std::shared_ptr<const sf::Texture>& ref_texture, const sf::String& image_path) : Widget(root, position, size)
{
    // this guarantees sprite allocation with the correct texture
    if(!setImage(ref_texture, image_path)) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& image_path) : Widget(root, position, size)
{
    // this guarantees sprite allocation with the correct texture
    if(image_path.isEmpty() || setImage(image_path)) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::Image& image, const sf::String& image_path) : Widget(root, position, size)
{
    // this guarantees sprite allocation with the correct texture
    if(!setImage(image, image_path)) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, std::shared_ptr<const sf::Texture>& ref_texture, const sf::String& image_path) : Widget(root, position, size)
{
    // this guarantees sprite allocation with the correct texture
    if(!setImage(ref_texture, image_path)) writeText();

    setWidgetPosition(m_position);
    setWidgetSize(m_size);
    loadClipShader(SPRITE_SHADER_FRAG, W_LoadType::Memory);

    // no need for background texture
    setFlag(m_flags, FLAGS::NOBGIMAGE);
}

ImageFrame::~ImageFrame()
{
    if(m_imageTexturePtr)
    {
        if(checkFlag(m_flags, FLAGS::SET))
        {
            ImageCache::getInstance().decrementRefCounter(m_imagePath);
            resetFlag(m_flags, FLAGS::SET);
        }
    }
}

bool ImageFrame::setImage(const sf::String& image_path, const std::string_view& shader, W_LoadType loading_type)
{
    // load from disk or cache
    ImageCache& global_image_buffer = ImageCache::getInstance(); 
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(image_path);
    if(!disk_image) return false;

    // attempt to make a resized copy
    sf::Vector2u original_size = (*disk_image)->getSize();
    std::optional<sf::Image> resized_image;

    // skip resizing if image size = frame size
    if(original_size == sf::Vector2u(m_size)) resized_image = **disk_image;
    else resized_image = resize_image(**disk_image, sf::Vector2u(m_size));

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

    // texture update
    if(m_imageTexturePtr)
    {
        global_image_buffer.decrementRefCounter(m_imagePath);
        *m_imageTexturePtr = std::move(temp);
    }
    else
    {
        m_imageTexturePtr = std::make_unique<sf::Texture>(std::move(temp));
    }
    m_imagePath = std::move(image_path);
    m_imageTexturePtr->setSmooth(true);
    m_originalImageSize = original_size;

    // allocate sprite if needed
    if(!m_spritePtr) m_spritePtr = std::make_unique<sf::Sprite>(*m_imageTexturePtr);
    else m_spritePtr->setTexture(*m_imageTexturePtr, true);

    // allocate clip shader if provided
    if(!shader.empty() && !m_clipShaderPtr) loadClipShader(shader, loading_type);

    resetExternal();

    {
        std::lock_guard<std::mutex> lock(m_guard);
        // free text memory
        resetText();
    }

    // IMAGE flag for isFlag check, SET to clear the ImageCache reference later
    setFlag(m_flags, FLAGS::IMAGE | FLAGS::SET);
    return true;
}

bool ImageFrame::setImage(const sf::Image& image, const sf::String& ref_path, const std::string_view& shader, W_LoadType loading_type)
{
    // attempt to make a resized copy
    sf::Vector2u original_size = image.getSize();
    std::optional<sf::Image> resized_image;

    // skip resizing if image size = frame size
    if(original_size == sf::Vector2u(m_size)) resized_image = image;
    else resized_image = resize_image(image, sf::Vector2u(m_size));

    if(!resized_image) return false;

    sf::Texture temp;
    if(!temp.loadFromImage(*resized_image)) return false;

    // texture update
    if(m_imageTexturePtr)
    {
        if(checkFlag(m_flags, FLAGS::SET))
        {
            // we know that imagePath belongs to the image directly loaded from disk
            ImageCache::getInstance().decrementRefCounter(m_imagePath);
            resetFlag(m_flags, FLAGS::SET);
        }
        *m_imageTexturePtr = std::move(temp);
    }
    else
    {
        m_imageTexturePtr = std::make_unique<sf::Texture>(std::move(temp));
    }

    m_imagePath = ref_path;
    m_imageTexturePtr->setSmooth(true);
    m_originalImageSize = image.getSize();

    // allocate sprite if needed
    if(!m_spritePtr) m_spritePtr = std::make_unique<sf::Sprite>(*m_imageTexturePtr);
    else m_spritePtr->setTexture(*m_imageTexturePtr, true);

    // allocate clip shader if provided
    if(!shader.empty() && !m_clipShaderPtr) loadClipShader(shader, loading_type);

    resetExternal();

    {
        std::lock_guard<std::mutex> lock(m_guard);
        // free text memory
        resetText();
    }

    // IMAGE flag for isFlag check, not setting SET since we didn't load it directly from disk
    setFlag(m_flags, FLAGS::IMAGE);
    return true;
}

bool ImageFrame::setImage(std::shared_ptr<const sf::Texture>& texture_ref, const sf::String& ref_path, const std::string_view& shader, W_LoadType loading_type)
{
    if(!texture_ref) return false;

    m_extImagePtr = texture_ref;

    // texture update
    if(m_imageTexturePtr)
    {
        if(checkFlag(m_flags, FLAGS::SET))
        {
            // we know that imagePath belongs to the image directly loaded from disk
            ImageCache::getInstance().decrementRefCounter(m_imagePath);
            resetFlag(m_flags, FLAGS::SET);
        }
        m_imageTexturePtr.reset();
    }

    m_imagePath = ref_path;
    m_originalImageSize = texture_ref->getSize();
    setWidgetSize(sf::Vector2f(m_originalImageSize));

    // allocate sprite if needed
    if(!m_spritePtr) m_spritePtr = std::make_unique<sf::Sprite>(*m_extImagePtr.lock());
    else m_spritePtr->setTexture(*m_extImagePtr.lock(), true);

    // allocate clip shader if provided
    if(!shader.empty() && !m_clipShaderPtr) loadClipShader(shader, loading_type);

    {
        std::lock_guard<std::mutex> lock(m_guard);
        // free text memory
        resetText();
    }

    // IMAGE flag for isFlag check, not setting SET since we didn't load it directly from disk
    setFlag(m_flags, FLAGS::IMAGE | FLAGS::RESET);
    return true;
}

void ImageFrame::clearImage()
{
    if(m_imageTexturePtr)
    {
        if(checkFlag(m_flags, FLAGS::SET))
        {
            ImageCache::getInstance().decrementRefCounter(m_imagePath);
            resetFlag(m_flags, FLAGS::SET);
        }
        m_imageTexturePtr.reset();
    }

    if(!m_extImagePtr.expired()) m_extImagePtr.reset();

    m_imagePath.clear();
    m_originalImageSize = {0, 0};

    resetFlag(m_flags, FLAGS::IMAGE | FLAGS::RESET);
    writeText(); //!< replace with text
}

std::optional<const sf::Image> ImageFrame::getImage() const
{
    if(m_imageTexturePtr) return m_imageTexturePtr->copyToImage();
    else if(auto image = m_extImagePtr.lock()) return image->copyToImage();
    return std::nullopt;
}

const sf::String& ImageFrame::getImagePath() const
{
    return m_imagePath;
}

const sf::Vector2u& ImageFrame::getOriginalImageSize() const
{
    return m_originalImageSize;
}

void ImageFrame::setTheme(TextureStyle s_imageFrame)
{
    m_style = s_imageFrame;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void ImageFrame::fetchTheme()
{
    if(auto t = m_themePtr.lock()) setTheme(t->getStyle<ImageFrame>());
}

TextureStyle& ImageFrame::style()
{
    return m_style;
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void ImageFrame::drawWidget()
{
    std::lock_guard<std::mutex> lock(m_guard);

    if(checkFlag(m_flags, FLAGS::RESET) && m_extImagePtr.expired()) resetExternal();

    // check if we should draw text
    if(m_textTexturePtr && m_noImageTextPtr)
    {
        // render texture is already cleared in updateState
        m_textTexturePtr->draw(*m_noImageTextPtr);
        m_textTexturePtr->display();
    }

    // if we have an image, it would already be assigned to sprite
    // so no need for setTexture call
    if(m_clipShaderPtr) m_master.draw(*m_spritePtr, m_clipShaderPtr.get());
    else m_master.draw(*m_spritePtr);
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
    m_spritePtr->setPosition(m_position + m_size / 2.f);
}

void ImageFrame::updateSize()
{
    // using external reference = block resizing
    if(!m_extImagePtr.expired()) m_size = sf::Vector2f(m_originalImageSize);

    // diplaying image
    else if(m_imageTexturePtr && m_imageTexturePtr->resize(sf::Vector2u(m_size)))
        updateImageSize(); //!< updates texture

    // diplaying text
    else if(m_textTexturePtr && m_textTexturePtr->resize(sf::Vector2u(m_size)))
    {
        updateText();
        m_spritePtr->setTexture(m_textTexturePtr->getTexture(), true);
    }

    m_spritePtr->setOrigin(m_size / 2.f);
    updateClipShader();
}

void ImageFrame::updateRotation()
{
    // handles both cases
    m_spritePtr->setRotation(this->getRotation());
}

void ImageFrame::updateState()
{
    // here render txr clears
    if(m_textTexturePtr) m_textTexturePtr->clear(sf::Color::Transparent);

    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.5f;
        this->setFillColor(m_style.fill_blocked_color);
        this->setOutlineColor(m_style.outline_blocked_color);  
        this->setOutlineThickness(m_style.outline_blocked_thickness); 
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
    {
        m_opacity = 0.2f;
        this->setFillColor(m_style.fill_transparent_color);
        this->setOutlineColor(m_style.outline_transparent_color);
        this->setOutlineThickness(m_style.outline_transparent_thickness);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ACTIVE))
    {
        m_opacity = 1.f;
        this->setFillColor(m_style.fill_active_color);
        this->setOutlineColor(m_style.outline_active_color);
        this->setOutlineThickness(m_style.outline_active_thickness);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
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

void ImageFrame::resetExternal()
{
    resetFlag(m_flags, FLAGS::RESET);
    m_extImagePtr.reset();
    writeText();
}

void ImageFrame::resetText()
{
    if(m_textTexturePtr)
    {
        // clear content from window
        m_textTexturePtr->clear(sf::Color::Transparent);
        m_textTexturePtr.reset();
        m_noImageTextPtr.reset();
        resetFlag(m_flags, FLAGS::TEXT);
    }
}

void ImageFrame::writeText()
{
    if(m_imageTexturePtr) return; //!< we have an image loaded

    m_textTexturePtr = std::make_unique<sf::RenderTexture>(sf::Vector2u(m_size));
    m_noImageTextPtr = std::make_unique<sf::Text>(parameters::INTER_REGULAR_18, "\tNo\nImage");

    // allocate sprite here if needed
    if(!m_spritePtr) m_spritePtr = std::make_unique<sf::Sprite>(m_textTexturePtr->getTexture());
    else m_spritePtr->setTexture(m_textTexturePtr->getTexture(), true);

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
            if(!m_spritePtr) m_spritePtr = std::make_unique<sf::Sprite>(*m_imageTexturePtr);
            else m_spritePtr->setTexture(*m_imageTexturePtr, true);
        }
    }
}
}