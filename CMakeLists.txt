cmake_minimum_required(VERSION 3.1...3.26)

project(flow
  VERSION 1.0
  LANGUAGES CXX)

# Use -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

# Nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CTest)

find_package(Doxygen)
if(Doxygen_FOUND)
  add_subdirectory(doc)
else()
  message(STATUS "Doxygen not found, not building docs")
endif()

  
add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(test)
