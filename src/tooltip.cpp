#include <cmath>
#include "tooltip.hpp"

namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// ToolTip
///////////////////////////////////////////////////////////////////

ToolTip::ToolTip(sf::RenderWindow& window, sf::Vector2f root_pos) : m_shapePtr(std::make_unique<DrawBoard>(std::ref(window), sf::Vector2f{0.f, 0.f}, sf::Vector2u(s_MIN_SIZE)))
{
    updateRootPosition(root_pos);
    m_timer.reset();
}

std::size_t ToolTip::createContent(TT_RenderedType type, sf::Vector2f pos)
{
    std::size_t id{0};

    std::unique_ptr<Rendered_t> content = std::make_unique<Rendered_t>(
        type == TT_RenderedType::Text
        ? Rendered_t(std::in_place_type<RenderedText>, &m_updateShape)
        : Rendered_t(std::in_place_type<RenderedImage>, &m_updateShape)
    );

    std::visit([&](auto& obj)
    {
        obj.setPosition(pos);
        id = obj.getID();
    }, *content);

    m_content.emplace_back(std::move(content));
    return id;
}

bool ToolTip::deleteContent(std::size_t id)
{
    auto it = std::find_if(m_content.begin(), m_content.end(),
        [id](std::unique_ptr<Rendered_t>& element)
        {
            return std::visit([id](auto& obj) -> bool
                    {
                        return obj.getID() == id;
                    }, *element);
        });

    if(it != m_content.end())
    {
        m_content.erase(it);
        return m_updateShape = true;
    }
    return false;
}

void ToolTip::setTheme(ToolTipStyle style)
{
    m_style = style;
    m_updateTheme = true;
}

void ToolTip::setDelay(std::int32_t delay_ms)
{
    m_hoverDuration = sf::milliseconds(delay_ms);
}

void ToolTip::setPhaseDuration(float delay_ms)
{
    // 0 disables
    if(delay_ms >= 0.f) m_phaseDuration = delay_ms;
}

void ToolTip::setOffset(sf::Vector2f offset)
{
    m_shapePtr->setBoardPosition((m_shapePtr->getPosition() - m_offsetPosition) + offset);
    m_offsetPosition = offset;
}

void ToolTip::setLocalOffset(sf::Vector2f offset)
{
    if(offset.x < 0 || offset.y < 0) return;
    m_localOffset = offset;
    m_updateShape = true;
}

void ToolTip::setCornerRadius(float radius)
{
    m_shapePtr->setRadius(radius);
}

ToolTip::RenderedText* ToolTip::getText(std::size_t id) const
{
    for(const auto& element : m_content)
    {
        if(RenderedText* text = std::get_if<RenderedText>(element.get()))
        {
            if(text->getID() == id) return text;
        }
    }
    return nullptr;
}

ToolTip::RenderedImage* ToolTip::getImage(std::size_t id) const
{
    for(const auto& element : m_content)
    {
        if(RenderedImage* img = std::get_if<RenderedImage>(element.get()))
        {
            if(img->getID() == id) return img;
        }
    }
    return nullptr;
}

sf::Vector2f ToolTip::getPosition() const
{
    return m_shapePtr->getPosition();
}

sf::Vector2u ToolTip::getSize() const
{
    return m_shapePtr->getSize();
}

float ToolTip::getCornerRadius() const
{
    return m_shapePtr->getRadius();
}

std::int32_t ToolTip::getDelay() const
{
    return m_hoverDuration.asMilliseconds();
}

float ToolTip::getPhaseDuration() const
{
    return m_phaseDuration;
}

sf::Vector2f ToolTip::getOffset() const
{
    return m_offsetPosition;
}

sf::Vector2f ToolTip::getLocalOffset() const
{
    return m_localOffset;
}

