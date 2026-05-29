#include "../include/button.hpp"
#include <iostream>

namespace sfml_widgets // Button
{
///////////////////////////////////////////////////////////////////
/// Constructors / Destructor / Operators
///////////////////////////////////////////////////////////////////

Button::Button(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const std::string& text) : Widget(root, position, size),
m_text(parameters::font), m_texture(), m_image(m_texture)
{   
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    // update by order
    // skips certain parts that would be filled later anyways
    setWidgetPosition(m_position);
    // critical for text and shader afterwards
    setWidgetSize(m_size);
    // requires text and size to be updated
    setText(text);
    // depends on widget size
    loadClipShader(parameters::SPRITE_CLIP_COLOR_SHADER_PATH);

}

Button::Button(Frame& root, sf::Vector2f position, sf::Vector2u size, const std::string& text) : Widget(root, position, size),
m_text(parameters::font), m_texture(), m_image(m_texture)
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    // update by order
    // skips certain parts that would be filled later anyways
    setWidgetPosition(m_position);
    // critical for text and shader afterwards
    setWidgetSize(m_size);
    // requires text and size to be updated
    setText(text);
    // depends on widget size
    loadClipShader(parameters::SPRITE_CLIP_COLOR_SHADER_PATH);
}

Button::Button(Button&& other) noexcept : Widget(std::move(other)), m_text(std::move(other.m_text)), 
m_texture(std::move(other.m_texture)), m_image(std::move(other.m_image)), m_style(std::move(other.m_style)),
m_imagePath(std::move(other.m_imagePath))
{
    m_currentBgColorPtr = &m_style.fill_color;
    m_currentOutlineColorPtr = &m_style.outline_color;
    m_currentOutlineThicknessPtr = &m_style.outline_thickness;

    m_image.setTexture(m_texture, true);
}

Button& Button::operator=(Button&& other) noexcept
{
    if(this != &other)
    {
        static_cast<Widget&>(*this) = std::move(other);
        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        m_currentOutlineThicknessPtr = &m_style.outline_thickness;

        m_text = std::move(other.m_text);
        m_texture = std::move(other.m_texture);
        m_image = std::move(other.m_image);
        m_style = std::move(other.m_style);
        m_imagePath = std::move(other.m_imagePath);

        m_image.setTexture(m_texture, true);
    }
    return *this;
}

Button::~Button()
{
    clearImage();
}

bool Button::captureMouseEvents(const sf::Event& event, std::optional<sf::Vector2i> mouse_position)
{
    if(checkFlag(m_flags, FLAGS::HIDDEN | FLAGS::BLOCKED)) return false;

    if(!mouse_position) *mouse_position = getMousePosition();
    return mouseCallBackEvent(event, *mouse_position).is_used;
}

bool Button::captureKeyEvents(const sf::Event& event)
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

void Button::setText(const std::string& text)
{
    if(text.length() >= parameters::CHAR_LIMIT) return;

    if(!text.empty()) setFlag(m_flags, FLAGS::TEXT);
    else resetFlag(m_flags, FLAGS::TEXT);
    
    m_text.setString(text);
    updateTextSize();
    updateTextPosition();
    updateImagePosition(); //!< Fix image position in case TEXT flag state has changed
}

void Button::setTextSide(Button::Side text_side)
{
    if(m_textPosition == text_side) return;
    if(m_imagePosition == text_side)
        m_imagePosition = m_textPosition;
    m_textPosition = text_side;
    updateTextPosition();
    updateImagePosition();
}

void Button::setTextSize(unsigned int char_size)
{
    m_charSizeRatio = 0.f;
    m_charSize = char_size;
    updateTextSize();
    updateTextPosition();
}

void Button::setTextSize(float ratio)
{
    if(ratio < 0.f || ratio >= 1.f) return;
    m_charSizeRatio = ratio;
    updateTextSize();
    updateTextPosition();
}

void Button::setTextFont(sf::Font font)
{
    m_text.setFont(font);
    updateTextSize(); //!< auto update size
    updateTextPosition();
}

