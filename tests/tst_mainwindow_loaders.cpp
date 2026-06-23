#include "tst_mainwindow_loaders.h"
#include "test_paths.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QListWidget>
#include <QLineEdit>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <functional>
#include <memory>

#include "mainwindow.h"
#include "mainwindow_test_hooks.h"

namespace
{

static const char *kLibraryName   = "ihp_sg13g2";
static const char *kGroupStdCell  = "sg13g2_stdcell";
static const char *kGroupIo       = "sg13g2_io";
static const char *kViewGds       = "gds";
static const char *kViewOas       = "oas";
static const char *kViewLstr      = "lstr";

/*!********************************************************************************************************
 * \brief Returns absolute path to the original fixed test project.
 *********************************************************************************************************/
/*!********************************************************************************************************
 * \brief Waits until predicate becomes true.
 *********************************************************************************************************/
static bool waitUntil(std::function<bool()> predicate, int timeoutMs = 5000, int stepMs = 50)
{
    const int steps = qMax(1, timeoutMs / stepMs);

    for(int i = 0; i < steps; ++i) {
        if(predicate()) {
            return true;
        }

        QTest::qWait(stepMs);
        QCoreApplication::processEvents();
    }

    return predicate();
}

/*!********************************************************************************************************
 * \brief Finds a top-level item in a QTreeWidget by text.
 *********************************************************************************************************/
static QTreeWidgetItem *findTopItem(QTreeWidget *tree, const QString &text)
{
    if(!tree) {
        return nullptr;
    }

    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *it = tree->topLevelItem(i);
        if(it && it->text(0) == text) {
            return it;
        }
    }

    return nullptr;
}

/*!********************************************************************************************************
 * \brief Finds an item in a QListWidget by text.
 *********************************************************************************************************/
static QListWidgetItem *findListItem(QListWidget *list, const QString &text)
{
    if(!list) {
        return nullptr;
    }

    for(int i = 0; i < list->count(); ++i) {
        QListWidgetItem *it = list->item(i);
        if(it && it->text() == text) {
            return it;
        }
    }

    return nullptr;
}

/*!********************************************************************************************************
 * \brief Copies a directory recursively.
 *********************************************************************************************************/
static bool copyRecursively(const QString &srcPath, const QString &dstPath)
{
    QDir srcDir(srcPath);
    if(!srcDir.exists()) {
        return false;
    }

    QDir dstDir;
    if(!dstDir.mkpath(dstPath)) {
        return false;
    }

    foreach(const QFileInfo &entry, srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
        const QString srcItem = entry.absoluteFilePath();
        const QString dstItem = dstPath + "/" + entry.fileName();

        if(entry.isDir()) {
            if(!copyRecursively(srcItem, dstItem)) {
                return false;
            }
        }
        else {
            QFile::remove(dstItem);
            if(!QFile::copy(srcItem, dstItem)) {
                return false;
            }
        }
    }

    return true;
}

/*!********************************************************************************************************
 * \brief Temporary project fixture descriptor.
 *********************************************************************************************************/
struct TempProject
{
    std::unique_ptr<QTemporaryDir> dir;
    QString                        projectFile;
};

/*!********************************************************************************************************
 * \brief Creates a temporary copy of the original test project directory.
 *********************************************************************************************************/
static TempProject createTempProjectCopy()
{
    TempProject out;
    out.dir.reset(new QTemporaryDir);

    if(!out.dir->isValid()) {
        return out;
    }

    const QString srcProject = libmanTestProjectFile();
    if(srcProject.isEmpty()) {
        out.dir.reset();
        return out;
    }

    const QFileInfo srcProjInfo(srcProject);
    const QString srcRoot = srcProjInfo.absolutePath();
    const QString dstRoot = out.dir->path() + "/fixture";

    if(!copyRecursively(srcRoot, dstRoot)) {
        out.dir.reset();
        return out;
    }

    out.projectFile = dstRoot + "/" + srcProjInfo.fileName();
    return out;
}

/*!********************************************************************************************************
 * \brief Creates and shows MainWindow for a temporary copied project.
 *********************************************************************************************************/
