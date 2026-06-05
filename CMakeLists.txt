cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#======SFML setup====================================================
#SFML essentials go to "SFML/lib" and "SFML/include"

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/SFML/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/SFML/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/SFML/lib")

#for visual studio, libraries could end up in the wrong directory if we don't do this
foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_SOURCE_DIR}/SFML/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_SOURCE_DIR}/SFML/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_SOURCE_DIR}/SFML/lib")
endforeach()

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
file(COPY "${sfml_SOURCE_DIR}/include/" DESTINATION "${CMAKE_SOURCE_DIR}/SFML/include/")


#======widgets setup====================================================
#same process but with different directory "lib/"

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_SOURCE_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_SOURCE_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_SOURCE_DIR}/lib")
endforeach()


#core library
add_library(widgets
    src/widgets.cpp
    src/widget.cpp
    src/frame.cpp
)
target_include_directories(widgets PUBLIC include)
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
add_library(button
    src/button.cpp
)
target_include_directories(button PUBLIC include)
target_link_libraries(button PUBLIC 
    widgets
)

#entry library
add_library(entry
    src/entry.cpp
)
target_include_directories(entry PUBLIC include)
target_link_libraries(entry PUBLIC 
    widgets
)

#checkbox library
add_library(checkbox
    src/checkbox.cpp
)
target_include_directories(checkbox PUBLIC include)
target_link_libraries(checkbox PUBLIC 
    widgets
)

#grid canvas library
add_library(grid_canvas
    src/grid_canvas.cpp
)
target_include_directories(grid_canvas PUBLIC include)
target_link_libraries(grid_canvas PUBLIC 
    widgets
)

#image frame library
add_library(image_frame
    src/image_frame.cpp
)
target_include_directories(image_frame PUBLIC include)
target_link_libraries(image_frame PUBLIC 
    widgets
)

#label library
add_library(label
    src/label.cpp
)
target_include_directories(label PUBLIC include)
target_link_libraries(label PUBLIC 
    widgets
)