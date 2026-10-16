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

add_library(sfmlwidgets_tinyfd STATIC
    "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
)
target_include_directories(sfmlwidgets_tinyfd PUBLIC
    "${tinyfiledialogs_SOURCE_DIR}"
)

#======Editor=======================================
add_library(sfmlwidgets_editor STATIC
    "${PROJECT_SOURCE_DIR}/Editor/src/imgui_panel.cpp"
    "${PROJECT_SOURCE_DIR}/Editor/src/editor.cpp"
)
if(BUILD_EDITOR_GUI)
    target_compile_definitions(sfmlwidgets_editor PUBLIC BUILD_EDITOR_GUI)
endif()
target_include_directories(sfmlwidgets_editor
    PUBLIC "${PROJECT_SOURCE_DIR}/Editor/include"
)
add_library(sfml-widgets::editor ALIAS sfmlwidgets_editor)

target_link_libraries(sfmlwidgets_editor
    PRIVATE ImGui-SFML::ImGui-SFML sfmlwidgets_tinyfd
    PUBLIC
    sfmlwidgets_button
    sfmlwidgets_checkbox
    sfmlwidgets_entry
    sfmlwidgets_grid_canvas
    sfmlwidgets_image_frame
    sfmlwidgets_label
)