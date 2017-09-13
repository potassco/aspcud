source_group("Source Files" FILES lemon.c lempar.c)

add_executable(lemon lemon.c)
export(TARGETS lemon FILE ${CMAKE_BINARY_DIR}/import_lemon.cmake)
