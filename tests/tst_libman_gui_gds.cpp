/************************************************************************
 *  LibMan – Library & View Manager
 ************************************************************************/

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QFileInfo>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <functional>

#include "tst_libman_gui.h"
#include "mainwindow.h"

namespace
{

static const char *kProjectFile = "data/sg13g2.projects";
static const char *kLibraryName = "ihp_sg13g2";
static const char *kGroupTest = "Test";
static const char *kGroupIo = "sg13g2_io";
static const char *kGroupStdCell = "sg13g2_stdcell";
static const char *kViewGds = "gds";
static const char *kViewOas = "oas";
static const char *kViewLstr = "lstr";

/*!********************************************************************************************************
 * \brief Returns absolute path to the fixed test project.
 *********************************************************************************************************/
static QString testProjectPath()
{
    return QFINDTESTDATA(kProjectFile);
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
 * \brief Selects a library item and triggers the corresponding slot.
 *********************************************************************************************************/
static void selectLibrary(MainWindow &w, QTreeWidget *tree, QTreeWidgetItem *libItem)
{
    QVERIFY(libItem);

    tree->setCurrentItem(libItem);

    QMetaObject::invokeMethod(&w,
                              "on_treeLibs_itemClicked",
                              Qt::DirectConnection,
                              Q_ARG(QTreeWidgetItem*, libItem),
                              Q_ARG(int, 0));

    QCoreApplication::processEvents();
}

/*!********************************************************************************************************
 * \brief Selects a group item and triggers the corresponding slot.
 *********************************************************************************************************/
static void selectGroup(MainWindow &w, QListWidget *list, QListWidgetItem *groupItem)
{
    QVERIFY(groupItem);

    list->setCurrentItem(groupItem);

    QMetaObject::invokeMethod(&w,
                              "on_listGroups_itemClicked",
                              Qt::DirectConnection,
                              Q_ARG(QListWidgetItem*, groupItem));

    QCoreApplication::processEvents();
}

/*!********************************************************************************************************
 * \brief Expands a root view item and waits until children appear.
 *********************************************************************************************************/
static bool expandAndWaitForChildren(MainWindow &w,
                                     QTreeWidgetItem *rootItem,
                                     int timeoutMs = 7000)
{
    if(!rootItem) {
        return false;
    }

    QMetaObject::invokeMethod(&w,
                              "on_viewItemExpanded",
                              Qt::DirectConnection,
                              Q_ARG(QTreeWidgetItem*, rootItem));

    rootItem->setExpanded(true);

    return waitUntil([&]() {
        return rootItem->childCount() > 0;
    }, timeoutMs, 100);
}

/*!********************************************************************************************************
 * \brief Creates and shows test MainWindow.
 *********************************************************************************************************/
static MainWindow *createWindowForProject()
{
    const QString projPath = testProjectPath();
    if(projPath.isEmpty()) {
        return nullptr;
    }

    MainWindow *w = new MainWindow(projPath, QFileInfo(projPath).absolutePath());
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
 * \brief Selects fixed test library and fixed stdcell group.
 *********************************************************************************************************/
static void prepareStdCellViews(MainWindow &w,
                                QTreeWidget *treeLibs,
                                QListWidget *listGroups,
                                QTreeWidget *listViews)
{
    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listViews, "listViews widget not found");

    QTreeWidgetItem *libItem = findTopItem(treeLibs, kLibraryName);
    QVERIFY2(libItem, qPrintable(QString("Library not found in treeLibs: %1").arg(kLibraryName)));

    selectLibrary(w, treeLibs, libItem);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    QListWidgetItem *groupItem = findListItem(listGroups, kGroupStdCell);
    QVERIFY2(groupItem, qPrintable(QString("Group not found in listGroups: %1").arg(kGroupStdCell)));

    selectGroup(w, listGroups, groupItem);

    QVERIFY2(waitUntil([&]() {
                 return listViews->topLevelItemCount() > 0;
             }),
             "Views were not populated after selecting stdcell group.");
}

} // namespace

/*!********************************************************************************************************
 * \brief Basic availability check of fixed test data.
 *********************************************************************************************************/
void LibManGui::initTestCase()
{
    const QString projPath = testProjectPath();
    QVERIFY2(!projPath.isEmpty(), "data/sg13g2.projects not found");
}

/*!********************************************************************************************************
 * \brief Verifies that project loading populates expected library, groups and views.
 *********************************************************************************************************/
void LibManGui::loadProject_hasLibrariesGroupsAndViews()
{
    std::unique_ptr<MainWindow> w(createWindowForProject());
    QVERIFY2(w.get(), "Failed to create MainWindow for test project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listViews, "listViews widget not found");

    QTreeWidgetItem *libItem = findTopItem(treeLibs, kLibraryName);
    QVERIFY2(libItem, qPrintable(QString("Library not found in treeLibs: %1").arg(kLibraryName)));

    selectLibrary(*w, treeLibs, libItem);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    QVERIFY2(findListItem(listGroups, kGroupTest),
             qPrintable(QString("Expected group not found: %1").arg(kGroupTest)));

    QVERIFY2(findListItem(listGroups, kGroupIo),
             qPrintable(QString("Expected group not found: %1").arg(kGroupIo)));

    QListWidgetItem *stdCellItem = findListItem(listGroups, kGroupStdCell);
    QVERIFY2(stdCellItem,
             qPrintable(QString("Expected group not found: %1").arg(kGroupStdCell)));

    selectGroup(*w, listGroups, stdCellItem);

    QVERIFY2(waitUntil([&]() {
                 return listViews->topLevelItemCount() > 0;
             }),
             "Views were not populated after selecting stdcell group.");

    QVERIFY2(findTopItem(listViews, kViewGds),
             qPrintable(QString("Expected view not found: %1").arg(kViewGds)));

    QVERIFY2(findTopItem(listViews, kViewOas),
             qPrintable(QString("Expected view not found: %1").arg(kViewOas)));

    QVERIFY2(findTopItem(listViews, kViewLstr),
             qPrintable(QString("Expected view not found: %1").arg(kViewLstr)));
}

/*!********************************************************************************************************
 * \brief Verifies that expanding GDS view populates hierarchy.
 *********************************************************************************************************/
void LibManGui::expandGdsView_populatesHierarchy()
{
    std::unique_ptr<MainWindow> w(createWindowForProject());
    QVERIFY2(w.get(), "Failed to create MainWindow for test project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    prepareStdCellViews(*w, treeLibs, listGroups, listViews);

    QTreeWidgetItem *gdsRoot = findTopItem(listViews, kViewGds);
    QVERIFY2(gdsRoot, "Expected 'gds' view root not found.");

    const bool populated = expandAndWaitForChildren(*w, gdsRoot);
    QVERIFY2(populated, "GDS hierarchy was not populated after expand.");

    QVERIFY2(gdsRoot->childCount() > 0, "GDS root has no child cells after expansion.");

    QTreeWidgetItem *firstCell = gdsRoot->child(0);
    QVERIFY2(firstCell, "First GDS child cell is null.");
    QVERIFY2(!firstCell->text(0).trimmed().isEmpty(), "First GDS child cell name is empty.");

    listViews->setCurrentItem(firstCell);
    QVERIFY2(listViews->currentItem() == firstCell, "Failed to select GDS child cell.");
}

/*!********************************************************************************************************
 * \brief Verifies that expanding OAS view populates hierarchy.
 *********************************************************************************************************/
void LibManGui::expandOasView_populatesHierarchy()
{
    std::unique_ptr<MainWindow> w(createWindowForProject());
    QVERIFY2(w.get(), "Failed to create MainWindow for test project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    prepareStdCellViews(*w, treeLibs, listGroups, listViews);

    QTreeWidgetItem *oasRoot = findTopItem(listViews, kViewOas);
    QVERIFY2(oasRoot, "Expected 'oas' view root not found.");

    const bool populated = expandAndWaitForChildren(*w, oasRoot);
    QVERIFY2(populated, "OAS hierarchy was not populated after expand.");

    QVERIFY2(oasRoot->childCount() > 0, "OAS root has no child cells after expansion.");

    QTreeWidgetItem *firstCell = oasRoot->child(0);
    QVERIFY2(firstCell, "First OAS child cell is null.");
    QVERIFY2(!firstCell->text(0).trimmed().isEmpty(), "First OAS child cell name is empty.");
}

/*!********************************************************************************************************
 * \brief Verifies that expanding LStream view populates hierarchy.
 *********************************************************************************************************/
void LibManGui::expandLstrView_populatesHierarchy()
{
    std::unique_ptr<MainWindow> w(createWindowForProject());
    QVERIFY2(w.get(), "Failed to create MainWindow for test project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    prepareStdCellViews(*w, treeLibs, listGroups, listViews);

    QTreeWidgetItem *lstrRoot = findTopItem(listViews, kViewLstr);
    QVERIFY2(lstrRoot, "Expected 'lstr' view root not found.");

    const bool populated = expandAndWaitForChildren(*w, lstrRoot);
    QVERIFY2(populated, "LStream hierarchy was not populated after expand.");

    QVERIFY2(lstrRoot->childCount() > 0, "LStream root has no child cells after expansion.");

    QTreeWidgetItem *firstCell = lstrRoot->child(0);
    QVERIFY2(firstCell, "First LStream child cell is null.");
    QVERIFY2(!firstCell->text(0).trimmed().isEmpty(), "First LStream child cell name is empty.");
}

/*!********************************************************************************************************
 * \brief Cleanup hook.
 *********************************************************************************************************/
void LibManGui::cleanupTestCase()
{
}
