set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/examples")
file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/examples/*.cpp")

#======libtinyfiledialogs setup=======================================
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://github.com/native-toolkit/libtinyfiledialogs.git
    GIT_TAG master
)
FetchContent_MakeAvailable(tinyfiledialogs)

foreach(SOURCE ${SOURCES})
    get_filename_component(FILENAME "${SOURCE}" NAME)
    get_filename_component(BASENAME "${SOURCE}" NAME_WE)
    add_executable(${BASENAME} ${FILENAME})

    # include file dialog's source file if the source is ImageViewer.cpp
    if(FILENAME STREQUAL "ImageViewer.cpp")
        enable_language(C)
        target_sources(${BASENAME} PRIVATE
            "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
        )
        target_include_directories(${BASENAME} PRIVATE
        "${tinyfiledialogs_SOURCE_DIR}"
        )
    endif()

    target_link_libraries(${BASENAME} PRIVATE
        button
        checkbox
        entry
        image_frame
        label
    )
endforeach()