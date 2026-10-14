#======Compiling tests=======================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/tests")
file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(source ${SOURCES})
    get_filename_component(test_name "${source}" NAME_WE)
    add_executable(${test_name} "${source}")
    
    target_link_libraries(${test_name} PRIVATE
        button
        checkbox
        entry
        grid_canvas
        image_frame
        label
    )
    if(TARGET widgets-dbgGui)
        target_link_libraries(${test_name} PRIVATE widgets-dbgGui)
    endif()

endforeach()