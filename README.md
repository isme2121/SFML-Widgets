[![C++](https://img.shields.io/badge/C%2B%2B-17-darkblue)](https://en.cppreference.com/w/cpp/17) [![SFML](https://img.shields.io/badge/SFML-3.0.2-8cc8e6)](https://www.sfml-dev.org/) [![CMake](https://img.shields.io/badge/CMake-%E2%89%A53.28-3673e1)](https://cmake.org/) [![Platform](https://img.shields.io/badge/platform-linux-5cb3e1)](#requirements)

A lightweight widget framework built with SFML library.

---

## Platform

| Platform          | Support                                                     |
|-------------------|-------------------------------------------------------------|
| Linux             | Developed and tested                                        |
| Windows (MSVC)    | Will be supported soon                                      | 
| linux + mingw-w64 | Experimental, limited testing                               |
| Mac OS            | Not tested                                                  |

---

## Requirements

Make sure you have the following tools first:
- C++17
- CMake ≥ 3.28
- git
- SFML dependencies listed in [documentation](https://www.sfml-dev.org/tutorials/3.0/getting-started/build-from-source) page

---

## Build

Build from source:
```bash
git clone https://github.com/isme2121/SFML-Widgets.git
cd SFML-Widgets
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

You can also build with the following options:
- `-DBUILD_EDITOR_GUI=ON`: Builds `Editor` GUI from source, `OFF` by default.
- `-DBUILD_EXAMPLES=ON`: Builds the example executables, `OFF` by default.
- `-DBUILD_TESTS=ON`: Builds the test executables, `OFF` by default.
- `-DSFML_WIDGETS_SHARED=ON`: Builds widgets as shared libraries, `ON` by default.
- `-DCANVAS_MORE_LINE_TYPES=ON`: Enables additional built-in line types in `grid_canvas`, `ON` by default.

---

## Getting started

Easiest method to get started using the library:
```cmake
cmake_minimum_required(VERSION 3.28)
project(MyProject LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    sfml-widgets
    GIT_REPOSITORY https://github.com/isme2121/SFML-Widgets.git
    GIT_TAG        a80604a377022949ab205b2b353d28c8b66f9ac8
)
FetchContent_MakeAvailable(sfml-widgets)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE
    sfml-widgets::button
)
```

Simple button example:

```cpp
#include "button.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Button Window");

    sfml_widgets::Button button(window, {300.f, 275.f}, {200, 50}, "My Button");
    
    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            button.captureMouseEvents(*event);
            button.captureKeyEvents(*event);
        }

        window.clear(sf::Color::Black);
        button.draw();
        window.display();
    }
}
```

## Documentation

`/docs` will be available soon.