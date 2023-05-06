
include(cmake/embed.cmake)
create_resources("assets" "html_assets")

if(NOT TARGET cpptok)
    add_subdirectory(cpptok)
endif()

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/**.h)

add_library(csnap-exporter STATIC ${SRC_FILES} ${HDR_FILES} ${EMBED_RESOURCE_html_assets})
target_include_directories(csnap-exporter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(csnap-exporter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/csnap/exporter")
target_link_libraries(csnap-exporter csnap-model csnap-database)
target_link_libraries(csnap-exporter cpptok)

set_target_properties(csnap-exporter PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(_source IN ITEMS ${SRC_FILES} ${HDR_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
