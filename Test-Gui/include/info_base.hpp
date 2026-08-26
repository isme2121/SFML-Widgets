#pragma once
#include "wrapper_base.hpp"

#if defined(WIDGETS_HPP) && defined(COMPILE_TESTS) || 1
#include "button.hpp"
#include "frame.hpp"
#include "label.hpp"
#include "entry.hpp"
#include "checkbox.hpp"
#include "image_frame.hpp"
#include "grid_canvas.hpp"
#include <type_traits>
#include <utility>

namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Make sure `fn` always return a boolean
///
/// For lambda type matching, `invoke_setter` may always return a boolean
/// for any setter method
///
///////////////////////////////////////////////////////////////////
template<typename Fn>
bool invoke_setter(Fn&& fn)
{
    if constexpr(std::is_void_v<std::invoke_result_t<Fn>>)
    {
        std::forward<Fn>(fn)();
        return true;
    }
    else
    {
        return std::forward<Fn>(fn)();
    }
}

// Default visitor
template<class... Ts>
struct visitor : Ts... { using Ts::operator()...; };
template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;

// All getter/setter types collapsed here
using Val__ = std::variant<bool, char, float, unsigned int, std::size_t, std::string, sfml_widgets::Button::SIDE,
                           Frame::F_SeparatorInfo, Frame::F_SeparatorType, Frame::F_AfterPreProcess,
                           DIRECTION, sf::Text::Style, Label::HAlign, Label::VAlign, Cell*,
                           sf::String, sf::FloatRect, sf::Angle, sf::Vector2f, sf::Vector2u, sf::Vector2i,
                           std::optional<sf::Vector2u>>;


// Map for getter methods
template<typename WidgetT>
using LookupMapT = std::unordered_map<std::string, Val__(*)(const WidgetT&)>;

// Map for setter methods
template<typename WidgetT>
using SetterMapT = std::unordered_map<std::string, bool(*)(WidgetT&, const Val__&)>;


///////////////////////////////////////////////////////////////////
/// @brief Base getter methods linker struct
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
struct WidgetGetterTraits
{
    static void link(LookupMapT<WidgetT>&){}
};

///////////////////////////////////////////////////////////////////
/// @brief Base setter methods linker struct
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
struct WidgetSetterTraits
{
    static void link(SetterMapT<WidgetT>&){}
};

