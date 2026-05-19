#-------------------------------------------------
#
# Project created by QtCreator 2023-05-03T09:50:11
#
#-------------------------------------------------

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = libman
TEMPLATE = app

CONFIG += c++17
DEFINES += QT_NO_DEPRECATED_WARNINGS

LIBS += -lz

win32:QMAKE_CXXFLAGS += -Wa,-mbig-obj
QMAKE_CXXFLAGS += -g1

SOURCES += \
    lstream/lstreamcellwriter.cpp \
    oas/oasCreate.cpp \
    src/main.cpp \
    gds/gdsReadAsync.cpp \
    src/libfileparser.cpp \
    oas/oasReadAsync.cpp \
    oas/oasReader.cpp \
    src/klayoutServer.cpp \
    src/lstreamcellreader.cpp \
    src/mainwindow.cpp \
    extension/variantmanager.cpp \
    extension/variantfactory.cpp \
    extension/qlineeditd2.cpp \
    extension/filepathmanager.cpp \
    extension/fileeditfactory.cpp \
    extension/fileedit.cpp \
    QtPropertyBrowser/qtvariantproperty.cpp \
    QtPropertyBrowser/qttreepropertybrowser.cpp \
    QtPropertyBrowser/qtpropertymanager.cpp \
    QtPropertyBrowser/qtpropertybrowserutils.cpp \
    QtPropertyBrowser/qtpropertybrowser.cpp \
    QtPropertyBrowser/qtgroupboxpropertybrowser.cpp \
    QtPropertyBrowser/qteditorfactory.cpp \
    QtPropertyBrowser/qtbuttonpropertybrowser.cpp \
    gds/gdsreader.cpp \
    src/projectmanager.cpp \
    lstream/lstrReadAsync.cpp \
    src/property.cpp \
    src/toolcustomtool.cpp \
    src/toolmanager.cpp \
    src/projectfile.cpp \
    src/categories.cpp \
    src/viewcontextmenu.cpp \
    src/groupcontextmenu.cpp \
    src/projectcontextmenu.cpp \
    src/categorycontextmenu.cpp \
    src/about.cpp \
    src/newview.cpp

HEADERS += \
    lstream/lstreamcellwriter.h \
    src/mainwindow.h \
    extension/variantmanager.h \
    extension/variantfactory.h \
    extension/qlineeditd2.h \
    extension/filepathmanager.h \
    extension/fileeditfactory.h \
    extension/fileedit.h \
    QtPropertyBrowser/qtvariantproperty.h \
    QtPropertyBrowser/qttreepropertybrowser.h \
    QtPropertyBrowser/qtpropertymanager.h \
    QtPropertyBrowser/qtpropertybrowserutils_p.h \
    QtPropertyBrowser/qtpropertybrowser.h \
    QtPropertyBrowser/qtgroupboxpropertybrowser.h \
    QtPropertyBrowser/qteditorfactory.h \
    QtPropertyBrowser/qtbuttonpropertybrowser.h \
    gds/gdsreader.h \
    src/libfileparser.h \
    oas/oasReader.h \
    src/lstreamcellreader.h \
    src/projectmanager.h \
    src/property.h \
    src/toolmanager.h \
    src/about.h \
    src/newview.h

FORMS += \
    src/mainwindow.ui \
    src/projectmanager.ui \
    src/toolmanager.ui \
    src/about.ui \
    src/newview.ui

RESOURCES += icons.qrc

# -----------------------------
# Cap'n Proto paths
# -----------------------------
CAPNP_ROOT = $$PWD/capnp-install
CAPNP_REPO_DIR = $$PWD/capnproto
CAPNP_GEN_DIR = $$PWD/capnp

INCLUDEPATH += $$CAPNP_GEN_DIR
INCLUDEPATH += $$CAPNP_ROOT/include
LIBS += -L$$CAPNP_ROOT/lib
LIBS += -lcapnp -lkj

# -----------------------------
# Upstream repos
# -----------------------------
CAPNP_GIT_URL = https://github.com/capnproto/capnproto.git
CAPNP_VERSION_MODE = branch
CAPNP_GIT_BRANCH = master
CAPNP_GIT_TAG =
CAPNP_GIT_COMMIT =

LSTREAM_GIT_URL = https://codeberg.org/klayoutmatthias/lstream.git
LSTREAM_VERSION_MODE = branch
LSTREAM_GIT_BRANCH = main
LSTREAM_GIT_TAG =
LSTREAM_GIT_COMMIT =
LSTREAM_SCHEMA_REPO_DIR = $$PWD/.deps/lstream

