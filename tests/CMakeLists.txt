enable_language(C)

#======Dear ImGui=======================================
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.1
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(imgui)

#======SFML+ImGui=======================================
find_package(ImGui-SFML CONFIG QUIET)

if(NOT ImGui-SFML_FOUND)
    set(IMGUI_SFML_FIND_SFML OFF)
    set(IMGUI_DIR "${imgui_SOURCE_DIR}")

    FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG        v3.0
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(imgui-sfml)
endif()

#======libtinyfiledialogs=======================================
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://github.com/native-toolkit/libtinyfiledialogs.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(tinyfiledialogs)

#======TestGui=======================================
add_library(widgets-dbgGui STATIC
    "${PROJECT_SOURCE_DIR}/Test-Gui/src/wrapper_base.cpp"
    "${PROJECT_SOURCE_DIR}/Test-Gui/src/wrapper_info.cpp"
    "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
)

target_include_directories(widgets-dbgGui
    PRIVATE "${tinyfiledialogs_SOURCE_DIR}"
    PUBLIC "${PROJECT_SOURCE_DIR}/Test-Gui/include"
)

target_link_libraries(widgets-dbgGui PUBLIC
    ImGui-SFML::ImGui-SFML
    widgets
    button
    checkbox
    entry
    grid_canvas
    image_frame
    label
)

#======Compiling tests=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/tests")
file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(source ${SOURCES})
    get_filename_component(test_name "${source}" NAME_WE)
    add_executable(${test_name} "${source}")
    target_link_libraries(${test_name} PRIVATE widgets-dbgGui)
endforeach()