
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/**.h)

add_library(csnap-indexer STATIC ${SRC_FILES} ${HDR_FILES})
target_include_directories(csnap-indexer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(csnap-indexer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/csnap/indexer")
target_link_libraries(csnap-indexer csnap-model csnap-database)

set_target_properties(csnap-indexer PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(_source IN ITEMS ${SRC_FILES} ${HDR_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
