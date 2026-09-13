#include <cmath>
#include "tooltip.hpp"

namespace sfml_widgets
{
NewToolTip::NewToolTip(sf::RenderWindow& window) : m_shape(window, {0.f, 0.f}, sf::Vector2u(s_MIN_SIZE))
{
    m_timer.reset();
}

NewToolTip* NewToolTip::init_tooltip(sf::RenderWindow& window)
{
    return new NewToolTip(std::ref(window));
}

std::size_t NewToolTip::generateID()
{
    return s_ID_SEED++;
}


void NewToolTip::setDelay(sf::Time delay_ms)
{
    m_hoverDuration = delay_ms;
}

void NewToolTip::setOffset(sf::Vector2f offset)
{
    m_offsetPosition = offset;
}

void NewToolTip::setTheme(ToolTipStyle style)
{
    m_style = style;
    m_updateTheme = true;
}

std::size_t NewToolTip::createContent(ContentType type, sf::Vector2f pos)
{
    std::size_t id{0};

    std::unique_ptr<Content_t> content = std::make_unique<Content_t>(
        type == ContentType::Text
        ? Content_t(std::in_place_type<ToolTipText>, &m_updateShape)
        : Content_t(std::in_place_type<ToolTipImage>, &m_updateShape)
    );

    std::visit([&](auto& obj)
    {
        obj.setPosition(pos);
        id = obj.getID();
    }, *content);

    m_content.emplace_back(std::move(content));
    return id;
}

bool NewToolTip::deleteContent(std::size_t id)
{
    auto it = std::find_if(m_content.begin(), m_content.end(),
        [id](std::unique_ptr<Content_t>& element)
        {
            return std::visit([id](auto& obj) -> bool
                    {
                        return obj.getID() == id;
                    }, *element);
        });

    if(it != m_content.end())
    {
        m_content.erase(it);
        return true;
    }
    return false;
}

NewToolTip::ToolTipText* NewToolTip::getText(std::size_t id) const
{
    for(const auto& element : m_content)
    {
        if(ToolTipText* text = std::get_if<ToolTipText>(element.get()))
        {
            if(text->getID() == id) return text;
        }
    }
    return nullptr;
}

NewToolTip::ToolTipImage* NewToolTip::getImage(std::size_t id) const
{
    for(const auto& element : m_content)
    {
        if(ToolTipImage* img = std::get_if<ToolTipImage>(element.get()))
        {
            if(img->getID() == id) return img;
        }
    }
    return nullptr;
}

void NewToolTip::setCornerRadius(float radius)
{
    m_shape.setRadius(radius);
}

void NewToolTip::draw(bool root_is_hovered)
{
    if(root_is_hovered)
    {
        if(!m_timer.isRunning()) m_timer.restart();
        if(m_timer.getElapsedTime() > m_hoverDuration)
        {
            stepTransparency();
            updateTheme();
            updateShapeBounds();
            if(auto* base = m_shape.beginDraw())
            {
                for(auto& element_ptr : m_content)
                {
                    std::visit([base](auto& element){element.draw(*base);}, *element_ptr);
                }
                m_shape.endDraw();
            }
        }
    }
    else
    {
        m_currentLevel = 1;
        m_timer.reset();
    }
}

void NewToolTip::stepTransparency()
{
    if(m_currentLevel > s_FRAMES_TO_DRAW) return;

    const float T_LEVEL = static_cast<float>(m_currentLevel) / static_cast<float>(s_FRAMES_TO_DRAW);

    auto scale_transparency = [&T_LEVEL](sf::Color color) -> sf::Color
    {
        color.a = std::round<std::uint8_t>(static_cast<float>(color.a) * T_LEVEL);
        return color;
    };

    m_shape.setFillColor(scale_transparency(m_style.fill_color));
    m_shape.setOutlineColor(scale_transparency(m_style.outline_color));
    ++m_currentLevel;
}

void NewToolTip::updateTheme()
{
    if(!m_updateTheme) return;
    m_updateTheme = false;
    m_shape.setFillColor(m_style.fill_color);
    m_shape.setOutlineColor(m_style.outline_color);
    m_shape.setOutlineThickness(m_style.outline_thickness);
}

void NewToolTip::updateShapeBounds()
{
    if(!m_updateShape) return;
    m_updateShape = false;

    sf::Vector2f max_xy = s_MIN_SIZE;
    for(auto& content_ptr : m_content)
    {
        std::visit([&max_xy](auto& element)
        {
            sf::FloatRect bounds = element.getBounds();
            max_xy.x = std::max(max_xy.x, bounds.position.x + bounds.size.x);
            max_xy.y = std::max(max_xy.y, bounds.position.y + bounds.size.y);
        }, *content_ptr);
    }

    m_shape.setBodySize(sf::Vector2u(max_xy));
}

void NewToolTip::updateRootPosition(sf::Vector2f new_root_pos)
{
    m_shape.setBodyPosition(new_root_pos + m_offsetPosition);
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

NewToolTip::_Body::_Body(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2u size) : m_window(window), m_size(size),
m_texture(size), m_sprite(m_texture.getTexture()), m_shader(SPRITE_SHADER_FRAG, sf::Shader::Type::Fragment)
{

}

void NewToolTip::_Body::setRadius(float rad)
{
    m_radius = std::max(0.f, std::min(rad, std::min(m_size.x, m_size.y) / 2.f));
    if(m_radius == 0.f)
    {
        m_radius = 0.f;
        m_cornerPoints = 1;
    }
    else
    {
        m_cornerPoints = s_CORNER_POINTS;
    }
    updateUniforms();
    update();
}

void NewToolTip::_Body::setBodyPosition(sf::Vector2f new_pos)
{
    this->setPosition(new_pos);
    m_sprite.setPosition(new_pos);
}

void NewToolTip::_Body::setBodySize(sf::Vector2u new_size)
{
    if(!m_texture.resize(new_size)) return;
    m_sprite.setTexture(m_texture.getTexture(), true);
    m_size = new_size;
    setRadius(m_radius);
}

sf::RenderTexture* NewToolTip::_Body::beginDraw()
{
    m_texture.clear(sf::Color::Transparent);
    m_window.draw(*this);
    return &m_texture;
}

void NewToolTip::_Body::endDraw()
{
    m_texture.display();
    m_window.draw(m_sprite, &m_shader);
}

void NewToolTip::_Body::updateUniforms()
{
    m_shader.setUniform("texture", sf::Shader::CurrentTexture);
    m_shader.setUniform("size", sf::Glsl::Vec2(m_size));
    m_shader.setUniform("radius", m_radius);
    m_shader.setUniform("opacity", 1.f);
}

std::size_t NewToolTip::_Body::getPointCount() const
{
    return m_cornerPoints * 4;
}

sf::Vector2f NewToolTip::_Body::getPoint(std::size_t index) const {
    std::size_t corner = index / m_cornerPoints;
    float angle = static_cast<float>(index) / m_cornerPoints * constants::DEGREE90;
    float rad = angle * constants::PI / constants::DEGREE180;

    // Center of each corner's arc circle
    sf::Vector2f centers[4] = {
        { m_radius, m_radius },
        { m_size.x - m_radius, m_radius },
        { m_size.x - m_radius, m_size.y - m_radius },
        { m_radius, m_size.y - m_radius }
    };

    float a = constants::START_ANGLES[corner] + static_cast<float>(index % m_cornerPoints) / m_cornerPoints * (constants::PI / 2.f);

    return {
        centers[corner].x + std::cos(a) * m_radius,
        centers[corner].y + std::sin(a) * m_radius
    };
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

NewToolTip::_ContentBase::_ContentBase(bool* flag) : id(NewToolTip::generateID()), signal_root(flag)
{

}

void NewToolTip::_ContentBase::setPosition(sf::Vector2f new_pos)
{
    updatePosition(new_pos);
    *signal_root = true;
}

std::size_t NewToolTip::_ContentBase::getID() const
{
    return id;
}

///////////////////////////////////////////////////////////////////

NewToolTip::ToolTipText::ToolTipText(bool* flag) : _ContentBase(flag), content(parameters::INTER_LIGHT_18)
{

}

void NewToolTip::ToolTipText::draw(sf::RenderTexture& base)
{
    base.draw(content);
}

void NewToolTip::ToolTipText::setCharacterSize(unsigned int char_size)
{
    content.setCharacterSize(char_size);
    *signal_root = true;
}

void NewToolTip::ToolTipText::setText(const sf::String& text)
{
    content.setString(text);
    *signal_root = true;
}

void NewToolTip::ToolTipText::setFont(const sf::Font& font)
{
    content.setFont(font);
    *signal_root = true;
}

void NewToolTip::ToolTipText::setStyle(const sf::Text::Style& style)
{
    content.setStyle(style);
    *signal_root = true;
}

void NewToolTip::ToolTipText::setLineSpacing(float factor)
{
    content.setLineSpacing(factor);
    *signal_root = true;
}

void NewToolTip::ToolTipText::setLetterSpacing(float factor)
{
    content.setLetterSpacing(factor);
    *signal_root = true;
}


void NewToolTip::ToolTipText::updatePosition(sf::Vector2f new_pos)
{
    content.setPosition(new_pos);
    *signal_root = true;
}

sf::FloatRect NewToolTip::ToolTipText::getBounds() const
{
    return content.getGlobalBounds();
}

///////////////////////////////////////////////////////////////////

NewToolTip::ToolTipImage::ToolTipImage(bool* flag) : _ContentBase(flag), texture({1, 1}), sprite(texture) 
{

}

void NewToolTip::ToolTipImage::draw(sf::RenderTexture& base)
{
    base.draw(sprite);
}

void NewToolTip::ToolTipImage::setSize(sf::Vector2u new_size)
{
    if(!texture.resize(new_size)) return;
    sprite.setTexture(texture, true);
    *signal_root = true;
}

bool NewToolTip::ToolTipImage::setImage(const sf::String& path, sf::Vector2u size)
{
    sf::Texture tmp;
    if(!tmp.loadFromFile(path.toAnsiString())) return false;

    if(size != texture.getSize())
    {
        std::optional<sf::Image> resized_image = resize_image(tmp.copyToImage(), size);
        if(!resized_image) return false;
        if(!texture.loadFromImage(*resized_image)) return false;
    }
    else
    {
        texture = std::move(tmp);
    }

    sprite.setTexture(texture, true);
    *signal_root = true;
    return true;
}

bool NewToolTip::ToolTipImage::setImage(const sf::Image& img, sf::Vector2u size)
{
    if(size != img.getSize())
    {
        std::optional<sf::Image> resized_image = resize_image(img, size);
        if(!resized_image) return false;
        if(!texture.loadFromImage(*resized_image)) return false;
    }
    else
    {
        if(!texture.loadFromImage(img)) return false;
    }

    sprite.setTexture(texture, true);
    *signal_root = true;
    return true;
}

bool NewToolTip::ToolTipImage::setImage(const std::shared_ptr<sf::Texture>& texture_ref)
{
    return false;
}

void NewToolTip::ToolTipImage::clearImage()
{
    texture = sf::Texture();
}

void NewToolTip::ToolTipImage::updatePosition(sf::Vector2f new_pos)
{
    sprite.setPosition(new_pos);
    *signal_root = true;
}

sf::FloatRect NewToolTip::ToolTipImage::getBounds() const
{
    return sprite.getGlobalBounds();
}





// ToolTip::ToolTip(sf::RenderWindow& window, sf::Vector2f position, sf::Time ms_delay, ToolTipStyle style) : m_window(window),
// m_text(parameters::INTER_REGULAR_18, "", parameters::CHAR_SIZE), m_hoverDuration(ms_delay)
// {
//     setBoxPosition(position);
//     setTheme(style);
//     m_timer.reset();
// }
// void ToolTip::setDelay(sf::Time delay_ms)
// {
//     m_hoverDuration = delay_ms;
// }

// void ToolTip::setTheme(ToolTipStyle style)
// {
//     m_style = style;
//     this->setFillColor(m_style.fill_color);
//     this->setOutlineColor(m_style.outline_color);
//     this->setOutlineThickness(m_style.outline_thickness);
//     m_text.setFillColor(m_style.text_color);
//     m_text.setOutlineColor(m_style.text_outline_color);
//     m_text.setOutlineThickness(m_style.text_outline_thickness);
// }

// void ToolTip::setText(sf::String text)
// {
//     m_text.setString(text);
//     m_needUpdate = true;
// }

// const sf::String& ToolTip::getText() const
// {
//     return m_text.getString();
// }

// void ToolTip::clearText()
// {
//     m_text.setString("");
//     m_needUpdate = true;
// }

// void ToolTip::setCornerRadius(float radius)
// {
//     m_radius = radius;
//     m_needUpdate = true;
// }

// void ToolTip::setTextFont(const sf::Font& font)
// {
//     m_text.setFont(font);
//     m_needUpdate = true;
// }

// void ToolTip::setTextStyle(sf::Text::Style style)
// {
//     m_text.setStyle(style);
//     m_needUpdate = true;
// }

// void ToolTip::setCharacterSize(unsigned int char_size)
// {
//     m_text.setCharacterSize(char_size);
//     m_needUpdate = true;
// }

// void ToolTip::setLineSpacing(float factor)
// {
//     m_text.setLineSpacing(factor);
//     m_needUpdate = true;
// }

// void ToolTip::setLetterSpacing(float factor)
// {
//     m_text.setLetterSpacing(factor);
//     m_needUpdate = true;
// }

// void ToolTip::setBoxPosition(sf::Vector2f position)
// {
//     this->setPosition(position);
//     m_text.setPosition(position);
// }

// void ToolTip::draw(bool is_hovered)
// {
//     if(is_hovered)
//     {
//         if(!m_timer.isRunning()) m_timer.restart();
//         if(m_timer.getElapsedTime() >= m_hoverDuration)
//         {
//             if(m_needUpdate) updateShapeBounds();
//             stepTransparency();
//             m_window.draw(*this);
//             m_window.draw(m_text);
//         }
//     }
//     else
//     {
//         m_currentLevel = 1;
//         m_timer.reset();
//     }
// }

// void ToolTip::stepTransparency()
// {
//     if(m_currentLevel > s_FRAMES_TO_DRAW) return;

//     const float T_LEVEL = static_cast<float>(m_currentLevel) / static_cast<float>(s_FRAMES_TO_DRAW);

//     std::function<sf::Color(sf::Color)> scaled = [T_LEVEL](sf::Color c)
//     {
//         c.a = std::round<uint8_t>(static_cast<float>(c.a) * T_LEVEL);
//         return c;
//     };
//     this->setFillColor(scaled(m_style.fill_color));
//     this->setOutlineColor(scaled(m_style.outline_color));
//     m_text.setFillColor(scaled(m_style.text_color));
//     m_text.setOutlineColor(scaled(m_style.text_outline_color));
//     ++m_currentLevel;
// }

// void ToolTip::updateShapeBounds()
// {
//     m_needUpdate = false;

//     const sf::FloatRect bounds = m_text.getLocalBounds();
//     const sf::String& text = m_text.getString();
//     const float line_height = m_text.getFont().getLineSpacing(m_text.getCharacterSize());

//     const int n_lines = 1 + static_cast<int>
//     (
//         std::count(text.begin(), text.end(), static_cast<uint32_t>('\n'))
//     );

//     const float width = bounds.position.x + bounds.size.x;

//     const float height = n_lines * line_height;

//     m_size.x = std::max(s_MIN_SIZE.x, width);
//     m_size.y = std::max(s_MIN_SIZE.y, height);
//     m_radius = std::max(0.f, std::min(m_radius, std::min<float>(m_size.x, m_size.y) / 2.f));
//     if(m_radius == 0.f)
//     {
//         m_radius = 0.f;
//         m_cornerPoints = 1;
//     }
//     else
//     {
//         m_cornerPoints = s_CORNER_POINTS;
//     }
//     update();
// }

// std::size_t ToolTip::getPointCount() const
// {
//     return m_cornerPoints * 4;
// }

// sf::Vector2f ToolTip::getPoint(std::size_t index) const {
//     std::size_t corner = index / m_cornerPoints;
//     float angle = static_cast<float>(index) / m_cornerPoints * constants::DEGREE90;
//     float rad = angle * constants::PI / constants::DEGREE180;

//     // Center of each corner's arc circle
//     sf::Vector2f centers[4] = {
//         { m_radius, m_radius },
//         { m_size.x - m_radius, m_radius },
//         { m_size.x - m_radius, m_size.y - m_radius },
//         { m_radius, m_size.y - m_radius }
//     };

//     float a = constants::START_ANGLES[corner] + static_cast<float>(index % m_cornerPoints) / m_cornerPoints * (constants::PI / 2.f);

//     return {
//         centers[corner].x + std::cos(a) * m_radius,
//         centers[corner].y + std::sin(a) * m_radius
//     };
// }
}