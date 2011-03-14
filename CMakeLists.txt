cmake_minimum_required(VERSION 2.6)

# user configurables
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type: Release Debug")

project(aspcud)

include(cmake/re2c.cmake)
include(cmake/lemon.cmake)
include(cmake/macros.cmake)

IF(USE_STATIC_LIBS)
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
	set(CMAKE_SHARED_LIBRARY_C_FLAGS)
	set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
	set(Boost_USE_STATIC_LIBS TRUE)
	set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -static)
	set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
ENDIF()

ENSURE_OUT_OF_SOURCE_BUILD("you have to create an out-of-source build")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake CACHE INTERNAL "" FORCE)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)

set(Boost_USE_MULTITHREADED OFF CACHE STRING "Whether to Use multi-threaded boost libraries")
find_package(Boost REQUIRED)
#find_package(Boost COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(libcudf)
add_subdirectory(libprogram_opts)
add_subdirectory(app)

