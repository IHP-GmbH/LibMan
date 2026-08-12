#include <QMenu>
#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QProcess>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QInputDialog>
#include <QGuiApplication>
#include <QListWidgetItem>
#include <QStandardPaths>

#include "property.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "gds/gdsreader.h"
#include "lstream/lstreamcellwriter.h"
#include "core/core_path_utils.h"
#include "core/corecellreader.h"
#include "libman_test_mode.h"

/*!*********************************************************************************************************************
 * \brief Displays menu for view widget.
 * \param pos       Point(x, y) where menu will be displayed.
 **********************************************************************************************************************/
void MainWindow::showViewMenu(const QPoint &pos)
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&event);

    QMenu *menu = new QMenu(this);

    QMenu *menuViews = menu->addMenu(tr("New"));

    QAction *schematic = new QAction(tr("&Schematic"), this);
    schematic->setIcon(QIcon(":/icons/schematic.svg"));
    schematic->setStatusTip(tr("Create new schematic view."));
    connect(schematic, &QAction::triggered, this, &MainWindow::addNewSchematicView);
    menuViews->addAction(schematic);

    QAction *layout = new QAction(tr("&Layout"), this);
    layout->setIcon(QIcon(":/icons/layout.svg"));
    layout->setStatusTip(tr("Create new layout view."));
    connect(layout, &QAction::triggered, this, &MainWindow::addNewLayoutView);
    menuViews->addAction(layout);

    QAction *spice = new QAction(tr("&Spice"), this);
    spice->setIcon(QIcon(":/icons/spice.svg"));
    spice->setStatusTip(tr("Create new spice view."));
    connect(spice, &QAction::triggered, this, &MainWindow::addNewSpiceView);
    menuViews->addAction(spice);

    if(isViewCopied() && m_copyData.count()) {
        const QStringList views = getCurrentViews(libName, groupName);
        const QString viewPath = m_copyData.first();
        const QString viewName = QFileInfo(viewPath).suffix().toLower();

        if(!views.contains(viewName)) {
            QAction *pasteView = new QAction(tr("&Paste"), this);
            pasteView->setIcon(QIcon(":/icons/paste.svg"));
            pasteView->setShortcut(QKeySequence::Paste);
            pasteView->setStatusTip(tr("Paste view."));
            pasteView->setShortcutContext(Qt::WidgetShortcut);
            connect(pasteView, &QAction::triggered, this, &MainWindow::pasteSelectedData);
            menu->addAction(pasteView);
            addAction(pasteView);
        }
    }

    QList<QTreeWidgetItem *> items = m_ui->listViews->selectedItems();
    if(items.count()) {
        menu->addSeparator();

        QAction *copyView = new QAction(tr("&Copy"), this);
        copyView->setIcon(QIcon(":/icons/copy.svg"));
        copyView->setShortcut(QKeySequence::Copy);
        copyView->setStatusTip(tr("Copy view."));
        copyView->setShortcutContext(Qt::WidgetShortcut);
        connect(copyView, &QAction::triggered, this, &MainWindow::copySelectedView);
        menu->addAction(copyView);
        addAction(copyView);

        QAction *delView = new QAction(tr("&Delete"), this);
        delView->setIcon(QIcon(":/icons/delete.svg"));
        delView->setShortcut(QKeySequence::Delete);
        delView->setStatusTip(tr("Delete view."));
        delView->setShortcutContext(Qt::WidgetShortcut);
        connect(delView, &QAction::triggered, this, &MainWindow::removeSelectedView);
        menu->addAction(delView);
        addAction(delView);

        QAction *viewInfo = new QAction(tr("&Info"), this);
        viewInfo->setIcon(QIcon(":/icons/info.svg"));
        viewInfo->setStatusTip(tr("Show view information."));
        viewInfo->setShortcutContext(Qt::WidgetShortcut);
        connect(viewInfo, &QAction::triggered, this, &MainWindow::showViewInfo);
        menu->addAction(viewInfo);
        addAction(viewInfo);

        if(isLayoutXorMenuCandidate(items.first())) {
            menu->addSeparator();

            QAction *xorMark = new QAction(tr("XOR..."), this);
            xorMark->setIcon(QIcon(":/icons/layout.svg"));
            xorMark->setStatusTip(tr("Mark this layout as the first XOR operand."));
            connect(xorMark, &QAction::triggered, this, &MainWindow::markXorFirstLayout);
            menu->addAction(xorMark);

            if(!m_xorFirst.klayoutPath.isEmpty()) {
                QString curView;
                QString curPath;
                QString curKlayout;
                QString curCell;
                QStringList ignoreErrors;
                const bool resolved = resolveSelectedLayoutForXor(&curView, &curPath, &curKlayout, &curCell, &ignoreErrors);
                const bool sameAsFirst = resolved
                        && !curKlayout.isEmpty()
                        && (QFileInfo(curKlayout).absoluteFilePath()
                            == QFileInfo(m_xorFirst.klayoutPath).absoluteFilePath());

                if(resolved && !sameAsFirst) {
                    QAction *xorWith = new QAction(
                                tr("XOR with %1").arg(m_xorFirst.displayLabel), this);
                    xorWith->setIcon(QIcon(":/icons/layout.svg"));
                    xorWith->setStatusTip(tr("Compare this layout with the previously marked XOR layout via KLayout."));
                    connect(xorWith, &QAction::triggered, this, &MainWindow::runXorWithSelectedLayout);
                    menu->addAction(xorWith);
                }

                QAction *xorClear = new QAction(tr("Clear XOR selection"), this);
                xorClear->setStatusTip(tr("Clear the pending first XOR layout."));
                connect(xorClear, &QAction::triggered, this, &MainWindow::clearXorSelection);
                menu->addAction(xorClear);
            }
        }
    }

    menu->addSeparator();

    QMenu *gitMenu = menu->addMenu(tr("Git"));
    gitMenu->setIcon(QIcon(":/icons/git_branch.svg"));

    QAction *gitStatus = new QAction(tr("Status"), this);
    gitStatus->setIcon(QIcon(":/icons/git_status.svg"));
    gitStatus->setStatusTip(tr("Show git status."));
    connect(gitStatus, &QAction::triggered, this, &MainWindow::gitShowStatus);
    gitMenu->addAction(gitStatus);

    QAction *gitCommit = new QAction(tr("Commit"), this);
    gitCommit->setIcon(QIcon(":/icons/git_commit.svg"));
    gitCommit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_K));
    gitCommit->setStatusTip(tr("Commit changes."));
    gitCommit->setShortcutContext(Qt::WidgetShortcut);
    connect(gitCommit, &QAction::triggered, this, &MainWindow::gitCommitChanges);
    gitMenu->addAction(gitCommit);
    addAction(gitCommit);

    QAction *gitLog = new QAction(tr("Log"), this);
    gitLog->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    gitLog->setStatusTip(tr("Show git log."));
    connect(gitLog, &QAction::triggered, this, &MainWindow::gitShowLog);
    gitMenu->addAction(gitLog);

    QAction *gitDiff = new QAction(tr("Diff"), this);
    gitDiff->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    gitDiff->setStatusTip(tr("Show git diff."));
    connect(gitDiff, &QAction::triggered, this, &MainWindow::gitShowDiff);
    gitMenu->addAction(gitDiff);

    QAction *gitPull = new QAction(tr("Pull"), this);
    gitPull->setIcon(QIcon(":/icons/git_pull.svg"));
    gitPull->setStatusTip(tr("Pull changes from remote repository."));
    connect(gitPull, &QAction::triggered, this, &MainWindow::gitPull);
    gitMenu->addAction(gitPull);

    QAction *gitPush = new QAction(tr("Push"), this);
    gitPush->setIcon(QIcon(":/icons/git_push.svg"));
    gitPush->setStatusTip(tr("Push changes to remote repository."));
    connect(gitPush, &QAction::triggered, this, &MainWindow::gitPush);
    gitMenu->addAction(gitPush);

    QAction *gitCheckout = new QAction(tr("Checkout..."), this);
    gitCheckout->setIcon(QIcon(":/icons/git_branch.svg"));
    gitCheckout->setStatusTip(tr("Checkout branch."));
    connect(gitCheckout, &QAction::triggered, this, &MainWindow::gitCheckout);
    gitMenu->addAction(gitCheckout);

    menu->exec(QCursor::pos());

    delete menu;
}

