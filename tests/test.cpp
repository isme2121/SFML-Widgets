#if defined(BUILD_EDITOR_GUI)
#include "editor.hpp"

using namespace sfml_widgets;

int main()
{
    Widget::assign_external<WidgetEditor>();
    sf::RenderWindow window(sf::VideoMode({960, 640}), "Widget Sandbox");
    WidgetEditor imgui(window, "Test");
    imgui.addWidget<Frame>(window, sf::Vector2f{0.f, 0.f}, static_cast<sf::Vector2u>(window.getSize()));
    imgui.enableFps(true);

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

            Widget::beginEventLoop();
            imgui.captureEvents(*event);
        }

        window.clear({0, 0, 0});
        imgui.draw();
        window.display();
    }
    return 0;
}
#else
#include <iostream>
int main() { std::cout << "empty\n"; return 0; }
#endif