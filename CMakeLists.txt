cmake_minimum_required(VERSION 2.6)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(SEND_ERROR  "You have to create an out-of-source build")
	message(SEND_ERROR  "Create a sub-folder and execute cmake there")
	message(FATAL_ERROR "The file CmakeCache.txt and directory CMakeFiles in the root directory have to be deleted manually now")
endif()

# user configurables
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type: Release Debug")

project(aspcud)

include(cmake/re2c.cmake)
include(cmake/lemon.cmake)
include(GNUInstallDirs)

IF(USE_STATIC_LIBS)
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
	set(CMAKE_SHARED_LIBRARY_C_FLAGS)
	set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
	set(Boost_USE_STATIC_LIBS TRUE)
	set(CMAKE_EXE_LINKER_FLAGS "-static")
	set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
ENDIF()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake CACHE INTERNAL "" FORCE)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)

IF(EXISTS "${CMAKE_SOURCE_DIR}/libboost/")
	include_directories("libboost")
ELSE()
	set(Boost_USE_MULTITHREADED OFF CACHE STRING "Whether to Use multi-threaded boost libraries")
	find_package(Boost REQUIRED)
	#find_package(Boost COMPONENTS filesystem system)
	include_directories(${Boost_INCLUDE_DIRS})
ENDIF()

add_subdirectory(libcudf)
add_subdirectory(libprogram_opts)
add_subdirectory(app)

install(FILES "${CMAKE_SOURCE_DIR}/scripts/encodings/misc2012.lp" "${CMAKE_SOURCE_DIR}/scripts/encodings/specification.lp" DESTINATION "${CMAKE_INSTALL_DATADIR}/aspcud")
install(FILES "${CMAKE_SOURCE_DIR}/man/man1/aspcud.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

