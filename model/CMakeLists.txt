
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/**.h)

add_library(csnap-model STATIC ${SRC_FILES} ${HDR_FILES})
target_include_directories(csnap-model PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(csnap-model PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/csnap/model")

set_target_properties(csnap-model PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message("GCC compiler: linking to stdc++fs")
    target_link_libraries(csnap-model stdc++fs)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

  if (CLANG_VERSION_STRING VERSION_GREATER_EQUAL 9.0)
    message("Clang version greater than 9.0, no need to link to c++fs")
  else()
    message("Clang compiler: linking to c++fs")
    target_link_libraries(csnap-model c++fs)
  endif()
    
  message("See https://releases.llvm.org/10.0.0/projects/libcxx/docs/UsingLibcxx.html#using-filesystem for information about using #include<filesystem> with clang")

endif()

foreach(_source IN ITEMS ${SRC_FILES} ${HDR_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()



