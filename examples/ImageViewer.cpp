#include "button.hpp"
#include "entry.hpp"
#include "frame.hpp"
#include "image_frame.hpp"
#include "tinyfiledialogs.h"
#include "base.hpp"
#include <filesystem>
#include <thread>
#include <atomic>
#include <cmath>

/*
* A simple image viewer program with the use of `ImageFrame` widget
* NOTE: This process runs on multiple threads:
*   - main thread for widgets
*   - second thread for file dialog
*   - third thread for loading images to ImageFrame widgets
*   - forth thread for opening an image on a separate window
*
* However, widgets do not provide thread safety, with the exception of `ImageFrame` class.
* `ImageFrame` can call `setImage` method on a separate thread safely, making this
* implementation safe. Any other method however might cause a crash due to a nanosecond miss-timing.
*/

constexpr sf::Vector2f IMAGE_SIZE{100.f, 100.f}; //!< don't touch
const std::vector<const char*> EXTENSIONS{".png", ".jpg", ".jpeg"}; //!< Valid image extensions (you can touch)
constexpr sf::Color MAIN_WIN_BG_COLOR{53, 53, 53}; //!< irrelevant
constexpr sf::Color IMAGE_WIN_BG_COLOR{0, 0, 0};   //!< irrelevant

// Manages dialog and selected directory path
struct DialogThread
{
    std::thread thread;                     //!< Dialog thread
    std::mutex guard;                       //!< Guard for `directory_path`
    std::string directory_path;             //!< Currently selected path
    std::atomic<bool> is_open{false};       //!< Indicates if dialog is currently open
    std::atomic<bool> path_found{false};    //!< Indicates if a new path is available
};

// Manages all image data and the thread responsible for loading thumbnails
struct GalleryThread
{
    std::thread thread;                             //!< Thread for loading thumbnails
    std::vector<std::filesystem::path> paths;       //!< Paths to all existing images in selected directory
    std::vector<sfml_widgets::ImageFrame*> images;  //!< Gallery images
    std::atomic<bool> break_process{false};         //!< Tells the thumbnails thread to break it's process if true
};

// Manages pop up window that holds the currently selected image on a different thread
struct WindowThread
{
    std::thread thread;                             //!< Thread for image window loop
    std::unique_ptr<sf::RenderWindow> window;       //!< Image window ptr
    std::unique_ptr<sfml_widgets::ImageFrame> image;//!< ImageFrame object ptr, drawn on top of image window
    std::atomic<bool> force_close{false};           //!< External call to break from the window's thread

    // Closes the window safely
    void close()
    {
        window.reset();
        image.reset();
        force_close.store(false);
    }
};

// Opens dialog, called on a separate thread
void open_dialog(DialogThread& dialog);

// Loads images for existing imageFrames, called on a separate thread
void load_thumbnails(GalleryThread& gallery);

// Creates a second window to display full size image, called on a separate thread
void image_window(WindowThread& win_guard, sf::Vector2u size, sf::String path);

// Clears all loaded images, called on the main thread
void reset_gallery(GalleryThread& gallery);

// Loads all available paths for images in selected directory, creates image frames and calls a separate process for thumbnails.
// Called on the main thread
void load_frames(DialogThread& dialog, GalleryThread& gallery, WindowThread& win_guard, sfml_widgets::Frame& frame, sfml_widgets::Entry& entry);

// Returns a new image size for images with larger dimensions than the screen
sf::Vector2u fit_to_screen(sf::Vector2u image_size);

