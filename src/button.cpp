#include <cmath>
#include "button.hpp"

namespace sfml_widgets // Button
{
///////////////////////////////////////////////////////////////////
/// Constructors / Destructor / Operators
///////////////////////////////////////////////////////////////////

Button::Button(sf::RenderWindow& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text, const sf::Font& font) : Widget(root, position, size),
m_text(font, "", parameters::CHAR_SIZE)
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
    loadClipShader(BUTTON_SHADER_FRAG, W_LoadType::Memory);

}

Button::Button(Frame& root, sf::Vector2f position, sf::Vector2u size, const sf::String& text, const sf::Font& font) : Widget(root, position, size),
m_text(font, "", parameters::CHAR_SIZE)
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
    loadClipShader(BUTTON_SHADER_FRAG, W_LoadType::Memory);
}

Button::~Button()
{
    clearIcon();
}

void Button::setText(const sf::String& text)
{
    m_text.setString(text.substring(0, std::min(m_charLimit, static_cast<unsigned int>(text.getSize()))));

    if(text.isEmpty()) resetFlag(m_flags, FLAGS::TEXT);
    else setFlag(m_flags, FLAGS::TEXT);
    
    updateTextSize();
    setFlag(m_flags, FLAGS::UPDATE); //!< Fix image position in case TEXT flag state has changed
}

bool Button::setIcon(const sf::String& image_path, const std::string_view& shader, W_LoadType loading_type)
{
    // load from disk
    ImageCache& global_image_buffer = ImageCache::getInstance(); 
    std::optional<const sf::Image*> disk_image = global_image_buffer.getImage(image_path);
    if(!disk_image) return false;

    // attempt to make a resized copy
    std::optional<sf::Image> resized_image = resize_image(
        **disk_image,
        sf::Vector2u{static_cast<unsigned int>(m_size.x * m_imageSizeRatio), std::min<unsigned int>(m_size.x * m_imageSizeRatio, m_size.y)});

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

    // load clip shader
    if(!shader.empty() && !m_clipShaderPtr) loadClipShader(shader, loading_type);

    // texture update
    if(m_texturePtr)
    {
        global_image_buffer.decrementRefCounter(m_imagePath);
        *m_texturePtr = std::move(temp);
    }
    else
    {
        m_texturePtr = std::make_unique<sf::Texture>(std::move(temp));
    }

    m_imagePath = std::move(image_path);
    m_texturePtr->setSmooth(true);

    // construct sprite
    if(!m_imagePtr)
    {
        m_imagePtr = std::make_unique<sf::Sprite>(*m_texturePtr);
        m_imagePtr->setRotation(this->getRotation());
    }

    else m_imagePtr->setTexture(*m_texturePtr, true);

    setFlag(m_flags, FLAGS::UPDATE | FLAGS::IMAGE);
    return true;
}

bool Button::setTextLimit(unsigned int char_limit)
{
    if(char_limit > s_MAX_CHAR_COUNT) return false;
    m_charLimit = char_limit;
    return true;
}

void Button::setTextPosition(Button::SIDE text_side)
{
    if(m_textPosition == text_side) return;

    m_textPosition = text_side;
    setFlag(m_flags, FLAGS::UPDATE);
}

bool Button::setTextSize(unsigned int char_size)
{
    m_charSizeRatio = 0.f;
    m_charSize = char_size;
    updateTextSize();
    setFlag(m_flags, FLAGS::UPDATE);
    return true;
}

bool Button::setTextSize(float ratio)
{
    if(ratio < 0.f || ratio >= 1.f) return false;
    m_charSizeRatio = ratio;
    updateTextSize();
    setFlag(m_flags, FLAGS::UPDATE);
    return true;
}

void Button::setTextFont(const sf::Font& font)
{
    // outside reference of font so any internal reference gets cleared
    m_text.setFont(font);
    if(m_fontPtr) m_fontPtr.reset();

    updateTextSize(); //!< auto update size
    setFlag(m_flags, FLAGS::UPDATE);
}

void Button::setTextFont(const sf::Font&& font)
{
    m_fontPtr = std::make_unique<const sf::Font>(std::move(font));
    m_text.setFont(*m_fontPtr);

    updateTextSize(); //!< auto update size
    setFlag(m_flags, FLAGS::UPDATE);
}

