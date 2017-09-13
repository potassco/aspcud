file(GLOB_RECURSE LIBCUDF_PARSER RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.y)

# [[[header: cudf
set(ide_header_group "Header Files")
set(header-group-cudf
    "${CMAKE_CURRENT_SOURCE_DIR}/cudf/critparser.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/cudf/dependency.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/cudf/lexer_impl.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/cudf/packages.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/cudf/parser.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/cudf/version.hh")
source_group("${ide_header_group}\\cudf" FILES ${header-group-cudf})
set(header
    ${header-group-cudf})
# ]]]
# [[[source: src
re2c_target_or_gen("/src/critlexer.xh")
lemon_target(NAME "critparser_impl" INPUT "src/critparser_impl.y" OUTPUT ${CMAKE_CURRENT_BINARY_DIR})
re2c_target_or_gen("/src/lexer.xh")
lemon_target(NAME "parser_impl" INPUT "src/parser_impl.y" OUTPUT ${CMAKE_CURRENT_BINARY_DIR})
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/critlexer.xh"
    ${RE2C_critlexer_OUTPUT}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/critparser.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dependency.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.xh"
    ${RE2C_lexer_OUTPUT}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/packages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source-group-critparser_impl
    "${CMAKE_CURRENT_SOURCE_DIR}/src/critparser_impl.y"
    ${LEMON_critparser_impl_OUTPUTS})
source_group("${ide_source_group}\\critparser_impl" FILES ${source-group-critparser_impl})
set(source-group-parser_impl
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parser_impl.y"
    ${LEMON_parser_impl_OUTPUTS})
source_group("${ide_source_group}\\parser_impl" FILES ${source-group-parser_impl})
set(source
    ${source-group}
    ${source-group-critparser_impl}
    ${source-group-parser_impl})
# ]]]

add_library(libcudf STATIC ${header} ${source})
target_include_directories(libcudf
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>")
target_link_libraries(libcudf PRIVATE Boost::boost)
set_target_properties(libcudf PROPERTIES OUTPUT_NAME cudf FOLDER lib)

if (ASPCUD_BUILD_TESTS)
    add_subdirectory(tests)
endif()
