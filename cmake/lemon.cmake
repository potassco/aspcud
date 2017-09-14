include(CMakeParseArguments)

macro(LEMON_TARGET)
    cmake_parse_arguments(PARSED_ARGS "" "NAME;INPUT;OUTPUT" "DEPENDS" ${ARGN})

    if(NOT PARSED_ARGS_OUTPUT)
        message(FATAL_ERROR "LEMON_TARGET expects an output filename")
    endif()
    if(NOT PARSED_ARGS_INPUT)
        message(FATAL_ERROR "LEMON_TARGET expects an input filename")
    endif()
    if(NOT PARSED_ARGS_NAME)
        message(FATAL_ERROR "LEMON_TARGET expects a target name")
    endif()
    if(UNIX)
        set(copy_or_link create_symlink)
    else()
        set(copy_or_link copy_if_different)
    endif()

    get_filename_component(input "${PARSED_ARGS_INPUT}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    get_filename_component(directory "${PARSED_ARGS_INPUT}" DIRECTORY)
    get_filename_component(filename "${PARSED_ARGS_INPUT}" NAME)
    get_filename_component(basename "${filename}" NAME_WE)
    set(bin_path "${PARSED_ARGS_OUTPUT}/${directory}")
    set(gen_path "${CMAKE_CURRENT_SOURCE_DIR}/gen/${directory}")

    file(MAKE_DIRECTORY "${bin_path}")

    if(NOT EXISTS ${gen_path}/${basename}.cc)
        if(NOT TARGET lemon)
            if(CMAKE_CROSSCOMPILING)
                # for some reason this has to be done directly in front of the custom command
                set(IMPORT_LEMON "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Path to the export file of the native lemon build")
                include(${IMPORT_LEMON})
            else()
                add_subdirectory("${CMAKE_SOURCE_DIR}/lemon" "${CMAKE_BINARY_DIR}/lemon")
            endif()
        endif()
        add_custom_command(
            OUTPUT "${bin_path}/lempar.c"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${bin_path}"
            COMMAND ${CMAKE_COMMAND}  -E ${copy_or_link} "${CMAKE_SOURCE_DIR}/lemon/lempar.c" "${bin_path}/lempar.c"
            MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/lemon/lempar.c"
        )
        add_custom_command(
            OUTPUT "${bin_path}/${basename}.cc" "${bin_path}/${basename}.h"
            COMMAND ${CMAKE_COMMAND}  -E ${copy_or_link} "${input}" "${basename}.y"
            COMMAND lemon -q "${basename}.y"
            COMMAND ${CMAKE_COMMAND}  -E ${copy_or_link} "${basename}.c" "${basename}.cc"
            MAIN_DEPENDENCY "${PARSED_ARGS_INPUT}"
            DEPENDS lemon "${bin_path}/lempar.c"
            WORKING_DIRECTORY "${bin_path}"
            COMMENT "[LEMON][${PARSED_ARGS_NAME}] Generating parser with lemon"
        )
    else()
        file(COPY "${gen_path}/${basename}.h" DESTINATION "${bin_path}")
        file(COPY "${gen_path}/${basename}.cc" DESTINATION "${bin_path}")
    endif()

    set(LEMON_${PARSED_ARGS_NAME}_OUTPUT "${bin_path}/${basename}.cc")
    set(LEMON_${PARSED_ARGS_NAME}_OUTPUTS "${LEMON_${PARSED_ARGS_NAME}_OUTPUT}" "${bin_path}/${basename}.h")
endmacro()
