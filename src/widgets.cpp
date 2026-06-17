#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <cmath>
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

bool checkFlags(const FLAGS& self, FLAGS bits)
{
    return (self & bits) == bits;
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
    if(sequence_length == 0) return;

    // name tag already exists
    if(std::find(m_sequences.begin(), m_sequences.end(), name) != m_sequences.end()) return;

    SequenceEntry temp{std::move(name), std::move(sequence), std::move(callback)};
    
    // insert new sequence based on length
    // longer sequences take priority while checking sequences
    std::vector<SequenceEntry>::iterator it = std::lower_bound(m_sequences.begin(), m_sequences.end(), sequence_length,
    [](const SequenceEntry& existing_sequence, std::size_t size)
    {
        return existing_sequence.sequence.size() > size;
    });

    m_sequences.insert(it, std::move(temp));
    m_max_sequence = std::max(m_max_sequence, sequence_length);
}

void CaptureSequence::deleteSequence(const std::string& name)
{
    std::vector<SequenceEntry>::iterator it = std::find(m_sequences.begin(), m_sequences.end(), name);
    
    if(it != m_sequences.end())
    {
        size_t removed_size = it->sequence.size();
        m_sequences.erase(it);
        if(m_max_sequence == removed_size && m_sequences.size() > 0)
            m_max_sequence = m_sequences.at(0).sequence.size(); //!< take the first index size since it's ordered
    }
}

bool CaptureSequence::checkEvents(const sf::Event& key_event)
{
    const sf::Event::KeyPressed* key = key_event.getIf<sf::Event::KeyPressed>();
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
        
        // not enough keys pressed to match the sequence
        if(m_current_sequence.size() < saved_sequence.size()) continue;

        // knowing that dynamic sequence is equal or larger than saved sequence
        // it's safe to take only the range that matches the saved sequence's length
        Sequence::iterator current_sequence_start_it = m_current_sequence.end() - static_cast<std::ptrdiff_t>(saved_sequence.size());
        if(std::equal(current_sequence_start_it, m_current_sequence.end(), saved_sequence.begin(), saved_sequence.end()))
        {
            items.callback(); //!< assigned callback
            reset();          //!< clear sequence
            return true;
        }

    }
    // remove older key presses to maintain max size
    if(m_current_sequence.size() > m_max_sequence)
    {
        m_current_sequence.erase(m_current_sequence.begin(), 
        m_current_sequence.begin() + static_cast<std::ptrdiff_t>(m_current_sequence.size() - m_max_sequence));
    }
    return false;
}

bool CaptureSequence::SequenceEntry::operator==(const std::string& other_tag)
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

void RenderContext::draw(const sf::Drawable& widget, const sf::RenderStates state)
{
    if(m_window) m_window->draw(widget, state);
    else m_frame->drawChildWidget(widget, state);
}

sf::Vector2f RenderContext::mapMousePosition(sf::Vector2i mouse_position) const
{
    if(m_window) return m_window->mapPixelToCoords(mouse_position);
    return m_frame->mapPixelToCoords(mouse_position);
}

void RenderContext::setWindowView(const sf::View& v)
{
    if(!m_window) m_frame->getWindowPtr()->setView(v);
    else m_window->setView(v);
}

void RenderContext::setWindowPtr(sf::RenderWindow* win_ptr)
{
    m_window = win_ptr;
}

void RenderContext::setFramePtr(Frame* frame_ptr)
{
    m_frame = frame_ptr;
}

const sf::RenderWindow* RenderContext::getWindowPtr() const
{
    return m_window;
} 

sf::RenderWindow* RenderContext::getRootWindowPtr() 
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

bool WidgetGroup::add(Widget* w)
{
    if(std::find(m_cult.begin(), m_cult.end(), w) != m_cult.end())
        return false; //!< widget already in cult

    m_cult.push_back(w);
    return true;
}

bool WidgetGroup::remove(Widget* w)
{
    auto it = std::find(m_cult.begin(), m_cult.end(), w);
    if(it == m_cult.end()) return false; //!< widget not found in cult

    *it = m_cult.back(); //!< copy the w* of back element to removed element position
    m_cult.pop_back(); //!< remove last element
    return true;
}

bool WidgetGroup::contains(const Widget* w) const
{
    return std::find(m_cult.begin(), m_cult.end(), w) != m_cult.end();
}

void WidgetGroup::focusOn(std::optional<sf::Event> event)
{
    for(Widget* w : m_cult)
        if(w->m_gainFocusCallBack) w->m_gainFocusCallBack(event);
}

void WidgetGroup::focusOff(std::optional<sf::Event> event)
{
    for(Widget* w : m_cult)
        if(w->m_loseFocusCallBack) w->m_loseFocusCallBack(event);
}
}

namespace sfml_widgets // WidgetTheme
{
WidgetTheme::WidgetTheme(P) {}

WidgetTheme::WidgetTheme(P, ButtonStyle s_button, TextureStyle s_texture, CheckboxStyle s_checkbox, EntryStyle s_entry, LabelStyle s_label) :
m_buttonTheme(s_button), m_frameTheme(s_texture), m_gridCanvasTheme(s_texture), m_imageFrameTheme(s_texture),
m_checkboxTheme(s_checkbox), m_entryTheme(s_entry), m_labelTheme(s_label)
{

}

std::shared_ptr<WidgetTheme> WidgetTheme::make_theme(ButtonStyle s_button, TextureStyle s_texture, CheckboxStyle s_checkbox, EntryStyle s_entry, LabelStyle s_label)
{
    return std::shared_ptr<WidgetTheme>(new WidgetTheme(P{}, s_button, s_texture, s_checkbox, s_entry, s_label));
}

std::shared_ptr<WidgetTheme> WidgetTheme::make_theme(const WidgetTheme& t)
{
    std::shared_ptr<WidgetTheme> temp = std::shared_ptr<WidgetTheme>(new WidgetTheme(P{}));
    temp->copy_theme(t);
    return temp;
}

void WidgetTheme::copy_theme(const WidgetTheme& t)
{
    if(&t != this)
    {
        m_buttonTheme = t.m_buttonTheme;
        m_frameTheme = t.m_frameTheme;
        m_gridCanvasTheme = t.m_gridCanvasTheme;
        m_imageFrameTheme = t.m_imageFrameTheme;
        m_checkboxTheme = t.m_checkboxTheme;
        m_entryTheme = t.m_entryTheme;
        m_labelTheme = t.m_labelTheme;

        ++m_buttonToken;
        ++m_frameToken;
        ++m_gridCanvasToken;
        ++m_imageFrameToken;
        ++m_checkboxToken;
        ++m_entryToken;
        ++m_labelToken;
    }
}
}