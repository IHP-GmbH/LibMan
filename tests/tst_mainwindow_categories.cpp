#include "tst_mainwindow_categories.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QListWidget>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <functional>
#include <memory>

#define private public
#include "mainwindow.h"
#undef private

namespace
{

static const char *kProjectFile = "data/sg13g2.projects";
static const char *kLibraryName = "ihp_sg13g2";

/*!********************************************************************************************************
 * \brief Returns absolute path to the original fixed test project.
 *********************************************************************************************************/
static QString originalProjectPath()
{
    return QFINDTESTDATA(kProjectFile);
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

    const QString srcProject = originalProjectPath();
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
    QVERIFY2(libItem, qPrintable(QString("Library not found: %1").arg(libName)));

    tree->setCurrentItem(libItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_treeLibs_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, libItem),
                                      Q_ARG(int, 0)));

    QCoreApplication::processEvents();
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies that test data is available.
 *********************************************************************************************************/
void MainWindowCategoriesTest::initTestCase()
{
    const QString projPath = originalProjectPath();
    QVERIFY2(!projPath.isEmpty(), "data/sg13g2.projects not found");
}

/*!********************************************************************************************************
 * \brief Verifies that missing category file returns empty list.
 *********************************************************************************************************/
void MainWindowCategoriesTest::readLibraryCategories_missingFile_returnsEmpty()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    const QStringList cats = w->readLibraryCategories(temp.fixtureRoot, "NoSuchCategory");
    QVERIFY(cats.isEmpty());
}

/*!********************************************************************************************************
 * \brief Verifies that category file is parsed into sorted unique cell names.
 *********************************************************************************************************/
void MainWindowCategoriesTest::readLibraryCategories_readsSortedUniqueCells()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    const QString libPath = temp.fixtureRoot + "/sg13g2_stdcell";
    QVERIFY2(QFileInfo(libPath).isDir(), "Temporary library directory not found.");

    const QString catPath = QDir::toNativeSeparators(libPath + "/MyCat.group");

    QFile file(catPath);
    QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Text), "Failed to create category file.");

    QTextStream out(&file);
    out << "B A C\n";
    out << "A   D\n";
    out << "C\n";
    file.close();

    const QStringList cats = w->readLibraryCategories(libPath, "MyCat");

    QCOMPARE(cats.size(), 4);
    QCOMPARE(cats.at(0), QString("A"));
    QCOMPARE(cats.at(1), QString("B"));
    QCOMPARE(cats.at(2), QString("C"));
    QCOMPARE(cats.at(3), QString("D"));
}

/*!********************************************************************************************************
 * \brief Verifies that addNewCategory creates default file and tree item.
 *********************************************************************************************************/
void MainWindowCategoriesTest::addNewCategory_createsDefaultCategoryFileAndTreeItem()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs       = w->findChild<QTreeWidget*>("treeLibs");
    QTreeWidget *listCategories = w->findChild<QTreeWidget*>("listCategories");
    QVERIFY2(treeLibs, "treeLibs widget not found.");
    QVERIFY2(listCategories, "listCategories widget not found.");

    selectLibrary(*w, treeLibs, kLibraryName);

    const QString libPath = w->getCurrentLibraryPath();
    QVERIFY2(QFileInfo(libPath).isDir(), "Current library path is not a directory.");

    const QString catPath = QDir::toNativeSeparators(libPath + "/Category.group");
    QFile::remove(catPath);

    QVERIFY2(!QFileInfo(catPath).exists(), "Category.group should not exist before test.");

    const int countBefore = listCategories->topLevelItemCount();

    w->addNewCategory();

    QVERIFY2(QFileInfo(catPath).exists(), "Category.group was not created.");

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listCategories, "Category") != nullptr;
             }),
             "Category tree item was not added.");

    QCOMPARE(listCategories->topLevelItemCount(), countBefore + 1);
}

/*!********************************************************************************************************
 * \brief Verifies that addNewCategory creates copy-name when default file already exists.
 *********************************************************************************************************/
void MainWindowCategoriesTest::addNewCategory_createsCopyNameWhenDefaultExists()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs       = w->findChild<QTreeWidget*>("treeLibs");
    QTreeWidget *listCategories = w->findChild<QTreeWidget*>("listCategories");
    QVERIFY2(treeLibs, "treeLibs widget not found.");
    QVERIFY2(listCategories, "listCategories widget not found.");

    selectLibrary(*w, treeLibs, kLibraryName);

    const QString libPath = w->getCurrentLibraryPath();
    QVERIFY2(QFileInfo(libPath).isDir(), "Current library path is not a directory.");

    const QString catPath = QDir::toNativeSeparators(libPath + "/Category.group");
    {
        QFile file(catPath);
        QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Text), "Failed to create initial Category.group.");
        file.close();
    }

    const int countBefore = listCategories->topLevelItemCount();

    w->addNewCategory();

    QVERIFY2(QFileInfo(catPath).exists(), "Original Category.group must still exist.");

    QVERIFY2(waitUntil([&]() {
                 for(int i = 0; i < listCategories->topLevelItemCount(); ++i) {
                     QTreeWidgetItem *it = listCategories->topLevelItem(i);
                     if(it && it->text(0).startsWith("Category_copy")) {
                         return true;
                     }
                 }
                 return false;
             }),
             "Copy category tree item was not added.");

    QCOMPARE(listCategories->topLevelItemCount(), countBefore + 1);
}

/*!********************************************************************************************************
 * \brief Verifies that clicking category loads groups from category file.
 *********************************************************************************************************/
void MainWindowCategoriesTest::categoryClick_loadsGroupsFromCategoryFile()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "Failed to prepare temporary project copy.");

    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "Failed to create MainWindow.");

    QTreeWidget *treeLibs       = w->findChild<QTreeWidget*>("treeLibs");
    QTreeWidget *listCategories = w->findChild<QTreeWidget*>("listCategories");
    QListWidget *listGroups     = w->findChild<QListWidget*>("listGroups");

    QVERIFY2(treeLibs, "treeLibs widget not found.");
    QVERIFY2(listCategories, "listCategories widget not found.");
    QVERIFY2(listGroups, "listGroups widget not found.");

    selectLibrary(*w, treeLibs, kLibraryName);

    const QString libPath = w->getCurrentLibraryPath();
    QVERIFY2(QFileInfo(libPath).isDir(), "Current library path is not a directory.");

    const QString catPath = QDir::toNativeSeparators(libPath + "/MyCategory.group");
    QFile file(catPath);
    QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Text), "Failed to create category file.");

    QTextStream out(&file);
    out << "CellB CellA\n";
    out << "CellA\n";
    file.close();

    w->loadCategories(libPath);

    QTreeWidgetItem *catItem = findTopItem(listCategories, "MyCategory");
    QVERIFY2(catItem, "MyCategory was not loaded into category tree.");

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_listCategories_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, catItem)));

    QCOMPARE(listGroups->count(), 2);
    QVERIFY2(findListItem(listGroups, "CellA"), "CellA group was not loaded.");
    QVERIFY2(findListItem(listGroups, "CellB"), "CellB group was not loaded.");
}
