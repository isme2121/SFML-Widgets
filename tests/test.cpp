#if defined(BUILD_TESTS) && defined(BUILD_DEBUG_GUI)
#include "wrapper_info.hpp"

using namespace sfml_widgets;

int main()
{
    sf::RenderWindow window(sf::VideoMode({960, 640}), "Widget Sandbox");
    InfoWrapper imgui(window, "Test");
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
        imgui.render();
        window.display();
    }
    return 0;
}
#else
#include <iostream>
int main() { std::cout << "empty\n"; return 0; }
#endif