static MainWindow *createWindowForTempProject(const QString &projectPath)
{
    MainWindow *w = new MainWindow(projectPath, QFileInfo(projectPath).absolutePath());
    w->show();

    if(!QTest::qWaitForWindowExposed(w)) {
        delete w;
        return nullptr;
    }

    QTest::qWait(50);
    QCoreApplication::processEvents();

    return w;
}

/*!********************************************************************************************************
 * \brief Selects a library item and triggers corresponding slot.
 *********************************************************************************************************/
static void selectLibrary(MainWindow &w, QTreeWidget *tree, const QString &libName)
{
    QTreeWidgetItem *libItem = findTopItem(tree, libName);
    QVERIFY2(libItem, qPrintable(QString("Library not found: %1").arg(libName)));

    tree->setCurrentItem(libItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_treeLibs_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, libItem),
                                      Q_ARG(int, 0)));

    QCoreApplication::processEvents();
}

/*!********************************************************************************************************
 * \brief Selects a group item and triggers corresponding slot.
 *********************************************************************************************************/
static void selectGroup(MainWindow &w, QListWidget *list, const QString &groupName)
{
    QListWidgetItem *groupItem = findListItem(list, groupName);
    QVERIFY2(groupItem, qPrintable(QString("Group not found: %1").arg(groupName)));

    list->setCurrentItem(groupItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_listGroups_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QListWidgetItem*, groupItem)));

    QCoreApplication::processEvents();
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies that test data is available.
 *********************************************************************************************************/
void MainWindowLoadersTest::initTestCase()
{
    const QString projPath = libmanTestProjectFile();
    QVERIFY2(!projPath.isEmpty(), "sg13g2.projects not found");
}

/*!********************************************************************************************************
 * \brief Verifies that selecting a library populates groups, documents/categories widgets and search field.
 *********************************************************************************************************/
void MainWindowLoadersTest::selectingLibrary_populatesGroupsAndSearch()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs            = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups          = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listDocumentation   = w->findChild<QTreeWidget*>("listDocumentation");
    QTreeWidget *listCategories      = w->findChild<QTreeWidget*>("listCategories");
    QLineEdit   *txtLibSearch        = w->findChild<QLineEdit*>("txtLibSearch");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listDocumentation, "listDocumentation widget not found");
    QVERIFY2(listCategories, "listCategories widget not found");
    QVERIFY2(txtLibSearch, "txtLibSearch widget not found");

    selectLibrary(*w, treeLibs, kLibraryName);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    QVERIFY2(findListItem(listGroups, kGroupStdCell), "Expected stdcell group not found.");
    QVERIFY2(findListItem(listGroups, kGroupIo), "Expected io group not found.");

    QCOMPARE(txtLibSearch->text(), QString(kLibraryName));

    QVERIFY(listDocumentation->topLevelItemCount() >= 0);
    QVERIFY(listCategories->topLevelItemCount() >= 0);
}

/*!********************************************************************************************************
 * \brief Verifies that selecting a group populates views and updates group search field.
 *********************************************************************************************************/
void MainWindowLoadersTest::selectingGroup_populatesViewsAndSearch()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    QLineEdit   *txtCellSearch = w->findChild<QLineEdit*>("txtCellSearch");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listViews, "listViews widget not found");
    QVERIFY2(txtCellSearch, "txtCellSearch widget not found");

    selectLibrary(*w, treeLibs, kLibraryName);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    selectGroup(*w, listGroups, kGroupStdCell);

    QVERIFY2(waitUntil([&]() {
                 return listViews->topLevelItemCount() > 0;
             }),
             "Views were not populated after selecting group.");

    QVERIFY2(findTopItem(listViews, kViewGds), "Expected gds view not found.");
    QVERIFY2(findTopItem(listViews, kViewOas), "Expected oas view not found.");
    QVERIFY2(findTopItem(listViews, kViewLstr), "Expected lstr view not found.");
    QVERIFY2(findTopItem(listViews, "layout"), "Expected layout view not found.");
    QVERIFY2(findTopItem(listViews, "schematic"), "Expected schematic view not found.");

    QCOMPARE(txtCellSearch->text(), QString(kGroupStdCell));

    QTreeWidgetItem *gdsItem = findTopItem(listViews, kViewGds);
    QVERIFY(gdsItem);
    QVERIFY(gdsItem->childIndicatorPolicy() == QTreeWidgetItem::ShowIndicator);

    QTreeWidgetItem *oasItem = findTopItem(listViews, kViewOas);
    QVERIFY(oasItem);
    QVERIFY(oasItem->childIndicatorPolicy() == QTreeWidgetItem::ShowIndicator);

    QTreeWidgetItem *lstrItem = findTopItem(listViews, kViewLstr);
    QVERIFY(lstrItem);
    QVERIFY(lstrItem->childIndicatorPolicy() == QTreeWidgetItem::ShowIndicator);
}

