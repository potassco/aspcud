# [[[source: .
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/catch.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/criteria.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/critparser.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/helpers.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/other.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

add_executable(test_libcudf ${header} ${source})
target_link_libraries(test_libcudf PRIVATE libcudf Boost::boost)

add_test(NAME test_libcudf COMMAND test_libcudf)
