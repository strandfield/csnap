
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/**.h)

add_executable(csnap ${SRC_FILES} ${HDR_FILES})
target_link_libraries(csnap csnap-indexer csnap-exporter)

set_target_properties(csnap PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(csnap PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (WIN32)
  set_target_properties(csnap PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${TINYXML2_INCLUDE}/../bin;%PATH%")
endif()
