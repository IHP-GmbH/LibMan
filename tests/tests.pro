# -------------------------------------------------
# LibMan GUI tests (QtTest)
# -------------------------------------------------

QT += core gui widgets testlib concurrent
LIBS += -lz

TEMPLATE = app
TARGET = tst_libman_gui
CONFIG += console c++17 debug
CONFIG+=coverage
DEFINES += QT_NO_DEPRECATED_WARNINGS LIBMAN_TESTING

win32:QMAKE_CXXFLAGS += -Wa,-mbig-obj

INCLUDEPATH += \
    $$PWD/.. \
    $$PWD/../src \
    $$PWD/../gds \
    $$PWD/../oas \
    $$PWD/../lstream \
    $$PWD/../extension \
    $$PWD/../QtPropertyBrowser

DEPENDPATH += $$INCLUDEPATH

# IMPORTANT:
# Do NOT include src/main.cpp here (QTEST_MAIN provides main())!
SOURCES += \
    $$PWD/tst_libman_gui_gds.cpp \
    $$PWD/../gds/gdsReadAsync.cpp \
    $$PWD/../oas/oasReadAsync.cpp \
    $$PWD/../oas/oasReader.cpp \
    $$PWD/../oas/oasCreate.cpp \
    $$PWD/../lstream/lstreamcellwriter.cpp \
    $$PWD/../lstream/lstrReadAsync.cpp \
    $$PWD/../src/libfileparser.cpp \
    $$PWD/../src/klayoutServer.cpp \
    $$PWD/../src/lstreamcellreader.cpp \
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
    $$PWD/../src/newview.cpp \
    main.cpp \

contains(CONFIG, no_core) {
    DEFINES += LIBMAN_NO_CORE
    SOURCES += $$PWD/../core/libman_no_core_stubs.cpp
} else {
    SOURCES += \
        $$PWD/../core/corecellreader.cpp \
        $$PWD/../core/coreReadAsync.cpp \
        $$PWD/../core/coreKlayoutBridge.cpp
}

SOURCES += \
    tst_dialogs.cpp \
    tst_klayout_requests.cpp \
    tst_libfileparser.cpp \
    tst_libman_layoutview_create.cpp \
    tst_libman_viewops.cpp \
    tst_lstream_writer.cpp \
    tst_mainwindow_categories.cpp \
    tst_mainwindow_loaders.cpp \
    tst_oas_writer.cpp \
    tst_toolmanager.cpp \
    tst_coverage_expansion.cpp \
    tst_coverage_80.cpp

HEADERS += \
    $$PWD/tst_libman_gui.h \
    $$PWD/../src/mainwindow.h \
    $$PWD/../src/libfileparser.h \
    $$PWD/../src/lstreamcellreader.h \
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
    $$PWD/../src/newview.h \
    $$PWD/../core/corecellreader.h \
    $$PWD/../core/coreKlayoutBridge.h \
    tst_dialogs.h \
    tst_klayout_requests.h \
    tst_libfileparser.h \
    tst_libman_layoutview_create.h \
    tst_libman_viewops.h \
    tst_lstream_writer.h \
    tst_mainwindow_categories.h \
    tst_mainwindow_loaders.h \
    tst_oas_writer.h \
    tst_toolmanager.h \
    tst_coverage_expansion.h \
    tst_coverage_80.h \
    test_paths.h

FORMS += \
    $$PWD/../src/mainwindow.ui \
    $$PWD/../src/projectmanager.ui \
    $$PWD/../src/toolmanager.ui \
    $$PWD/../src/about.ui \
    $$PWD/../src/newview.ui

RESOURCES += \
    $$PWD/../icons.qrc

TESTDATA += \
    $$PWD/data/sg13g2.projects \
    $$PWD/data/sample.gds \
    $$PWD/data/sg13g2_stdcell/Test/Test.gds \
    $$PWD/data/sg13g2_stdcell/Test/Test.oas \
    $$PWD/data/sg13g2_stdcell/Test/Test.lstr \
    $$PWD/data/sg13g2_stdcell/sg13g2_io/sg13g2_io.gds \
    $$PWD/data/sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.gds \
    $$PWD/data/sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.oas \
    $$PWD/data/sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.lstr \
    $$PWD/data/sg13g2_stdcell/lstr/sg13g2_stdcell.lstr

# Cap'n Proto (same clone-on-make flow as libman.pro)
LIBMAN_ROOT = $$dirname(_PRO_FILE_)/..
include($$LIBMAN_ROOT/capnp_deps.pri)
!contains(CONFIG, no_core) {
    include($$LIBMAN_ROOT/core_deps.pri)
}

CAPNP_GEN_DIR = $$LIBMAN_ROOT/capnp
SOURCES += $$files($$CAPNP_GEN_DIR/*.cc)
HEADERS += $$files($$CAPNP_GEN_DIR/*.h)

coverage {
    win32-g++|unix:!macx {
        QMAKE_CXXFLAGS += -O0 -g --coverage -fprofile-abs-path
        QMAKE_CFLAGS   += -O0 -g --coverage -fprofile-abs-path
        QMAKE_LFLAGS   += --coverage

        DEFINES += COVERAGE_BUILD

        QMAKE_DISTCLEAN += *.gcda *.gcno

        report.commands = gcovr -r $$PWD/.. --html-details -o coverage.html --print-summary --exclude ".*moc_.*" --exclude ".*qrc_.*"
        QMAKE_EXTRA_TARGETS += report
    }
}

include($$LIBMAN_ROOT/capnp_deps_finalize.pri)
!contains(CONFIG, no_core) {
    include($$LIBMAN_ROOT/core_deps_finalize.pri)
}
