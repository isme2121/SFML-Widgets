#pragma once
#include "info_base.hpp"

#if defined(WIDGETS_HPP) && defined(COMPILE_TESTS)
namespace sfml_widgets
{

///////////////////////////////////////////////////////////////////
/// @brief Wrapper for widget for setters/getters accessability
///
/// @tparam WidgetT must be a derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
class InfoWidget
{
    static_assert(std::is_base_of_v<Widget, WidgetT>, "Type is not a widget.");
    static_assert(!std::is_abstract_v<WidgetT>, "Type is not an abstract.");

public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs a sfml widget with a wrapper
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename... Ts, typename = std::enable_if_t<std::is_constructible_v<WidgetT, Ts...>>>
    explicit InfoWidget(Ts&&... args) : m_widget(std::forward<Ts>(args)...)
    {
        // link all getters/setters for this widget
        WidgetBaseGetters::link(m_getters);
        WidgetGetterTraits<WidgetT>::link(m_getters);
        WidgetBaseSetters::link(m_setters);
        WidgetSetterTraits<WidgetT>::link(m_setters);
    }

    InfoWidget(const InfoWidget&) = delete;
    InfoWidget& operator=(const InfoWidget&) = delete;
    InfoWidget(InfoWidget&&) = delete;
    InfoWidget& operator=(InfoWidget&&) = delete;

    WidgetT& widget()
    {
        return m_widget;
    }

    const WidgetT& widget() const
    {
        return m_widget;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns value relative to key, otherwise `nullopt` if
    /// key is not found
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<Val__> get(const std::string& key) const
    {
        auto it = m_getters.find(key);
        if(it == m_getters.end()) return std::nullopt;
        return it->second(m_widget);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the proper setter based on `key` string, `v` must
    /// match the proper type of the setter
    ///
    ///////////////////////////////////////////////////////////////////
    bool set(const std::string& key, Val__ v)
    {
        auto it = m_setters.find(key);
        if(it != m_setters.end()) return it->second(m_widget, v);
        return false;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a map with all the existing key and values for this
    /// widget type
    ///
    ///////////////////////////////////////////////////////////////////
    std::unordered_map<std::string, Val__> fetch() const
    {
        std::unordered_map<std::string, Val__> tmp;
        tmp.reserve(m_getters.size());
        for(const auto& [key, getter] : m_getters) tmp.emplace(key, getter(m_widget));
        return tmp;
    }

private:
    WidgetT m_widget;
    LookupMapT<WidgetT> m_getters;
    SetterMapT<WidgetT> m_setters;
};


#ifdef WRAPPER_DEFINED

///////////////////////////////////////////////////////////////////
/// @brief Integration of InfoWidget to dear ImGui's interface
///
///////////////////////////////////////////////////////////////////
class InfoWrapper : public ImGuiWrapper
{
public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs ImGui frame
    ///
    ///////////////////////////////////////////////////////////////////
    explicit InfoWrapper(sf::RenderWindow&, const std::string& = "");

    InfoWrapper(const InfoWrapper&) = delete;
    InfoWrapper& operator=(const InfoWrapper&) = delete;
    InfoWrapper(InfoWrapper&&) = delete;
    InfoWrapper& operator=(InfoWrapper&&) = delete;

    ~InfoWrapper() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Adds a button to the frame with the passed callback for
    /// click event
    ///
    ///////////////////////////////////////////////////////////////////
    void addButton(const std::string&, std::function<void()>);

    ///////////////////////////////////////////////////////////////////
    /// @brief Adds text label to the frame
    ///
    ///////////////////////////////////////////////////////////////////
    void addText(const std::string&);

    /////////////////////////////////////////////////////////////////// 
    /// @brief Updates FPS display toggle, disabled by default
    ///
    ///////////////////////////////////////////////////////////////////
    void enableFps(bool);
    
private:
    bool processEvents(const sf::Event&) override;
    void renderWidgets() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief @temp
    ///
    ///////////////////////////////////////////////////////////////////
    struct GuiButton
    {
        std::string label;
        std::function<void()> callback;
    };

    std::vector<GuiButton> m_buttons;       //!< buttons
    std::vector<std::string> m_texts;       //!< labels
    bool m_revealFps{false};                //!< displays FPS when set to true
};
}

#endif
#endif