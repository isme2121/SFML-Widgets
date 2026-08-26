#include "button.hpp"
#include <iostream>

/*
* Short program to render a functional button that prints "Button Clicked"
* whenever it's released by the mouse.
*/

int main()
{
    sf::RenderWindow window(sf::VideoMode({480, 640}), "Simple Button");

    // Constructing the button
    sfml_widgets::Button button(window,                 // root window
                            {140.f, 290.f},     // position
                                {200, 60},          // size
                                "My Button");           // button's text

    // OPTIONAL: updating the style and adding rounded corners
    button.setTheme(sfml_widgets::Theme::button_style::BUTTON_STEEL_GRAPHITE);
    button.setCornerRadius(12.f);

    // Binding lambda to button
    button.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release, // The event type that triggers the callback
        [&button](std::optional<sf::Event> e){std::cout << "Button Clicked\n";}, // The callback, must always take an optional sf::Event as a parameter, any passed references are manually managed
        sf::Mouse::Button::Left); // For `Press` and `Release` types, you must specify which mouse button which can trigger the callback
                        
    while(window.isOpen())
    {
        while(const std::optional event = window.pollEvent())
        {
            if(event->is<sf::Event::Closed>())
                window.close();

            // Required for the event loop
            sfml_widgets::Widget::beginEventLoop();

            // Updates the visual state + triggers any bound mouse event
            // Important even if button has no bound mouse event
            button.captureMouseEvents(*event);
        }

        window.clear({48, 48, 48});

        // Drawing the button on root window
        button.draw();
        window.display();
    }
    return 0;
}
