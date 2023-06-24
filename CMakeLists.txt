cmake_minimum_required(VERSION 3.1...3.26)

# Important for import to Eclipse:
# Project name should be different to executable name and different to build folder name.
# Otherwise, Eclipse will NOT pick up your executable as you build them.

project(flow
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

# Use -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

# Nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CTest)

enable_testing()

find_package(Doxygen
            REQUIRED dot 
            OPTIONAL_COMPONENTS mscgen dia)
if(Doxygen_FOUND)
  add_subdirectory(doc)
else()
  message(STATUS "Doxygen not found, not building docs")
endif()

  
add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(test)
