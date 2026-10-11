enable_language(C)

#======SFML+ImGui=======================================
find_package(ImGui-SFML CONFIG QUIET)

if(NOT ImGui-SFML_FOUND)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.1
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(imgui)

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
    GIT_TAG        cc6b593c029110af8045826ce691f540c85e850c
)
FetchContent_MakeAvailable(tinyfiledialogs)

add_library(widgets-tinyfd STATIC
    "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
)
target_include_directories(widgets-tinyfd PUBLIC
    "${tinyfiledialogs_SOURCE_DIR}"
)

#======TestGui=======================================
add_library(widgets-dbgGui STATIC
    "${PROJECT_SOURCE_DIR}/Test-Gui/src/wrapper_base.cpp"
    "${PROJECT_SOURCE_DIR}/Test-Gui/src/wrapper_info.cpp"
)

target_include_directories(widgets-dbgGui
    PUBLIC "${PROJECT_SOURCE_DIR}/Test-Gui/include"
)

target_link_libraries(widgets-dbgGui PUBLIC
    ImGui-SFML::ImGui-SFML
    button
    checkbox
    entry
    grid_canvas
    image_frame
    label
    widgets-tinyfd
)