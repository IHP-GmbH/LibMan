#include "tst_coverage_expansion.h"
#include "test_paths.h"

#include "mainwindow_test_hooks.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QListWidget>
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
#include "newview.h"
#include "oas/oasReader.h"

namespace
{

static const char *kLibraryName = "ihp_sg13g2";
static const char *kGroupTest    = "Test";

static bool waitUntil(std::function<bool()> predicate, int timeoutMs = 8000, int stepMs = 50)
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

static bool expandAndWaitForChildren(MainWindow &w, QTreeWidgetItem *rootItem, int timeoutMs = 15000)
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

static QString fixtureSiblingPath(const QString &projPath, const char *relativeUnderData)
{
    Q_UNUSED(projPath);
    return libmanTestDataFile(QString::fromUtf8(relativeUnderData));
}

} // namespace

void FormatReadersCoverageTest::initTestCase()
{
    QVERIFY2(!libmanTestProjectFile().isEmpty(), "sg13g2.projects not found");
}

void FormatReadersCoverageTest::gdsReader_readHierarchy_fixtureFile()
{
    const QString gdsPath = fixtureSiblingPath(QString(), "sg13g2_stdcell/Test/Test.gds");
    QVERIFY2(QFileInfo::exists(gdsPath), qPrintable(QString("Missing fixture: %1").arg(gdsPath)));

    GdsReader reader(gdsPath);
    GdsReader::GdsHierarchy hierarchy;
    QVERIFY2(reader.readHierarchy(hierarchy), qPrintable(reader.getErrors().join("; ")));
    QVERIFY(!hierarchy.allCells.isEmpty() || !hierarchy.topCells.isEmpty());
}

void FormatReadersCoverageTest::oasReader_readHierarchy_fixtureFile()
{
    const QString oasPath = fixtureSiblingPath(QString(), "sg13g2_stdcell/Test/Test.oas");
    QVERIFY2(QFileInfo::exists(oasPath), qPrintable(QString("Missing fixture: %1").arg(oasPath)));

    oasReader reader(oasPath);
    LayoutHierarchy hierarchy;
    QVERIFY2(reader.readHierarchy(hierarchy), qPrintable(reader.getErrors().join("; ")));
    QVERIFY(!hierarchy.allCells.isEmpty() || !hierarchy.topCells.isEmpty());
}

void FormatReadersCoverageTest::lstreamReader_read_fixtureFile()
{
    const QString lstrPath = fixtureSiblingPath(QString(), "sg13g2_stdcell/Test/Test.lstr");
    QVERIFY2(QFileInfo::exists(lstrPath), qPrintable(QString("Missing fixture: %1").arg(lstrPath)));

    const LStreamCellReader::Result r = LStreamCellReader::read(lstrPath);
    QVERIFY2(r.errors.isEmpty(), qPrintable(r.errors.join("; ")));
    QVERIFY(r.loaded);
    QVERIFY(!r.cellNames.isEmpty());
}

void MainWindowCoverageExpansionTest::initTestCase()
{
    QVERIFY2(!libmanTestProjectFile().isEmpty(), "sg13g2.projects not found");
}

void MainWindowCoverageExpansionTest::hooks_removeDirAndCopyDir()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QVERIFY(MainWindowTestHooks::removeDir(w.get(), "/path/that/does/not/exist_12345"));

    QTemporaryDir src;
    QVERIFY(src.isValid());
    QVERIFY(QDir().mkpath(src.path() + "/nested"));
    QVERIFY(QFile::copy(temp.projectFile, src.path() + "/nested/a.txt"));

    const QString dst = src.path() + "_copy_dest";
    MainWindowTestHooks::copyDir(w.get(), src.path(), dst);
    QVERIFY(QFileInfo::exists(dst + "/nested/a.txt"));

    QVERIFY(MainWindowTestHooks::removeDir(w.get(), dst));
    QVERIFY(!QDir(dst).exists());
}

void MainWindowCoverageExpansionTest::hooks_expandShellVariables_detectView_generateCopyName()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    const QString envName = QStringLiteral("LIBMAN_COVERAGE_ENV_%1").arg(QCoreApplication::applicationPid());
    qputenv(envName.toLatin1(), QByteArray("expanded_value"));
    const QString pattern = QString("$%1").arg(envName);
    QCOMPARE(MainWindowTestHooks::expandShellVariables(w.get(), pattern), QStringLiteral("expanded_value"));

    QCOMPARE(MainWindowTestHooks::detectViewFromPath(w.get(), "C:/foo/bar.GDS"), QStringLiteral("gds"));

    QTemporaryDir d;
    QVERIFY(d.isValid());
    const QString base = d.path() + "/cell";
    QVERIFY(QFile::copy(temp.projectFile, base + "_copy.gds"));
    QVERIFY(QFile::copy(temp.projectFile, base + "_copy1.gds"));

    const QString generated = MainWindowTestHooks::generateCopyName(w.get(), "cell", d.path(), ".gds");
    QVERIFY2(!generated.isEmpty(), "generateCopyName");
    QVERIFY2(!QFileInfo::exists(generated), qPrintable(QString("expected unused path: %1").arg(generated)));
}

