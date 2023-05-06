
find_package(SQLite3)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/**.h)

add_library(csnap-database STATIC ${SRC_FILES} ${HDR_FILES})
target_include_directories(csnap-database PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(csnap-database PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/csnap/database")
target_link_libraries(csnap-database csnap-model)
target_link_libraries(csnap-database SQLite::SQLite3)

set_target_properties(csnap-database PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(_source IN ITEMS ${SRC_FILES} ${HDR_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


