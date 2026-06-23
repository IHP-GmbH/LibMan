#include "tst_libman_viewops.h"
#include "test_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QListWidget>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <memory>
#include <functional>

#include "mainwindow.h"

namespace
{

static const char *kLibraryName = "ihp_sg13g2";
static const char *kGroupTest   = "Test";

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
 * \brief Returns all files with the given suffix below the root recursively.
 *********************************************************************************************************/
static QStringList findFilesBySuffixRecursive(const QString &rootPath, const QString &suffix)
{
    QStringList out;

    QDir root(rootPath);
    if(!root.exists()) {
        return out;
    }

    const QFileInfoList entries =
        root.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    foreach(const QFileInfo &entry, entries) {
        if(entry.isDir()) {
            out << findFilesBySuffixRecursive(entry.absoluteFilePath(), suffix);
        }
        else if(entry.suffix().compare(suffix, Qt::CaseInsensitive) == 0) {
            out << QDir::toNativeSeparators(entry.absoluteFilePath());
        }
    }

    out.sort();
    return out;
}

/*!********************************************************************************************************
 * \brief Temporary project fixture descriptor.
 *********************************************************************************************************/
struct TempProject
{
    std::unique_ptr<QTemporaryDir> dir;
    QString                        projectFile;
    QString                        fixtureRoot;
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

    if(!QDir().mkpath(dstRoot)) {
        out.dir.reset();
        return out;
    }

    if(!QFile::copy(srcProject, dstRoot + "/" + srcProjInfo.fileName())) {
        out.dir.reset();
        return out;
    }

    // View-ops tests only need the Test cell subtree; avoid copying the full ~500MB fixture tree.
    if(!copyRecursively(srcRoot + "/sg13g2_stdcell/Test", dstRoot + "/sg13g2_stdcell/Test")) {
        out.dir.reset();
        return out;
    }

    out.fixtureRoot = dstRoot;
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
 * \brief Selects a library item and triggers the corresponding slot.
 *********************************************************************************************************/
static void selectLibrary(MainWindow &w, QTreeWidget *tree, const QString &libName)
{
    QTreeWidgetItem *libItem = findTopItem(tree, libName);
    QVERIFY(libItem);

    tree->setCurrentItem(libItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_treeLibs_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, libItem),
                                      Q_ARG(int, 0)));

    QCoreApplication::processEvents();
}

/*!********************************************************************************************************
 * \brief Selects a group item and triggers the corresponding slot.
 *********************************************************************************************************/
static void selectGroup(MainWindow &w, QListWidget *list, const QString &groupName)
{
    QListWidgetItem *groupItem = findListItem(list, groupName);
    QVERIFY(groupItem);

    list->setCurrentItem(groupItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_listGroups_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QListWidgetItem*, groupItem)));

    QCoreApplication::processEvents();
}

/*!********************************************************************************************************
 * \brief Prepares the fixed test group in the temporary project.
 *********************************************************************************************************/