int main()
{
    sf::RenderWindow window(sf::VideoMode({400, 600}), "Gallery");

    sfml_widgets::Frame frame(window, {0.f, 0.f}, {400, 600});
    sfml_widgets::Entry entry(frame, {0.f, 0.f}, {320, 25});
    sfml_widgets::Button search_button(frame, {320.f, 0.f}, {80, 25}, "Browse");
    sfml_widgets::Frame images_frame(frame, {0.f, 25.f}, {400, 575});   //!< Holds all images

    // Theme setup
    frame.linkTheme(sfml_widgets::Theme::light_theme);
    search_button.linkTheme(sfml_widgets::Theme::light_theme);
    entry.linkTheme(sfml_widgets::Theme::light_theme);
    images_frame.linkTheme(sfml_widgets::Theme::light_theme);

    // Search button setup
    search_button.setIcon(sfml_widgets::images::IMG_SEARCH);
    search_button.setIconPosition(sfml_widgets::Button::SIDE::Right);
    search_button.setIconSizeRatio(0.2f);
    search_button.setTextSize(0.4f);

    // Path entry setup
    entry.configure(sfml_widgets::FLAGS::SCROLL);
    entry.setTextOffsetRatio(0.02f);
    entry.setCursorWidth(2);
    entry.setBackGroundText("...");
    entry.setTextLimit(500);
    entry.setTextSize(0.5f);

    // Frames setup
    frame.setStale(true);
    images_frame.setStale(true);
    images_frame.setScrollSpeed(0.2f);

    // Threads setup
    DialogThread dialog;
    GalleryThread gallery;
    WindowThread win_guard;

    // Mouse callback for search button, opens dialog
    search_button.bindMouseEvent(sfml_widgets::Widget::W_MouseEventType::Release,
        [&dialog](std::optional<sf::Event>)
        {
            // dialog already open? do nothing
            if(dialog.is_open.load()) return;

            // wait for previous dialog to close if currently closing
            if(dialog.thread.joinable()) dialog.thread.join();

            // open new dialog on a separate thread
            dialog.is_open.store(true);
            dialog.thread = std::thread(open_dialog, std::ref(dialog));
        },
        sf::Mouse::Button::Left
    );


    // Entry callback for manually selecting path, searches entered path
    entry.bindKeyEvent("Search path",
            {sfml_widgets::Key{sf::Keyboard::Key::Enter}},  //!< Triggers after pressing `Enter`
            [&entry, &dialog]()
            {
                // get current input
                std::filesystem::directory_entry path{std::string(entry.getText())};
                // check if path exists
                if(path.exists())
                {
                    // select path
                    entry.clearText();
                    std::lock_guard<std::mutex> lock(dialog.guard);
                    dialog.directory_path = path.path();
                    dialog.path_found.store(true);
                }
            });

    // main loop
    while(window.isOpen())
    {
        // events
        while(const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            sfml_widgets::Widget::beginEventLoop();
            frame.captureMouseEvents(*event);
            frame.captureKeyEvents(*event);
        }

        if(dialog.path_found.load())
        {
            // new path? initialize image frames & initiate thumbnails thread
            load_frames(dialog, gallery, win_guard, images_frame, entry);
        }

        window.clear(MAIN_WIN_BG_COLOR);
        frame.draw();
        window.display();
    }

    // clean up before exiting
    if(dialog.thread.joinable()) dialog.thread.join();
    gallery.break_process.store(true);
    if(gallery.thread.joinable()) gallery.thread.join();
    win_guard.force_close.store(true);
    if(win_guard.thread.joinable()) win_guard.thread.join();
    reset_gallery(gallery);
    return 0;
}

void open_dialog(DialogThread& dialog)
{
    const char* dir_path = tinyfd_selectFolderDialog("Open Folder", "");

    {
        std::lock_guard<std::mutex> lock(dialog.guard);
        dialog.directory_path = dir_path ? dir_path : "";
    }
    dialog.path_found.store(true);
    dialog.is_open.store(false);
}

std::vector<std::filesystem::path> collect_pngs(const std::filesystem::path& dir) {
    std::vector<std::filesystem::path> output_paths;

    // go through all files in directory, dir must exist
    for (const auto& file : std::filesystem::directory_iterator(dir))
    {
        // check using the extension
        if (file.is_regular_file() && (std::find(EXTENSIONS.begin(), EXTENSIONS.end(), file.path().extension()) != EXTENSIONS.end())) 
        {
            output_paths.push_back(file.path());
        }
    }

    return output_paths;
}

void load_thumbnails(GalleryThread& gallery)
{
    const std::size_t num_paths = gallery.paths.size();
    for(std::size_t i{0}; i < num_paths; i++)
    {
        if(gallery.break_process.load()) break; // break process from external signal

        // load images from disk
        gallery.images[i]->setImage(gallery.paths[i].string());
    }
}

void reset_gallery(GalleryThread& gallery)
{
    // clear all image data in gallery
    for(sfml_widgets::ImageFrame* img : gallery.images) delete img;
    gallery.images.clear();
    gallery.paths.clear();
}

