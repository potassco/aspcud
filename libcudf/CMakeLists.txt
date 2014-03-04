file(GLOB_RECURSE LIBCUDF_SRC    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp *.h)
file(GLOB_RECURSE LIBCUDF_LEXER  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.r2c)
file(GLOB_RECURSE LIBCUDF_PARSER RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.y)
file(GLOB_RECURSE TEST_SRC       RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp test/*.h)


include_directories(src .)

RE2C(LIBCUDF_SRC ${LIBCUDF_LEXER})
LEMON(LIBCUDF_SRC ${LIBCUDF_PARSER})

add_library(cudf-lib STATIC ${LIBCUDF_SRC})

target_link_libraries(cudf-lib)
set_target_properties(cudf-lib PROPERTIES OUTPUT_NAME cudf)

find_package(Boost COMPONENTS unit_test_framework)

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
    if("${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}" MATCHES ".*\\.((so)|(dll)|(dynlib))$")
            add_definitions("-DBOOST_TEST_DYN_LINK")
    endif()

    add_executable(test-app EXCLUDE_FROM_ALL ${TEST_SRC})
	target_link_libraries(test-app cudf-lib ${Boost_LIBRARIES})
    set_target_properties(test-app PROPERTIES OUTPUT_NAME test)
    
    add_custom_target(test COMMAND test-app DEPENDS test-app)
endif()