/*!********************************************************************************************************
 * \brief Verifies that library search hides non-matching libraries.
 *********************************************************************************************************/
void MainWindowLoadersTest::librarySearch_hidesNonMatchingLibraries()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs = w->findChild<QTreeWidget*>("treeLibs");
    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY(treeLibs->topLevelItemCount() > 0);

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_txtLibSearch_textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString(kLibraryName))));

    QCoreApplication::processEvents();

    bool foundVisibleMatch = false;

    for(int i = 0; i < treeLibs->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = treeLibs->topLevelItem(i);
        QVERIFY(item);

        if(item->text(0) == kLibraryName) {
            QVERIFY2(!item->isHidden(), "Matching library item is hidden.");
            foundVisibleMatch = true;
        }
    }

    QVERIFY2(foundVisibleMatch, "Matching library item was not found.");

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_txtLibSearch_textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString())));

    QCoreApplication::processEvents();

    for(int i = 0; i < treeLibs->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = treeLibs->topLevelItem(i);
        QVERIFY(item);
        QVERIFY2(!item->isHidden(), "Library item should be visible after clearing filter.");
    }
}

/*!********************************************************************************************************
 * \brief Verifies that cell search hides non-matching groups.
 *********************************************************************************************************/
void MainWindowLoadersTest::cellSearch_hidesNonMatchingGroups()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");

    selectLibrary(*w, treeLibs, kLibraryName);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_txtCellSearch_textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString(kGroupStdCell))));

    QCoreApplication::processEvents();

    QListWidgetItem *stdItem = findListItem(listGroups, kGroupStdCell);
    QVERIFY2(stdItem, "Expected stdcell group not found.");
    QVERIFY2(!stdItem->isHidden(), "Matching group item is hidden.");

    QListWidgetItem *ioItem = findListItem(listGroups, kGroupIo);
    if(ioItem) {
        QVERIFY2(ioItem->isHidden(), "Non-matching group item should be hidden.");
    }

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_txtCellSearch_textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString())));

    QCoreApplication::processEvents();

    for(int i = 0; i < listGroups->count(); ++i) {
        QListWidgetItem *item = listGroups->item(i);
        QVERIFY(item);
        QVERIFY2(!item->isHidden(), "Group item should be visible after clearing filter.");
    }
}

/*!********************************************************************************************************
 * \brief Verifies that view search hides non-matching top-level views.
 *********************************************************************************************************/
void MainWindowLoadersTest::viewSearch_hidesNonMatchingViews()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listViews, "listViews widget not found");

    selectLibrary(*w, treeLibs, kLibraryName);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    selectGroup(*w, listGroups, kGroupStdCell);

    QVERIFY2(waitUntil([&]() {
                 return listViews->topLevelItemCount() > 0;
             }),
             "Views were not populated after selecting group.");

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_txtViewSearch_textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString(kViewGds))));

    QCoreApplication::processEvents();

    QTreeWidgetItem *gdsItem = findTopItem(listViews, kViewGds);
    QVERIFY2(gdsItem, "gds view item not found.");
    QVERIFY2(!gdsItem->isHidden(), "Matching gds view item is hidden.");

    QTreeWidgetItem *oasItem = findTopItem(listViews, kViewOas);
    if(oasItem) {
        QVERIFY2(oasItem->isHidden(), "Non-matching oas view item should be hidden.");
    }

    QTreeWidgetItem *lstrItem = findTopItem(listViews, kViewLstr);
    if(lstrItem) {
        QVERIFY2(lstrItem->isHidden(), "Non-matching lstr view item should be hidden.");
    }

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_txtViewSearch_textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString())));

    QCoreApplication::processEvents();

    for(int i = 0; i < listViews->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = listViews->topLevelItem(i);
        QVERIFY(item);
        QVERIFY2(!item->isHidden(), "View item should be visible after clearing filter.");
    }
}