bool Button::setImage(const std::string& image_path)
{
    // load from disk
    ImageCache& global_image_buffer = ImageCache::getInstance(); 
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(image_path);
    if(!disk_image) return false;

    // attempt to make a resized copy
    std::optional<sf::Image> resized_image = resize_image(**disk_image, sf::Vector2u(m_size.x / 3, m_size.y)); //!< image takes a third of the button's width
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
    // texture update
    m_texture = std::move(temp);
    m_imagePath = std::move(image_path);
    m_texture.setSmooth(true);
    m_image.setTexture(m_texture, true);

    setFlag(m_flags, FLAGS::IMAGE);
    updateClipShader();
    updateTextPosition();
    updateImagePosition();
    return true;
}

void Button::setImageSide(Button::Side image_side)
{
    if(m_imagePosition == image_side) return;
    if(m_textPosition == image_side)
        m_textPosition = m_imagePosition;
    m_imagePosition = image_side;
    updateTextPosition();
    updateImagePosition();
}

void Button::clearImage()
{
    if(!checkFlag(m_flags, FLAGS::IMAGE)) return;
    resetFlag(m_flags, FLAGS::IMAGE);
    m_texture = sf::Texture();
    ImageCache::getInstance().decrementRefCounter(m_imagePath);
    m_imagePath.clear();
}

void Button::setTheme(ButtonStyle __button_style)
{
    m_style = __button_style;
    setFlag(m_flags, FLAGS::MODIFIED);
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void Button::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::IMAGE)) m_master.draw(m_image, &m_clipShader);
    if(checkFlag(m_flags, FLAGS::TEXT)) m_master.draw(m_text);
}

void Button::config(FLAGS mask)
{

}

void Button::updatePosition()
{
    // updates text and image
    updateTextPosition();
    updateImagePosition();
}

void Button::updateSize()
{
    // all methods that scale with size
    updateClipShader();
    updateTextSize();
    updateTextPosition(); //!< keep text aligned
    updateImageSize();
    updateImagePosition();  //!< image position scales with button size
    update();
}

void Button::updateRotation()
{
    m_text.setRotation(this->getRotation());
    m_image.setRotation(this->getRotation());
    updateTextPosition();
    updateImagePosition();
}

void Button::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.2f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_blocked_color);
    }
    else if(checkFlag(m_flags, FLAGS::TRANSPARENT))
    {
        m_opacity = 0.1f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_transparent_color);
    }
    else if(checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
    {
        m_opacity = 0.8f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.outline_active_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_active_color);
    }
    else if(checkFlag(m_flags, FLAGS::HOVERED))
    {
        m_opacity = 0.9f;
        m_currentBgColorPtr = &m_style.fill_hover_color;
        m_currentOutlineColorPtr = &m_style.outline_hover_color;
        m_currentOutlineThicknessPtr = &m_style.outline_hover_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_hover_color);
    }
    else
    {
        m_opacity = 1.f;
        m_currentBgColorPtr = &m_style.fill_color;
        m_currentOutlineColorPtr = &m_style.outline_color;
        m_currentOutlineThicknessPtr = &m_style.outline_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_color);
    }
    this->setFillColor(*m_currentBgColorPtr);
    this->setOutlineColor(*m_currentOutlineColorPtr);
    this->setOutlineThickness(*m_currentOutlineThicknessPtr);
    updateClipShader();
    updateColorShader();
}

void Button::updateTheme()
{
    if(auto t = m_theme.lock())
    {
        const std::size_t synced_token = t->getToken<Button>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Button>());
    }
}

void Button::updateClipShader()
{
    if(!checkFlag(m_flags, FLAGS::CLIPSHADER)) return;

    // side image sprite
    m_clipShader.setUniform("texture",  sf::Shader::CurrentTexture);
    m_clipShader.setUniform("size",     sf::Glsl::Vec2(m_size));
    m_clipShader.setUniform("radius",   m_radius);
    m_clipShader.setUniform("opacity", m_opacity);
    m_clipShader.setUniform("side", m_imagePosition == Side::Right ? 1.f : 0.f);
}

