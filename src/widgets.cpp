#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../external-include/stb_image_resize2.h"
#include "../include/frame.hpp"

namespace sfml_widgets // FLAGS
{
FLAGS operator&(FLAGS first, FLAGS second)
{
    return static_cast<FLAGS>(static_cast<std::int32_t>(first) & static_cast<std::int32_t>(second));
}
FLAGS& operator&=(FLAGS& self, FLAGS other)
{
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) & static_cast<std::int32_t>(other)) | FLAGS::MODIFIED;
    return self;
}
FLAGS operator|(FLAGS first, FLAGS second)
{
    return static_cast<FLAGS>(static_cast<std::int32_t>(first) | static_cast<std::int32_t>(second));
}
FLAGS& operator|=(FLAGS& self, FLAGS other)
{
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) | static_cast<std::int32_t>(other)) | FLAGS::MODIFIED;
    return self;
}
FLAGS operator^(FLAGS first, FLAGS second)
{
    return static_cast<FLAGS>(static_cast<std::int32_t>(first) ^ static_cast<std::int32_t>(second));
}
FLAGS& operator^=(FLAGS& self, FLAGS other)
{
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) ^ static_cast<std::int32_t>(other)) | FLAGS::MODIFIED;
    return self;
}
FLAGS& setFlag(FLAGS& self, FLAGS bits)
{
    self |= bits;    //!< sets MODIFIED
    return self;
};

FLAGS& resetFlag(FLAGS& self, FLAGS bits)
{
    std::int32_t mask = ~static_cast<std::int32_t>(bits);
    self = static_cast<FLAGS>(static_cast<std::int32_t>(self) & mask);
    if(!checkFlag(bits, FLAGS::MODIFIED)) self |= FLAGS::MODIFIED;
    return self;
};

FLAGS& toggle(FLAGS& self, FLAGS bits)
{
    self = self ^ bits;
    return self;
}

bool checkFlag(const FLAGS& self, FLAGS bits)
{
    return static_cast<bool>(self & bits);
}
}

namespace sfml_widgets // resize_image
{
    std::optional<sf::Image> resize_image(const sf::Image source, sf::Vector2u targetSize)
    {
        const sf::Vector2u current_size = source.getSize();
        const std::uint8_t* sourcePtr = source.getPixelsPtr();
        std::uint8_t result_data[targetSize.x * targetSize.y * 4];

        stbir_resize_uint8_linear
        (
            sourcePtr,
            static_cast<int>(current_size.x),
            static_cast<int>(current_size.y),
            0,                       // input stride in bytes (0 = tightly packed)
            result_data,
            static_cast<int>(targetSize.x),
            static_cast<int>(targetSize.y),
            0,                       // output stride in bytes (0 = tightly packed)
            STBIR_RGBA
        );

        sf::Image result(targetSize, result_data);
        return result;
    }
}

namespace sfml_widgets // get_prime_combination
{
sf::Vector2u get_prime_combination(unsigned int n)
{
    if (n == 0) return {0, 0};

    unsigned int first_div = n, second_div = 1;
    unsigned int root = std::sqrt<unsigned int>(n);

    for (unsigned int i = root; i > 2; --i)
    {
        if (n % i == 0)
        {
            first_div = n / i;
            second_div = i;
            break;
        }
    }

    return {first_div, second_div};
}
}

namespace sfml_widgets // ImageCache
{
ImageCache& ImageCache::getInstance()
{
    static ImageCache image_buffer_instance;
    return image_buffer_instance;
}

std::optional<const sf::Image*> ImageCache::getImage(const std::string& image_path, bool increment_ref)
{
    std::unordered_map<std::string, ImageReferenceCounter>::iterator it = m_cachedImages.find(image_path);
    if(it != m_cachedImages.end())
    {
        if(increment_ref) ++(it->second.ref_count);
        return &it->second.image;
    }

    // load image from disk
    sf::Image img;
    if(img.loadFromFile(image_path))
    {
        return &m_cachedImages.emplace(image_path, ImageReferenceCounter{std::move(img), 1}).first->second.image;
    };
    return std::nullopt;
}

void ImageCache::decrementRefCounter(const std::string& image_path)
{
    std::unordered_map<std::string, ImageReferenceCounter>::iterator it = m_cachedImages.find(image_path);
    if(it != m_cachedImages.end())
    {
        --it->second.ref_count;
        if(it->second.ref_count < 1) m_cachedImages.erase(it);
    }
}
}

