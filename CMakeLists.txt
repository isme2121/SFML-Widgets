cmake_minimum_required(VERSION 3.28)
project(SFML-widgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

option(SFML_WIDGETS_SHARED "Build SFML widgets as dynamic libraries" OFF)
option(SFML_WIDGETS_MORE_LINE_TYPES "Defines more default lines for GridCanvas" ON)
option(BUILD_DEBUG_GUI "Build debugging GUI" OFF)
option(BUILD_EXAMPLES "Build examples to '${PROJECT_SOURCE_DIR}/bin/executables/'" OFF)
option(BUILD_TESTS "Build tests to '${PROJECT_SOURCE_DIR}/bin/tests/'" OFF)

#======SFML setup====================================================
find_package(SFML 3 CONFIG QUIET)
if(NOT SFML_FOUND)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        3.0.2
        GIT_SHALLOW    ON
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    set(SFML_BUILD_AUDIO OFF)
    set(SFML_BUILD_NETWORK OFF)
    set(BUILD_SHARED_LIBS ON)
    FetchContent_MakeAvailable(SFML)
    unset(BUILD_SHARED_LIBS)

else()
    message(STATUS "SFML ${SFML_VERSION} found in system.")
endif()

#======widgets setup====================================================
set(EXTERNAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/external-include")
set(WIDGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(BUILD_SHARED_LIBS ${SFML_WIDGETS_SHARED})

#core library
add_library(widgets
    src/widgets.cpp
    src/widget.cpp
    src/frame.cpp
    src/tooltip.cpp
)
target_compile_definitions(widgets PUBLIC 
    STB_IMAGE_RESIZE_IMPLEMENTATION
    BUILD_TESTS
    SFML_WIDGETS_MORE_LINE_TYPES
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
unset(BUILD_SHARED_LIBS)

#======Examples setup====================================================
if(BUILD_EXAMPLES)
    if(EXISTS "${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
    else()
        message(WARNING "'${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt' file is missing, skipping build.") 
    endif()
endif()

#======Test setup====================================================
if(BUILD_TESTS)
    if(EXISTS "${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory("${PROJECT_SOURCE_DIR}/tests")

    else()
        message(WARNING "'${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt' file is missing, skipping build.")
    endif()
endif()