void Button::setIconPosition(Button::SIDE image_side)
{
    if(m_imagePosition == image_side) return;

    m_imagePosition = image_side;
    setFlag(m_flags, FLAGS::UPDATE);
}

bool Button::setIconSizeRatio(float ratio)
{
    if(ratio < 0.1f || ratio > 1.f) return false;
    m_imageSizeRatio = ratio;
    updateImageSize();
    setFlag(m_flags, FLAGS::MODIFIED);
    return true;
}

void Button::clearIcon()
{
    if(!m_texturePtr) return;

    ImageCache::getInstance().decrementRefCounter(m_imagePath);
    m_texturePtr.reset();
    m_imagePtr.reset();
    m_imagePath.clear();
    resetFlag(m_flags, FLAGS::IMAGE);
}

void Button::setTheme(ButtonStyle s_button)
{
    m_style = s_button;
    setFlag(m_flags, FLAGS::MODIFIED);
}

void Button::fetchTheme()
{
    if(auto t = m_themePtr.lock())
    {
        setTheme(t->getStyle<Button>());
        setFlag(m_flags, FLAGS::MODIFIED);
    }
}

ButtonStyle& Button::style()
{
    return m_style;
}

const sf::String& Button::getText() const
{
    return m_text.getString();
}

std::optional<sf::Image> Button::getIcon()
{
    if(m_texturePtr) return m_texturePtr->copyToImage();
    return std::nullopt;
}

const sf::String& Button::getIconPath() const
{
    return m_imagePath;
}

unsigned int Button::getTextSize() const
{
    return m_charSize;
}

float Button::getIconSizeRatio() const
{
    return m_imageSizeRatio;
}

unsigned int Button::getTextLimit() const
{
    return m_charLimit;
}

Button::SIDE Button::getIconPosition() const
{
    return m_imagePosition;
}

Button::SIDE Button::getTextPosition() const
{
    return m_textPosition;
}

const sf::Font& Button::getTextFont() const
{
    return m_text.getFont();
}

///////////////////////////////////////////////////////////////////
/// Private
///////////////////////////////////////////////////////////////////

void Button::drawWidget()
{
    if(checkFlag(m_flags, FLAGS::UPDATE))
    {
        resetFlag(m_flags, FLAGS::UPDATE);
        updateTextPosition();
        updateImagePosition();
    }

    if(m_imagePtr)
    {
        if(m_clipShaderPtr) m_master.draw(*m_imagePtr, m_clipShaderPtr.get());
        else m_master.draw(*m_imagePtr);
    }
    if(checkFlag(m_flags, FLAGS::TEXT)) m_master.draw(m_text);
}

void Button::updatePosition()
{
    // updates text and image
    setFlag(m_flags, FLAGS::UPDATE);
}

void Button::updateSize()
{
    // all methods that scale with size
    updateTextSize();       //!< if text scales with ratio
    updateImageSize();
    updateClipShader();

    // keep text aligned
    // image position scales with size
    setFlag(m_flags, FLAGS::UPDATE);
}

void Button::updateRotation()
{
    m_text.setRotation(this->getRotation());
    if(m_imagePtr) m_imagePtr->setRotation(this->getRotation());

    // there is no render object that handles text & image transforms
    // they are handled in their update position methods
    setFlag(m_flags, FLAGS::UPDATE);
}

void Button::updateState()
{
    if(!checkFlag(m_flags, FLAGS::MODIFIED)) return;
    toggle(m_flags, FLAGS::MODIFIED);

    if(!m_stale && checkFlag(m_flags, FLAGS::BLOCKED))
    {
        m_opacity = 0.4f;
        m_currentBgColorPtr = &m_style.fill_blocked_color;
        m_currentOutlineColorPtr = &m_style.outline_blocked_color;
        m_currentOutlineThicknessPtr = &m_style.outline_blocked_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_blocked_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::ALPHAREDUCED))
    {
        m_opacity = 0.25f;
        m_currentBgColorPtr = &m_style.fill_transparent_color;
        m_currentOutlineColorPtr = &m_style.outline_transparent_color;
        m_currentOutlineThicknessPtr = &m_style.outline_transparent_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_transparent_color);
    }
    else if(!m_stale && checkFlag(m_flags, (FLAGS::RESTRICTED | FLAGS::ACTIVE)))
    {
        m_opacity = 0.8f;
        m_currentBgColorPtr = &m_style.fill_active_color;
        m_currentOutlineColorPtr = &m_style.outline_active_color;
        m_currentOutlineThicknessPtr = &m_style.outline_active_thickness;
        if(checkFlag(m_flags, FLAGS::TEXT)) m_text.setFillColor(m_style.text_active_color);
    }
    else if(!m_stale && checkFlag(m_flags, FLAGS::HOVERED))
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
    if(auto t = m_themePtr.lock())
    {
        const std::size_t synced_token = t->getToken<Button>();
        if(synced_token == m_generationToken) return;

        m_generationToken = synced_token;
        setTheme(t->getStyle<Button>());
    }
}