void MainWindowCoverageExpansionTest::hooks_getViewPath_andRepresentativeFile()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    const QString gdsPath = MainWindowTestHooks::getViewPath(w.get(), kLibraryName, kGroupTest, "gds");
    QVERIFY2(QFileInfo::exists(gdsPath), qPrintable(QString("view path: %1").arg(gdsPath)));

    const QString rep = MainWindowTestHooks::findRepresentativeLibraryFile(w.get(), kLibraryName);
    QVERIFY2(QFileInfo::exists(rep), qPrintable(QString("rep file: %1").arg(rep)));

    QVERIFY(MainWindowTestHooks::getCurrentViewFilePath(w.get(), "gds").isEmpty()
            || QFileInfo::exists(MainWindowTestHooks::getCurrentViewFilePath(w.get(), "gds")));

    QString resolved;
    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "resolveProjectPath",
                                      Qt::DirectConnection,
                                      Q_RETURN_ARG(QString, resolved),
                                      Q_ARG(QString, temp.projectFile),
                                      Q_ARG(QString, QStringLiteral("sg13g2_stdcell/Test/Test.gds"))));
    QVERIFY2(QFileInfo::exists(resolved), qPrintable(resolved));
}

void MainWindowCoverageExpansionTest::mainWindow_searches_toggles_gitStatus_and_infoSlots()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtLibSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("ihp"))));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtCatSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("a"))));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtCellSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("T"))));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtViewSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("g"))));

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionShow_Categories_toggled", Qt::DirectConnection, Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionShow_Categories_toggled", Qt::DirectConnection, Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionShow_Documents_toggled", Qt::DirectConnection, Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionShow_Documents_toggled", Qt::DirectConnection, Q_ARG(bool, true)));

    QVERIFY(QMetaObject::invokeMethod(w.get(), "clearCurrentCopyState", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "addNewGroup", Qt::DirectConnection));

    QTreeWidgetItem *libItem = findTopItem(treeLibs, kLibraryName);
    QVERIFY(libItem);
    treeLibs->setCurrentItem(libItem);
    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "on_treeLibs_itemClicked",
                                      Qt::DirectConnection,
                                      Q_ARG(QTreeWidgetItem*, libItem),
                                      Q_ARG(int, 0)));
    QCoreApplication::processEvents();

    QVERIFY(QMetaObject::invokeMethod(w.get(), "copySelectedProject", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "gitShowStatus", Qt::DirectConnection));

    QVERIFY(QMetaObject::invokeMethod(w.get(), "showProjectInfo", Qt::DirectConnection));

    QListWidgetItem *groupItem = findListItem(listGroups, kGroupTest);
    QVERIFY(groupItem);
    selectGroup(*w, listGroups, groupItem);
    QVERIFY(QMetaObject::invokeMethod(w.get(), "showGroupInfo", Qt::DirectConnection));

    QTreeWidgetItem *gdsView = findTopItem(listViews, "gds");
    QVERIFY(gdsView);
    listViews->setCurrentItem(gdsView);
    gdsView->setSelected(true);
    QVERIFY(QMetaObject::invokeMethod(w.get(), "showViewInfo", Qt::DirectConnection));

    QTimer::singleShot(80, []() {
        if(QWidget *aw = QApplication::activeModalWidget()) {
            aw->close();
        }
    });
    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "loadProjectFile",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QStringLiteral("/nonexistent/libman_missing_file.lib"))));
    QTest::qWait(200);

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionReload_triggered", Qt::DirectConnection));
    QCoreApplication::processEvents();
}

