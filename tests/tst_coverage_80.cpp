#include "tst_coverage_80.h"
#include "test_paths.h"

#include "mainwindow_test_hooks.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QListWidget>
#include <QMenu>
#include <QMetaObject>
#include <QStringList>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

#include <functional>
#include <memory>

#include <QtTest/QtTest>

#include "gds/gdsreader.h"
#include "lstreamcellreader.h"
#include "mainwindow.h"

namespace
{

static const char *kLibraryName = "ihp_sg13g2";
static const char *kGroupTest    = "Test";

static bool waitUntil(std::function<bool()> predicate, int timeoutMs = 15000, int stepMs = 50)
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

struct TempProject
{
    std::unique_ptr<QTemporaryDir> dir;
    QString                        projectFile;
    QString                        fixtureRoot;
};

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

static void selectLibrary(MainWindow &w, QTreeWidget *tree, QTreeWidgetItem *libItem)
{
    QVERIFY(libItem);
    tree->setCurrentItem(libItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_treeLibs_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, libItem),
                                      Q_ARG(int, 0)));

    QCoreApplication::processEvents();
}

static void selectGroup(MainWindow &w, QListWidget *list, QListWidgetItem *groupItem)
{
    QVERIFY(groupItem);
    list->setCurrentItem(groupItem);

    QVERIFY(QMetaObject::invokeMethod(&w,
                                      "on_listGroups_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QListWidgetItem*, groupItem)));

    QCoreApplication::processEvents();
}

static void prepareLibraryAndGroup(MainWindow &w,
                                   QTreeWidget *treeLibs,
                                   QListWidget *listGroups,
                                   QTreeWidget *listViews)
{
    QVERIFY(treeLibs);
    QVERIFY(listGroups);
    QVERIFY(listViews);

    QTreeWidgetItem *libItem = findTopItem(treeLibs, kLibraryName);
    QVERIFY(libItem);
    selectLibrary(w, treeLibs, libItem);

    QVERIFY(waitUntil([&]() {
        return listGroups->count() > 0;
    }));

    QListWidgetItem *groupItem = findListItem(listGroups, kGroupTest);
    QVERIFY(groupItem);
    selectGroup(w, listGroups, groupItem);
}

static bool expandRootAndWait(MainWindow &w, QTreeWidgetItem *root, int timeoutMs = 15000)
{
    if(!root) {
        return false;
    }

    QMetaObject::invokeMethod(&w,
                              "on_viewItemExpanded",
                              Qt::DirectConnection,
                              Q_ARG(QTreeWidgetItem*, root));
    root->setExpanded(true);

    return waitUntil([&]() {
        return root->childCount() > 0;
    }, timeoutMs, 100);
}

static void armAutoCloseDialogs(int totalMs = 2500)
{
    for(int t = 40; t <= totalMs; t += 40) {
        QTimer::singleShot(t, []() {
            const QWidgetList tl = QApplication::topLevelWidgets();
            for(QWidget *widget : tl) {
                if(!widget || !widget->isVisible()) {
                    continue;
                }

                if(qobject_cast<QMenu*>(widget) || qobject_cast<QDialog*>(widget)) {
                    widget->close();
                }
            }
        });
    }
}

} // namespace

void Coverage80MainWindowDeepTest::initTestCase()
{
    QVERIFY2(!libmanTestProjectFile().isEmpty(), "fixture projects file missing");
}

void Coverage80MainWindowDeepTest::expandOasAndLstrRoots_populateHierarchy()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    QTreeWidgetItem *oasItem = findTopItem(listViews, "oas");
    if(!oasItem) {
        oasItem = findTopItem(listViews, "oasis");
    }
    QVERIFY2(oasItem, "oas/oasis view root");
    QVERIFY2(expandRootAndWait(*w, oasItem), "OAS hierarchy");

    QTreeWidgetItem *lstrItem = findTopItem(listViews, "lstr");
    if(!lstrItem) {
        lstrItem = findTopItem(listViews, "lstream");
    }
    QVERIFY2(lstrItem, "lstr view root");
    QVERIFY2(expandRootAndWait(*w, lstrItem), "LStream hierarchy");

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtViewSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("cell"))));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtViewSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QString())));
}

