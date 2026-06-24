#-------------------------------------------------
#
# Project created by QtCreator 2023-05-03T09:50:11
#
#-------------------------------------------------

QT += core gui concurrent
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
    src/klayoutCellResolver.cpp \
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
    src/projecteditor.cpp \
    src/importdialog.cpp \
    src/core_import_service.cpp \
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
    src/newview.cpp \
    core/core_path_utils.cpp \
    core/converter_paths.cpp

contains(CONFIG, no_core) {
    DEFINES += LIBMAN_NO_CORE
    SOURCES += core/libman_no_core_stubs.cpp
} else {
    SOURCES += \
        core/corecellreader.cpp \
        core/coreReadAsync.cpp \
        core/coreKlayoutBridge.cpp
}

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
    src/projecteditor.h \
    src/importdialog.h \
    src/core_import_service.h \
    src/property.h \
    src/toolmanager.h \
    src/about.h \
    src/newview.h \
    core/corecellreader.h \
    core/coreKlayoutBridge.h \
    core/core_path_utils.h \
    core/converter_paths.h

FORMS += \
    src/mainwindow.ui \
    src/projectmanager.ui \
    src/projecteditor.ui \
    src/importdialog.ui \
    src/toolmanager.ui \
    src/about.ui \
    src/newview.ui

RESOURCES += icons.qrc

# -----------------------------
# Cap'n Proto (clone + build on first make via capnp_deps.pri)
# -----------------------------
# Repo root (directory of this .pro file), not the shadow-build cwd.
LIBMAN_ROOT = $$dirname(_PRO_FILE_)
include(capnp_deps.pri)
!contains(CONFIG, no_core) {
    include(core_deps.pri)
}

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

include(capnp_deps_finalize.pri)
!contains(CONFIG, no_core) {
    include(core_deps_finalize.pri)
    include(core_converter_deploy.pri)
}
