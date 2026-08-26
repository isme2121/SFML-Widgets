#include "button.hpp"
#include "frame.hpp"
#include "checkbox.hpp"
#include "entry.hpp"
#include "label.hpp"
#include "image_frame.hpp"
#include "base.hpp"
#include <iostream>

/*
* A login/sign in window example
* Creating and signing into accounts stored in a database
* This program uses routine to create a sliding animation between the different frames
*/

struct Account
{
    sf::String username;
    sf::String password;

    bool operator==(const Account& other) const
    {
        return username == other.username && password == other.password;
    }

    bool operator==(const sf::String& other_name) const
    {
        return username == other_name;
    }

};

using DataBase = std::vector<Account>;
DataBase db;
static bool guard = false; //!< prevents routine overlap
std::shared_ptr<sfml_widgets::WidgetTheme> my_theme = sfml_widgets::WidgetTheme::make_theme(*sfml_widgets::Theme::light_theme);
sfml_widgets::ButtonStyle dark_button;

// Initiates theme and dark_button
void setup_theme();

// integral of step(x) = 3x^2 - 2x^3 over [0, x], for x in [0,1]
float smoothstepIntegral(float x);

/// Eased progress in [0,1] for a trapezoidal velocity profile
/// Result integrates to exactly 1 at progress = 1.f, regardless of accel_end/decel_start
float trapezoidEase(float progress, float end_acceleration, float start_slowdown);

/* Creates a routine that shifts both frames together to different coordinates
* assigner: the widget that calls the routine, which is the button that calls the routine
* frame_1: first frame
* frame_2: second frame
* frame_1_target_x: horizontal target coordinate for frame 1 to reach after period ends
* frame_2_target_x: horizontal target coordinate for frame 2 to reach after period ends
* period: event period
*/
void setup_routine(sfml_widgets::Widget& assigner, sfml_widgets::Frame& frame_1, sfml_widgets::Frame& frame_2, float frame_1_target_x, float frame_2_target_x, float period = 1000.f);

// Base frame for holding widgets
class BaseFrame
{
public:
    BaseFrame(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size) : m_frame(window, position, size) {}
    virtual void setupBindings(BaseFrame* sign_frame, DataBase& db) = 0;
    virtual void linkTheme(std::shared_ptr<sfml_widgets::WidgetTheme>& theme, sfml_widgets::ButtonStyle unique_btn_style) = 0;

    sfml_widgets::Frame& getFrame()
    {
        return m_frame;
    }

protected:
    sfml_widgets::Frame m_frame;
};

class LoginFrame : public BaseFrame
{
public:
    LoginFrame(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size) : BaseFrame(window, position, size),
    // row 0
    m_label(m_frame, {0.f, 30.f}, {500, 55}, "USER LOGIN", sfml_widgets::parameters::INTER_SEMIBOLD_18),
    // row 1
    m_nameImage(m_frame, {20.f, 96.f}, {26, 26}, sfml_widgets::images::IMG_PEOPLE),
    m_nameEntry(m_frame, {60.f, 92.f}, {340, 36}),
    // row 2
    m_pwImage(m_frame, {20.f, 164.f}, {26, 26}, sfml_widgets::images::IMG_LOCK),
    m_pwEntry(m_frame, {60.f, 160.f}, {340, 36}),
    m_showPwCheckbox(m_frame, {415.f, 164.f}, {26, 26}, "show", true, sfml_widgets::parameters::INTER_SEMIBOLD_18),
    // row 3
    m_loginButton(m_frame, {60.f, 222.f}, {160, 44}, "Login"),
    m_signButton(m_frame, {240.f, 222.f}, {160, 44}, "Sign In")
    {
        // setup for login frame

        // remove annoying visual effects for stale widgets
        m_frame.setStale(true);
        m_label.setStale(true);
        m_nameImage.setStale(true);
        m_pwImage.setStale(true);
        m_showPwCheckbox.setStale(true);

        // updating text size
        m_label.setTextSize(0.4f);
        m_loginButton.setTextSize(0.45f);
        m_signButton.setTextSize(0.45f);
        m_signButton.setIconSizeRatio(0.15f);

        // adding rounded corners
        m_frame.setCornerRadius(4.f);
        m_nameEntry.setCornerRadius(2.f);
        m_pwEntry.setCornerRadius(2.f);
        m_showPwCheckbox.setCornerRadius(4.f);
        m_loginButton.setCornerRadius(2.f);
        m_signButton.setCornerRadius(2.f);

        // adding background text for entries
        m_nameEntry.setBackGroundText("Username");
        m_pwEntry.setBackGroundText("Password");

        // adding a check image for the checkbox
        m_showPwCheckbox.setBackgroundTexture(sfml_widgets::images::IMG_LOCK);
        m_showPwCheckbox.setTextPosition(sfml_widgets::DIRECTION::RIGHT);

        // adding an icon to sign in button
        m_signButton.setIcon(sfml_widgets::images::IMG_LOGIN_2);
        m_signButton.setIconPosition(sfml_widgets::Button::SIDE::Right);
        
        // more entry setup
        m_nameEntry.setCursorWidth(3);
        m_pwEntry.setCursorWidth(3);
        m_nameEntry.setTextOffsetRatio(0.04f);
        m_pwEntry.setTextOffsetRatio(0.04f);
    }