static void prepareTestGroup(MainWindow &w,
                             QTreeWidget *treeLibs,
                             QListWidget *listGroups,
                             QTreeWidget *listViews)
{
    QVERIFY(treeLibs);
    QVERIFY(listGroups);
    QVERIFY(listViews);

    selectLibrary(w, treeLibs, kLibraryName);

    QVERIFY(waitUntil([&]() {
        return listGroups->count() > 0;
    }));

    selectGroup(w, listGroups, kGroupTest);
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies that test data is available.
 *********************************************************************************************************/
void LibManViewOpsTest::initTestCase()
{
    const QString projPath = libmanTestProjectFile();
    QVERIFY2(!projPath.isEmpty(), "sg13g2.projects not found");
}

/*!********************************************************************************************************
 * \brief Verifies that addNewSpiceView creates file and registers top-level item.
 *********************************************************************************************************/
void LibManViewOpsTest::addNewSpiceView_createsFileAndRegistersView()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");
    QVERIFY2(!temp.fixtureRoot.isEmpty(), "Temporary fixture root is empty.");

    const QStringList filesBefore = findFilesBySuffixRecursive(temp.fixtureRoot, "spice");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow for temporary project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    prepareTestGroup(*w, treeLibs, listGroups, listViews);

    QVERIFY2(!findTopItem(listViews, "spice"), "spice view already exists before test start.");

    const int viewCountBefore = listViews->topLevelItemCount();

    QVERIFY(QMetaObject::invokeMethod(w.get(), "addNewSpiceView", Qt::DirectConnection));

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listViews, "spice") != nullptr;
             }),
             "spice view was not added to listViews.");

    QVERIFY2(listViews->topLevelItemCount() == viewCountBefore + 1,
             "Top-level view count did not increase after spice creation.");

    const QStringList filesAfter = findFilesBySuffixRecursive(temp.fixtureRoot, "spice");
    QVERIFY2(filesAfter.size() == filesBefore.size() + 1,
             "Exactly one new .spice file was expected.");

    QStringList newFiles = filesAfter;
    foreach(const QString &oldFile, filesBefore) {
        newFiles.removeAll(oldFile);
    }

    QVERIFY2(newFiles.size() == 1, "Could not identify exactly one newly created .spice file.");
    QVERIFY2(QFileInfo(newFiles.first()).exists(),
             qPrintable(QString("Newly created spice file does not exist: %1").arg(newFiles.first())));
}

/*!********************************************************************************************************
 * \brief Verifies that addNewSchematicView creates file and registers top-level item.
 *********************************************************************************************************/
void LibManViewOpsTest::addNewSchematicView_createsFileAndRegistersView()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");
    QVERIFY2(!temp.fixtureRoot.isEmpty(), "Temporary fixture root is empty.");

    const QStringList filesBefore = findFilesBySuffixRecursive(temp.fixtureRoot, "cdl");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow for temporary project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    prepareTestGroup(*w, treeLibs, listGroups, listViews);

    QVERIFY2(!findTopItem(listViews, "cdl"), "cdl view already exists before test start.");

    const int viewCountBefore = listViews->topLevelItemCount();

    QVERIFY(QMetaObject::invokeMethod(w.get(), "addNewSchematicView", Qt::DirectConnection));

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listViews, "cdl") != nullptr;
             }),
             "cdl view was not added to listViews.");

    QVERIFY2(listViews->topLevelItemCount() == viewCountBefore + 1,
             "Top-level view count did not increase after cdl creation.");

    const QStringList filesAfter = findFilesBySuffixRecursive(temp.fixtureRoot, "cdl");
    QVERIFY2(filesAfter.size() == filesBefore.size() + 1,
             "Exactly one new .cdl file was expected.");

    QStringList newFiles = filesAfter;
    foreach(const QString &oldFile, filesBefore) {
        newFiles.removeAll(oldFile);
    }

    QVERIFY2(newFiles.size() == 1, "Could not identify exactly one newly created .cdl file.");
    QVERIFY2(QFileInfo(newFiles.first()).exists(),
             qPrintable(QString("Newly created cdl file does not exist: %1").arg(newFiles.first())));
}

/*!********************************************************************************************************
 * \brief Verifies that copySelectedView works as a smoke-test on an existing created view.
 *********************************************************************************************************/
void LibManViewOpsTest::copySelectedView_smoke()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");
    QVERIFY2(!temp.projectFile.isEmpty(), "Temporary project file path is empty.");
    QVERIFY2(!temp.fixtureRoot.isEmpty(), "Temporary fixture root is empty.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow for temporary project.");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");

    prepareTestGroup(*w, treeLibs, listGroups, listViews);

    QVERIFY(QMetaObject::invokeMethod(w.get(), "addNewSpiceView", Qt::DirectConnection));

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listViews, "spice") != nullptr;
             }),
             "spice view was not added before copySelectedView test.");

    QTreeWidgetItem *spiceItem = findTopItem(listViews, "spice");
    QVERIFY2(spiceItem, "spice view item not found.");

    listViews->setCurrentItem(spiceItem);
    spiceItem->setSelected(true);

    QVERIFY(QMetaObject::invokeMethod(w.get(), "copySelectedView", Qt::DirectConnection));

    QVERIFY(true);
}