void ToolTip::draw(bool root_is_hovered)
{
    if(root_is_hovered)
    {
        if(!m_timer.isRunning()) m_timer.restart();
        if(!m_isDrawing && m_timer.getElapsedTime() > m_hoverDuration)
        {
            m_timer.restart();
            m_isDrawing = true;
        }

        if(m_isDrawing)
        {
            stepTransparency();
            updateTheme();
            updateShapeBounds();
            if(auto* base = m_shapePtr->beginDraw())
            {
                for(auto& element_ptr : m_content)
                {
                    std::visit([base](auto& element){element.draw(*base);}, *element_ptr);
                }
                m_shapePtr->endDraw();
            }
        }
    }
    else
    {
        m_isDrawing = false;
        m_timer.reset();
    }
}

void ToolTip::stepTransparency()
{
    if(m_phaseDuration == 0.f) return;

    const float T_LEVEL = static_cast<float>(m_timer.getElapsedTime().asMilliseconds()) / m_phaseDuration;
    if(T_LEVEL > 1.f) return;   //!< fix: rounding error on small durations

    auto scale_transparency = [&T_LEVEL](sf::Color color) -> sf::Color
    {
        color.a = std::round<std::uint8_t>(static_cast<float>(color.a) * T_LEVEL);
        return color;
    };

    m_shapePtr->setFillColor(scale_transparency(m_style.fill_color));
    m_shapePtr->setOutlineColor(scale_transparency(m_style.outline_color));
    m_shapePtr->updateUniforms(T_LEVEL);    //!< thank god
}

void ToolTip::updateTheme()
{
    if(!m_updateTheme) return;
    m_updateTheme = false;
    m_shapePtr->setFillColor(m_style.fill_color);
    m_shapePtr->setOutlineColor(m_style.outline_color);
    m_shapePtr->setOutlineThickness(m_style.outline_thickness);
}

void ToolTip::updateShapeBounds()
{
    if(!m_updateShape) return;
    m_updateShape = false;

    sf::Vector2f max_xy = s_MIN_SIZE;
    for(auto& content_ptr : m_content)
    {
        std::visit([&max_xy, this](auto& element)
        {
            sf::FloatRect bounds = element.getBounds();
            max_xy.x = std::max(max_xy.x, bounds.position.x + bounds.size.x + m_localOffset.x);
            max_xy.y = std::max(max_xy.y, bounds.position.y + bounds.size.y + m_localOffset.y);
        }, *content_ptr);
    }

    m_shapePtr->setBoardSize(sf::Vector2u(max_xy));
}

void ToolTip::updateRootPosition(sf::Vector2f new_root_pos)
{
    m_shapePtr->setBoardPosition(new_root_pos + m_offsetPosition);
}

ToolTip* ToolTip::make_tooltip(sf::RenderWindow& window, sf::Vector2f root_pos)
{
    return new ToolTip(window, root_pos);
}

std::size_t ToolTip::generateID()
{
    return s_seed++;
}

///////////////////////////////////////////////////////////////////
/// DrawBoard
///////////////////////////////////////////////////////////////////

ToolTip::DrawBoard::DrawBoard(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2u size) : m_window(window), m_size(size),
m_texture(size), m_sprite(m_texture.getTexture()), m_shader(SPRITE_SHADER_FRAG, sf::Shader::Type::Fragment)
{

}

void ToolTip::DrawBoard::setBoardPosition(sf::Vector2f new_pos)
{
    this->setPosition(new_pos);
    m_sprite.setPosition(new_pos);
}

void ToolTip::DrawBoard::setBoardSize(sf::Vector2u new_size)
{
    if(!m_texture.resize(new_size)) return;
    m_sprite.setTexture(m_texture.getTexture(), true);
    m_size = new_size;
    setRadius(m_radius);
}

void ToolTip::DrawBoard::setRadius(float rad)
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

sf::RenderTexture* ToolTip::DrawBoard::beginDraw()
{
    if(!m_isDrawing)
    {
        m_isDrawing = true;
        m_texture.clear(sf::Color::Transparent);
        m_window.draw(*this);
    }
    return &m_texture;
}

