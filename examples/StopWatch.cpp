#include "button.hpp"
#include "label.hpp"

/*
* This program renders a simple stopwatch using 3 buttons and a label with
* the use of `sf::Clock` and a function to format time into MM:ss::fff.
* The time format is constantly updated using a routine lambda bound to label
*/
// function for formatting time to min:sec:fff
template<typename T>
std::string format(const T elapsed_time, std::size_t max_size)
{
    std::string str_ms = std::to_string(elapsed_time);
    std::size_t size = str_ms.size();

    if(size > max_size) str_ms = str_ms.substr(size - max_size, max_size);
    else if(size < max_size) str_ms = std::string(max_size - size, '0') + str_ms;
    
    return str_ms;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({640, 210}), "Stopwatch");

    sfml_widgets::Button start_clock_btn(window, {30.f, 20.f}, {200, 50}, "Start");
    sfml_widgets::Button pause_clock_btn(window, {30.f, 80.f}, {200, 50}, "Pause");
    sfml_widgets::Button reset_clock_btn(window, {30.f, 140.f}, {200, 50}, "Reset");
    sfml_widgets::Label label(window, {250.f, 30.f}, {250, 150});

    // OPTIONAL: setting all widgets we created to pre-built theme
    start_clock_btn.linkTheme(sfml_widgets::Theme::dark_theme);
    pause_clock_btn.linkTheme(sfml_widgets::Theme::dark_theme);
    reset_clock_btn.linkTheme(sfml_widgets::Theme::dark_theme);
    label.linkTheme(sfml_widgets::Theme::dark_theme);

    // Updating corner radius
    start_clock_btn.setCornerRadius(12.f);
    pause_clock_btn.setCornerRadius(12.f);
    reset_clock_btn.setCornerRadius(12.f);

    label.setStale(true);                                           // Changes label to not visually react to events
    label.setPaddingRatio(0.f);                                     // no extra padding on alignment
    label.setHorizontalAlignment(sfml_widgets::Label::HAlign::Left);// text aligned to the left
    label.setTextSize(0.5f);                                        // text size now scales with label's shape size and takes half of it's height

    // Timer
    sf::Clock clock;
    clock.reset();

    // Starting the clock bind
    start_clock_btn.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release,
        [&clock](std::optional<sf::Event> e){clock.start();},
        sf::Mouse::Button::Left
    );

    // Pausing the clock bind
    pause_clock_btn.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release,
        [&clock](std::optional<sf::Event> e){clock.stop();},
        sf::Mouse::Button::Left
    );

    // Reseting the clock bind
    reset_clock_btn.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release,
        [&clock](std::optional<sf::Event> e){
            if(clock.isRunning()) clock.restart();
            else clock.reset();
        },
        sf::Mouse::Button::Left
    );

    
    // Binding a routine to label that updates it's content every 1ms at most
    label.addRoutine(
        sf::milliseconds(1), // period for this callback to trigger
        [&clock, &label]() -> bool
        {
            sf::Time t = clock.getElapsedTime();
            unsigned int seconds = static_cast<int>(t.asSeconds());

            // formatted time to MM:ss:fff
            std::string content = format<int>(seconds / 60, 2) + ":" + format<int>(seconds % 60, 2) + ":" + format<std::int32_t>(t.asMilliseconds(), 3);
            label.setText(content);

            // Returning false only will make this callback run for the duration of the program
            return false;
        });

    // main loop
    while(window.isOpen())
    {
        while(const std::optional event = window.pollEvent())
        {
            if(event->is<sf::Event::Closed>())
                window.close();

            sfml_widgets::Widget::beginEventLoop();

            start_clock_btn.captureMouseEvents(*event);
            pause_clock_btn.captureMouseEvents(*event);
            reset_clock_btn.captureMouseEvents(*event);
            // in this case, it's not necessary to capture label's mouse events
            // since it has no mouse event bounds and it's stale is set to true
            label.captureMouseEvents(*event);
        }

        window.clear({32, 32, 32});
        // drawing all widgets on the window
        start_clock_btn.draw();
        pause_clock_btn.draw();
        reset_clock_btn.draw();
        label.draw();
        window.display();
    }
    return 0;
}
