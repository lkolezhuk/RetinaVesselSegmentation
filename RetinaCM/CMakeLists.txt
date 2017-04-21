# Top-Level CmakeLists.txt to build AIA projects
project( aia )
cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0054 OLD) 
cmake_policy(SET CMP0045 OLD)

# hide unused predefined variables
set (CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")

# include cmake scripts
include(cmake/utils.cmake)

# set up library and binary directories
if(NOT LIBRARY_OUTPUT_PATH)
  set (LIBRARY_OUTPUT_PATH ${aia_BINARY_DIR}/libs CACHE INTERNAL "Single output directory for building all libraries.")
endif(NOT LIBRARY_OUTPUT_PATH)
if(NOT EXECUTABLE_OUTPUT_PATH)
  set (EXECUTABLE_OUTPUT_PATH ${aia_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)

# set up OpenCV library
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
  message(FATAL_ERROR "OpenCV library not found or not properly installed")
endif()
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})

# set up "Release" and "Debug" as default build mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING 
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." 
        FORCE) # Default to Release
endif()

# disable annoying warnings on MSVC compilers
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	warnings_disable(CMAKE_CXX_FLAGS /wd4290) # vs2012: can't properly handle exceptions
	warnings_disable(CMAKE_CXX_FLAGS /wd4996) # vs2012: complains about unsafe standard C++ functions
	warnings_disable(CMAKE_CXX_FLAGS /wd4530) # vs2012: C++ exception handler used, but unwind semantics are not enabled
	warnings_disable(CMAKE_CXX_FLAGS /wd4503) # vs2012: decorated name length exceeded
endif()

# define folder where example images are stored
add_definitions(-DEXAMPLE_IMAGES_PATH="${aia_SOURCE_DIR}/example_images")

# build modules
add_subdirectory( utils )
add_subdirectory( project0 )

