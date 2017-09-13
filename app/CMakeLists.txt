source_group("Source Files" FILES aspcud.c cudf2lp.cc)

add_executable(aspcud aspcud.c)
target_include_directories(aspcud PRIVATE ${CMAKE_SOURCE_DIR}/libcudf)

add_executable(cudf2lp cudf2lp.cc)
target_link_libraries(cudf2lp PRIVATE libcudf Boost::boost Boost::program_options)

install(
    FILES "${CMAKE_SOURCE_DIR}/encodings/misc2012.lp" "${CMAKE_SOURCE_DIR}/encodings/specification.lp"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
install(
    FILES "${CMAKE_SOURCE_DIR}/doc/man/aspcud.1" "${CMAKE_SOURCE_DIR}/doc/man/cudf2lp.1"
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

if (NOT ASPCUD_CUDF2LP_PATH)
    set(ASPCUD_CUDF2LP_PATH "${CMAKE_INSTALL_FULL_BINDIR}/cudf2lp")
endif()
set_property(SOURCE aspcud.c APPEND PROPERTY COMPILE_DEFINITIONS "ASPCUD_CUDF2LP_BIN=\"${ASPCUD_CUDF2LP_PATH}\"")
if (NOT ASPCUD_GRINGO_PATH)
    set(ASPCUD_GRINGO_PATH "${CMAKE_INSTALL_FULL_BINDIR}/gringo")
endif()
set_property(SOURCE aspcud.c APPEND PROPERTY COMPILE_DEFINITIONS "ASPCUD_GRINGO_BIN=\"${ASPCUD_GRINGO_PATH}\"")
if (NOT ASPCUD_CLASP_PATH)
    set(ASPCUD_CLASP_PATH "${CMAKE_INSTALL_FULL_BINDIR}/clasp")
endif()
set_property(SOURCE aspcud.c APPEND PROPERTY COMPILE_DEFINITIONS "ASPCUD_CLASP_BIN=\"${ASPCUD_CLASP_PATH}\"")
if (NOT ASPCUD_ENCODING_PATH)
    set(ASPCUD_ENCODING_PATH "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/misc2012.lp")
endif()
set_property(SOURCE aspcud.c APPEND PROPERTY COMPILE_DEFINITIONS "ASPCUD_DEFAULT_ENCODING=\"${ASPCUD_ENCODING_PATH}\"")

install(TARGETS cudf2lp aspcud RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