void MainWindowCoverageExpansionTest::newView_dialogInvokesMainWindowCreatePaths()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    QVERIFY2(!findTopItem(listViews, "spice"), "spice view should not exist before NewView test");

    const int viewsBefore = listViews->topLevelItemCount();

    {
        NewView nv(w.get(), kLibraryName, kGroupTest);
        nv.show();
        QComboBox *cb = nv.findChild<QComboBox*>("cbxView");
        QVERIFY(cb);
        const int spiceIdx = cb->findText(QStringLiteral("spice"));
        QVERIFY2(spiceIdx >= 0, "spice in NewView combo");
        cb->setCurrentIndex(spiceIdx);
        QVERIFY(QMetaObject::invokeMethod(&nv, "on_btnCreate_clicked", Qt::DirectConnection));
        QCoreApplication::processEvents();
    }

    QVERIFY2(waitUntil([&]() { return findTopItem(listViews, "spice") != nullptr; }),
             "spice view via NewView");
    QVERIFY(listViews->topLevelItemCount() >= viewsBefore);
}

void MainWindowCoverageExpansionTest::mainWindow_expandGdsAndViewSearchFilter()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    QTreeWidgetItem *gdsItem = findTopItem(listViews, "gds");
    QVERIFY(gdsItem);
    QVERIFY2(expandAndWaitForChildren(*w, gdsItem), "GDS hierarchy children");

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtViewSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("gds"))));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_txtViewSearch_textEdited", Qt::DirectConnection, Q_ARG(QString, QString())));
}

void MainWindowCoverageExpansionTest::mainWindow_save_supportedViews_session_category()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    (void)MainWindowTestHooks::getSupportedViewsByTool(w.get());

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionSave_triggered", Qt::DirectConnection));
    QCoreApplication::processEvents();

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionCategory_triggered", Qt::DirectConnection));
    QCoreApplication::processEvents();

    QTreeWidget *cats = w->findChild<QTreeWidget*>("listCategories");
    QVERIFY(cats);
    if(cats->topLevelItemCount() > 0) {
        QTreeWidgetItem *cat = cats->topLevelItem(0);
        QVERIFY(cat);
        cats->setCurrentItem(cat);
        cat->setSelected(true);
        QVERIFY(QMetaObject::invokeMethod(w.get(),
                                          "on_listCategories_itemClicked",
                                          Qt::DirectConnection,
                                          Q_ARG(QTreeWidgetItem*, cat)));
        QCoreApplication::processEvents();
        QVERIFY(QMetaObject::invokeMethod(w.get(), "showCategoryInfo", Qt::DirectConnection));
    }

    QTimer::singleShot(150, []() {
        if(QWidget *aw = QApplication::activeModalWidget()) {
            QTest::keyClick(aw, Qt::Key_Escape);
        }
    });
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionSession_triggered", Qt::DirectConnection));
    QTest::qWait(300);
}

void MainWindowSlotSmokeTest::initTestCase()
{
    QVERIFY2(!libmanTestProjectFile().isEmpty(), "sg13g2.projects not found");
}

void MainWindowSlotSmokeTest::invokeSafeSlots_withPartialSelection()
{
    TempProject temp = createTempProjectCopy();
    QVERIFY2(temp.dir.get(), "temp project");
    std::unique_ptr<MainWindow> w(createWindowForTempProject(temp.projectFile));
    QVERIFY2(w.get(), "MainWindow");

    QTreeWidget *treeLibs   = w->findChild<QTreeWidget*>("treeLibs");
    QListWidget *listGroups = w->findChild<QListWidget*>("listGroups");
    QTreeWidget *listViews  = w->findChild<QTreeWidget*>("listViews");
    prepareLibraryAndGroup(*w, treeLibs, listGroups, listViews);

    QVERIFY(QMetaObject::invokeMethod(w.get(), "updateRecentProjectActions", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionClear_Recent_File_Stack_triggered", Qt::DirectConnection));

    QTreeWidgetItem *gdsView = findTopItem(listViews, "gds");
    QVERIFY(gdsView);
    listViews->setCurrentItem(gdsView);
    gdsView->setSelected(true);
    QVERIFY(QMetaObject::invokeMethod(w.get(), "copySelectedView", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "copySelectedGroup", Qt::DirectConnection));

    listViews->clearSelection();
    QVERIFY(QMetaObject::invokeMethod(w.get(), "removeSelectedView", Qt::DirectConnection));

    QVERIFY(QMetaObject::invokeMethod(w.get(), "removeSelectedCategory", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "showCategoryInfo", Qt::DirectConnection));

    QVERIFY(QMetaObject::invokeMethod(w.get(),
                                      "addViewToBeCopied",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, temp.projectFile)));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "clearCurrentCopyState", Qt::DirectConnection));

    QVERIFY(QMetaObject::invokeMethod(w.get(), "removeGroupUnion", Qt::DirectConnection));

    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_treeLibs_itemSelectionChanged", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(w.get(), "on_actionGroup_triggered", Qt::DirectConnection));
}
