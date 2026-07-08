# Probe whether CommonDB (CORE) is reachable: local tree or GitHub access.
# Sets ${OUT_VAR} to ON/OFF. Optional FORCE_CORE / FORCE_NO_CORE override.

function(libman_probe_core_access OUT_VAR)
    set(options)
    set(oneValueArgs FORCE_CORE FORCE_NO_CORE)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_FORCE_NO_CORE)
        set(${OUT_VAR} OFF PARENT_SCOPE)
        return()
    endif()
    if(ARG_FORCE_CORE)
        set(${OUT_VAR} ON PARENT_SCOPE)
        return()
    endif()

    if(LIBMAN_CORE_SOURCE_DIR AND EXISTS "${LIBMAN_CORE_SOURCE_DIR}/src/core_paths.h")
        set(${OUT_VAR} ON PARENT_SCOPE)
        return()
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/.deps/CommonDB/src/core_paths.h")
        set(${OUT_VAR} ON PARENT_SCOPE)
        return()
    endif()

    if(WIN32)
        set(_probe_script "${CMAKE_SOURCE_DIR}/scripts/probe_core_access.cmd")
        execute_process(
            COMMAND cmd /c "${_probe_script}" "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _probe_result
            OUTPUT_QUIET ERROR_QUIET
        )
    else()
        set(_probe_script "${CMAKE_SOURCE_DIR}/scripts/probe_core_access.sh")
        execute_process(
            COMMAND bash "${_probe_script}" "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _probe_result
            OUTPUT_QUIET ERROR_QUIET
        )
    endif()

    if(_probe_result EQUAL 0)
        set(${OUT_VAR} ON PARENT_SCOPE)
    else()
        set(${OUT_VAR} OFF PARENT_SCOPE)
    endif()
endfunction()