    void setupBindings(BaseFrame* sign_frame, DataBase& db) override
    {
        // add functionality to login frame widgets

        // login button triggering login event
        m_loginButton.bindMouseEvent(
            sfml_widgets::Widget::W_MouseEventType::Release,
            [this, &db](std::optional<sf::Event> e)
            {
                // get the input data
                const sf::String& name = m_nameEntry.getText();
                const sf::String& password = m_pwEntry.getText();
                Account temp{name, password};

                // search for any matches
                auto it = std::find(db.begin(), db.end(), temp);
                if(it != db.end())
                {
                    std::cout << "Login successful.\n";
                    m_nameEntry.clearText(); // clear name from entry when logged in
                }
                else
                {
                    // data does not match
                    std::cout << "Failed to login, name or password do not match.\n";
                }
                // clear password everytime
                m_pwEntry.clearText();

            },
            sf::Mouse::Button::Left);

        // Sign-in button bind, triggers frame swap
        m_signButton.bindMouseEvent(
            sfml_widgets::Widget::W_MouseEventType::Release,
            [this, sign_frame](std::optional<sf::Event> e)
            {
                // create a routine inside the bind callback to begin the shifting process
                setup_routine(m_signButton, m_frame, sign_frame->getFrame(), 600.f, 100.f, 1400.f);
            },
            sf::Mouse::Button::Left);


        // checkbox hide password bind for login frame
        m_showPwCheckbox.bindToggleEvent(
            [this](std::optional<sf::Event> e)
            {
                // if current state = 0 --> set placeholder to '*'
                if(!m_showPwCheckbox.getState()) m_pwEntry.setPlaceholderChar('*');
                else m_pwEntry.setPlaceholderChar(); // no placeholder
            }
        );
    }

    void linkTheme(std::shared_ptr<sfml_widgets::WidgetTheme>& theme, sfml_widgets::ButtonStyle unique_btn_style) override
    {
        // link global theme to the frame widgets
        m_frame.linkTheme(theme);
        m_label.linkTheme(theme);
        m_nameImage.linkTheme(theme);
        m_nameEntry.linkTheme(theme);
        m_pwEntry.linkTheme(theme);
        m_pwImage.linkTheme(theme);
        m_showPwCheckbox.linkTheme(theme);
        m_signButton.linkTheme(theme);

        // no theme for login button, instead, we just assign a constant style
        m_loginButton.setTheme(unique_btn_style);
    }

private:
    sfml_widgets::Label m_label;
    sfml_widgets::ImageFrame m_nameImage;
    sfml_widgets::Entry m_nameEntry;
    sfml_widgets::ImageFrame m_pwImage;
    sfml_widgets::Entry m_pwEntry;
    sfml_widgets::CheckBox m_showPwCheckbox;
    sfml_widgets::Button m_loginButton;
    sfml_widgets::Button m_signButton;
};