/*!*********************************************************************************************************************
 * \brief Creates new spice model view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewSpiceView()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libRoot = getLibraryPath(libName);
    if(libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QStringList views = getCurrentViews(libName, groupName);
    if(views.contains("spice")) {
        return;
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if(!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".spice");
    if(QFileInfo(viewPath).exists()) {
        return;
    }

    if(createNewFile(viewPath)) {
        registerCreatedView(libName, groupName, "spice", viewPath);
    }
}

/*!*********************************************************************************************************************
 * \brief Displays a dialog that allows the user to choose which layout view to create.
 *
 * Supported layout view types:
 *  - gds
 *  - oas
 *  - lstr
 **********************************************************************************************************************/
void MainWindow::addNewLayoutView()
{
    QString selectedView;
    if (libmanAutomatedTestRun()) {
        selectedView = QStringLiteral("gds");
    } else {
        QStringList items;
        items << "gds" << "oas" << "lstr" << "layout";

        bool ok = false;
        selectedView = QInputDialog::getItem(this,
                                             tr("Create Layout View"),
                                             tr("Select layout view type:"),
                                             items,
                                             0,
                                             false,
                                             &ok);
        if (!ok || selectedView.isEmpty()) {
            return;
        }
    }

    if(selectedView == "gds") {
        addNewGdsView();
    }
    else if(selectedView == "oas") {
        addNewOasView();
    }
    else if(selectedView == "lstr") {
        addNewLStreamView();
    }
    else if(selectedView == "layout") {
        addNewCoreView();
    }
}

