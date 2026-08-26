set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/executables/examples")

# image viewer requires file dialog library
set(TINY_DIALOG_SOURCE_DIR "${PROJECT_SOURCE_DIR}/examples/libtinyfiledialogs")

file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/examples/*.cpp")

foreach(SOURCE ${SOURCES})
    get_filename_component(FILENAME "${SOURCE}" NAME)
    get_filename_component(BASENAME "${SOURCE}" NAME_WE)
    add_executable(${BASENAME} ${FILENAME})

    # include file dialog's source file if the source is ImageViewer.cpp
    if(FILENAME STREQUAL "ImageViewer.cpp")
        enable_language(C)
        target_sources(${BASENAME} PRIVATE "${TINY_DIALOG_SOURCE_DIR}/tinyfiledialogs.c")
        target_include_directories(${BASENAME} PRIVATE "${TINY_DIALOG_SOURCE_DIR}")
    endif()

    target_link_libraries(${BASENAME} PRIVATE
        button
        checkbox
        entry
        image_frame
        label
    )
endforeach()