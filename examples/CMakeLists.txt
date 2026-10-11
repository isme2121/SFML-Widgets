set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/examples")
file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/examples/*.cpp")

#======libtinyfiledialogs setup=======================================
if(NOT TARGET widgets-tinyfd)
    enable_language(C)

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
endif()

foreach(source ${SOURCES})
    get_filename_component(BASENAME "${source}" NAME_WE)
    add_executable(${BASENAME} "${source}")

    # include file dialog's source file if the source is ImageViewer.cpp
    if(FILENAME STREQUAL "ImageViewer.cpp")
        target_link_libraries(${BASENAME} PRIVATE widgets-tinyfd)
    endif()


    if(BUILD_DEBUG_GUI)
        target_link_libraries(${BASENAME} PRIVATE widgets-dbgGui)
    else()
        target_link_libraries(${BASENAME} PRIVATE
            button
            checkbox
            entry
            image_frame
            label
            grid_canvas
        )
    endif()

endforeach()