void Button::updateClipShader()
{
    if (!m_clipShaderPtr)
        return;

    sf::Vector2f icon_size;
    if(m_imagePtr) icon_size = m_imagePtr->getGlobalBounds().size;

    m_clipShaderPtr->setUniform("texture", sf::Shader::CurrentTexture);
    m_clipShaderPtr->setUniform("size", sf::Glsl::Vec2(m_size));
    m_clipShaderPtr->setUniform("radius", m_radius);
    m_clipShaderPtr->setUniform("opacity", m_opacity);
    m_clipShaderPtr->setUniform("icon_size", sf::Glsl::Vec2(icon_size));
    m_clipShaderPtr->setUniform("icon_local_center", sf::Glsl::Vec2(m_imageCenter));
}

void Button::updateTextPosition()
{
    if(!checkFlag(m_flags, FLAGS::TEXT)) return;

    sf::FloatRect bounds = m_text.getLocalBounds();
    m_text.setOrigin(bounds.position + bounds.size / 2.f);

    sf::Vector2f local_offset;
    local_offset.y = m_size.y / 2.f;

    switch (m_textPosition)
    {
    case SIDE::Left:
        local_offset.x = m_size.x * 0.33f;
        break;
    
    case SIDE::Center:
        local_offset.x = m_size.x * 0.5f;
        break;

    case SIDE::Right:
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
            m_charSize = std::floor<unsigned int>(m_charSize * ratio);
        }
    }
    m_text.setCharacterSize(m_charSize);
}

void Button::updateImagePosition()
{
    if(!m_imagePtr) return;

    sf::FloatRect bounds = m_imagePtr->getLocalBounds();
    m_imagePtr->setOrigin(bounds.position + bounds.size / 2.f);

    m_imageCenter.y = m_size.y * 0.5f;

    switch (m_imagePosition)
    {
    case SIDE::Left:
        m_imageCenter.x = m_size.x * 0.165f;
        break;

    case SIDE::Center:
        m_imageCenter.x = m_size.x * 0.5f;
        break;

    case SIDE::Right:
        m_imageCenter.x = m_size.x * 0.835f;
        break;
    }
    m_imagePtr->setPosition(this->getTransform().transformPoint(m_imageCenter));
    updateClipShader();
}

bool Button::updateImageSize()
{
    if(!m_texturePtr || !m_imagePtr) return false;
    sf::Vector2u new_image_size
    {
        static_cast<unsigned int>(m_size.x * m_imageSizeRatio),
        std::min<unsigned int>(m_size.x * m_imageSizeRatio, m_size.y)
    };

    std::optional<sf::Image> resized_image;

    // obtains original image from cache
    std::optional<const sf::Image*> disk_image = ImageCache::getInstance().getImage(m_imagePath, false);
    
    // copy and resize the image
    if(disk_image) resized_image = resize_image(**disk_image, new_image_size);
    // image somehow not found in cache nor disk, try to copy existing data instead
    else resized_image = resize_image(m_texturePtr->copyToImage(), new_image_size);

    if(resized_image)
    {
        sf::Texture temp;
        if(temp.loadFromImage(*resized_image))
        {
            // try to load this from memory to skip temp
            m_texturePtr = std::make_unique<sf::Texture>(std::move(temp));
            m_texturePtr->setSmooth(true);
            m_imagePtr->setTexture(*m_texturePtr, true);
            return true;
        }
    }
    return false;
}
}