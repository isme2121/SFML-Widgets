#include "../include/image_frame.hpp"

namespace sfml_widgets
{
ImageFrame::ImageFrame(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& image_path) : Widget(root, position, size), m_imageTexture(), m_imageSprite(m_imageTexture), m_imagePath(image_path)
{
    setWidgetPosition(position);
    setWidgetSize(size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);
    if(!m_imagePath.empty()) setImage(m_imagePath);
    setFlag(m_flags, FLAGS::BLOCKBGIMAGE);
}

ImageFrame::ImageFrame(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& image_path) : Widget(root, position, size), m_imageTexture(), m_imageSprite(m_imageTexture), m_imagePath(image_path)
{
    setWidgetPosition(position);
    setWidgetSize(size);
    loadClipShader(parameters::SPRITE_FULLCLIP_OPACITY_SHADER_PATH);
    if(!m_imagePath.empty()) setImage(m_imagePath);
    setFlag(m_flags, FLAGS::BLOCKBGIMAGE);
}

ImageFrame::ImageFrame(ImageFrame&& other) noexcept : Widget(std::move(other)), m_imageTexture(std::move(other.m_imageTexture)),
m_imageSprite(std::move(other.m_imageSprite)), m_imagePath(std::move(other.m_imagePath)), m_style(other.m_style)
{
    m_imageSprite.setTexture(m_imageTexture, true);
}

ImageFrame& ImageFrame::operator=(ImageFrame&& other) noexcept
{
    if(this != &other)
    {
        static_cast<Widget&>(*this) = std::move(other);

        m_style = other.m_style;
        m_imageTexture= std::move(other.m_imageTexture);
        m_imageSprite = std::move(other.m_imageSprite);
        m_imagePath = std::move(other.m_imagePath);

        m_imageSprite.setTexture(m_imageTexture, true);
    }
    return *this;
}

ImageFrame::~ImageFrame()
{
    clearImage(false);
}

bool ImageFrame::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED | FLAGS::RESTRICTED)) return false;

    if(!mouse_position) *mouse_position = getMousePosition();

    return mouseCallBackEvent(event, *mouse_position).is_used;
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
    std::optional<sf::Image> resized_image = resize_image(**disk_image, m_size);
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
    else if(checkFlag(m_flags, FLAGS::DEFAULTIMAGE)) global_image_buffer.decrementRefCounter(parameters::DEFAULT_IMAGE_PATH);

    // texture update
    m_imageTexture = std::move(temp);
    m_imagePath = std::move(image_path);
    m_imageTexture.setSmooth(true);
    m_imageSprite.setTexture(m_imageTexture, true);

    setFlag(m_flags, FLAGS::IMAGE);
    resetFlag(m_flags, FLAGS::DEFAULTIMAGE);
    updateClipShader();
    return true;
}

void ImageFrame::clearImage(bool load_default)
{
    if(!checkFlag(m_flags, FLAGS::IMAGE)) return;
    ImageCache::getInstance().decrementRefCounter(m_imagePath);
    resetFlag(m_flags, FLAGS::IMAGE);
    m_imageTexture = sf::Texture();
    m_imagePath.clear();
    if(load_default) loadDefaultImage();
}

void ImageFrame::drawWidget()
{
    m_master.draw(m_imageSprite, &m_clipShader);
}

void ImageFrame::config(FLAGS mask)
{
    if(!checkFlag(m_flags, FLAGS::BLOCKBGIMAGE)) setFlag(m_flags, FLAGS::BLOCKBGIMAGE);
}

void ImageFrame::updatePosition()
{
    // updates text and image
    m_imageSprite.setPosition(m_position);
}

void ImageFrame::updateSize()
{
    // all methods that scale with size
    if(!m_imageTexture.resize(m_size)) return;

    // update texture in sprite
    m_imageSprite.setTexture(m_imageTexture, true);

    updateClipShader();
    update();
}

void ImageFrame::updateRotation()
{
    m_imageSprite.setRotation(this->getRotation());
}

void ImageFrame::updateState()
{
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

void ImageFrame::updateClipShader()
{
    if(!checkFlag(m_flags, FLAGS::CLIPSHADER)) return;

    m_clipShader.setUniform("texture",  sf::Shader::CurrentTexture);
    m_clipShader.setUniform("size",     sf::Glsl::Vec2(m_size));
    m_clipShader.setUniform("radius",   m_radius);
    m_clipShader.setUniform("opacity", m_opacity);
}

void ImageFrame::updateColorShader()
{
    
}

bool ImageFrame::loadDefaultImage()
{
    if(checkFlag(m_flags, FLAGS::DEFAULTIMAGE)) return false;
    ImageCache& global_image_buffer = ImageCache::getInstance(); 
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(parameters::DEFAULT_IMAGE_PATH);
    if(!disk_image) return false;

    // attempt to make a resized copy
    std::optional<sf::Image> resized_image = resize_image(**disk_image, m_size);
    if(!resized_image)
    {
        global_image_buffer.decrementRefCounter(parameters::DEFAULT_IMAGE_PATH);
        return false;
    }

    // attempt to copy it
    sf::Texture temp;
    if(!temp.loadFromImage(*resized_image))
    {
        global_image_buffer.decrementRefCounter(parameters::DEFAULT_IMAGE_PATH);
        return false;
    }

    if(checkFlag(m_flags, FLAGS::IMAGE)) global_image_buffer.decrementRefCounter(m_imagePath);
    // texture update
    m_imageTexture = std::move(temp);
    m_imageTexture.setSmooth(true);
    m_imageSprite.setTexture(m_imageTexture, true);

    setFlag(m_flags, FLAGS::DEFAULTIMAGE);
    resetFlag(m_flags, FLAGS::IMAGE);
    updateClipShader();
    return true;
}
}