void Button::updateColorShader()
{
    if(!checkFlag(m_flags, FLAGS::COLORSHADER)) return;
    bool render_texture = checkFlag(m_flags, FLAGS::BGIMAGE); 

    // for background image texture
    m_bgShaderPtr->setUniform("texture", sf::Shader::CurrentTexture);
    if(render_texture)
    {
        m_bgShaderPtr->setUniform("bg_color",
            sf::Glsl::Vec4(m_currentBgColorPtr->r / 255.f,
                           m_currentBgColorPtr->g / 255.f,
                           m_currentBgColorPtr->b / 255.f,
                           m_currentBgColorPtr->a / 255.f));
    }
    else
    {
        m_bgShaderPtr->setUniform("bg_color", sf::Glsl::Vec4(*m_currentBgColorPtr));
    }
    m_bgShaderPtr->setUniform("opacity", m_opacity);
    m_bgShaderPtr->setUniform("outline_color", sf::Glsl::Vec4(*m_currentOutlineColorPtr));
    m_bgShaderPtr->setUniform("has_texture", render_texture ? 1.f : 0.f);
}

void Button::updateTextPosition()
{
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    if(checkFlag(m_flags, FLAGS::IMAGE) && m_imagePosition == Side::Center)
    {
        if(m_textPosition == Side::Left) m_imagePosition = Side::Right;
        else m_imagePosition = Side::Left;
    }

    sf::FloatRect bounds = m_text.getLocalBounds();
    m_text.setOrigin(bounds.position + bounds.size / 2.f);

    sf::Vector2f local_offset;
    local_offset.y = m_size.y / 2.f;

    switch (m_textPosition)
    {
    case Side::Left:
        local_offset.x = m_size.x * 0.33f;
        break;
    
    case Side::Center:
        local_offset.x = m_size.x * 0.5f;
        break;

    case Side::Right:
        local_offset.x = m_size.x * 0.67f;
        break;
    }
    m_text.setPosition(this->getTransform().transformPoint(local_offset));
}

void Button::updateTextSize()
{
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    // update character size
    if(m_charSizeRatio != 0.f)
    {
        float final_height = m_size.y * m_charSizeRatio;
        float current_height = W_CharHeight::getHeight(m_text.getFont(), m_text.getCharacterSize());
        if(current_height > 0.f)
        {
            float ratio = final_height / current_height;
            m_charSize = std::floor<unsigned int>(m_text.getCharacterSize() * ratio);
        }
    }
    m_text.setCharacterSize(m_charSize);
}

void Button::updateImagePosition()
{
    if(!checkFlag(m_flags, FLAGS::IMAGE)) return;

    sf::FloatRect bounds = m_image.getLocalBounds();
    m_image.setOrigin(bounds.position + bounds.size / 2.f);

    sf::Vector2f local_offset;
    local_offset.y = m_size.y / 2.f;

    switch (m_imagePosition)
    {
    case Side::Left:
        local_offset.x = m_size.x * 0.165f;
        break;

    case Side::Center:
        local_offset.x = m_size.x * 0.5f;
        break;
    
    case Side::Right:
        local_offset.x = m_size.x * 0.835f;
        break;
    
    }
    m_image.setPosition(this->getTransform().transformPoint(local_offset));
}

bool Button::updateImageSize()
{
    if(!checkFlag(m_flags, FLAGS::IMAGE)) return false;
    sf::Vector2u new_image_size{static_cast<unsigned int>(m_size.x) / 3, static_cast<unsigned int>(m_size.y)}; //!< image takes a third of the button's width
    std::optional<sf::Image> resized_image;
    // Since IMAGE flag is up, it's safe to assume that image is already loaded in memory
    // unless if another widget double cleared the same path, then the following loads from disk
    // and if the image is not found on disk, then texture resizes with degraded quality
    std::optional<const sf::Image*> disk_image = ImageCache::getInstance().getImage(m_imagePath, false);
    
    if(disk_image) resized_image = resize_image(**disk_image, new_image_size);
    else resized_image = resize_image(m_texture.copyToImage(), new_image_size);

    if(resized_image)
    {
        sf::Texture temp;
        if(temp.loadFromImage(*resized_image))
        {
            m_texture = std::move(temp);
            m_texture.setSmooth(true);
            m_image.setTexture(m_texture, true);
            return true;
        }
    }
    return false;
}
}