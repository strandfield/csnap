cmake_minimum_required(VERSION 3.13)
project(csnap VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # disables warning about DLL export 
  # this is annoying when using C++ STL
  add_compile_options(/wd4251)  
endif()

set(CSNAP_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})

#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(WIN32)
  list(APPEND CMAKE_PREFIX_PATH "C:/usr")
  list(APPEND CMAKE_LIBRARY_PATH "C:/usr/bin")
endif()

##################################################################
###### coverage build
##################################################################

if(ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

##################################################################
###### csnap modules
##################################################################

add_subdirectory(model)
add_subdirectory(db)
add_subdirectory(indexer)
add_subdirectory(exporter)

##################################################################
###### app
##################################################################

add_subdirectory(src)

##################################################################
###### tests & examples
##################################################################

#add_subdirectory(examples)
#add_subdirectory(tests)

##################################################################
###### virtual env
##################################################################

file(WRITE "${CMAKE_BINARY_DIR}/activate.bat" "set PATH=%PATH%")
