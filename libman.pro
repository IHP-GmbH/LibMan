#-------------------------------------------------
#
# Project created by QtCreator 2023-05-03T09:50:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = libman
TEMPLATE = app


SOURCES += src/main.cpp\
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
    src/property.cpp \
    src/toolmanager.cpp \
    src/projectfile.cpp \
    src/categories.cpp \
    src/viewcontextmenu.cpp \
    src/groupcontextmenu.cpp \
    src/projectcontextmenu.cpp \    
    src/categorycontextmenu.cpp \
    src/about.cpp \
    src/newview.cpp

HEADERS  += src/mainwindow.h \
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
    src/projectmanager.h \
    src/property.h \
    src/toolmanager.h \    
    src/about.h \
    src/newview.h

FORMS    += src/mainwindow.ui \
    src/projectmanager.ui \
    src/toolmanager.ui \
    src/about.ui \
    src/newview.ui

RESOURCES += \
    icons.qrc
