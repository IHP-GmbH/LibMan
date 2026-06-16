# Fetch and build CORE (CommonDB) from GitHub as a CMake subproject.

include(FetchContent)

set(CORE_GIT_URL "https://github.com/IHP-GmbH/CommonDB.git"
    CACHE STRING "CORE (CommonDB) Git repository URL")
set(CORE_GIT_TAG "main"
    CACHE STRING "CORE Git branch, tag, or commit hash")
set(LIBMAN_CORE_SOURCE_DIR ""
    CACHE PATH "Local CORE/CommonDB checkout (skips git fetch; for development)")

option(LIBMAN_FETCH_CORE "Download and build CORE from GitHub" ON)

include("${CMAKE_CURRENT_LIST_DIR}/EnsureCapnp.cmake")

function(_libman_configure_core_subproject)
    libman_ensure_capnp()

    set(CAPNP_ROOT "${CMAKE_SOURCE_DIR}/capnp-install" CACHE PATH "Cap'n Proto install prefix" FORCE)
    set(CORE_BOOTSTRAP_CAPNP OFF CACHE BOOL "CORE uses LibMan Cap'n Proto prefix" FORCE)
    set(CORE_BUILD_TESTS OFF CACHE BOOL "Do not build CORE tests inside LibMan" FORCE)
    set(CORE_BUILD_OAS_TESTS OFF CACHE BOOL "Do not build CORE OAS tests inside LibMan" FORCE)
    set(CORE_BUILD_EXAMPLES OFF CACHE BOOL "Do not build CORE example tools inside LibMan" FORCE)
endfunction()

function(_libman_add_core_aliases)
    if(TARGET core AND NOT TARGET CORE::core)
        add_library(CORE::core ALIAS core)
    endif()
    if(TARGET core_utils AND NOT TARGET CORE::core_utils)
        add_library(CORE::core_utils ALIAS core_utils)
    endif()
endfunction()

if(LIBMAN_CORE_SOURCE_DIR)
  if(NOT IS_DIRECTORY "${LIBMAN_CORE_SOURCE_DIR}")
    message(FATAL_ERROR "LIBMAN_CORE_SOURCE_DIR is not a directory: ${LIBMAN_CORE_SOURCE_DIR}")
  endif()
  message(STATUS "Using local CORE tree: ${LIBMAN_CORE_SOURCE_DIR}")
  _libman_configure_core_subproject()
  add_subdirectory("${LIBMAN_CORE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/commondb-build")
  _libman_add_core_aliases()
elseif(LIBMAN_FETCH_CORE)
  _libman_configure_core_subproject()

  set(_CORE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/.deps/CommonDB")

  FetchContent_Declare(
      commondb
      GIT_REPOSITORY "${CORE_GIT_URL}"
      GIT_TAG "${CORE_GIT_TAG}"
      GIT_SHALLOW TRUE
      SOURCE_DIR "${_CORE_SOURCE_DIR}"
      BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/commondb-build"
  )

  message(STATUS "Fetching CORE from ${CORE_GIT_URL} (${CORE_GIT_TAG})...")
  FetchContent_MakeAvailable(commondb)
  _libman_add_core_aliases()
else()
  find_package(CORE REQUIRED)
endif()

if(NOT TARGET CORE::core)
    message(FATAL_ERROR "CORE target CORE::core is missing after configuration.")
endif()

message(STATUS "CORE linked (CORE::core, CORE::core_utils)")
