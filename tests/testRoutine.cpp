#ifdef COMPILE_TESTS
#include "button.hpp"
#include "frame.hpp"
#include "wrapper_info.hpp"
#include <chrono>
#include <iostream>

using SFK = sfml_widgets::Key;
// Routine period
static constexpr std::int32_t PERIOD{1000};

int main()
{
    sf::RenderWindow window(sf::VideoMode({960, 640}), "Test Routine");
    sfml_widgets::Frame frame(window, {280.f, 120.f}, {400, 400});
    sfml_widgets::Button my_butt(frame, {100.f, 100.f}, {200, 60}, "Button");
    auto timer = std::chrono::high_resolution_clock::now();
    std::size_t loops{0};
    double average_ms{0};
    double average_us{0};

    frame.setTheme(sfml_widgets::Theme::texture_style::TEXTURE_STEEL_GRAPHITE);
    my_butt.setTheme(sfml_widgets::Theme::button_style::BUTTON_STEEL_GRAPHITE);

    frame.setCornerRadius(12.f);
    frame.configure(sfml_widgets::FLAGS::ALWAYSSELECTED);
    frame.bindKeyEvent("Toggle hide", {SFK(sf::Keyboard::Key::H)},
    [&frame](){
        frame.configure(sfml_widgets::FLAGS::HIDDEN);
    });

    frame.bindKeyEvent("Toggle block button", {SFK(sf::Keyboard::Key::B)},
    [&my_butt](){
        my_butt.configure(sfml_widgets::FLAGS::BLOCKED);
    });

    frame.bindKeyEvent("Toggle Transparent", {SFK(sf::Keyboard::Key::T)},
    [&frame](){
        frame.configure(sfml_widgets::FLAGS::TRANSPARENT);
    });

    frame.bindKeyEvent("Toggle Auto-hide", {SFK(sf::Keyboard::Key::H, true)},
    [&frame](){
        frame.configure(sfml_widgets::FLAGS::AUTOHIDE);
    });

    frame.bindKeyEvent("Toggle Auto-transparent", {SFK(sf::Keyboard::Key::T, true)},
    [&frame](){
        frame.configure(sfml_widgets::FLAGS::AUTOTRANSPARENT);
    });


    my_butt.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release,
        [&my_butt](std::optional<sf::Event>){
            my_butt.configure(sfml_widgets::FLAGS::BLOCKED);
        });
                  
    my_butt.addRoutine(sf::milliseconds(PERIOD),
        [&my_butt, &timer, &loops, &average_us]() -> bool
        {
            auto end_timer = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(timer).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timer).time_since_epoch().count();
            auto duration = end - start;
            average_us += duration;
            double ms = duration * 0.001;
            std::cout << "Loop: " << loops++ << "\n";
            std::cout << "chronos duration: " << duration << " us" << " (" << ms << " ms)\n";
            timer = std::chrono::high_resolution_clock::now();
            return false;
        });

    sfml_widgets::InfoWrapper* Ptest = new sfml_widgets::InfoWrapper(window);
    Ptest->enableFps(true);
    Ptest->addText("Toggle Block Button: B");
    Ptest->addText("Toggle Hide Frame: H");
    Ptest->addText("Toggle Transparent Frame: T");
    Ptest->addText("Toggle AutoHide Frame: Ctrl+H");
    Ptest->addText("Toggle AutoTransparent Frame: Ctrl+T");

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            sfml_widgets::Widget::beginEventLoop();
            Ptest->captureEvents(*event);
            frame.captureMouseEvents(*event);
            frame.captureKeyEvents(*event);
        }
        
        window.clear({0, 0, 0});
        frame.draw();
        Ptest->render();
        window.display();
    }

    average_us = average_us / static_cast<double>(loops);
    average_ms = average_us * 0.001;

    std::cout << "Average: " << average_us << " us(" << average_ms << " ms)\n";
    delete Ptest;
    return 0;
}

#endif