namespace sfml_widgets // Key
{
    Key::Key(sf::Keyboard::Key key) : code(key) , scancode(sf::Keyboard::delocalize(key)) {};
    Key::Key(sf::Keyboard::Key key, bool control) : code(key), scancode(sf::Keyboard::delocalize(key)), control(control) {};
    Key::Key(sf::Keyboard::Key key, bool control, bool alt, bool shift, bool system) : code(key), scancode(sf::Keyboard::delocalize(key)), control(control), alt(alt), shift(shift), system(system) {};
    Key::Key(sf::Keyboard::Scancode scan_key) : code(sf::Keyboard::localize(scan_key)), scancode(scan_key) {};
    Key::Key(sf::Keyboard::Scancode scan_key, bool control) : code(sf::Keyboard::localize(scan_key)), scancode(scan_key), control(control) {};
    Key::Key(sf::Keyboard::Scancode scan_key, bool control, bool alt, bool shift, bool system) : code(sf::Keyboard::localize(scan_key)), scancode(scan_key), control(control), alt(alt), shift(shift), system(system) {};
    Key::Key(sf::Keyboard::Key key, sf::Keyboard::Scancode scan_key) : code(key), scancode(scan_key) {};
    Key::Key(sf::Keyboard::Key key, sf::Keyboard::Scancode scan_key, bool control) : code(key), scancode(scan_key), control(control) {};
    Key::Key(sf::Keyboard::Key key, sf::Keyboard::Scancode scan_key, bool control, bool alt, bool shift, bool system) : code(key), scancode(scan_key), control(control), alt(alt), shift(shift), system(system) {};
    Key::Key(const sf::Event::KeyPressed& event) : code(event.code), scancode(event.scancode), control(event.control), alt(event.alt), shift(event.shift), system(event.system) {};
    Key::Key(Key&& other) noexcept : code(other.code), scancode(other.scancode), control(other.control), alt(other.alt), shift(other.shift), system(other.system) {};
    Key::Key(const Key& other) : code(other.code), scancode(other.scancode), control(other.control), alt(other.alt), shift(other.shift), system(other.system) {};
    Key& Key::operator=(Key&& other) noexcept
    {
        if(this != &other)
        {
            code = other.code;
            scancode = other.scancode;
            control = other.control;
            alt = other.alt;
            shift = other.shift;
            system = other.system;
        }
        return *this;
    };

    Key& Key::operator=(const Key& other)
    {
        if(this != &other)
        {
            code = other.code;
            scancode = other.scancode;
            control = other.control;
            alt = other.alt;
            shift = other.shift;
            system = other.system;
        }
        return *this;
    };

    bool Key::operator==(const Key& other)
    {
        bool checked = false;
        // only compare code if both sides specified it
        if(code != sf::Keyboard::Key::Unknown && other.code != sf::Keyboard::Key::Unknown)
        {
            if(code != other.code) return false;
            checked = true;
        }

        // only compare scancode if both sides specified it
        if(scancode != sf::Keyboard::Scancode::Unknown && other.scancode != sf::Keyboard::Scancode::Unknown)
        {
            if(scancode != other.scancode) return false;
            checked = true;
        }

        return checked && //!< true = they are equal so far
               control == other.control &&
               shift == other.shift &&
               alt == other.alt &&
               system == other.system;
    };

    bool Key::operator!=(const Key& other)
    {
        bool checked = false;
        // only compare code if both sides specified it
        if(code != sf::Keyboard::Key::Unknown && other.code != sf::Keyboard::Key::Unknown)
        {
            if(code != other.code) return true;
            checked = true;
        }

        // only compare scancode if both sides specified it
        if(scancode != sf::Keyboard::Scancode::Unknown && other.scancode != sf::Keyboard::Scancode::Unknown)
        {
            if(scancode != other.scancode) return true;
            checked = true;
        }
                
        return !checked || //!< false = they are equal so far
               control != other.control ||
               shift != other.shift ||
               alt != other.alt ||
               system != other.system;
    };
}
namespace sfml_widgets // CaptureSequence
{
void CaptureSequence::addSequence(const std::string& name, Sequence sequence, SequenceFunction callback)
{
    std::size_t sequence_length = sequence.size();
    SequenceEntry temp{std::move(name), std::move(sequence), std::move(callback)};
    
    // look for the optimal index for the new sequence based on sequence length
    std::vector<SequenceEntry>::iterator it = std::lower_bound(m_sequences.begin(), m_sequences.end(), sequence_length,
    [](const SequenceEntry& existing_sequence, std::size_t size)
    {
        return existing_sequence.sequence.size() > size; // longer sequence = more priority
    });

    m_sequences.insert(it, std::move(temp));
    m_max_sequence = std::max(m_max_sequence, sequence_length);
}

void CaptureSequence::deleteSequence(const std::string& name)
{
    std::vector<SequenceEntry>::iterator it = std::find(m_sequences.begin(), m_sequences.end(), name);
    
    if(it != m_sequences.end())
    {
        size_t temp = it->sequence.size();
        m_sequences.erase(it);
        if(m_max_sequence == temp && m_sequences.size() > 0) m_max_sequence = m_sequences.at(0).sequence.size();
    }
}

bool CaptureSequence::checkEvents(const sf::Event& _event)
{
    const sf::Event::KeyPressed* key = _event.getIf<sf::Event::KeyPressed>();
    if(!key) return 0;
    m_current_sequence.push_back(*key);
    return compareSequence();
}

void CaptureSequence::reset()
{
    m_current_sequence.clear();
}

const Sequence& CaptureSequence::getCurrentSequence() const
{
    return m_current_sequence;
}

inline bool CaptureSequence::compareSequence()
{
    for(const SequenceEntry& items : m_sequences)
    {
        const Sequence& saved_sequence = items.sequence;
        if(m_current_sequence.size() < saved_sequence.size()) continue; // not enough keys pressed

        Sequence::iterator current_sequence_start_it = m_current_sequence.end() - static_cast<std::ptrdiff_t>(saved_sequence.size()); // ignore extra keys at the front of the sequence
        if(std::equal(current_sequence_start_it, m_current_sequence.end(), saved_sequence.begin(), saved_sequence.end()))
        {
            items.callback();
            reset();
            return true;
        }

    }
    // remove keys at first side that exceed max limit
    if(m_current_sequence.size() > m_max_sequence)
    {
        m_current_sequence.erase(m_current_sequence.begin(), 
        m_current_sequence.begin() + static_cast<std::ptrdiff_t>(m_current_sequence.size() - m_max_sequence));
    }
    return false;
}
}

