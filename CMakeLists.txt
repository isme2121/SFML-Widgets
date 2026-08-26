cmake_minimum_required(VERSION 3.28)
project(SFML-widgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#======SFML setup====================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/libs/SFML")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/libs/SFML")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/libs/SFML")

set(BUILD_SHARED_LIBS ON)

#install sfml to local repo
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.2
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)
file(COPY "${sfml_SOURCE_DIR}/include/" DESTINATION "${PROJECT_SOURCE_DIR}/SFML/include/")

#======widgets setup====================================================
set(EXTERNAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/external-include")
set(WIDGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(WIDGET_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/SFML-widgets")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${WIDGET_LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${WIDGET_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${WIDGET_LIB_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#core library
add_library(widgets
    src/widgets.cpp
    src/widget.cpp
    src/frame.cpp
    src/tooltip.cpp
)
target_compile_definitions(widgets PUBLIC 
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    STB_IMAGE_RESIZE_IMPLEMENTATION
    COMPILE_TESTS
)

target_include_directories(widgets PUBLIC 
    "${WIDGET_INCLUDE_DIR}"
    "${EXTERNAL_INCLUDE_DIR}"
)

target_link_libraries(widgets PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
)

#math library must be linked explicitly for linux
if(UNIX)
    target_link_libraries(widgets PUBLIC m)
endif()

#button library
add_library(button src/button.cpp)
target_link_libraries(button PUBLIC widgets)

#entry library
add_library(entry src/entry.cpp)
target_link_libraries(entry PUBLIC widgets)

#checkbox library
add_library(checkbox src/checkbox.cpp)
target_link_libraries(checkbox PUBLIC widgets)

#grid canvas library
add_library(grid_canvas src/grid_canvas.cpp)
target_link_libraries(grid_canvas PUBLIC widgets)

#image frame library
add_library(image_frame src/image_frame.cpp)
target_link_libraries(image_frame PUBLIC widgets)

#label library
add_library(label src/label.cpp)
target_link_libraries(label PUBLIC widgets)

#======Examples setup====================================================
option(COMPILE_EXAMPLES "Build examples to '${PROJECT_SOURCE_DIR}/examples/executables/'" OFF)
if(COMPILE_EXAMPLES)
    if(EXISTS "${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
    else()
        message(WARNING "'${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt' file is missing, skipping build.") 
    endif()
endif()

#======Test setup====================================================
option(COMPILE_TESTS "Build tests to '${PROJECT_SOURCE_DIR}/tests/executables/'" OFF)

if(COMPILE_TESTS)
    if(EXISTS "${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory("${PROJECT_SOURCE_DIR}/tests")

    else()
        message(WARNING "'${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt' file is missing, skipping build.")
    endif()
endif()