///////////////////////////////////////////////////////////////////
/// @brief Base `Widget` class getters
///
///////////////////////////////////////////////////////////////////
struct WidgetBaseGetters
{
    template<typename WidgetT>
    static void link(LookupMapT<WidgetT>& map)
    {
        map.emplace("position", + [](const WidgetT& w) -> Val__ {return w.getWidgetPosition();});
        map.emplace("absolute_position", + [](const WidgetT& w) -> Val__ {return w.getAbsolutePosition();});
        map.emplace("size", + [](const WidgetT& w) -> Val__ {return w.getWidgetSize();});
        map.emplace("rotation", + [](const WidgetT& w) -> Val__ {return w.getWidgetRotation();});
        map.emplace("bounds", + [](const WidgetT& w) -> Val__ {return w.getBounds();});
        map.emplace("absolute_bounds", + [](const WidgetT& w) -> Val__ {return w.getAbsoluteBounds();});
        map.emplace("is_hidden", + [](const WidgetT& w) -> Val__ {return w.isHidden();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Button` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<Button>
{
    static void link(LookupMapT<Button>& map)
    {
        map.emplace("text", + [](const Button& w) -> Val__  {return w.getText();});
        map.emplace("text_size", + [](const Button& w) -> Val__ {return w.getTextSize();});
        map.emplace("text_limit", + [](const Button& w) -> Val__ {return w.getTextLimit();});
        map.emplace("text_position", + [](const Button& w) -> Val__ {return w.getTextPosition();});
        map.emplace("icon_path", + [](const Button& w) -> Val__ {return w.getIconPath();});
        map.emplace("icon_side", + [](const Button& w) -> Val__ {return w.getIconPosition();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Frame` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<Frame>
{
    static void link(LookupMapT<Frame>& map)
    {
        map.emplace("child_count", + [](const Frame& w) -> Val__ {return w.getChildCount();});
        map.emplace("scroll_offset", + [](const Frame& w) -> Val__ {return w.getScrollOffset();});
        map.emplace("scroll_speed", + [](const Frame& w) -> Val__ {return w.getScrollSpeed();});
        map.emplace("separator_info", + [](const Frame& w) -> Val__ {return w.getSeparatorInfo();});
        map.emplace("process_mode", + [](const Frame& w) -> Val__ {return w.getProcessMode();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `CheckBox` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<CheckBox>
{
    static void link(LookupMapT<CheckBox>& map)
    {
        map.emplace("state", + [](const CheckBox& w) -> Val__ {return w.getState();});
        map.emplace("text_size", + [](const CheckBox& w) -> Val__ {return w.getTextSize();});
        map.emplace("text_limit", + [](const CheckBox& w) -> Val__ {return w.getTextLimit();});
        map.emplace("text_padding", + [](const CheckBox& w) -> Val__ {return w.getTextPadding();});
        map.emplace("text_position", + [](const CheckBox& w) -> Val__ {return w.getTextPosition();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Entry` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<Entry>
{
    static void link(LookupMapT<Entry>& map)
    {
        map.emplace("text", + [](const Entry& w) -> Val__ {return w.getText();});
        map.emplace("background_text", + [](const Entry& w) -> Val__ {return w.getBackGroundText();});
        map.emplace("text_size", + [](const Entry& w) -> Val__ {return w.getTextSize();});
        map.emplace("text_limit", + [](const Entry& w) -> Val__ {return w.getTextLimit();});
        map.emplace("text_offset", + [](const Entry& w) -> Val__ {return w.getTextOffsetRatio();});
        map.emplace("placeholder_char", + [](const Entry& w) -> Val__ {return w.getPlaceholderChar();});
        map.emplace("cursor_index", + [](const Entry& w) -> Val__ {return w.getCursorIndex();});
        map.emplace("cursor_width", + [](const Entry& w) -> Val__ {return w.getCursorWidth();});
        map.emplace("blink_ratio", + [](const Entry& w) -> Val__ {return w.getBlinkUpRatio();});
        map.emplace("scroll_offset", + [](const Entry& w) -> Val__ {return w.getScrollOffset();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `ImageFrame` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<ImageFrame>
{
    static void link(LookupMapT<ImageFrame>& map)
    {
        map.emplace("state", + [](const ImageFrame& w) -> Val__ {return w.getImagePath();});
        map.emplace("text_size", + [](const ImageFrame& w) -> Val__ {return w.getOriginalImageSize();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Label` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<Label>
{
    static void link(LookupMapT<Label>& map)
    {
        map.emplace("state", + [](const Label& w) -> Val__ {return w.getText();});
        map.emplace("text_size", + [](const Label& w) -> Val__ {return w.getTextSize();});
        map.emplace("text_limit", + [](const Label& w) -> Val__ {return w.getTextLimit();});
        map.emplace("text_style", + [](const Label& w) -> Val__ {return w.getTextStyle();});
        map.emplace("h_align", + [](const Label& w) -> Val__ {return w.getHorizontalAlignment();});
        map.emplace("v_align", + [](const Label& w) -> Val__ {return w.getVerticalAlignment();});
        map.emplace("line_spacing", + [](const Label& w) -> Val__ {return w.getLineSpacing();});
        map.emplace("letter_spacing", + [](const Label& w) -> Val__ {return w.getLetterSpacing();});
        map.emplace("padding_ratio", + [](const Label& w) -> Val__ {return w.getPaddingRatio();});
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `GridCanvas` class getters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetGetterTraits<GridCanvas>
{
    static void link(LookupMapT<GridCanvas>& map)
    {
        map.emplace("local_pixel", + [](const GridCanvas& w) -> Val__ {return w.getLocalPixel(sf::Mouse::getPosition(*w.getRootWindow()));});
        map.emplace("view_pixel", + [](const GridCanvas& w) -> Val__ {return w.getViewPixel(sf::Mouse::getPosition(*w.getRootWindow()));});
        map.emplace("local_to_view", + [](const GridCanvas& w) -> Val__ {return w.mapLocalToView(w.getLocalPixel(sf::Mouse::getPosition(*w.getRootWindow())));});
        map.emplace("coordinates", + [](const GridCanvas& w) -> Val__ {return w.mapCoordinates(sf::Mouse::getPosition(*w.getRootWindow()));});
        map.emplace("selected_cell", + [](const GridCanvas& w) -> Val__ {return w.getSelectedCellPtr();});
        map.emplace("clicked_cell", + [](const GridCanvas& w) -> Val__ {return w.getClickedCellPtr();});
        map.emplace("cell_count", + [](const GridCanvas& w) -> Val__ {return w.getCellsCount();});

    }
};


///////////////////////////////////////////////////////////////////
/// @brief Base `Widget` class setters
///
///////////////////////////////////////////////////////////////////
struct WidgetBaseSetters
{
    template<typename WidgetT>
    static void link(SetterMapT<WidgetT>& map)
    {
        map.emplace("position", +[](WidgetT& w, const Val__& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&w, &p]{return w.setWidgetPosition(*p);});
            return false;
        });
        map.emplace("size", +[](WidgetT& w, const Val__& v) -> bool
        {
            if(const sf::Vector2f* p = std::get_if<sf::Vector2f>(&v))
                return invoke_setter([&w, &p]{return w.setWidgetSize(*p);});
            return false;
        });
        map.emplace("rotation", +[](WidgetT& w, const Val__& v) -> bool
        {
            if(const float* p = std::get_if<float>(&v))
                return invoke_setter([&w, &p]{return w.setWidgetRotation(*p);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Button` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<Button>
{
    static void link(SetterMapT<Button>& map)
    {
        map.emplace("text", +[](Button& w, const Val__& v) -> bool
        {
            if(const sf::String* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        map.emplace("text_size", +[](Button& w, const Val__& v) -> bool
        {
            if(const float* f = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*f);});
            return false;
        });
        map.emplace("text_side", +[](Button& w, const Val__& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return w.setTextPosition(*side);});
            return false;
        });
        map.emplace("image_side", +[](Button& w, const Val__& v) -> bool
        {
            if(const Button::SIDE* side = std::get_if<Button::SIDE>(&v))
                return invoke_setter([&]{return w.setIconPosition(*side);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Frame` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<Frame>
{
    static void link(SetterMapT<Frame>& map)
    {
        map.emplace("grid", +[](Frame& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.setGrid(*s);});
            return false;
        });
        map.emplace("separator", +[](Frame& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_SeparatorInfo>(&v))
                return invoke_setter([&]{return w.setSeparators((*s).type, (*s).ratio, (*s).dist);});
            return false;
        });
        map.emplace("process_mode", +[](Frame& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<Frame::F_AfterPreProcess>(&v))
                return invoke_setter([&]{return w.setProcessMode(*s);});
            return false;
        });
        map.emplace("scroll_speed", +[](Frame& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setScrollSpeed(*s);});
            return false;
        });
        map.emplace("scroll", +[](Frame& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.scroll(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `CheckBox` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<CheckBox>
{
    static void link(SetterMapT<CheckBox>& map)
    {
        map.emplace("text", +[](CheckBox& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        map.emplace("text_size", +[](CheckBox& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        map.emplace("text_limit", +[](CheckBox& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        map.emplace("text_position", +[](CheckBox& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<DIRECTION>(&v))
                return invoke_setter([&]{return w.setTextPosition(*s);});
            return false;
        });
        map.emplace("text_padding", +[](CheckBox& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextPadding(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Entry` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<Entry>
{
    static void link(SetterMapT<Entry>& map)
    {
        map.emplace("text", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        map.emplace("text_size", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        map.emplace("text_limit", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        map.emplace("background_text", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setBackGroundText(*s);});
            return false;
        });
        map.emplace("blink_ratio", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setBLinkUpRatio(*s);});
            return false;
        });
        map.emplace("cursor_index", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setCursorIndex(*s);});
            return false;
        });
        map.emplace("placeholder_char", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<char>(&v))
                return invoke_setter([&]{return w.setPlaceholderChar(*s);});
            return false;
        });
        map.emplace("text_offset", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextOffsetRatio(*s);});
            return false;
        });
        map.emplace("cursor_width", +[](Entry& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setCursorWidth(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `ImageFrame` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<ImageFrame>
{
    static void link(SetterMapT<ImageFrame>& map)
    {
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `Label` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<Label>
{
    static void link(SetterMapT<Label>& map)
    {
        map.emplace("text", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::String>(&v))
                return invoke_setter([&]{return w.setText(*s);});
            return false;
        });
        map.emplace("text_limit", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<unsigned int>(&v))
                return invoke_setter([&]{return w.setTextLimit(*s);});
            return false;
        });
        map.emplace("text_size", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setTextSize(*s);});
            return false;
        });
        map.emplace("text_style", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Text::Style>(&v))
                return invoke_setter([&]{return w.setTextStyle(*s);});
            return false;
        });
        map.emplace("h_align", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<Label::HAlign>(&v))
                return invoke_setter([&]{return w.setHorizontalAlignment(*s);});
            return false;
        });
        map.emplace("v_align", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<Label::VAlign>(&v))
                return invoke_setter([&]{return w.setVerticalAlignment(*s);});
            return false;
        });
        map.emplace("line_spacing", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setLineSpacing(*s);});
            return false;
        });
        map.emplace("letter_spacing", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setLetterSpacing(*s);});
            return false;
        });
        map.emplace("padding_ratio", +[](Label& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setPaddingRatio(*s);});
            return false;
        });
    }
};

///////////////////////////////////////////////////////////////////
/// @brief `GridCanvas` class setters
///
///////////////////////////////////////////////////////////////////
template<>
struct WidgetSetterTraits<GridCanvas>
{
    static void link(SetterMapT<GridCanvas>& map)
    {
        map.emplace("insert", +[](GridCanvas& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.insert(*s);});
            return false;
        });
        map.emplace("remove", +[](GridCanvas& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.remove(*s);});
            return false;
        });
        map.emplace("dead_zone", +[](GridCanvas& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<float>(&v))
                return invoke_setter([&]{return w.setDeadZone(*s);});
            return false;
        });
        map.emplace("is_taken", +[](GridCanvas& w, const Val__& v) -> bool
        {
            if(const auto* s = std::get_if<sf::Vector2u>(&v))
                return invoke_setter([&]{return w.isTaken(*s);});
            return false;
        });
    }
};
}

#endif