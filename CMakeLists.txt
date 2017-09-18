cmake_minimum_required(VERSION 3.1)

file(READ "libcudf/cudf/version.hh" cudfh)
string(REGEX MATCH "#define CUDF_VERSION \"([^\"]*)\"" cudfv ${cudfh})

project(aspcud VERSION "${CMAKE_MATCH_1}" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(lemon)
include(functions)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ASPCUD_BUILD_STATIC "link aspcud statically" OFF)
option(ASPCUD_BUILD_TESTS "build tests" ON)

set(ASPCUD_ENCODING_PATH     "" CACHE STRING "optional to overwrite default path to the encoding")
set(ASPCUD_CUDF2LP_PATH      "" CACHE STRING "optional to overwrite default path to cudf2lp")
set(ASPCUD_GRINGO_PATH       "" CACHE STRING "optional to overwrite default path to gringo")
set(ASPCUD_CLASP_PATH        "" CACHE STRING "optional to overwrite default path to clasp")

mark_as_advanced(ASPCUD_BUILD_STATIC)
mark_as_advanced(ASPCUD_ENCODING_PATH)
mark_as_advanced(ASPCUD_CUDF2LP_PATH)
mark_as_advanced(ASPCUD_GRINGO_PATH)
mark_as_advanced(ASPCUD_CLASP_PATH)

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if (NOT CMAKE_IMPORT_LIBRARY_PREFIX)
    set(CMAKE_IMPORT_LIBRARY_PREFIX import_)
endif()

if(ASPCUD_BUILD_STATIC)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
    set(Boost_USE_STATIC_LIBS TRUE)
    set(Boost_USE_MULTITHREADED OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
endif()

find_package(RE2C)
find_package(Boost REQUIRED)

if(NOT TARGET Boost::boost)
  add_library(Boost::boost INTERFACE IMPORTED)
  if(Boost_INCLUDE_DIRS)
    set_target_properties(Boost::boost PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
  endif()
endif()

enable_testing()

add_subdirectory(libcudf)
add_subdirectory(app)
