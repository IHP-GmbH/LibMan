#ifndef TST_MAINWINDOW_LOADERS_H
#define TST_MAINWINDOW_LOADERS_H

#include <QtTest/QtTest>

class MainWindowLoadersTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void selectingLibrary_populatesGroupsAndSearch();
    void selectingGroup_populatesViewsAndSearch();
    void librarySearch_hidesNonMatchingLibraries();
    void cellSearch_hidesNonMatchingGroups();
    void viewSearch_hidesNonMatchingViews();
    void emptyLibrarySelection_clearsDependentViews();
};

#endif // TST_MAINWINDOW_LOADERS_H