void Coverage80MainWindowDeepTest::hooks_documents_categories_filtersAndCombinedLibs()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    const QString libRoot = MainWindowTestHooks::getLibraryPath(w.get(), kLibraryName);
    QVERIFY2(!libRoot.isEmpty(), "library path");
    MainWindowTestHooks::loadDocuments(w.get(), libRoot);
    MainWindowTestHooks::loadCategories(w.get(), libRoot);

    QTreeWidget *cats = w->findChild<QTreeWidget*>("listCategories");
    QVERIFY(cats);
    if(cats->topLevelItemCount() > 0) {
        QTreeWidgetItem *cat = cats->topLevelItem(0);
        (void)MainWindowTestHooks::filterTreeItem(w.get(), cat, QStringLiteral("___nomatch___"));
        (void)MainWindowTestHooks::filterTreeItem(w.get(), cat, QString());
    }

    MainWindowTestHooks::hideTreeItem(w.get(), treeLibs, QStringLiteral("___"));
    MainWindowTestHooks::hideTreeItem(w.get(), treeLibs, QString());
    MainWindowTestHooks::hideListItem(w.get(), listGroups, QStringLiteral("___"));
    MainWindowTestHooks::hideListItem(w.get(), listGroups, QString());

    QMap<QString, QStringList> combined;
    MainWindowTestHooks::loadCombinedLibs(w.get(), combined);
}

void Coverage80MainWindowDeepTest::dialogs_about_toolsAndProjectManager_autoClose()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    armAutoCloseDialogs();
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionAbout_triggered", Qt::DirectConnection));
    QTest::qWait(400);

    armAutoCloseDialogs();
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionTools_triggered", Qt::DirectConnection));
    QTest::qWait(400);

    armAutoCloseDialogs(3500);
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionProjects_triggered", Qt::DirectConnection));
    QTest::qWait(500);
}

void Coverage80MainWindowDeepTest::documentationTree_doubleClick_smoke()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    QTreeWidget *docs = w->findChild<QTreeWidget*>("listDocumentation");
    QVERIFY(docs);
    if(docs->topLevelItemCount() > 0) {
        QTreeWidgetItem *docItem = docs->topLevelItem(0);
        QVERIFY(docItem);
        QVERIFY(QMetaObject::invokeMethod(w.get(),
                                          "on_listDocumentation_itemDoubleClicked",
                                          Qt::DirectConnection,
                                          Q_ARG(QTreeWidgetItem*, docItem)));
    }
}

void Coverage80MainWindowDeepTest::nullItemSlots_noCrash()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_treeLibs_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, nullptr),
                                      Q_ARG(int, 0)));
    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_listGroups_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QListWidgetItem*, nullptr)));
    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_listCategories_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, nullptr)));
    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_listDocumentation_itemDoubleClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, nullptr)));
}

void Coverage80LStreamReaderTest::initTestCase()
{
    QVERIFY2(!libmanTestProjectFile().isEmpty(), "fixture");
}

void Coverage80LStreamReaderTest::read_missingFile_reportsError()
{
    const LStreamCellReader::Result r = LStreamCellReader::read(QStringLiteral("/nonexistent/path_12345/Test.lstr"));
    QVERIFY(!r.loaded);
    QVERIFY(!r.errors.isEmpty());
}

void Coverage80LStreamReaderTest::read_emptyFile_reportsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/empty.lstr";
    QVERIFY(QFile(path).open(QIODevice::WriteOnly | QIODevice::Truncate));

    const LStreamCellReader::Result r = LStreamCellReader::read(path);
    QVERIFY(!r.loaded);
    QVERIFY(!r.errors.isEmpty());
}

void Coverage80LStreamReaderTest::read_badPrefix_reportsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/bad.lstr";
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write("NotLStream");
    f.close();

    const LStreamCellReader::Result r = LStreamCellReader::read(path);
    QVERIFY(!r.loaded);
    QVERIFY(!r.errors.isEmpty());
}

void Coverage80LStreamReaderTest::read_unterminatedHeader_reportsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/badhdr.lstr";
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write("LStream_1");
    f.close();

    const LStreamCellReader::Result r = LStreamCellReader::read(path);
    QVERIFY(!r.loaded);
    QVERIFY(!r.errors.isEmpty());
}

void Coverage80LStreamReaderTest::read_truncatedPayload_reportsCapnpError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/trunc.lstr";
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QByteArray payload("LStream_1");
    payload.append(char(0));
    payload.append(QByteArray(8, char(0)));
    f.write(payload);
    f.close();

    const LStreamCellReader::Result r = LStreamCellReader::read(path);
    QVERIFY(!r.loaded);
    QVERIFY(!r.errors.isEmpty());
}

void Coverage80GdsExtraTest::initTestCase()
{
    QVERIFY2(!libmanTestProjectFile().isEmpty(), "fixture");
}

void Coverage80GdsExtraTest::readHierarchy_secondStdcellGds_fixture()
{
    const QString gdsPath = libmanTestDataFile(QStringLiteral("sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.gds"));
    QVERIFY2(QFileInfo::exists(gdsPath), qPrintable(gdsPath));

    GdsReader reader(gdsPath);
    GdsReader::GdsHierarchy hierarchy;
    QVERIFY2(reader.readHierarchy(hierarchy), qPrintable(reader.getErrors().join("; ")));
}
