#include "wrapper_info.hpp"
#ifdef COMPILE_TESTS

using namespace sfml_widgets;

int main()
{
    sf::RenderWindow window(sf::VideoMode({960, 640}), "Widget Sandbox");
    InfoWrapper imgui(window, "Test");
    imgui.addWidget<Frame>(window, sf::Vector2f{0.f, 0.f}, static_cast<sf::Vector2u>(window.getSize()));
    imgui.enableFps(true);

    Button button(window, {100.f, 200.f}, {200, 50}, "ToolTip");
    auto* tooltip_ptr = button.EnableToolTip();
    if(tooltip_ptr)
    {
        tooltip_ptr->setCornerRadius(12.f);
        tooltip_ptr->setTheme(Theme::tooltip_style::TOOLTIP_EMERALD_FOREST);
        tooltip_ptr->setOffset({200.f, 0.f});
        tooltip_ptr->getText(tooltip_ptr->createText({10.f, 10.f}))->setText("HELLO WORLDS\n GOODBYE WORLDS");
        // if(tooltip_ptr->getImage(tooltip_ptr->createImage({100.f, 100.f}))->setImage(asa, {200, 200}))
        // {
        //     std::cout << "Yes\n";
        // }
        // else
        // {
        //     std::cout << "No\n";
        // }
    }

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

            Widget::beginEventLoop();
            button.captureMouseEvents(*event);
            button.captureKeyEvents(*event);
            imgui.captureEvents(*event);
        }

        window.clear({0, 0, 0});
        imgui.render();
        button.draw();
        window.display();
    }
    return 0;
}
#else
#include <iostream>
int main() { std::cout << "empty\n"; return 0; }
#endif