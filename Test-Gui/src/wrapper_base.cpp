#include "wrapper_base.hpp"
#ifdef WRAPPER_DEFINED
#include <stdexcept>

namespace sfml_widgets
{
ImGuiWrapper::ImGuiWrapper(sf::RenderWindow& window, const std::string& title) : m_window(window), m_title(title)
{
    if(!ImGui::SFML::Init(m_window))
        throw std::runtime_error("Failed to initialize ImGui-Test.");
    
    m_ID = ++s_instanceCount;
    setTitle(m_title);
}

ImGuiWrapper::~ImGuiWrapper()
{
    ImGui::SFML::Shutdown(m_window);
}

void ImGuiWrapper::render()
{
    if(!m_guiVisible)
    {
        m_hovered = false;
        return;
    }
    ImGui::SFML::Update(m_window, m_deltaClock.restart());
    ImGui::Begin(m_title.c_str());
    renderWidgets();
    ImGui::End();
    m_hovered = ImGui::GetIO().WantCaptureMouse;
    ImGui::SFML::Render(m_window);
}

bool ImGuiWrapper::captureEvents(const sf::Event& event)
{
    if(!m_guiVisible)
    {
        Widget::s_externalEventUsed = false;
        return false;
    }

    ImGui::SFML::ProcessEvent(m_window, event);
    if(event.is<sf::Event::MouseButtonPressed>() && m_hovered)
    {
        // widgets cannot capture this event nor update their states for this loop
        Widget::s_externalEventUsed = true;
        return true;
    }
    else
    {
        Widget::s_externalEventUsed = false;
    }

    return processEvents(event);
}

void ImGuiWrapper::setTitle(const std::string& title)
{
    // ## keeps the id invisible, frame maintains a unique internal label
    m_title = title + "##" + std::to_string(m_ID);
}

void ImGuiWrapper::show()
{
    m_guiVisible = true;
}

void ImGuiWrapper::hide()
{
    m_guiVisible = false;
}

void ImGuiWrapper::toggle()
{
    m_guiVisible = !m_guiVisible;
}

bool ImGuiWrapper::isHidden() const
{
    return !m_guiVisible;
}
}
#endif