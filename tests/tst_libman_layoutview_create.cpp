#include "tst_libman_layoutview_create.h"
#include "test_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QListWidget>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <functional>
#include <memory>

#define private public
#include "mainwindow.h"
#undef private

#include "property.h"

namespace
{

static const char *kLibraryName = "ihp_sg13g2";
static const char *kGroupTest   = "Test";

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

    const QFileInfoList entries = root.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

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

/*!********************************************************************************************************
 * \brief Selects a group item and triggers the corresponding slot.
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

/*!********************************************************************************************************
 * \brief Prepares the fixed test group in the temporary project.
 *********************************************************************************************************/
static void prepareTestGroup(MainWindow &w,
                             QTreeWidget *treeLibs,
                             QListWidget *listGroups,
                             QTreeWidget *listViews)
{
    QVERIFY2(treeLibs, "treeLibs widget not found.");
    QVERIFY2(listGroups, "listGroups widget not found.");
    QVERIFY2(listViews, "listViews widget not found.");

    selectLibrary(w, treeLibs, kLibraryName);

    QVERIFY2(waitUntil([&]() {
                 return listGroups->count() > 0;
             }),
             "Groups were not populated after selecting library.");

    selectGroup(w, listGroups, kGroupTest);
}

/*!********************************************************************************************************
 * \brief Removes one existing view entry from MainWindow properties, deletes its file and reloads views.
 *********************************************************************************************************/
static void removeExistingViewFromUi(MainWindow &w,
                                     const QString &libName,
                                     const QString &groupName,
                                     const QString &viewName)
{
    const QString key = w.getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;

    QString viewPath;
    if(w.m_properties->exists(key)) {
        viewPath = w.m_properties->get<QString>(key).trimmed();
        w.m_properties->remove(key);
    }
    else {
        const QString libRoot = w.getLibraryPath(libName);
        if(!libRoot.isEmpty()) {
            viewPath = QDir::toNativeSeparators(libRoot + "/" + groupName + "/" + groupName + "." + viewName);
        }
    }

    if(!viewPath.isEmpty() && QFileInfo(viewPath).exists()) {
        QFile::remove(viewPath);
    }

    w.loadViews(libName, groupName);
    QCoreApplication::processEvents();
}

/*!********************************************************************************************************
 * \brief Verifies common properties of a created top-level layout view item.
 *********************************************************************************************************/
static void verifyCreatedLayoutView(QTreeWidget *listViews,
                                    const QString &viewName,
                                    int expectedTypeRole,
                                    int expectedPathRole,
                                    const QString &expectedSuffix)
{
    QVERIFY2(listViews, "listViews widget not found.");

    QTreeWidgetItem *item = findTopItem(listViews, viewName);
    QVERIFY2(item, qPrintable(QString("View item '%1' not found.").arg(viewName)));

    QCOMPARE(item->text(0), viewName);
    QCOMPARE(item->data(0, Qt::UserRole + 1).toInt(), expectedTypeRole);
    QVERIFY(item->childIndicatorPolicy() == QTreeWidgetItem::ShowIndicator);

    const QString storedPath = item->data(0, expectedPathRole).toString();
    QVERIFY2(!storedPath.isEmpty(),
             qPrintable(QString("Stored path role is empty for '%1'.").arg(viewName)));
    QVERIFY2(QFileInfo(storedPath).exists(),
             qPrintable(QString("Stored file path does not exist: %1").arg(storedPath)));
    QCOMPARE(QFileInfo(storedPath).suffix().toLower(), expectedSuffix);
}

/*!********************************************************************************************************
 * \brief Checks that exactly one new file with a given suffix has appeared.
 *********************************************************************************************************/
static QString verifyExactlyOneNewFile(const QStringList &filesBefore,
                                       const QStringList &filesAfter,
                                       const QString &suffix)
{
    if(filesAfter.size() != filesBefore.size() + 1) {
        qWarning() << QString("Exactly one new .%1 file was expected.").arg(suffix);
        return QString();
    }

    QStringList newFiles = filesAfter;
    foreach(const QString &oldFile, filesBefore) {
        newFiles.removeAll(oldFile);
    }

    if(newFiles.size() != 1) {
        qWarning() << QString("Could not identify exactly one newly created .%1 file.").arg(suffix);
        return QString();
    }

    if(!QFileInfo(newFiles.first()).exists()) {
        qWarning() << QString("Newly created file does not exist: %1").arg(newFiles.first());
        return QString();
    }

    return newFiles.first();
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies that test data is available.
 *********************************************************************************************************/
void LibManLayoutViewCreateTest::initTestCase()
{
    const QString projPath = libmanTestProjectFile();
    QVERIFY2(!projPath.isEmpty(), "sg13g2.projects not found");
}

/*!********************************************************************************************************
 * \brief Verifies that addNewGdsView creates file and registers top-level item.
 *********************************************************************************************************/
void LibManLayoutViewCreateTest::addNewGdsView_createsFileAndRegistersView()
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
    removeExistingViewFromUi(*w, kLibraryName, kGroupTest, "gds");

    QVERIFY2(!findTopItem(listViews, "gds"), "gds view already exists before test start.");

    const QStringList filesBefore = findFilesBySuffixRecursive(temp.fixtureRoot, "gds");
    const int viewCountBefore = listViews->topLevelItemCount();

    w->addNewGdsView();

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listViews, "gds") != nullptr;
             }),
             "gds view was not added to listViews.");

    QCOMPARE(listViews->topLevelItemCount(), viewCountBefore + 1);

    const QStringList filesAfter = findFilesBySuffixRecursive(temp.fixtureRoot, "gds");
    const QString newFile = verifyExactlyOneNewFile(filesBefore, filesAfter, "gds");
    QVERIFY2(!newFile.isEmpty(), "Failed to detect exactly one newly created .gds file.");

    verifyCreatedLayoutView(listViews,
                            "gds",
                            1,
                            Qt::UserRole + 2,
                            "gds");

    QTreeWidgetItem *item = findTopItem(listViews, "gds");
    QVERIFY(item);
    QCOMPARE(QDir::toNativeSeparators(item->data(0, Qt::UserRole + 2).toString()),
             QDir::toNativeSeparators(newFile));
}

