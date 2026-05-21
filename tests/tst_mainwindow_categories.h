#ifndef TST_MAINWINDOW_CATEGORIES_H
#define TST_MAINWINDOW_CATEGORIES_H

#include <QtTest/QtTest>

class MainWindowCategoriesTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void readLibraryCategories_missingFile_returnsEmpty();
    void readLibraryCategories_readsSortedUniqueCells();

    void addNewCategory_createsDefaultCategoryFileAndTreeItem();
    void addNewCategory_createsCopyNameWhenDefaultExists();

    void categoryClick_loadsGroupsFromCategoryFile();
};

#endif // TST_MAINWINDOW_CATEGORIES_H
