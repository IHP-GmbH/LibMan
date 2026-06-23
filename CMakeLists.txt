cmake_minimum_required(VERSION 3.16)

project(LibMan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/src
)
set(CMAKE_AUTORCC ON)

add_compile_definitions(QT_NO_DEPRECATED_WARNINGS)

if(WIN32)
    add_compile_options(-Wa,-mbig-obj)
endif()

add_compile_options(-g1)

if(NOT Qt5_DIR)
    set(Qt5_DIR "C:/Qt/5.15.2/mingw81_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent)

if(WIN32)
    set(ZLIB_INCLUDE_DIR "C:/Qt/Tools/mingw810_64/x86_64-w64-mingw32/include")
    set(ZLIB_LIBRARY "C:/Qt/Tools/mingw810_64/x86_64-w64-mingw32/lib/libz.a")
endif()

find_package(ZLIB REQUIRED)

# -----------------------------------------------------------------------------
# Cap'n Proto paths
# -----------------------------------------------------------------------------

set(CAPNP_ROOT "${CMAKE_SOURCE_DIR}/capnp-install")
set(CAPNP_REPO_DIR "${CMAKE_SOURCE_DIR}/capnproto")
set(CAPNP_GEN_DIR "${CMAKE_SOURCE_DIR}/capnp")

set(CAPNP_GIT_URL "https://github.com/capnproto/capnproto.git")
set(CAPNP_VERSION_MODE "branch")
set(CAPNP_GIT_BRANCH "master")
set(CAPNP_GIT_TAG "")
set(CAPNP_GIT_COMMIT "")

set(LSTREAM_GIT_URL "https://codeberg.org/klayoutmatthias/lstream.git")
set(LSTREAM_VERSION_MODE "branch")
set(LSTREAM_GIT_BRANCH "main")
set(LSTREAM_GIT_TAG "")
set(LSTREAM_GIT_COMMIT "")
set(LSTREAM_SCHEMA_REPO_DIR "${CMAKE_SOURCE_DIR}/.deps/lstream")

# -----------------------------------------------------------------------------
# External dependency preparation
# -----------------------------------------------------------------------------

set(CAPNP_STAMP "${CAPNP_ROOT}/capnp_install_stamp")
set(LSTREAM_STAMP "${CAPNP_GEN_DIR}/schemas_built_stamp")

if(WIN32)
    add_custom_command(
        OUTPUT "${CAPNP_STAMP}"
        COMMAND cmd /c
            "${CMAKE_SOURCE_DIR}/scripts/mkcapnp.cmd"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Building Cap'n Proto for Windows"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LSTREAM_STAMP}"
        COMMAND cmd /c
            "${CMAKE_SOURCE_DIR}/scripts/mklstream.cmd"
        DEPENDS "${CAPNP_STAMP}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Updating LStream schemas for Windows"
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT "${CAPNP_STAMP}"
        COMMAND bash
            "${CMAKE_SOURCE_DIR}/scripts/build_capnp_linux.sh"
            "${CAPNP_GIT_URL}"
            "${CAPNP_VERSION_MODE}"
            "${CAPNP_GIT_BRANCH}"
            "${CAPNP_GIT_TAG}"
            "${CAPNP_GIT_COMMIT}"
            "${CAPNP_REPO_DIR}"
            "${CAPNP_ROOT}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Building Cap'n Proto for Linux"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LSTREAM_STAMP}"
        COMMAND bash
            "${CMAKE_SOURCE_DIR}/scripts/update_lstream_schemas_linux.sh"
            "${LSTREAM_GIT_URL}"
            "${LSTREAM_VERSION_MODE}"
            "${LSTREAM_GIT_BRANCH}"
            "${LSTREAM_GIT_TAG}"
            "${LSTREAM_GIT_COMMIT}"
            "${LSTREAM_SCHEMA_REPO_DIR}"
            "${CAPNP_GEN_DIR}"
            "${CAPNP_ROOT}"
        DEPENDS "${CAPNP_STAMP}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Updating LStream schemas for Linux"
        VERBATIM
    )
endif()

add_custom_target(prepare_libman_deps
    DEPENDS "${LSTREAM_STAMP}"
)

# -----------------------------------------------------------------------------
# CORE (CommonDB) — fetched from GitHub on first configure
# -----------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/cmake/FetchCore.cmake")

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------