sf::Vector2u fit_to_screen(sf::Vector2u image_size)
{
    if(image_size.x != 0 && image_size.y != 0)
    {
        // screen size
        const sf::Vector2u screen_size = sf::VideoMode::getDesktopMode().size;
        
        // find the larger dimension of image that exceeds screen size
        const float RATIO_X = std::min(1.f, static_cast<float>(screen_size.x) / static_cast<float>(image_size.x));
        const float RATIO_Y = std::min(1.f, static_cast<float>(screen_size.y) / static_cast<float>(image_size.y));
        const float SCALE = std::min(RATIO_X, RATIO_Y);
    
        // screen is larger than image
        if(SCALE == 1.f) return image_size;

        // image is larger than screen, shrink image by the largest scale ratio
        image_size.x = std::round(image_size.x * SCALE);
        image_size.y = std::round(image_size.y * SCALE);
    }

    // min value = 100, avoids VideoMode error
    return sf::Vector2u
    {
        std::max(100u, image_size.x),
        std::max(100u, image_size.y)
    };
}

void image_window(WindowThread& win_guard, sf::Vector2u size, sf::String path)
{
    // secondary window for selected image

    // re-initialize window
    win_guard.close();
    win_guard.window = std::make_unique<sf::RenderWindow>(sf::VideoMode{size}, path);
    win_guard.image = std::make_unique<sfml_widgets::ImageFrame>(
                                    *win_guard.window,
                                    sf::Vector2f{0.f, 0.f},
                                    size,
                                    path); // initialized from cache as image already exists in thumbnails gallery


    // secondary window's main loop
    while(win_guard.window->isOpen())
    {
        // external signal to close the window
        if(win_guard.force_close.load())
        {
            win_guard.close();
            return;
        }

        while(const std::optional<sf::Event> event = win_guard.window->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                win_guard.close();
                return;
            }
        }

        win_guard.window->clear(IMAGE_WIN_BG_COLOR);
        win_guard.image->draw();
        win_guard.window->display();
    }
}

void load_frames(DialogThread& dialog, GalleryThread& gallery, WindowThread& win_guard, sfml_widgets::Frame& frame, sfml_widgets::Entry& entry)
{
    /*
    * MAIN THREAD PROCESS
    * 1- get new path
    * 2- clears any existing data
    * 3- collect new images from selected path
    * 4- initialize ImageFrame widgets without an image to not block the main thread
    * 5- launch a separate thread for loading thumbnails
    */

    dialog.path_found.store(false);

    // get newly selected path
    std::string selected_path;
    {
        std::lock_guard<std::mutex> lock(dialog.guard);
        selected_path = dialog.directory_path;
    }

    if(selected_path.empty()) return;

    // clear previous data
    gallery.break_process.store(true);
    if(gallery.thread.joinable()) gallery.thread.join();
    reset_gallery(gallery);

    // update entry from dialog
    entry.setText(selected_path);

    // get all available paths from selected path
    gallery.paths = collect_pngs(selected_path);

    // initialize image frames without data
    std::size_t num_images = gallery.paths.size();
    std::size_t row = 0, col = 0;
    gallery.images.reserve(num_images);
    for(const std::filesystem::path& path : gallery.paths)
    {
        gallery.images.emplace_back(new sfml_widgets::ImageFrame(frame, 
                                    sf::Vector2f{IMAGE_SIZE.x * row, IMAGE_SIZE.y * col},
                                    sf::Vector2u{IMAGE_SIZE}));
        
        ++row;
        if(row > 3)
        {
            row = 0;
            ++col;
        }

        // setup the new ImageFrame widget
        auto& back = gallery.images.back();

        back->linkTheme(sfml_widgets::Theme::light_theme);

        // Mouse click callback, creates a new window on a separate thread for the selected image
        back->bindMouseEvent(sfml_widgets::Widget::W_MouseEventType::Release,
            [&back, &win_guard](std::optional<sf::Event>)
            {
                sf::Vector2u new_size = fit_to_screen(back->getOriginalImageSize());
                const sf::String& path = back->getImagePath();

                // close previous window if any
                if(win_guard.thread.joinable())
                {
                    win_guard.force_close.store(true);
                    win_guard.thread.join();
                }

                // create a new window
                win_guard.thread = std::thread(image_window, std::ref(win_guard), std::move(new_size), std::ref(path));
            },
            sf::Mouse::Button::Left);
    }

    // start loading thumbnails on a separate thread
    gallery.break_process.store(false);
    gallery.thread = std::thread(load_thumbnails, std::ref(gallery));
}