/*!*********************************************************************************************************************
 * \brief Registers a newly created cell view and updates the UI.
 *
 * This function creates a corresponding LibMan property entry for a newly created
 * cell view and adds it to the views tree widget.
 *
 * The property key is constructed in the form:
 *   LIBRARY_<libName>/<groupName>/<viewName>
 *
 * The function also configures the tree item with appropriate role data depending
 * on the view type (e.g. GDS, OAS, LStream) and ensures correct sorting of the list.
 *
 * \param libName     Name of the library.
 * \param groupName   Name of the cell (group).
 * \param viewName    View type suffix (e.g. "gds", "oas", "lstr").
 * \param viewPath    Absolute path to the created view file.
 *
 * \return True if the view was successfully registered and added to the UI,
 *         false otherwise.
 *********************************************************************************************************************/
bool MainWindow::registerCreatedView(const QString &libName,
                                     const QString &groupName,
                                     const QString &viewName,
                                     const QString &viewPath)
{
    if(libName.isEmpty() || groupName.isEmpty() || viewName.isEmpty() || viewPath.isEmpty()) {
        return false;
    }

    if(!QFileInfo(viewPath).exists()) {
        return false;
    }

    const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
    m_properties->set(key, viewPath);

    QTreeWidgetItem *viewItem = new QTreeWidgetItem(m_ui->listViews);
    viewItem->setText(0, viewName);

    if(viewName == "gds") {
        viewItem->setData(0, RoleType, ItemViewGds);
        viewItem->setData(0, RoleGdsPath, viewPath);
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    else if(viewName == "oas") {
        viewItem->setData(0, RoleType, ItemViewOas);
        viewItem->setData(0, RoleOasPath, viewPath);
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    else if(viewName == "lstr") {
        viewItem->setData(0, RoleType, ItemViewLStream);
        viewItem->setData(0, RoleLStreamPath, viewPath);
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    else if(isCoreViewName(viewName)) {
        configureCoreViewTreeItem(viewItem, viewName, viewPath);
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);

    setStateChanged();

    if(!m_currentProjFile.isEmpty() && !libmanAutomatedTestRun()) {
        saveProjectFile(m_currentProjFile);
    }

    return true;
}

/*!*********************************************************************************************************************
 * \brief Creates new GDS layout view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewGdsView()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libRoot = getLibraryPath(libName);
    if(libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QStringList views = getCurrentViews(libName, groupName);
    if(views.contains("gds")) {
        return;
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if(!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".gds");
    if(QFileInfo(viewPath).exists()) {
        return;
    }

    GdsReader gdsReader(viewPath);
    gdsReader.gdsCreate(groupName);

    const QStringList errors = gdsReader.getErrors();
    if(errors.count()) {
        foreach(const QString &explain, errors) {
            error(explain, false);
        }
        return;
    }

    registerCreatedView(libName, groupName, "gds", viewPath);
}

/*!*********************************************************************************************************************
 * \brief Creates new OAS layout view and adds it to the list widget.
 *
 * Currently this function creates an empty .oas file placeholder.
 **********************************************************************************************************************/
void MainWindow::addNewOasView()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libRoot = getLibraryPath(libName);
    if(libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QStringList views = getCurrentViews(libName, groupName);
    if(views.contains("oas")) {
        return;
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if(!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".oas");
    if(QFileInfo(viewPath).exists()) {
        return;
    }

    oasReader reader(viewPath);
    reader.oasCreate(groupName);

    const QStringList errors = reader.getErrors();
    if(errors.count()) {
        foreach(const QString &explain, errors) {
            error(explain, false);
        }
        return;
    }

    registerCreatedView(libName, groupName, "oas", viewPath);
}

/*!*********************************************************************************************************************
 * \brief Creates new LStream layout view and adds it to the list widget.
 *
 * Currently this function creates an empty .lstr file placeholder.
 **********************************************************************************************************************/
/*!*********************************************************************************************************************
 * \brief Creates new LStream layout view and adds it to the list widget.
 *
 * This function creates a minimal valid .lstr file using Cap'n Proto serialization
 * and registers it in the project if creation succeeds.
 **********************************************************************************************************************/
void MainWindow::addNewLStreamView()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libRoot = getLibraryPath(libName);
    if(libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QStringList views = getCurrentViews(libName, groupName);
    if(views.contains("lstr")) {
        return;
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if(!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".lstr");
    if(QFileInfo(viewPath).exists()) {
        return;
    }

    const QStringList cellNames{groupName};

    const LStreamCellWriter::Result result =
        LStreamCellWriter::write(viewPath,
                                 libName,
                                 cellNames,
                                 "unknown",
                                 "LibMan");

    if(!result.written) {
        foreach(const QString &explain, result.errors) {
            error(explain, false);
        }
        return;
    }

    registerCreatedView(libName, groupName, "lstr", viewPath);
}

/*!*********************************************************************************************************************
 * \brief Creates a new empty CORE view file for the current cell.
 **********************************************************************************************************************/
void MainWindow::createCoreView(const QString &viewName)
{
    const QString normalizedView = viewName.trimmed().toLower();
    if (!isCoreViewName(normalizedView) || normalizedView == QStringLiteral("core")) {
        return;
    }

    const QString libName = getCurrentLibraryName();
    if (libName.isEmpty()) {
        return;
    }

    const QString libRoot = getLibraryPath(libName);
    if (libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if (groupName.isEmpty()) {
        return;
    }

    const QStringList views = getCurrentViews(libName, groupName);
    if (views.contains(normalizedView)) {
        return;
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if (!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString viewPath = QDir::toNativeSeparators(coreViewFilePath(groupPath, groupName, normalizedView));
    if (QFileInfo(viewPath).exists()) {
        return;
    }

    CoreCellReader reader(viewPath);
    reader.coreCreate(groupName, normalizedView);

    const QStringList errors = reader.getErrors();
    if (errors.count()) {
        foreach (const QString &explain, errors) {
            error(explain, false);
        }
        return;
    }

    registerCreatedView(libName, groupName, normalizedView, viewPath);
}

/*!*********************************************************************************************************************
 * \brief Creates new CORE layout view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewCoreView()
{
    createCoreView(QStringLiteral("layout"));
}

void MainWindow::addNewCoreSchematicView()
{
    createCoreView(QStringLiteral("schematic"));
}

void MainWindow::addNewCoreSymbolView()
{
    createCoreView(QStringLiteral("symbol"));
}

/*!*********************************************************************************************************************
 * \brief Creates new schematic view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewSchematicView()
{
    QString selectedView;
    if (libmanAutomatedTestRun()) {
        selectedView = QStringLiteral("cdl");
    } else {
        QStringList items;
        items << "cdl" << "schematic" << "symbol";

        bool ok = false;
        selectedView = QInputDialog::getItem(this,
                                             tr("Create Schematic View"),
                                             tr("Select schematic view type:"),
                                             items,
                                             0,
                                             false,
                                             &ok);
        if (!ok || selectedView.isEmpty()) {
            return;
        }
    }

    if (selectedView == QStringLiteral("schematic")) {
        addNewCoreSchematicView();
        return;
    }
    if (selectedView == QStringLiteral("symbol")) {
        addNewCoreSymbolView();
        return;
    }

    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libRoot = getLibraryPath(libName);
    if(libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QStringList views = getCurrentViews(libName, groupName);
    if(views.contains("cdl")) {
        return;
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if(!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".cdl");
    if(QFileInfo(viewPath).exists()) {
        return;
    }

    if(createNewFile(viewPath)) {
        registerCreatedView(libName, groupName, "cdl", viewPath);
    }
}

/*!*********************************************************************************************************************
 * \brief Adds selected view to the buffer for coping.
 **********************************************************************************************************************/
void MainWindow::copySelectedView()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QString viewName = getCurrentViewName();
    if(viewName.isEmpty()) {
        return;
    }

    const QString viewPath = getViewPath(libName, groupName, viewName);
    if(!QFileInfo(viewPath).exists()) {
        return;
    }

    m_copyData.clear();
    addViewToBeCopied(viewPath);
}

/*!*********************************************************************************************************************
 * \brief Adds group to the buffer for later coping.
 * \param viewPath      Path of the view from where to copy.
 **********************************************************************************************************************/
void MainWindow::addViewToBeCopied(const QString &viewPath)
{
    m_copyData<<viewPath;
    m_currentCopyState = VIEW;
}

/*!*********************************************************************************************************************
 * \brief Removes selected view.
 **********************************************************************************************************************/
void MainWindow::removeSelectedView()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QList<QTreeWidgetItem*> items = m_ui->listViews->selectedItems();
    if(!items.count()) {
        return;
    }

    bool deleteFiles = false;
    if(!promptDeleteChoice(&deleteFiles)) {
        return;
    }

    QStringList viewNames;
    for(QTreeWidgetItem *item : items) {
        if(!item || item->parent()) {
            continue;
        }

        const QString viewName = item->text(0).trimmed();
        if(!viewName.isEmpty() && !viewNames.contains(viewName)) {
            viewNames << viewName;
        }
    }

    if(viewNames.isEmpty()) {
        return;
    }

    bool changed = false;
    for(const QString &viewName : viewNames) {
        const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
        const QString viewPath = getViewPath(libName, groupName, viewName);

        if(deleteFiles && QFileInfo(viewPath).exists()) {
            info(QString("Removing view '%1'").arg(viewPath));
            QFile::remove(viewPath);
        }

        if(m_properties->exists(key)) {
            m_properties->remove(key);
        }

        for(int idx = 0; idx < m_ui->listViews->topLevelItemCount(); ++idx) {
            QTreeWidgetItem *viewItem = m_ui->listViews->topLevelItem(idx);
            if(viewItem && viewItem->text(0).trimmed() == viewName) {
                delete m_ui->listViews->takeTopLevelItem(idx);
                break;
            }
        }

        changed = true;
    }

    if(changed) {
        setStateChanged();
        if(!m_currentProjFile.isEmpty()) {
            saveProjectFile(m_currentProjFile);
        }
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
}

/*!*********************************************************************************************************************
 * \brief Prints view file Unix information into the MainWindow output window.
 **********************************************************************************************************************/
void MainWindow::showViewInfo()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString viewName = getCurrentViewName();
    if(viewName.isEmpty()) {
        return;
    }

    const QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    const QString viewPath = getViewPath(libName, groupName, viewName);
    if(viewPath.isEmpty()) {
        return;
    }

    showFolderInfo("View", viewName, viewPath);
}

/*!*********************************************************************************************************************
 * \brief Returns the most relevant directory path for Git operations based on the current selection.
 *
 * This function checks the currently selected item in the following order:
 * - View item (from listViews): returns the parent directory of the selected view file.
 * - Group item (from listGroups): returns the parent directory of the selected group view file.
 * - Library item (from treeLibs): returns the current library path.
 *
 * If none of the above are selected, the user's home directory is returned as a fallback.
 *
 * \return A directory path suitable as a working directory for Git operations.
 **********************************************************************************************************************/
QString MainWindow::getCurrentGitPathForItem() const
{
    if (QTreeWidgetItem *viewItem = m_ui->listViews->currentItem()) {

        if (viewItem->data(0, RoleType).toInt() == ItemCell) {
            QTreeWidgetItem *parent = viewItem->parent();
            if (parent) {
                viewItem = parent;
            }
        }

        const QString viewName = viewItem->text(0);
        const QString viewPath = getCurrentViewFilePath(viewName);

        if (!viewPath.isEmpty()) {
            return QFileInfo(viewPath).absolutePath();
        }
    }

    if (QListWidgetItem *groupItem = m_ui->listGroups->currentItem()) {
        const QString groupName = groupItem->text();
        const QString libPath   = getCurrentLibraryPath();

        if (!libPath.isEmpty() && !groupName.isEmpty()) {
            return QFileInfo(libPath + "/" + groupName).absolutePath();
        }
    }

    if (QTreeWidgetItem *libItem = m_ui->treeLibs->currentItem()) {
        Q_UNUSED(libItem);
        return getCurrentLibraryPath();
    }

    return QDir::homePath();
}

/*!*********************************************************************************************************************
 * \brief Shows the current Git status for the active library.
 **********************************************************************************************************************/
void MainWindow::gitShowStatus()
{
    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());
    git.start("git", QStringList() << "status");
    git.waitForFinished();

    QString output = git.readAllStandardOutput();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief Commits changes in the active library with a message entered by the user.
 *        Automatically stages all changes before committing.
 **********************************************************************************************************************/
void MainWindow::gitCommitChanges()
{
    bool ok;
    QString message = QInputDialog::getText(this, tr("Commit Message"),
                                            tr("Enter commit message:"), QLineEdit::Normal,
                                            "", &ok);
    if (!ok || message.isEmpty())
        return;

    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());

    git.start("git", QStringList() << "add" << ".");
    git.waitForFinished();

    git.start("git", QStringList() << "commit" << "-m" << message);
    git.waitForFinished();

    QString output = git.readAllStandardOutput() + git.readAllStandardError();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief Displays the recent Git commit log for the active library.
 **********************************************************************************************************************/
void MainWindow::gitShowLog()
{
    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());
    git.start("git", QStringList() << "log" << "--oneline" << "-n" << "10");
    git.waitForFinished();

    QString output = git.readAllStandardOutput();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief Shows unstaged changes in the active library using 'git diff'.
 **********************************************************************************************************************/
void MainWindow::gitShowDiff()
{
    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());
    git.start("git", QStringList() << "diff");
    git.waitForFinished();

    QString output = git.readAllStandardOutput();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief Performs a 'git pull' to update the active library with remote changes.
 **********************************************************************************************************************/
void MainWindow::gitPull()
{
    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());
    git.start("git", QStringList() << "pull");
    git.waitForFinished();

    QString output = git.readAllStandardOutput() + git.readAllStandardError();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief Pushes local commits in the active library to the remote repository.
 **********************************************************************************************************************/
void MainWindow::gitPush()
{
    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());
    git.start("git", QStringList() << "push");
    git.waitForFinished();

    QString output = git.readAllStandardOutput() + git.readAllStandardError();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief Allows user to checkout a branch by name using 'git checkout'.
 **********************************************************************************************************************/
void MainWindow::gitCheckout()
{
    bool ok;
    QString branch = QInputDialog::getText(this, tr("Checkout Branch"),
                                           tr("Enter branch name:"), QLineEdit::Normal,
                                           "", &ok);
    if (!ok || branch.isEmpty())
        return;

    QProcess git;
    git.setWorkingDirectory(getCurrentGitPathForItem());
    git.start("git", QStringList() << "checkout" << branch);
    git.waitForFinished();

    QString output = git.readAllStandardOutput() + git.readAllStandardError();
    info(output, true);
}

/*!*********************************************************************************************************************
 * \brief True when the selected views-tree item is a layout root suitable for XOR (gds/oas/lstr/layout).
 **********************************************************************************************************************/
bool MainWindow::isLayoutXorMenuCandidate(QTreeWidgetItem *item) const
{
    if(!item) {
        return false;
    }

    QTreeWidgetItem *root = item;
    while(root->parent()) {
        root = root->parent();
    }

    return isLayoutViewTreeItem(root);
}

/*!*********************************************************************************************************************
 * \brief Resolves the currently selected layout view file and preferred cell for XOR / KLayout.
 **********************************************************************************************************************/
bool MainWindow::resolveSelectedLayoutForXor(QString *viewName,
                                             QString *viewPath,
                                             QString *klayoutPath,
                                             QString *cellName,
                                             QStringList *errors) const
{
    if(viewName) {
        viewName->clear();
    }
    if(viewPath) {
        viewPath->clear();
    }
    if(klayoutPath) {
        klayoutPath->clear();
    }
    if(cellName) {
        cellName->clear();
    }

    QList<QTreeWidgetItem *> items = m_ui->listViews->selectedItems();
    if(items.isEmpty()) {
        if(errors) {
            *errors << tr("No view selected.");
        }
        return false;
    }

    QTreeWidgetItem *item = items.first();
    if(!item) {
        return false;
    }

    QString resolvedView;
    QString resolvedPath;
    QString resolvedCell;

    const int type = item->data(0, RoleType).toInt();
    if(type == ItemViewGds) {
        resolvedView = QStringLiteral("gds");
        resolvedPath = item->data(0, RoleGdsPath).toString();
    }
    else if(type == ItemViewOas) {
        resolvedView = QStringLiteral("oas");
        resolvedPath = item->data(0, RoleOasPath).toString();
    }
    else if(type == ItemViewLStream) {
        resolvedView = QStringLiteral("lstr");
        resolvedPath = item->data(0, RoleLStreamPath).toString();
    }
    else if(type == ItemViewCore && isLayoutCoreViewName(item->text(0))) {
        resolvedView = item->text(0);
        resolvedPath = item->data(0, RoleCorePath).toString();
    }
    else if(type == ItemCell) {
        QTreeWidgetItem *p = item->parent();
        while(p && p->parent()) {
            p = p->parent();
        }
        if(!p) {
            if(errors) {
                *errors << tr("Cannot resolve layout root for selected cell.");
            }
            return false;
        }

        const QString rootName = p->text(0);
        if(rootName == QLatin1String("gds")) {
            resolvedView = QStringLiteral("gds");
            resolvedPath = p->data(0, RoleGdsPath).toString();
        }
        else if(rootName == QLatin1String("oas") || rootName == QLatin1String("oasis")) {
            resolvedView = QStringLiteral("oas");
            resolvedPath = p->data(0, RoleOasPath).toString();
        }
        else if(rootName == QLatin1String("lstr") || rootName == QLatin1String("lstream")) {
            resolvedView = QStringLiteral("lstr");
            resolvedPath = p->data(0, RoleLStreamPath).toString();
        }
        else if(p->data(0, RoleType).toInt() == ItemViewCore) {
            resolvedView = rootName;
            resolvedPath = p->data(0, RoleCorePath).toString();
        }
        else {
            if(errors) {
                *errors << tr("Selected cell is not under a layout view.");
            }
            return false;
        }

        resolvedCell = item->data(0, RoleCellName).toString();
    }
    else {
        if(errors) {
            *errors << tr("Selected item is not a layout view (gds/oas/lstr/layout).");
        }
        return false;
    }

    if(resolvedPath.isEmpty() || !QFileInfo::exists(resolvedPath)) {
        if(errors) {
            *errors << tr("Layout file does not exist: %1").arg(resolvedPath);
        }
        return false;
    }

    QStringList bridgeErrors;
    const QString kPath = layoutPathForKLayout(resolvedView, resolvedPath, &bridgeErrors);
    if(kPath.isEmpty()) {
        if(errors) {
            if(!bridgeErrors.isEmpty()) {
                *errors << bridgeErrors;
            }
            else {
                *errors << tr("Failed to prepare layout for KLayout: %1").arg(resolvedPath);
            }
        }
        return false;
    }

    if(resolvedCell.isEmpty()) {
        resolvedCell = preferredKLayoutCellForRoot(kPath, getCurrentGroupName());
    }

    if(viewName) {
        *viewName = resolvedView;
    }
    if(viewPath) {
        *viewPath = resolvedPath;
    }
    if(klayoutPath) {
        *klayoutPath = kPath;
    }
    if(cellName) {
        *cellName = resolvedCell;
    }
    return true;
}

/*!*********************************************************************************************************************
 * \brief Marks the current layout view as the first XOR operand.
 **********************************************************************************************************************/
void MainWindow::markXorFirstLayout()
{
    QString viewName;
    QString viewPath;
    QString klayoutPath;
    QString cellName;
    QStringList errors;
    if(!resolveSelectedLayoutForXor(&viewName, &viewPath, &klayoutPath, &cellName, &errors)) {
        error(errors.isEmpty() ? tr("Cannot mark layout for XOR.") : errors.join(QLatin1Char('\n')), false);
        return;
    }

    const QString libName = getCurrentLibraryName();
    const QString groupName = getCurrentGroupName();
    const QString label = QStringLiteral("%1/%2/%3").arg(libName, groupName, viewName);

    m_xorFirst.libName = libName;
    m_xorFirst.groupName = groupName;
    m_xorFirst.viewName = viewName;
    m_xorFirst.displayLabel = label;
    m_xorFirst.viewPath = viewPath;
    m_xorFirst.klayoutPath = klayoutPath;
    m_xorFirst.cellName = cellName;

    info(tr("XOR: '%1' selected for XOR. Choose a second layout view and use \"XOR with %1\".")
             .arg(label),
         false);
}

/*!*********************************************************************************************************************
 * \brief Clears the pending first XOR layout selection.
 **********************************************************************************************************************/
void MainWindow::clearXorSelection()
{
    if(m_xorFirst.klayoutPath.isEmpty()) {
        return;
    }

    const QString label = m_xorFirst.displayLabel;
    m_xorFirst = XorSelection();
    info(tr("XOR: cleared selection '%1'.").arg(label), false);
}

/*!*********************************************************************************************************************
 * \brief Runs KLayout batch XOR between the pending first layout and the currently selected layout.
 **********************************************************************************************************************/
void MainWindow::runXorWithSelectedLayout()
{
    if(m_xorFirst.klayoutPath.isEmpty()) {
        error(tr("XOR: no first layout selected. Use \"XOR...\" first."), false);
        return;
    }

    QString viewName;
    QString viewPath;
    QString klayoutPath;
    QString cellName;
    QStringList errors;
    if(!resolveSelectedLayoutForXor(&viewName, &viewPath, &klayoutPath, &cellName, &errors)) {
        error(errors.isEmpty() ? tr("Cannot resolve second layout for XOR.") : errors.join(QLatin1Char('\n')), false);
        return;
    }

    if(QFileInfo(klayoutPath).absoluteFilePath()
       == QFileInfo(m_xorFirst.klayoutPath).absoluteFilePath()) {
        error(tr("XOR: choose a different layout as the second operand."), false);
        return;
    }

    const QString libName = getCurrentLibraryName();
    const QString groupName = getCurrentGroupName();
    const QString labelB = QStringLiteral("%1/%2/%3").arg(libName, groupName, viewName);

    const QString tool = getToolByView(viewName);
    if(tool.isEmpty()) {
        error(tr("XOR: please configure the Layout tool in Tool Manager first."), false);
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QStringList parts = QProcess::splitCommand(tool.trimmed());
#else
    const QStringList parts = tool.trimmed().split(QLatin1Char(' '), QString::SkipEmptyParts);
#endif
    if(parts.isEmpty()) {
        error(tr("XOR: invalid Layout tool command."), false);
        return;
    }

    QString program = parts.first();
    const QFileInfo fi(program);
    if(!(fi.isAbsolute() && fi.exists())) {
        const QString found = QStandardPaths::findExecutable(program);
        if(!found.isEmpty()) {
            program = found;
        }
    }

    QStringList args = parts.mid(1);
    args << QStringLiteral("-b")
         << QStringLiteral("-r");

    const QString outputPath = QDir(QDir::tempPath()).filePath(
        QStringLiteral("libman_xor_%1.gds")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));

    const QString scriptPath = m_klayoutTools.createXorScript(m_xorFirst.klayoutPath,
                                                              m_xorFirst.cellName,
                                                              klayoutPath,
                                                              cellName,
                                                              outputPath);
    if(scriptPath.isEmpty()) {
        error(tr("XOR: failed to create KLayout XOR script."), false);
        return;
    }
    args << scriptPath;

    info(tr("XOR: starting KLayout XOR\n  A: %1 (%2)\n  B: %3 (%4)")
             .arg(m_xorFirst.displayLabel,
                  m_xorFirst.klayoutPath,
                  labelB,
                  klayoutPath),
         false);

    m_klayoutTools.startXorProcess(program, args, scriptPath, outputPath);
    m_xorFirst = XorSelection();
}