set(SOURCES
    core/core_path_utils.cpp
    core/corecellreader.cpp
    core/coreKlayoutBridge.cpp
    core/coreReadAsync.cpp
    lstream/lstreamcellwriter.cpp
    oas/oasCreate.cpp
    src/main.cpp
    gds/gdsReadAsync.cpp
    src/libfileparser.cpp
    oas/oasReadAsync.cpp
    oas/oasReader.cpp
    src/klayoutServer.cpp
    src/klayoutCellResolver.cpp
    src/lstreamcellreader.cpp
    src/mainwindow.cpp

    extension/variantmanager.cpp
    extension/variantfactory.cpp
    extension/qlineeditd2.cpp
    extension/filepathmanager.cpp
    extension/fileeditfactory.cpp
    extension/fileedit.cpp

    QtPropertyBrowser/qtvariantproperty.cpp
    QtPropertyBrowser/qttreepropertybrowser.cpp
    QtPropertyBrowser/qtpropertymanager.cpp
    QtPropertyBrowser/qtpropertybrowserutils.cpp
    QtPropertyBrowser/qtpropertybrowser.cpp
    QtPropertyBrowser/qtgroupboxpropertybrowser.cpp
    QtPropertyBrowser/qteditorfactory.cpp
    QtPropertyBrowser/qtbuttonpropertybrowser.cpp

    gds/gdsreader.cpp
    src/projectmanager.cpp
    src/projecteditor.cpp
    lstream/lstrReadAsync.cpp
    src/property.cpp
    src/toolcustomtool.cpp
    src/toolmanager.cpp
    src/projectfile.cpp
    src/categories.cpp
    src/viewcontextmenu.cpp
    src/groupcontextmenu.cpp
    src/projectcontextmenu.cpp
    src/categorycontextmenu.cpp
    src/about.cpp
    src/newview.cpp

    capnp/cell.capnp.cc
    capnp/geometry.capnp.cc
    capnp/header.capnp.cc
    capnp/layoutView.capnp.cc
    capnp/library.capnp.cc
    capnp/metaData.capnp.cc
    capnp/metaDataView.capnp.cc
    capnp/propertySet.capnp.cc
    capnp/repetition.capnp.cc
    capnp/variant.capnp.cc
)

set(HEADERS
    core/core_path_utils.h
    core/corecellreader.h
    core/coreKlayoutBridge.h
    lstream/lstreamcellwriter.h
    src/mainwindow.h

    extension/variantmanager.h
    extension/variantfactory.h
    extension/qlineeditd2.h
    extension/filepathmanager.h
    extension/fileeditfactory.h
    extension/fileedit.h

    QtPropertyBrowser/qtvariantproperty.h
    QtPropertyBrowser/qttreepropertybrowser.h
    QtPropertyBrowser/qtpropertymanager.h
    QtPropertyBrowser/qtpropertybrowserutils_p.h
    QtPropertyBrowser/qtpropertybrowser.h
    QtPropertyBrowser/qtgroupboxpropertybrowser.h
    QtPropertyBrowser/qteditorfactory.h
    QtPropertyBrowser/qtbuttonpropertybrowser.h

    gds/gdsreader.h
    src/libfileparser.h
    oas/oasReader.h
    src/lstreamcellreader.h
    src/projectmanager.h
    src/projecteditor.h
    src/property.h
    src/toolmanager.h
    src/about.h
    src/newview.h

    capnp/cell.capnp.h
    capnp/geometry.capnp.h
    capnp/header.capnp.h
    capnp/layoutView.capnp.h
    capnp/library.capnp.h
    capnp/metaData.capnp.h
    capnp/metaDataView.capnp.h
    capnp/propertySet.capnp.h
    capnp/repetition.capnp.h
    capnp/variant.capnp.h
)

set(FORMS
    src/mainwindow.ui
    src/projectmanager.ui
    src/projecteditor.ui
    src/toolmanager.ui
    src/about.ui
    src/newview.ui
)

set(RESOURCES
    icons.qrc
)

add_executable(libman
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

add_dependencies(libman prepare_libman_deps)
if(TARGET core)
    add_dependencies(libman core)
endif()

target_include_directories(libman PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/extension
    ${CMAKE_SOURCE_DIR}/QtPropertyBrowser
    ${CAPNP_GEN_DIR}
    ${CAPNP_ROOT}/include
)

target_link_directories(libman PRIVATE
    ${CAPNP_ROOT}/lib
)

target_link_libraries(libman PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Concurrent
    ZLIB::ZLIB
    capnp
    kj
    CORE::core
    CORE::core_utils
)

