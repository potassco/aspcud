include_directories(${CMAKE_SOURCE_DIR}/libcudf ${CMAKE_SOURCE_DIR}/libprogram_opts .)

set(cudf-libs cudf-lib program_opts-lib ${Boost_LIBRARIES})

add_executable(cudf-app main.cpp)
target_link_libraries(cudf-app ${cudf-libs})
set_target_properties(cudf-app PROPERTIES OUTPUT_NAME cudf2lp)

add_executable(aspcud-app aspcud.c)
set_target_properties(aspcud-app PROPERTIES OUTPUT_NAME aspcud)
