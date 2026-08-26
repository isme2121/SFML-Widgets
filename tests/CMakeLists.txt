# Dear ImGui paths
set(IMGUI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/IMGUI/include")
set(IMGUI_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/Dear-ImGui")

# Glew paths
set(GLEW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/GLEW/include")
set(GLEW_SRC_DIR "${PROJECT_SOURCE_DIR}/GLEW/src")
set(GLEW_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/GLEW")

# SFML-ImGui paths
set(SFML_IMGUI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/SFML-ImGui/include")
set(SFML_IMGUI_SOURCE_DIR "${PROJECT_SOURCE_DIR}/SFML-ImGui/src")
set(SFML_IMGUI_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/SFML-ImGui")

# TestGui paths
set(IMGUI_TEST_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Test-Gui/include")
set(IMGUI_TEST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/Test-Gui/src")
set(IMGUI_TEST_LIB_DIR "${PROJECT_SOURCE_DIR}/libs/Test-Gui")

#======Fetch and setup third party sources=======================================

enable_language(C)
find_package(OpenGL REQUIRED)

#======Dear ImGui setup=======================================
if(NOT EXISTS "${IMGUI_INCLUDE_DIR}")
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.1
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(imgui)
    file(COPY "${imgui_SOURCE_DIR}/" DESTINATION "${IMGUI_INCLUDE_DIR}")
endif()

#======SFML + Dear ImGui setup=======================================
if(NOT EXISTS "${SFML_IMGUI_INCLUDE_DIR}")
    FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG        v3.0
        GIT_SHALLOW    ON
        SOURCE_SUBDIR  "cmake-integration-disabled"
    )
    FetchContent_MakeAvailable(imgui-sfml)

    file(COPY "${imgui-sfml_SOURCE_DIR}/" DESTINATION "${SFML_IMGUI_INCLUDE_DIR}")

    # must define before building
    file(WRITE "${SFML_IMGUI_INCLUDE_DIR}/imgui-SFML_export.h"
        "#pragma once
#define IMGUI_SFML_API
#define IMGUI_SFML_NO_EXPORT"
)
endif()

#======Glew setup=======================================
if(NOT EXISTS "${GLEW_INCLUDE_DIR}/GL" OR NOT EXISTS "${GLEW_SRC_DIR}")
    FetchContent_Declare(
        glew
        URL https://github.com/nigels-com/glew/releases/download/glew-2.3.1/glew-2.3.1.zip
    )
    FetchContent_MakeAvailable(glew)
    file(COPY "${glew_SOURCE_DIR}/include/GL/" DESTINATION "${GLEW_INCLUDE_DIR}/GL")
    file(COPY "${glew_SOURCE_DIR}/src/" DESTINATION "${GLEW_SRC_DIR}")
endif()

#======Building third party libraries=======================================

#======Dear ImGui build=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IMGUI_LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IMGUI_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${IMGUI_LIB_DIR}")
add_library(imgui STATIC
    "${IMGUI_INCLUDE_DIR}/imgui.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_demo.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_draw.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_tables.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp"
)

target_include_directories(imgui PUBLIC "${IMGUI_INCLUDE_DIR}")

# add sfml+imgui library's configs to dear imgui
target_compile_definitions(imgui PUBLIC
    IMGUI_USER_CONFIG="${SFML_IMGUI_INCLUDE_DIR}/imconfig-SFML.h"
)

#======Glew build=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${GLEW_LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${GLEW_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${GLEW_LIB_DIR}")
add_library(glew STATIC "${GLEW_SRC_DIR}/glew.c")

target_include_directories(glew PUBLIC "${GLEW_INCLUDE_DIR}")
target_compile_definitions(glew PUBLIC GLEW_STATIC GLEW_NO_GLU)
target_link_libraries(glew PUBLIC OpenGL::GL)

#======SFML+ImGui build=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SFML_IMGUI_LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${SFML_IMGUI_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${SFML_IMGUI_LIB_DIR}")
add_library(imgui-sfml STATIC "${SFML_IMGUI_INCLUDE_DIR}/imgui-SFML.cpp")

target_include_directories(imgui-sfml PUBLIC "${SFML_IMGUI_INCLUDE_DIR}")
target_link_libraries(imgui-sfml PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    imgui
    glew
)

#======TestGui build=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IMGUI_TEST_LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IMGUI_TEST_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${IMGUI_TEST_LIB_DIR}")

add_library(imgui-test STATIC
    "${IMGUI_TEST_SOURCE_DIR}/wrapper_base.cpp"
    "${IMGUI_TEST_SOURCE_DIR}/wrapper_info.cpp"
)

target_include_directories(imgui-test PUBLIC
    "${IMGUI_TEST_INCLUDE_DIR}"
    "${EXTERNAL_INCLUDE_DIR}"
    "${WIDGET_INCLUDE_DIR}"
)

target_link_libraries(imgui-test PUBLIC
    imgui-sfml
    widgets
    button
    checkbox
    entry
    grid_canvas
    image_frame
    label
)

#======Compiling tests=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/executables/tests")
file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")

foreach(SOURCE ${SOURCES})
    get_filename_component(FILENAME "${SOURCE}" NAME)
    get_filename_component(BASENAME "${SOURCE}" NAME_WE)
    add_executable(${BASENAME} ${FILENAME})
    target_link_libraries(${BASENAME} PRIVATE
        imgui-test
    )
endforeach()