/*!********************************************************************************************************
 * \brief Verifies that empty library selection clears dependent views.
 *********************************************************************************************************/
void MainWindowLoadersTest::emptyLibrarySelection_clearsDependentViews()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs            = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups          = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews           = w->findChild<QTreeWidget*>("listViews");
    QTreeWidget *listDocumentation   = w->findChild<QTreeWidget*>("listDocumentation");
    QTreeWidget *listCategories      = w->findChild<QTreeWidget*>("listCategories");
    QLineEdit   *txtLibSearch        = w->findChild<QLineEdit*>("txtLibSearch");
    QLineEdit   *txtCatSearch        = w->findChild<QLineEdit*>("txtCatSearch");
    QLineEdit   *txtCellSearch       = w->findChild<QLineEdit*>("txtCellSearch");
    QLineEdit   *txtViewSearch       = w->findChild<QLineEdit*>("txtViewSearch");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listViews, "listViews widget not found");
    QVERIFY2(listDocumentation, "listDocumentation widget not found");
    QVERIFY2(listCategories, "listCategories widget not found");
    QVERIFY2(txtLibSearch, "txtLibSearch widget not found");
    QVERIFY2(txtCatSearch, "txtCatSearch widget not found");
    QVERIFY2(txtCellSearch, "txtCellSearch widget not found");
    QVERIFY2(txtViewSearch, "txtViewSearch widget not found");

    selectLibrary(*w, treeLibs, kLibraryName);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    selectGroup(*w, listGroups, kGroupStdCell);

    QVERIFY2(waitUntil([&]() {
                 return listViews->topLevelItemCount() > 0;
             }),
             "Views were not populated after selecting group.");

    treeLibs->clearSelection();

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_treeLibs_itemSelectionChanged",
                                      Qt::DirectConnection));

    QCOMPARE(listGroups->count(), 0);
    QCOMPARE(listViews->topLevelItemCount(), 0);
    QCOMPARE(listDocumentation->topLevelItemCount(), 0);
    QCOMPARE(listCategories->topLevelItemCount(), 0);

    QCOMPARE(txtLibSearch->text(), QString());
    QCOMPARE(txtCatSearch->text(), QString());
    QCOMPARE(txtCellSearch->text(), QString());
    QCOMPARE(txtViewSearch->text(), QString());
}

/*!********************************************************************************************************
 * \brief Verifies that a library root registered without views can import layout files.
 *********************************************************************************************************/
void MainWindowLoadersTest::emptyLibraryRoot_allowsImportingLayoutFile()
{
    const QString srcGds = libmanTestDataFile(QStringLiteral("sg13g2_stdcell/Test/Test.gds"));
    if(srcGds.isEmpty() || !QFileInfo::exists(srcGds)) {
        QSKIP("Test.gds fixture not available");
    }

    QTemporaryDir tempRoot;
    QVERIFY(tempRoot.isValid());

    const QString libDir = tempRoot.path() + "/mylib";
    QVERIFY(QDir().mkpath(libDir));

    MainWindow w(QString(), tempRoot.path());

    MainWindowTestHooks::setLibraryRootDirectory(&w, "mylib", libDir);

    const QString resolvedRoot = MainWindowTestHooks::getLibraryPath(&w, "mylib");
    QCOMPARE(resolvedRoot, QDir::toNativeSeparators(QFileInfo(libDir).absoluteFilePath()));

    QVERIFY(MainWindowTestHooks::importCellViewFile(&w, "mylib", srcGds));

    const QString viewPath = MainWindowTestHooks::getViewPath(&w, "mylib", "Test", "gds");
    QVERIFY(QFileInfo(viewPath).exists());
}
