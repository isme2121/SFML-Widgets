#include "imgui_panel.hpp"
#ifdef WRAPPER_DEFINED
#include <imgui.h>
#include <imgui-SFML.h>
#include <stdexcept>
namespace
{

///////////////////////////////////////////////////////////////////
/// @brief Overall a mouse event
///
///////////////////////////////////////////////////////////////////
bool is_mouse_event(const sf::Event& event)
{
    return event.is<sf::Event::MouseButtonPressed>() ||
           event.is<sf::Event::MouseButtonReleased>() ||
           event.is<sf::Event::MouseMoved>() ||
           event.is<sf::Event::MouseWheelScrolled>() ||
           event.is<sf::Event::MouseEntered>() ||
           event.is<sf::Event::MouseLeft>();
}

///////////////////////////////////////////////////////////////////
/// @brief Overall a Keyboard event
///
///////////////////////////////////////////////////////////////////
bool is_keyboard_event(const sf::Event& event)
{
    return event.is<sf::Event::KeyPressed>() ||
           event.is<sf::Event::KeyReleased>() ||
           event.is<sf::Event::TextEntered>();
}
}

namespace sfml_widgets
{
ImGuiFrame::ImGuiFrame(sf::RenderWindow& window, const std::string& title) : m_window(window), m_title(title)
{
    if(!ImGui::SFML::Init(m_window))
        throw std::runtime_error("Failed to initialize ImGui-Test.");
    
    m_ID = ++s_instanceCount;
    setTitle(m_title);
}

ImGuiFrame::~ImGuiFrame()
{
    ImGui::SFML::Shutdown(m_window);
}

void ImGuiFrame::draw()
{
    if(!m_guiVisible)
    {
        m_hovered = false;
        m_keyboardEventUsed = false;
        renderWinElements();
        return;
    }

    ImGui::SFML::Update(m_window, m_deltaClock.restart());

    ImGui::Begin(m_title.c_str());
    renderWinElements();
    renderWidgets();
    ImGui::End();

    m_hovered = ImGui::GetIO().WantCaptureMouse;
    m_keyboardEventUsed = ImGui::GetIO().WantCaptureKeyboard;
    ImGui::SFML::Render(m_window);
}

bool ImGuiFrame::captureEvents(const sf::Event& event)
{
    if(!m_guiVisible)
    {
        s_eventUsed = false;
    }
    else
    {
        ImGui::SFML::ProcessEvent(m_window, event);
        s_eventUsed = (m_hovered && is_mouse_event(event)) ||
                      (m_keyboardEventUsed && is_keyboard_event(event));
    }

    return processEvents(event) || s_eventUsed;
}

void ImGuiFrame::setTitle(const std::string& title)
{
    // ## keeps the id invisible, frame maintains a unique internal label
    m_title = title + "##" + std::to_string(m_ID);
}

void ImGuiFrame::show()
{
    m_guiVisible = true;
}

void ImGuiFrame::hide()
{
    m_guiVisible = false;
}

void ImGuiFrame::toggle()
{
    m_guiVisible = !m_guiVisible;
}

bool ImGuiFrame::isHidden() const
{
    return !m_guiVisible;
}
}
#endif