class SignFrame : public BaseFrame
{
public:
    SignFrame(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2u size) : BaseFrame(window, position, size),
    // row 0
    m_label(m_frame, {0.f, 30.f}, {400, 55}, "USER SIGN IN", sfml_widgets::parameters::INTER_SEMIBOLD_18),
    // row 1
    m_nameEntry(m_frame, {70.f, 92.f}, {240, 36}),
    // row 2
    m_pwEntry(m_frame, {70.f, 160.f}, {240, 36}),
    // row 3
    m_rpwEntry(m_frame, {70.f, 228.f}, {240, 36}),
    m_showPwCheckbox(m_frame, {325.f, 232.f}, {26, 26}, "show", true, sfml_widgets::parameters::INTER_SEMIBOLD_18),
    // row 4
    m_signButton(m_frame, {70.f, 460.f}, {240, 44}, "Sign In"),
    m_loginButton(m_frame, {70.f, 520.f}, {240, 44}, "Login")
    {
        // sign-in frame setup

        // remove annoying visual effects for stale widgets
        m_frame.setStale(true);
        m_label.setStale(true);
        m_showPwCheckbox.setStale(true);

        // add rounded corners
        m_frame.setCornerRadius(4.f);
        m_nameEntry.setCornerRadius(2.f);
        m_pwEntry.setCornerRadius(2.f);
        m_rpwEntry.setCornerRadius(2.f);
        m_showPwCheckbox.setCornerRadius(4.f);
        m_loginButton.setCornerRadius(2.f);
        m_signButton.setCornerRadius(2.f);
    
        // setup text size
        m_label.setTextSize(0.4f);
        m_loginButton.setTextSize(0.45f);
        m_signButton.setTextSize(0.45f);
        
        // setup icon image for sign-in button
        m_signButton.setIcon(sfml_widgets::images::IMG_LOGIN_2);
        m_signButton.setIconPosition(sfml_widgets::Button::SIDE::Right);
        m_signButton.setIconSizeRatio(0.15f);

        // setup entries
        m_nameEntry.setBackGroundText("Username");
        m_pwEntry.setBackGroundText("Password");
        m_rpwEntry.setBackGroundText("Confirm Password");
        m_nameEntry.setCursorWidth(3);
        m_pwEntry.setCursorWidth(3);
        m_rpwEntry.setCursorWidth(3);
        m_nameEntry.setTextOffsetRatio(0.04f);
        m_pwEntry.setTextOffsetRatio(0.04f);
        m_rpwEntry.setTextOffsetRatio(0.04f);

        // setup hide password checkbox for sign-in frame
        m_showPwCheckbox.setBackgroundTexture(sfml_widgets::images::IMG_LOCK);
        m_showPwCheckbox.setTextPosition(sfml_widgets::DIRECTION::RIGHT);
    }

    void setupBindings(BaseFrame* login_frame, DataBase& db) override
    {
    // add functionality to sign-in frame

    // Login button bind, triggers frame swap
    m_loginButton.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release,
        [this, login_frame](std::optional<sf::Event> e)
        {
            // create a routine inside the bind callback to begin shifting process
            setup_routine(m_loginButton, m_frame, login_frame->getFrame(), -400.f, 50.f, 1400.f);
        },
        sf::Mouse::Button::Left);
    
    // Sign in button bind, add account to database if it has a unique name
    m_signButton.bindMouseEvent(
        sfml_widgets::Widget::W_MouseEventType::Release,
        [this, &db](std::optional<sf::Event> e)
        {
            // get the input data
            const sf::String& name = m_nameEntry.getText();
            const sf::String& password = m_pwEntry.getText();
            const sf::String& rpassword = m_rpwEntry.getText();

            if(password != rpassword)
            {
                std::cout << "Error, passwords do not match.\n";
                m_pwEntry.clearText();
                m_rpwEntry.clearText();
                return;
            }

            // check if the name is already taken
            auto it = std::find(db.begin(), db.end(), name);
            if(it != db.end())
            {
                std::cout << "Error, username is already taken.\n";
                m_nameEntry.clearText();
                return;
            }
            
            // add account to database
            Account temp{name, password};
            std::cout << "Welcome, " << name.toAnsiString() << ".\n";
            db.emplace_back(std::move(temp));
            m_nameEntry.clearText();
            m_pwEntry.clearText();
            m_rpwEntry.clearText();
        },
        sf::Mouse::Button::Left
    );