namespace sfml_widgets // SequenceEntry
{
bool CaptureSequence::SequenceEntry::operator==(const std::string other_tag)
{
    return tag == other_tag;
}

bool CaptureSequence::SequenceEntry::operator==(const SequenceEntry& other)
{
    return tag == other.tag;
}

}

namespace sfml_widgets // RenderContext
{
RenderContext::RenderContext(sf::RenderWindow& window) : m_window(&window), m_frame(nullptr)
{
    
}

RenderContext::RenderContext(Frame& frame) : m_window(nullptr), m_frame(&frame)
{

}

RenderContext::RenderContext(RenderContext&& other) noexcept : m_window(other.m_window), m_frame(other.m_frame)
{
    other.m_window = nullptr;
    other.m_frame  = nullptr;
}

RenderContext& RenderContext::operator=(RenderContext&& other) noexcept
{
    if(this != &other)
    {
        m_window = other.m_window;
        m_frame  = other.m_frame;
        other.m_window = nullptr;
        other.m_frame  = nullptr;
    }
    return *this;
}

void RenderContext::draw(const sf::Drawable& __widget, const sf::RenderStates __state)
{
    if(m_window) m_window->draw(__widget, __state);
    else m_frame->drawChildWidget(__widget, __state);
}

sf::Vector2f RenderContext::mapMousePosition(sf::Vector2i mouse_position) const
{
    if(m_window) return m_window->mapPixelToCoords(mouse_position);
    return m_frame->mapPixelToCoords(mouse_position);
}

void RenderContext::setWindowView(const sf::View& __v)
{
    if(!m_window) m_frame->getWindowPtr()->setView(__v);
    else m_window->setView(__v);
}

void RenderContext::setWindowPtr(sf::RenderWindow* __winScreenPtr)
{
    m_window = __winScreenPtr;
}

void RenderContext::setFramePtr(Frame* __frPtr)
{
    m_frame = __frPtr;
}

const sf::RenderWindow* RenderContext::getWindowPtr() const
{
    return m_window;
} 

const sf::RenderWindow* RenderContext::getRootWindowPtr() const
{
    if(m_frame) return m_frame->getWindowPtr();
    return m_window;
}

Frame* RenderContext::getFramePtr() const
{
    return m_frame;
}
}

namespace sfml_widgets // WidgetGroup
{
std::shared_ptr<WidgetGroup> WidgetGroup::make_group()
{
    return std::shared_ptr<WidgetGroup>(new WidgetGroup());
}

void WidgetGroup::add(Widget* w)
{
    m_cult.push_back(w);
}

void WidgetGroup::remove(Widget* w)
{
    m_cult.erase(std::remove(m_cult.begin(), m_cult.end(), w), m_cult.end());
}

bool WidgetGroup::contains(const Widget* w) const
{
    return std::find(m_cult.begin(), m_cult.end(), w) != m_cult.end();
}

void WidgetGroup::focusOn(std::optional<const sf::Event> event)
{
    for(Widget* w : m_cult)
        if(w->m_gainFocusCallBack) w->m_gainFocusCallBack(event);
}

void WidgetGroup::focusOff(std::optional<const sf::Event> event)
{
    for(Widget* w : m_cult)
        if(w->m_loseFocusCallBack) w->m_loseFocusCallBack(event);
}
}

namespace sfml_widgets
{
SharedTheme::SharedTheme(ButtonStyle _b, TextureStyle _f, CheckboxStyle _c, EntryStyle _e) : m_buttonTheme(_b), m_textureTheme(_f), m_checkboxTheme(_c), m_entryTheme(_e)
{

}

std::shared_ptr<SharedTheme> SharedTheme::make_theme(ButtonStyle _b, TextureStyle _f, CheckboxStyle _c, EntryStyle _e)
{
    return std::shared_ptr<SharedTheme>(new SharedTheme(_b, _f, _c, _e));
}
}