/*!********************************************************************************************************
 * \brief Verifies that addNewOasView creates file and registers top-level item.
 *********************************************************************************************************/
void LibManLayoutViewCreateTest::addNewOasView_createsFileAndRegistersView()
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
    removeExistingViewFromUi(*w, kLibraryName, kGroupTest, "oas");

    QVERIFY2(!findTopItem(listViews, "oas"), "oas view already exists before test start.");

    const QStringList filesBefore = findFilesBySuffixRecursive(temp.fixtureRoot, "oas");
    const int viewCountBefore = listViews->topLevelItemCount();

    w->addNewOasView();

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listViews, "oas") != nullptr;
             }),
             "oas view was not added to listViews.");

    QCOMPARE(listViews->topLevelItemCount(), viewCountBefore + 1);

    const QStringList filesAfter = findFilesBySuffixRecursive(temp.fixtureRoot, "oas");
    const QString newFile = verifyExactlyOneNewFile(filesBefore, filesAfter, "oas");
    QVERIFY2(!newFile.isEmpty(), "Failed to detect exactly one newly created .oas file.");

    verifyCreatedLayoutView(listViews,
                            "oas",
                            3,
                            Qt::UserRole + 4,
                            "oas");

    QTreeWidgetItem *item = findTopItem(listViews, "oas");
    QVERIFY(item);
    QCOMPARE(QDir::toNativeSeparators(item->data(0, Qt::UserRole + 4).toString()),
             QDir::toNativeSeparators(newFile));
}

/*!********************************************************************************************************
 * \brief Verifies that addNewLStreamView creates file and registers top-level item.
 *********************************************************************************************************/
void LibManLayoutViewCreateTest::addNewLStreamView_createsFileAndRegistersView()
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
    removeExistingViewFromUi(*w, kLibraryName, kGroupTest, "lstr");

    QVERIFY2(!findTopItem(listViews, "lstr"), "lstr view already exists before test start.");

    const QStringList filesBefore = findFilesBySuffixRecursive(temp.fixtureRoot, "lstr");
    const int viewCountBefore = listViews->topLevelItemCount();

    w->addNewLStreamView();

    QVERIFY2(waitUntil([&]() {
                 return findTopItem(listViews, "lstr") != nullptr;
             }),
             "lstr view was not added to listViews.");

    QCOMPARE(listViews->topLevelItemCount(), viewCountBefore + 1);

    const QStringList filesAfter = findFilesBySuffixRecursive(temp.fixtureRoot, "lstr");
    const QString newFile = verifyExactlyOneNewFile(filesBefore, filesAfter, "lstr");
    QVERIFY2(!newFile.isEmpty(), "Failed to detect exactly one newly created .lstr file.");

    verifyCreatedLayoutView(listViews,
                            "lstr",
                            4,
                            Qt::UserRole + 5,
                            "lstr");

    QTreeWidgetItem *item = findTopItem(listViews, "lstr");
    QVERIFY(item);
    QCOMPARE(QDir::toNativeSeparators(item->data(0, Qt::UserRole + 5).toString()),
             QDir::toNativeSeparators(newFile));
}