    // checkbox hide password bind for sign-in frame
    m_showPwCheckbox.bindToggleEvent(
        [this](std::optional<sf::Event> e)
        {
            // if current state = 0 --> set placeholder
            if(!m_showPwCheckbox.getState())
            {
                m_pwEntry.setPlaceholderChar('*');
                m_rpwEntry.setPlaceholderChar('*');
            }
            else
            {
                // no placeholder
                m_pwEntry.setPlaceholderChar();
                m_rpwEntry.setPlaceholderChar();
            }
        }
    );
    }

    void linkTheme(std::shared_ptr<sfml_widgets::WidgetTheme>& theme, sfml_widgets::ButtonStyle unique_btn_style) override
    {
        // setup frame theme
        m_frame.linkTheme(theme);
        m_label.linkTheme(theme);
        m_nameEntry.linkTheme(theme);
        m_pwEntry.linkTheme(theme);
        m_rpwEntry.linkTheme(theme);
        m_showPwCheckbox.linkTheme(theme);
        m_loginButton.linkTheme(theme);
        // separate style
        m_signButton.setTheme(unique_btn_style);
    }

private:
    sfml_widgets::Label m_label;
    sfml_widgets::Entry m_nameEntry;
    sfml_widgets::Entry m_pwEntry;
    sfml_widgets::Entry m_rpwEntry;
    sfml_widgets::CheckBox m_showPwCheckbox;
    sfml_widgets::Button m_signButton;
    sfml_widgets::Button m_loginButton;
};



int main()
{
    // main window
    sf::RenderWindow window(sf::VideoMode({600, 800}), "Login", sf::Style::Close | sf::Style::Titlebar);

    ///////////////// LOGIN PANEL /////////////////
    LoginFrame login_frame{window, {50.f, 200.f}, {500, 300}};

    ///////////////// SIGN IN PANEL /////////////////
    SignFrame sign_frame{window, {-400.f, 100.f}, {400, 600}};

    ///////////////// CONFIGURATIONS /////////////////
    // Hide the non-active frame
    sign_frame.getFrame().configure(sfml_widgets::FLAGS::HIDDEN);
    
    // initiate and assign theme
    setup_theme();
    login_frame.linkTheme(my_theme, dark_button);
    sign_frame.linkTheme(my_theme, dark_button);
    
    // add functionality
    login_frame.setupBindings(&sign_frame, db);
    sign_frame.setupBindings(&login_frame, db);

    // main loop
    while(window.isOpen())
    {
        while(const std::optional<sf::Event> event = window.pollEvent())
        {
            if(event->is<sf::Event::Closed>()) window.close();

            sfml_widgets::Widget::beginEventLoop();
            // only the widgets that are constructed using a sf::RenderWindow object as root require event checking
            login_frame.getFrame().captureMouseEvents(*event);
            login_frame.getFrame().captureKeyEvents(*event); //!< entries require this
            sign_frame.getFrame().captureMouseEvents(*event);
            sign_frame.getFrame().captureKeyEvents(*event);  //!< entries require this
        }

        window.clear({212, 212, 212});

        // only the widgets that are constructed under a window root require calling draw
        login_frame.getFrame().draw();
        sign_frame.getFrame().draw();
        window.display();
    }
    return 0;
}

float smoothstepIntegral(float x)
{
    return x * x * x - 0.5f * x * x * x * x;
}