# -----------------------------
# Generated Cap'n Proto files
# Must be listed explicitly so qmake knows them.
# -----------------------------
SOURCES += \
    capnp/cell.capnp.cc \
    capnp/geometry.capnp.cc \
    capnp/header.capnp.cc \
    capnp/layoutView.capnp.cc \
    capnp/library.capnp.cc \
    capnp/metaData.capnp.cc \
    capnp/metaDataView.capnp.cc \
    capnp/propertySet.capnp.cc \
    capnp/repetition.capnp.cc \
    capnp/variant.capnp.cc

HEADERS += \
    capnp/cell.capnp.h \
    capnp/geometry.capnp.h \
    capnp/header.capnp.h \
    capnp/layoutView.capnp.h \
    capnp/library.capnp.h \
    capnp/metaData.capnp.h \
    capnp/metaDataView.capnp.h \
    capnp/propertySet.capnp.h \
    capnp/repetition.capnp.h \
    capnp/variant.capnp.h

# -----------------------------
# Prepare external dependencies before target
# -----------------------------
win32 {
    CAPNP_STAMP = $$CAPNP_ROOT/.built
    LSTREAM_STAMP = $$CAPNP_GEN_DIR/.schemas_built

    CAPNP_BUILD_CMD = cmd /c \"$$shell_path($$PWD/scripts/build_capnp_windows.bat)\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_URL\"
    CAPNP_BUILD_CMD += \"$$CAPNP_VERSION_MODE\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_BRANCH\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_TAG\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_COMMIT\"
    CAPNP_BUILD_CMD += \"$$shell_path($$CAPNP_REPO_DIR)\"
    CAPNP_BUILD_CMD += \"$$shell_path($$CAPNP_ROOT)\"

    capnpbuild.target = $$CAPNP_STAMP
    capnpbuild.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnpbuild

    LSTREAM_BUILD_CMD = cmd /c \"$$shell_path($$PWD/scripts/update_lstream_schemas_windows.bat)\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_URL\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_VERSION_MODE\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_BRANCH\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_TAG\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_COMMIT\"
    LSTREAM_BUILD_CMD += \"$$shell_path($$LSTREAM_SCHEMA_REPO_DIR)\"
    LSTREAM_BUILD_CMD += \"$$shell_path($$CAPNP_GEN_DIR)\"
    LSTREAM_BUILD_CMD += \"$$shell_path($$CAPNP_ROOT)\"

    lstreamschemas.target = $$LSTREAM_STAMP
    lstreamschemas.commands = $$LSTREAM_BUILD_CMD
    lstreamschemas.depends = $$CAPNP_STAMP
    QMAKE_EXTRA_TARGETS += lstreamschemas

    PRE_TARGETDEPS += $$LSTREAM_STAMP
} else {
    CAPNP_STAMP = $$CAPNP_ROOT/.built
    LSTREAM_STAMP = $$CAPNP_GEN_DIR/.schemas_built

    CAPNP_BUILD_CMD = bash $$shell_path($$PWD/scripts/build_capnp_linux.sh)
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_URL\"
    CAPNP_BUILD_CMD += \"$$CAPNP_VERSION_MODE\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_BRANCH\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_TAG\"
    CAPNP_BUILD_CMD += \"$$CAPNP_GIT_COMMIT\"
    CAPNP_BUILD_CMD += \"$$CAPNP_REPO_DIR\"
    CAPNP_BUILD_CMD += \"$$CAPNP_ROOT\"

    capnpbuild.target = $$CAPNP_STAMP
    capnpbuild.commands = $$CAPNP_BUILD_CMD
    QMAKE_EXTRA_TARGETS += capnpbuild

    LSTREAM_BUILD_CMD = bash $$shell_path($$PWD/scripts/update_lstream_schemas_linux.sh)
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_URL\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_VERSION_MODE\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_BRANCH\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_TAG\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_GIT_COMMIT\"
    LSTREAM_BUILD_CMD += \"$$LSTREAM_SCHEMA_REPO_DIR\"
    LSTREAM_BUILD_CMD += \"$$CAPNP_GEN_DIR\"
    LSTREAM_BUILD_CMD += \"$$CAPNP_ROOT\"

    lstreamschemas.target = $$LSTREAM_STAMP
    lstreamschemas.commands = $$LSTREAM_BUILD_CMD
    lstreamschemas.depends = $$CAPNP_STAMP
    QMAKE_EXTRA_TARGETS += lstreamschemas

    PRE_TARGETDEPS += $$LSTREAM_STAMP
}
