
add_executable(EXAMPLE_create "create.cpp")
target_link_libraries(EXAMPLE_create csnap)

set_target_properties(EXAMPLE_create PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