void ToolTip::DrawBoard::endDraw()
{
    if(m_isDrawing)
    {
        m_isDrawing = false;
        m_texture.display();
        m_window.draw(m_sprite, &m_shader);
    }
}

void ToolTip::DrawBoard::updateUniforms(float t)
{
    m_shader.setUniform("texture", sf::Shader::CurrentTexture);
    m_shader.setUniform("size", sf::Glsl::Vec2(m_size));
    m_shader.setUniform("radius", m_radius);
    m_shader.setUniform("opacity", t);
}

sf::Vector2u ToolTip::DrawBoard::getSize() const
{
    return m_size;
}

float ToolTip::DrawBoard::getRadius() const
{
    return m_radius;
}

std::size_t ToolTip::DrawBoard::getPointCount() const
{
    return m_cornerPoints * 4;
}

sf::Vector2f ToolTip::DrawBoard::getPoint(std::size_t index) const {
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
/// RenderedObjectBase
///////////////////////////////////////////////////////////////////

ToolTip::RenderedObjectBase::RenderedObjectBase(bool* flag) : id(ToolTip::generateID()), m_pushShapeUpdatePtr(flag)
{

}

void ToolTip::RenderedObjectBase::setPosition(sf::Vector2f new_pos)
{
    updatePosition(new_pos);
    *m_pushShapeUpdatePtr = true;
}

std::size_t ToolTip::RenderedObjectBase::getID() const
{
    return id;
}

///////////////////////////////////////////////////////////////////
/// RenderedText
///////////////////////////////////////////////////////////////////

ToolTip::RenderedText::RenderedText(bool* flag) : RenderedObjectBase(flag), m_text(parameters::INTER_SEMIBOLD_18)
{

}

void ToolTip::RenderedText::setText(const sf::String& text)
{
    m_text.setString(text);
    *m_pushShapeUpdatePtr = true;
}

void ToolTip::RenderedText::setFont(const sf::Font& font)
{
    m_text.setFont(font);
    *m_pushShapeUpdatePtr = true;
}

void ToolTip::RenderedText::setCharacterSize(unsigned int char_size)
{
    m_text.setCharacterSize(char_size);
    *m_pushShapeUpdatePtr = true;
}

void ToolTip::RenderedText::setFillColor(sf::Color fill_color)
{
    m_text.setFillColor(fill_color);
}

void ToolTip::RenderedText::setOutlineColor(sf::Color fill_color)
{
    m_text.setOutlineColor(fill_color);
}

void ToolTip::RenderedText::setOutlineThickness(unsigned int outline_thickness)
{
    m_text.setOutlineThickness(outline_thickness);
}

void ToolTip::RenderedText::setStyle(const sf::Text::Style& style)
{
    m_text.setStyle(style);
    *m_pushShapeUpdatePtr = true;
}

void ToolTip::RenderedText::setLineSpacing(float factor)
{
    m_text.setLineSpacing(factor);
    *m_pushShapeUpdatePtr = true;
}

void ToolTip::RenderedText::setLetterSpacing(float factor)
{
    m_text.setLetterSpacing(factor);
    *m_pushShapeUpdatePtr = true;
}

sf::FloatRect ToolTip::RenderedText::getBounds() const
{
    return m_text.getGlobalBounds();
}

sf::Vector2f ToolTip::RenderedText::getPosition() const
{
    return m_text.getPosition();
}

const sf::String& ToolTip::RenderedText::getText() const
{
    return m_text.getString();
}

unsigned int ToolTip::RenderedText::getCharacterSize() const
{
    return m_text.getCharacterSize();
}

sf::Color ToolTip::RenderedText::getFillColor() const
{
    return m_text.getFillColor();
}

sf::Color ToolTip::RenderedText::getOutlineColor() const
{
    return m_text.getOutlineColor();
}

unsigned int ToolTip::RenderedText::getOutlineThickness() const
{
    return m_text.getOutlineThickness();
}

sf::Text::Style ToolTip::RenderedText::getStyle() const
{
    return static_cast<sf::Text::Style>(m_text.getStyle());
}

float ToolTip::RenderedText::getLineSpacing() const
{
    return m_text.getLineSpacing();
}

float ToolTip::RenderedText::getLetterSpacing() const
{
    return m_text.getLetterSpacing();
}

void ToolTip::RenderedText::draw(sf::RenderTexture& base)
{
    base.draw(m_text);
}

void ToolTip::RenderedText::updatePosition(sf::Vector2f new_pos)
{
    m_text.setPosition(new_pos);
}

///////////////////////////////////////////////////////////////////
/// RenderedImage
///////////////////////////////////////////////////////////////////

ToolTip::RenderedImage::RenderedImage(bool* flag) : RenderedObjectBase(flag), m_texture(sf::Vector2u{1, 1}), m_sprite(m_texture)
{

}

void ToolTip::RenderedImage::setSize(sf::Vector2u new_size)
{
    if(!m_externalTexturePtr.expired()) return;

    if(!m_texture.resize(new_size)) return;
    m_sprite.setTexture(m_texture, true);
    *m_pushShapeUpdatePtr = true;
}

bool ToolTip::RenderedImage::setImage(const sf::String& path, sf::Vector2u size)
{
    sf::Texture tmp;
    if(!tmp.loadFromFile(path.toAnsiString())) return false;

    if(size.x * size.y != 0)
    {
        std::optional<sf::Image> resized_image = resize_image(tmp.copyToImage(), size);
        if(!resized_image) return false;
        if(!m_texture.loadFromImage(*resized_image)) return false;
    }
    else
    {
        m_texture = std::move(tmp);
    }
    if(!m_externalTexturePtr.expired()) m_externalTexturePtr.reset();
    m_sprite.setTexture(m_texture, true);
    *m_pushShapeUpdatePtr = true;
    m_ownsData = true;
    return true;
}

bool ToolTip::RenderedImage::setImage(const sf::Image& img, sf::Vector2u size)
{
    if(size.x * size.y != 0 && size != img.getSize())
    {
        std::optional<sf::Image> resized_image = resize_image(img, size);
        if(!resized_image) return false;
        if(!m_texture.loadFromImage(*resized_image)) return false;
    }
    else
    {
        if(!m_texture.loadFromImage(img)) return false;
    }
    
    if(!m_externalTexturePtr.expired()) m_externalTexturePtr.reset();
    m_sprite.setTexture(m_texture, true);
    *m_pushShapeUpdatePtr = true;
    m_ownsData = true;
    return true;
}

bool ToolTip::RenderedImage::setImage(const std::shared_ptr<sf::Texture>& texture_ref)
{
    if(!texture_ref) return false;

    m_externalTexturePtr = texture_ref;

    if(m_ownsData) clearImage();

    m_sprite.setTexture(*m_externalTexturePtr.lock(), true);
    return false;
}

void ToolTip::RenderedImage::clearImage()
{
    m_texture = sf::Texture();
    m_ownsData = false;
    setSize({1, 1});
}

sf::FloatRect ToolTip::RenderedImage::getBounds() const
{
    return m_sprite.getGlobalBounds();
}

sf::Vector2f ToolTip::RenderedImage::getPosition() const
{
    return m_sprite.getPosition();
}

sf::Vector2u ToolTip::RenderedImage::getSize() const
{
    return m_externalTexturePtr.expired() ? m_texture.getSize() : m_externalTexturePtr.lock()->getSize();
}

void ToolTip::RenderedImage::draw(sf::RenderTexture& base)
{
    base.draw(m_sprite);
}

void ToolTip::RenderedImage::updatePosition(sf::Vector2f new_pos)
{
    m_sprite.setPosition(new_pos);
}
}