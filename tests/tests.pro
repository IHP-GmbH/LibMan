# -------------------------------------------------
# LibMan GUI tests (QtTest)
# -------------------------------------------------

QT += core gui widgets testlib
LIBS += -lz

TEMPLATE = app
TARGET = tst_libman_gui
CONFIG += console c++17
DEFINES += QT_NO_DEPRECATED_WARNINGS

# Allow large object files (fixes "too many sections" error)
win32:QMAKE_CXXFLAGS += -Wa,-mbig-obj
QMAKE_CXXFLAGS += -g1

# We build tests from repo root sources
INCLUDEPATH += \
    $$PWD/.. \
    $$PWD/../src \
    $$PWD/../gds \
    $$PWD/../oas \
    $$PWD/../extension \
    $$PWD/../QtPropertyBrowser

DEPENDPATH += $$INCLUDEPATH

# ----------------------------
# IMPORTANT:
# Do NOT include src/main.cpp here (QTEST_MAIN provides main())!
# ----------------------------
SOURCES += \
    $$PWD/tst_libman_gui_gds.cpp \
    $$PWD/../gds/gdsReadAsync.cpp \
    $$PWD/../oas/oasReadAsync.cpp \
    $$PWD/../oas/oasReader.cpp \
    $$PWD/../src/klayoutServer.cpp \
    $$PWD/../src/mainwindow.cpp \
    $$PWD/../extension/variantmanager.cpp \
    $$PWD/../extension/variantfactory.cpp \
    $$PWD/../extension/qlineeditd2.cpp \
    $$PWD/../extension/filepathmanager.cpp \
    $$PWD/../extension/fileeditfactory.cpp \
    $$PWD/../extension/fileedit.cpp \
    $$PWD/../QtPropertyBrowser/qtvariantproperty.cpp \
    $$PWD/../QtPropertyBrowser/qttreepropertybrowser.cpp \
    $$PWD/../QtPropertyBrowser/qtpropertymanager.cpp \
    $$PWD/../QtPropertyBrowser/qtpropertybrowserutils.cpp \
    $$PWD/../QtPropertyBrowser/qtpropertybrowser.cpp \
    $$PWD/../QtPropertyBrowser/qtgroupboxpropertybrowser.cpp \
    $$PWD/../QtPropertyBrowser/qteditorfactory.cpp \
    $$PWD/../QtPropertyBrowser/qtbuttonpropertybrowser.cpp \
    $$PWD/../gds/gdsreader.cpp \
    $$PWD/../src/projectmanager.cpp \
    $$PWD/../src/property.cpp \
    $$PWD/../src/toolcustomtool.cpp \
    $$PWD/../src/toolmanager.cpp \
    $$PWD/../src/projectfile.cpp \
    $$PWD/../src/categories.cpp \
    $$PWD/../src/viewcontextmenu.cpp \
    $$PWD/../src/groupcontextmenu.cpp \
    $$PWD/../src/projectcontextmenu.cpp \
    $$PWD/../src/categorycontextmenu.cpp \
    $$PWD/../src/about.cpp \
    $$PWD/../src/newview.cpp

HEADERS += \
    $$PWD/tst_libman_gui.h \
    $$PWD/../src/mainwindow.h \
    $$PWD/../extension/variantmanager.h \
    $$PWD/../extension/variantfactory.h \
    $$PWD/../extension/qlineeditd2.h \
    $$PWD/../extension/filepathmanager.h \
    $$PWD/../extension/fileeditfactory.h \
    $$PWD/../extension/fileedit.h \
    $$PWD/../QtPropertyBrowser/qtvariantproperty.h \
    $$PWD/../QtPropertyBrowser/qttreepropertybrowser.h \
    $$PWD/../QtPropertyBrowser/qtpropertymanager.h \
    $$PWD/../QtPropertyBrowser/qtpropertybrowserutils_p.h \
    $$PWD/../QtPropertyBrowser/qtpropertybrowser.h \
    $$PWD/../QtPropertyBrowser/qtgroupboxpropertybrowser.h \
    $$PWD/../QtPropertyBrowser/qteditorfactory.h \
    $$PWD/../QtPropertyBrowser/qtbuttonpropertybrowser.h \
    $$PWD/../gds/gdsreader.h \
    $$PWD/../oas/oasReader.h \
    $$PWD/../src/projectmanager.h \
    $$PWD/../src/property.h \
    $$PWD/../src/toolmanager.h \
    $$PWD/../src/about.h \
    $$PWD/../src/newview.h

FORMS += \
    $$PWD/../src/mainwindow.ui \
    $$PWD/../src/projectmanager.ui \
    $$PWD/../src/toolmanager.ui \
    $$PWD/../src/about.ui \
    $$PWD/../src/newview.ui

RESOURCES += \
    $$PWD/../icons.qrc

# Test data available via QFINDTESTDATA(...)
TESTDATA += \
    $$PWD/data/sg13g2.projects \
    $$PWD/data/sample.gds