float trapezoidEase(float progress, float end_acceleration, float start_slowdown)
{
    // constant change
    const float dt = 1.f / (0.5f * end_acceleration + (start_slowdown - end_acceleration) + 0.5f * (1.f - start_slowdown));
    // progress at end_acceleration
    const float s1 = dt * end_acceleration * 0.5f;
    // progress at start_slowdown
    const float s2 = s1 + dt * (start_slowdown - end_acceleration);

    // still accelerating -> ramp up speed
    if(progress <= end_acceleration)
    {
        return dt * end_acceleration * smoothstepIntegral(progress / end_acceleration);
    }
    // done accelerating -> very slight speed up relative to progress
    else if(progress <= start_slowdown)
    {
        return s1 + dt * (progress - end_acceleration);
    }
    // begin to slowdown -> slight reduction in speed
    else
    {
        // overall slowdown as we get close to 100%
        float tau = (progress - start_slowdown) / (1.f - start_slowdown);
        return s2 + dt * (1.f - start_slowdown) * (tau - smoothstepIntegral(tau));
    }
}

void setup_theme()
{
    if(!my_theme) return;

    // get button style
    auto button_style = my_theme->getStyle<sfml_widgets::Button>();

    // assign global independant button style
    dark_button = button_style;
    dark_button.fill_color = sf::Color(84, 84, 84);
    dark_button.text_color = sf::Color(212, 212, 212);

    // image frame style
    auto iframe_style = my_theme->getStyle<sfml_widgets::ImageFrame>();
    iframe_style.outline_color = iframe_style.fill_color;
    
    // checkbox style
    auto checkbox_style = my_theme->getStyle<sfml_widgets::CheckBox>();
    checkbox_style.outline_thickness = 1.f;

    auto entry_style = my_theme->getStyle<sfml_widgets::Entry>();
    entry_style.bg_text_color.a = 60;

    // sync changes
    my_theme->setStyle<sfml_widgets::ImageFrame>(iframe_style);
    my_theme->setStyle<sfml_widgets::CheckBox>(checkbox_style);
    my_theme->setStyle<sfml_widgets::Entry>(entry_style);
}

void setup_routine(sfml_widgets::Widget& assigner, sfml_widgets::Frame& frame_1, sfml_widgets::Frame& frame_2, float frame_1_target_x, float frame_2_target_x, float period)
{
    // prevents multiple routine calls with guard
    if(guard) return;

    // creating the routine
    assigner.addRoutine(sf::milliseconds(1), // triggers after every 1 ms
    [&frame_2, &frame_1,
    initialized = false,
    completed = false,
    target_x = sf::Vector2f{frame_2_target_x, frame_1_target_x},
    start_x = sf::Vector2f{0.f, 0.f},
    clock = sf::Clock{},
    period_ms = period,
    accel_end = 0.2f,
    decel_start = 0.5f
    ]() mutable -> bool
    {
        guard = true; // routine created

        // initial process
        if(!initialized)
        {
            if(frame_2.isFlag(sfml_widgets::FLAGS::HIDDEN)) frame_2.configure(sfml_widgets::FLAGS::HIDDEN);
            start_x = {frame_2.getWidgetPosition().x, frame_1.getWidgetPosition().x};
            initialized = true;
        }

        // get progress based on duration
        std::int32_t elapsed_ms = clock.getElapsedTime().asMilliseconds();
        float progress = std::clamp(static_cast<float>(elapsed_ms) / period_ms, 0.f, 1.f);

        // new progress based on current acceleration
        float eased = trapezoidEase(progress, accel_end, decel_start);

        // update the current position for both frames
        sf::Vector2f first_pos = frame_1.getWidgetPosition();
        first_pos.x = start_x.y + (target_x.y - start_x.y) * eased;
        sf::Vector2f second_pos = frame_2.getWidgetPosition();
        second_pos.x = start_x.x + (target_x.x - start_x.x) * eased;

        // check if the process is complete
        if(progress == 1.f)
        {
            completed = true;
            second_pos.x = target_x.x;
            first_pos.x = target_x.y;
            // hide irrelevant frame
            if(!frame_1.isFlag(sfml_widgets::FLAGS::HIDDEN)) frame_1.configure(sfml_widgets::FLAGS::HIDDEN);

            // allow the routine to be called again
            guard = false;
        }

        frame_2.setWidgetPosition(second_pos);
        frame_1.setWidgetPosition(first_pos);
        return completed;
    });
}