#include "wrapper_info.hpp"
#ifdef WRAPPER_DEFINED

namespace sfml_widgets
{
InfoWrapper::InfoWrapper(sf::RenderWindow& window, const std::string& title) : ImGuiWrapper(window, title)
{

}

InfoWrapper::~InfoWrapper()
{

}

void InfoWrapper::addButton(const std::string& label, std::function<void()> callback)
{
    m_buttons.push_back(GuiButton{label, std::move(callback)});
}

void InfoWrapper::addText(const std::string& text)
{
    m_texts.push_back(text);
}

void InfoWrapper::enableFps(bool show_fps)
{
    m_revealFps = show_fps;
}

bool InfoWrapper::processEvents(const sf::Event& event)
{
    bool event_used{false};
    if(const auto* key_press = event.getIf<sf::Event::KeyPressed>())
    {
        if(key_press->code == sf::Keyboard::Key::F1)
        {
            toggle();
            event_used = true;
        }
    }
    return event_used;
}

void InfoWrapper::renderWidgets()
{
    for(GuiButton& functional_button : m_buttons)
    {
        if(ImGui::Button(functional_button.label.c_str()) && functional_button.callback)
            functional_button.callback();
    }
    
    for(const std::string& label : m_texts)
    {
        ImGui::Text("%s", label.c_str());
    }

    if(m_revealFps)
    {        
        ImGui::Separator();
        ImGui::Text("Performance");
        const ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
        if(io.Framerate > 0.f)
        {
            ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        }
    }
}
}

#endif