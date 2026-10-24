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
#include <QMessageBox>
#include <QLineEdit>
#include <QGuiApplication>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QStandardPaths>

#include "property.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "gds/gdsreader.h"
#include "lstream/lstreamcellwriter.h"
#include "room/room_path_utils.h"
#include "room/roomcellreader.h"
#include "libman_test_mode.h"
#ifndef LIBMAN_NO_ROOM
#include "room_json_io.h"
#endif

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
    connect(schematic, &QAction::triggered, this, &MainWindow::addNewCoreSchematicView);
    menuViews->addAction(schematic);

    QAction *symbol = new QAction(tr("S&ymbol"), this);
    symbol->setIcon(QIcon(":/icons/schematic.svg"));
    symbol->setStatusTip(tr("Create new symbol view."));
    connect(symbol, &QAction::triggered, this, &MainWindow::addNewCoreSymbolView);
    menuViews->addAction(symbol);

    QAction *cdl = new QAction(tr("&CDL"), this);
    cdl->setIcon(QIcon(":/icons/spice.svg"));
    cdl->setStatusTip(tr("Create new CDL view."));
    connect(cdl, &QAction::triggered, this, [this]() {
        if(libmanAutomatedTestRun()) {
            addNewSchematicView();
            return;
        }
        const QString libName = getCurrentLibraryName();
        const QString groupName = getCurrentGroupName();
        if(libName.isEmpty() || groupName.isEmpty()) {
            return;
        }
        const QStringList views = getCurrentViews(libName, groupName);
        if(views.contains(QStringLiteral("cdl"))) {
            return;
        }
        const QString libRoot = getLibraryPath(libName);
        if(libRoot.isEmpty()) {
            return;
        }
        const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
        QDir().mkpath(groupPath);
        const QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".cdl");
        if(QFileInfo(viewPath).exists()) {
            return;
        }
        if(createNewFile(viewPath)) {
            registerCreatedView(libName, groupName, QStringLiteral("cdl"), viewPath);
        }
    });
    menuViews->addAction(cdl);

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

    QAction *emsetup = new QAction(tr("&EM Setup"), this);
    emsetup->setIcon(QIcon(":/icons/emsetup.svg"));
    emsetup->setStatusTip(
        tr("Create an empty EMStudio model under emsetup (nominal, or nominal_N if taken)."));
    connect(emsetup, &QAction::triggered, this, &MainWindow::addNewEmSetupView);
    menuViews->addAction(emsetup);

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

        bool emSetupRootSelected = false;
        bool emSetupVariantSelected = false;
        bool otherViewSelected = false;
        for(QTreeWidgetItem *it : items) {
            if(!it) {
                continue;
            }
            const int t = it->data(0, RoleType).toInt();
            if(t == ItemEmSetupVariant
               || (it->parent()
                   && (it->parent()->data(0, RoleType).toInt() == ItemViewEmSetup
                       || isEmSetupViewName(it->parent()->text(0))))) {
                emSetupVariantSelected = true;
            } else if(t == ItemViewEmSetup || isEmSetupViewName(it->text(0))) {
                emSetupRootSelected = true;
            } else {
                otherViewSelected = true;
            }
        }
        const bool emSetupOnly = (emSetupRootSelected || emSetupVariantSelected) && !otherViewSelected;

        if(emSetupOnly) {
            if(emSetupVariantSelected && !emSetupRootSelected && items.count() == 1) {
                QAction *renameVariant = new QAction(tr("&Rename..."), this);
                renameVariant->setStatusTip(tr("Rename this emsetup variant folder."));
                connect(renameVariant, &QAction::triggered, this, &MainWindow::renameEmSetupVariant);
                menu->addAction(renameVariant);
            }

            QAction *copyEm = new QAction(tr("&Copy..."), this);
            copyEm->setIcon(QIcon(":/icons/copy.svg"));
            copyEm->setShortcut(QKeySequence::Copy);
            copyEm->setShortcutContext(Qt::WidgetShortcut);
            if(emSetupRootSelected && !emSetupVariantSelected) {
                copyEm->setStatusTip(tr("Copy the whole emsetup folder into another library/cell."));
            } else {
                copyEm->setStatusTip(tr("Copy the selected emsetup variant into a library/cell."));
            }
            connect(copyEm, &QAction::triggered, this, &MainWindow::copyEmSetupSelection);
            menu->addAction(copyEm);
            addAction(copyEm);

            QAction *delView = new QAction(tr("&Delete"), this);
            delView->setIcon(QIcon(":/icons/delete.svg"));
            delView->setShortcut(QKeySequence::Delete);
            delView->setStatusTip(tr("Delete selected emsetup item(s)."));
            delView->setShortcutContext(Qt::WidgetShortcut);
            connect(delView, &QAction::triggered, this, &MainWindow::removeSelectedView);
            menu->addAction(delView);
            addAction(delView);

            QAction *attachModel = new QAction(tr("&Attach Model..."), this);
            attachModel->setIcon(QIcon(":/icons/emsetup.svg"));
            attachModel->setStatusTip(
                tr("Copy an EMStudio model into this emsetup (subview named after the model)."));
            connect(attachModel, &QAction::triggered, this, &MainWindow::attachEmSetupModel);
            menu->addAction(attachModel);

            QAction *viewInfo = new QAction(tr("&Info"), this);
            viewInfo->setIcon(QIcon(":/icons/info.svg"));
            viewInfo->setStatusTip(tr("Show view information."));
            connect(viewInfo, &QAction::triggered, this, &MainWindow::showViewInfo);
            menu->addAction(viewInfo);
        } else {
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

            // ROOM JSON exchange (schematic / symbol only) — Door still lists *.room.
            bool schematicOrSymbol = false;
            for(QTreeWidgetItem *it : items) {
                if(!it) {
                    continue;
                }
                const QString vn = it->text(0).trimmed().toLower();
                if(vn == QStringLiteral("schematic") || vn == QStringLiteral("symbol")) {
                    schematicOrSymbol = true;
                    break;
                }
            }
            if(schematicOrSymbol) {
                // Same *.room path; content toggles JSON ↔ Cap’n Proto (isJsonRoom).
                const QString viewPath = getViewPath(getCurrentLibraryName(), getCurrentGroupName(),
                                                     items.first()->text(0).trimmed());
                const QString absRoom = QFileInfo(viewPath).absoluteFilePath();
                if(QFileInfo::exists(absRoom)) {
                    menu->addSeparator();
#ifndef LIBMAN_NO_ROOM
                    const bool jsonContent =
                        room::RoomJsonIo::isJsonRoom(absRoom.toStdString());
#else
                    const bool jsonContent = false;
#endif
                    if(jsonContent) {
                        QAction *toRoom = new QAction(tr("Convert to &ROOM"), this);
                        toRoom->setStatusTip(
                            tr("Replace JSON content of this *.room with Cap’n Proto binary."));
                        connect(toRoom, &QAction::triggered, this,
                                &MainWindow::convertSelectedViewToRoom);
                        menu->addAction(toRoom);
                    } else {
                        QAction *toJson = new QAction(tr("Convert to &JSON"), this);
                        toJson->setStatusTip(
                            tr("Replace Cap’n Proto content of this *.room with room-json text."));
                        connect(toJson, &QAction::triggered, this,
                                &MainWindow::convertSelectedViewToJson);
                        menu->addAction(toJson);
                    }
                }
            }
        }

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
        applyViewTreeIcon(viewItem, viewName);
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    else if(viewName == "oas") {
        viewItem->setData(0, RoleType, ItemViewOas);
        viewItem->setData(0, RoleOasPath, viewPath);
        applyViewTreeIcon(viewItem, viewName);
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    else if(viewName == "lstr") {
        viewItem->setData(0, RoleType, ItemViewLStream);
        viewItem->setData(0, RoleLStreamPath, viewPath);
        applyViewTreeIcon(viewItem, viewName);
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    else if(isEmSetupViewName(viewName)) {
        configureEmSetupViewTreeItem(viewItem, viewPath);
    }
    else if(isRoomViewName(viewName)) {
        configureCoreViewTreeItem(viewItem, viewName, viewPath);
        applyCoreViewLockPresentation(viewItem, viewName, viewPath);
    }
    else {
        applyViewTreeIcon(viewItem, viewName);
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
    syncCoreLockWatches();

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
                                 "DOOR");

    if(!result.written) {
        foreach(const QString &explain, result.errors) {
            error(explain, false);
        }
        return;
    }

    registerCreatedView(libName, groupName, "lstr", viewPath);
}

/*!*********************************************************************************************************************
 * \brief Creates a new empty ROOM view file for the current cell.
 **********************************************************************************************************************/
void MainWindow::createCoreView(const QString &viewName)
{
    const QString normalizedView = viewName.trimmed().toLower();
    if (!isRoomViewName(normalizedView) || normalizedView == QStringLiteral("core")) {
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

    const QString viewPath = QDir::toNativeSeparators(roomViewFilePath(groupPath, groupName, normalizedView));
    if (QFileInfo(viewPath).exists()) {
        return;
    }

    bool asJson = false;
    if ((normalizedView == QStringLiteral("schematic")
         || normalizedView == QStringLiteral("symbol"))
        && m_properties != nullptr
        && m_properties->exists(QStringLiteral("SchematicSymbolFormat"))) {
        const QString fmt =
            m_properties->get<QString>(QStringLiteral("SchematicSymbolFormat")).trimmed().toUpper();
        asJson = (fmt == QStringLiteral("JSON"));
    }

    RoomCellReader reader(viewPath);
    reader.coreCreate(groupName, normalizedView, asJson);

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
 * \brief Creates new ROOM layout view and adds it to the list widget.
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
 * \brief Creates an empty EMStudio model under <cell>.emsetup/.
 *
 * First call creates <cell>.emsetup/nominal/model.py. If emsetup already exists (e.g. only
 * attached non-default variants), creates the next free default-style variant
 * (\c nominal, then \c nominal_2, …) so it can be renamed later.
 **********************************************************************************************************************/
void MainWindow::addNewEmSetupView()
{
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

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if (!dir.mkpath(groupPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString emRoot = emSetupDirPath(groupPath, groupName);
    const QString defaultVariant = emSetupDefaultVariantName();
    QString variant = defaultVariant;
    QString variantDir = QDir(emRoot).filePath(variant);
    if (QDir(variantDir).exists()) {
        int n = 2;
        while (n < 1000) {
            variant = QStringLiteral("%1_%2").arg(defaultVariant).arg(n);
            variantDir = QDir(emRoot).filePath(variant);
            if (!QDir(variantDir).exists()) {
                break;
            }
            ++n;
        }
        if (QDir(variantDir).exists()) {
            error(tr("Could not find a free emsetup variant name under '%1'.").arg(emRoot), false);
            return;
        }
    }

    if (!dir.mkpath(variantDir)) {
        error(QString("Failed to create emsetup directory '%1'.").arg(variantDir), false);
        return;
    }

    const QString modelPath = QDir(variantDir).filePath(QStringLiteral("model.py"));
    if (QFileInfo::exists(modelPath)) {
        error(tr("Model already exists:\n%1").arg(modelPath), false);
        return;
    }

    const QString layoutPath = preferredLayoutPathForEmSetup(libName, groupName);
    QString err;
    if (writeEmSetupModelTemplate(modelPath, groupName, layoutPath, &err).isEmpty()) {
        error(err.isEmpty() ? tr("Failed to write EM setup model.") : err, false);
        return;
    }

    // First emsetup: register the view. Further New → EM Setup: add a variant only.
    if (!findEmSetupViewItem()) {
        registerCreatedView(libName, groupName, QStringLiteral("emsetup"),
                            QDir::toNativeSeparators(emRoot));
    } else {
        const QString key = getLibraryKeyPrefix() + libName + "/" + groupName
                            + QStringLiteral("/emsetup");
        if (!m_properties->exists(key)) {
            m_properties->set(key, QDir::toNativeSeparators(emRoot));
        }
        setStateChanged();
        if (!m_currentProjFile.isEmpty() && !libmanAutomatedTestRun()) {
            saveProjectFile(m_currentProjFile);
        }
    }

    if (QTreeWidgetItem *emItem = findEmSetupViewItem()) {
        emItem->setData(0, RoleEmSetupPath, QDir::toNativeSeparators(emRoot));
        refreshEmSetupVariantItems(emItem);
        emItem->setExpanded(true);
        for (int i = 0; i < emItem->childCount(); ++i) {
            QTreeWidgetItem *child = emItem->child(i);
            if (child && child->text(0) == variant) {
                m_ui->listViews->setCurrentItem(child);
                break;
            }
        }
    }

    info(tr("Created EM setup variant: %1")
             .arg(QDir::toNativeSeparators(QDir(emRoot).filePath(variant))));
}

QString MainWindow::preferredLayoutPathForEmSetup(const QString &libName,
                                                  const QString &cellName) const
{
    const QStringList prefer{QStringLiteral("layout"), QStringLiteral("gds"),
                             QStringLiteral("oas"), QStringLiteral("lstr")};
    for (const QString &view : prefer) {
        const QString path = getViewPath(libName, cellName, view);
        if (!path.isEmpty() && QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
    return {};
}

QString MainWindow::writeEmSetupModelTemplate(const QString &modelPath,
                                              const QString &cellName,
                                              const QString &layoutPath,
                                              QString *errorMsg) const
{
    QFile f(modelPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMsg) {
            *errorMsg = tr("Cannot write model:\n%1").arg(modelPath);
        }
        return {};
    }

    // Solvers see ./<cellOrLayoutStem>.gds beside this script (RunDir = model dir);
    // layout_room is the ROOM source for EMStudio's Layout File field (converted on Run).
    QString gdsStem = cellName.trimmed();
    QString layoutRoomLine;
    if (!layoutPath.isEmpty()) {
        const QDir modelDir = QFileInfo(modelPath).absoluteDir();
        QString rel = QDir::fromNativeSeparators(modelDir.relativeFilePath(layoutPath));
        if (rel.isEmpty()) {
            rel = QDir::fromNativeSeparators(QFileInfo(layoutPath).absoluteFilePath());
        }
        rel.replace(QLatin1Char('\\'), QLatin1Char('/'));
        rel.replace(QStringLiteral("\""), QStringLiteral("\\\""));
        const QString lower = rel.toLower();
        if (lower.endsWith(QLatin1String(".layout.room")) || lower.endsWith(QLatin1String(".room"))) {
            layoutRoomLine = QStringLiteral("layout_room = \"%1\"\n").arg(rel);
            QString base = QFileInfo(layoutPath).fileName();
            if (base.endsWith(QLatin1String(".layout.room"), Qt::CaseInsensitive))
                base.chop(QStringLiteral(".layout.room").size());
            else if (base.endsWith(QLatin1String(".room"), Qt::CaseInsensitive))
                base.chop(QStringLiteral(".room").size());
            if (!base.isEmpty())
                gdsStem = base;
        } else if (lower.endsWith(QLatin1String(".gds")) || lower.endsWith(QLatin1String(".gdsii"))) {
            gdsStem = QFileInfo(layoutPath).completeBaseName();
        }
    }
    if (gdsStem.isEmpty())
        gdsStem = QFileInfo(modelPath).completeBaseName();
    gdsStem.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    const QString gdsLiteral = QStringLiteral("\"./%1.gds\"").arg(gdsStem);

    const QString cellEscaped = QString(cellName).replace(QStringLiteral("\""), QStringLiteral("\\\""));

    const QString body = QStringLiteral(
        "# EMStudio model created by DOOR (emsetup)\n"
        "# gds_filename is the companion GDS next to this model (ROOM→GDS on Run).\n"
        "\n"
        "start_simulation = False\n"
        "\n"
        "%1"
        "gds_filename = %2\n"
        "gds_cellname = \"%3\"\n"
        "cellname = \"%3\"\n"
        "XML_filename = \"\"\n"
        "\n"
        "settings = {}\n"
        "settings['unit'] = 1e-6\n"
        "settings['margin'] = 50\n"
        "settings['fstart'] = 0e9\n"
        "settings['fstop'] = 100e9\n"
        "settings['fstep'] = 2.5e9\n"
        "settings['purpose'] = [0]\n"
        "settings['preprocess_gds'] = False\n"
        "settings['start_simulation'] = False\n"
        "\n"
        "# Fill ports / stackup / solver settings in EMStudio, then Run.\n")
                             .arg(layoutRoomLine, gdsLiteral, cellEscaped);

    f.write(body.toUtf8());
    f.close();
    return QFileInfo(modelPath).absoluteFilePath();
}

QTreeWidgetItem *MainWindow::findEmSetupViewItem() const
{
    for(int i = 0; i < m_ui->listViews->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_ui->listViews->topLevelItem(i);
        if(!item) {
            continue;
        }
        const int type = item->data(0, RoleType).toInt();
        if(type == ItemViewEmSetup || isEmSetupViewName(item->text(0))) {
            return item;
        }
    }
    return nullptr;
}

void MainWindow::refreshEmSetupVariantItems(QTreeWidgetItem *emItem) const
{
    if(!emItem) {
        return;
    }
    while(emItem->childCount() > 0) {
        delete emItem->takeChild(0);
    }
    const QString emPath = emItem->data(0, RoleEmSetupPath).toString();
    if(emPath.isEmpty() || !QDir(emPath).exists()) {
        return;
    }
    const QFileInfoList variants =
        QDir(emPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for(const QFileInfo &variant : variants) {
        auto *child = new QTreeWidgetItem(emItem);
        child->setText(0, variant.fileName());
        child->setData(0, RoleType, ItemEmSetupVariant);
        child->setData(0, RoleEmSetupPath, variant.absoluteFilePath());
    }
}

bool MainWindow::copyEmSetupModelBundle(const QString &sourceModelPy,
                                        const QString &destVariantDir,
                                        QString *errorMsg) const
{
    const QFileInfo srcFi(sourceModelPy);
    if(!srcFi.isFile()) {
        if(errorMsg) {
            *errorMsg = tr("Model file not found:\n%1").arg(sourceModelPy);
        }
        return false;
    }

    QDir dest(destVariantDir);
    if(!dest.exists() && !QDir().mkpath(destVariantDir)) {
        if(errorMsg) {
            *errorMsg = tr("Cannot create directory:\n%1").arg(destVariantDir);
        }
        return false;
    }

    const QString destPy = dest.filePath(srcFi.fileName());
    if(QFileInfo::exists(destPy) && !QFile::remove(destPy)) {
        if(errorMsg) {
            *errorMsg = tr("Cannot overwrite:\n%1").arg(destPy);
        }
        return false;
    }
    if(!QFile::copy(srcFi.absoluteFilePath(), destPy)) {
        if(errorMsg) {
            *errorMsg = tr("Failed to copy model:\n%1").arg(srcFi.absoluteFilePath());
        }
        return false;
    }

    // Convenience alias for double-click open (looks for model.py first).
    if(srcFi.fileName().compare(QStringLiteral("model.py"), Qt::CaseInsensitive) != 0) {
        const QString alias = dest.filePath(QStringLiteral("model.py"));
        if(QFileInfo::exists(alias)) {
            QFile::remove(alias);
        }
        QFile::copy(destPy, alias);
    }

    // Sibling layout / stackup / local helper modules (same folder as the model).
    const QDir srcDir = srcFi.absoluteDir();
    const QStringList patterns{
        QStringLiteral("*.gds"),
        QStringLiteral("*.gdsii"),
        QStringLiteral("*.xml"),
        QStringLiteral("*.layout.room"),
        QStringLiteral("_*.py"),
    };
    for(const QString &pattern : patterns) {
        const QFileInfoList files = srcDir.entryInfoList(QStringList() << pattern, QDir::Files);
        for(const QFileInfo &fi : files) {
            if(fi.absoluteFilePath() == srcFi.absoluteFilePath()) {
                continue;
            }
            const QString target = dest.filePath(fi.fileName());
            if(QFileInfo::exists(target)) {
                QFile::remove(target);
            }
            if(!QFile::copy(fi.absoluteFilePath(), target)) {
                if(errorMsg) {
                    *errorMsg = tr("Failed to copy:\n%1").arg(fi.absoluteFilePath());
                }
                return false;
            }
        }
    }
    return true;
}

/*!*********************************************************************************************************************
 * \brief Copies an existing EMStudio model into <cell>.emsetup/<modelName>/ and refreshes the View tree.
 **********************************************************************************************************************/
void MainWindow::attachEmSetupModel()
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

    QString startDir;
    if(m_properties->exists(QStringLiteral("EmSetupAttachDir"))) {
        startDir = m_properties->get<QString>(QStringLiteral("EmSetupAttachDir"));
    }
    if(startDir.isEmpty() || !QDir(startDir).exists()) {
        startDir = QDir::homePath();
    }

    const QString sourcePy = libmanAutomatedTestRun()
        ? QString()
        : QFileDialog::getOpenFileName(this,
                                       tr("Attach EMStudio Model"),
                                       startDir,
                                       tr("Python models (*.py);;All files (*)"));
    if(sourcePy.isEmpty()) {
        return;
    }

    m_properties->set(QStringLiteral("EmSetupAttachDir"),
                      QFileInfo(sourcePy).absolutePath());

    QString variant = QFileInfo(sourcePy).completeBaseName().trimmed();
    variant.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    variant = variant.trimmed();
    if(variant.isEmpty()) {
        error(tr("Cannot derive a subview name from the model file."), false);
        return;
    }

    if(!libmanAutomatedTestRun()) {
        bool ok = false;
        variant = QInputDialog::getText(this,
                                        tr("EM Setup Subview"),
                                        tr("Subview name under emsetup:"),
                                        QLineEdit::Normal,
                                        variant,
                                        &ok).trimmed();
        if(!ok || variant.isEmpty()) {
            return;
        }
        variant.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
        if(variant.isEmpty() || variant == QLatin1String(".") || variant == QLatin1String("..")) {
            error(tr("Invalid subview name."), false);
            return;
        }
    }

    const QString groupPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    if(!QDir().mkpath(groupPath)) {
        error(tr("Failed to create cell directory '%1'.").arg(groupPath), false);
        return;
    }

    const QString emRoot = emSetupDirPath(groupPath, groupName);
    if(!QDir().mkpath(emRoot)) {
        error(tr("Failed to create emsetup directory '%1'.").arg(emRoot), false);
        return;
    }

    const QString variantDir = QDir(emRoot).filePath(variant);
    if(QDir(variantDir).exists()) {
        if(libmanAutomatedTestRun()) {
            QDir(variantDir).removeRecursively();
        } else {
            const auto reply = QMessageBox::question(
                this,
                tr("Attach Model"),
                tr("Subview '%1' already exists. Replace its contents?").arg(variant),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
            if(reply != QMessageBox::Yes) {
                return;
            }
            if(!QDir(variantDir).removeRecursively()) {
                error(tr("Failed to clear existing subview:\n%1").arg(variantDir), false);
                return;
            }
        }
    }

    QString err;
    if(!copyEmSetupModelBundle(sourcePy, variantDir, &err)) {
        error(err.isEmpty() ? tr("Failed to copy model.") : err, false);
        return;
    }

    // Register emsetup view if this is the first attach / create.
    if(!findEmSetupViewItem()) {
        registerCreatedView(libName, groupName, QStringLiteral("emsetup"),
                            QDir::toNativeSeparators(emRoot));
    } else {
        const QString key = getLibraryKeyPrefix() + libName + "/" + groupName
                            + QStringLiteral("/emsetup");
        if(!m_properties->exists(key)) {
            m_properties->set(key, QDir::toNativeSeparators(emRoot));
        }
        setStateChanged();
        if(!m_currentProjFile.isEmpty() && !libmanAutomatedTestRun()) {
            saveProjectFile(m_currentProjFile);
        }
    }

    if(QTreeWidgetItem *emItem = findEmSetupViewItem()) {
        emItem->setData(0, RoleEmSetupPath, QDir::toNativeSeparators(emRoot));
        refreshEmSetupVariantItems(emItem);
        emItem->setExpanded(true);
        // Select the new / updated variant.
        for(int i = 0; i < emItem->childCount(); ++i) {
            QTreeWidgetItem *child = emItem->child(i);
            if(child && child->text(0) == variant) {
                m_ui->listViews->setCurrentItem(child);
                break;
            }
        }
    }

    info(tr("Attached model '%1' as emsetup/%2")
             .arg(QFileInfo(sourcePy).fileName(), variant));
}

namespace {

QString sanitizeEmSetupVariantName(QString name)
{
    name = name.trimmed();
    name.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    if (name == QLatin1String(".") || name == QLatin1String("..")) {
        name.clear();
    }
    return name;
}

} // namespace

/*!*********************************************************************************************************************
 * \brief Asks for a target library and cell (combo boxes filled from the open project).
 **********************************************************************************************************************/
bool MainWindow::pickTargetLibraryAndCell(QString *libOut, QString *cellOut, const QString &dialogTitle)
{
    if (!libOut || !cellOut) {
        return false;
    }

    const QStringList libs = registeredLibraryNames();
    if (libs.isEmpty()) {
        error(tr("No libraries are loaded in the current project."), false);
        return false;
    }

    int libIndex = libs.indexOf(getCurrentLibraryName());
    if (libIndex < 0) {
        libIndex = 0;
    }

    bool ok = false;
    const QString lib = QInputDialog::getItem(this,
                                              dialogTitle,
                                              tr("Library:"),
                                              libs,
                                              libIndex,
                                              false,
                                              &ok);
    if (!ok || lib.isEmpty()) {
        return false;
    }

    QStringList cells = getCurrentGroups(lib);
    if (cells.isEmpty()) {
        error(tr("Library '%1' has no cells.").arg(lib), false);
        return false;
    }

    int cellIndex = 0;
    if (lib == getCurrentLibraryName()) {
        const int idx = cells.indexOf(getCurrentGroupName());
        if (idx >= 0) {
            cellIndex = idx;
        }
    }

    const QString cell = QInputDialog::getItem(this,
                                               dialogTitle,
                                               tr("Cell:"),
                                               cells,
                                               cellIndex,
                                               false,
                                               &ok);
    if (!ok || cell.isEmpty()) {
        return false;
    }

    *libOut = lib;
    *cellOut = cell;
    return true;
}

/*!*********************************************************************************************************************
 * \brief Registers an emsetup view path for \p libName/\p cellName when missing.
 **********************************************************************************************************************/
bool MainWindow::ensureEmSetupViewRegistered(const QString &libName,
                                             const QString &cellName,
                                             const QString &emRoot)
{
    if (libName.isEmpty() || cellName.isEmpty() || emRoot.isEmpty() || !QDir(emRoot).exists()) {
        return false;
    }

    const QString key = getLibraryKeyPrefix() + libName + "/" + cellName + QStringLiteral("/emsetup");
    if (!m_properties->exists(key)) {
        m_properties->set(key, QDir::toNativeSeparators(emRoot));
        setStateChanged();
        if (!m_currentProjFile.isEmpty() && !libmanAutomatedTestRun()) {
            saveProjectFile(m_currentProjFile);
        }
    }

    if (libName == getCurrentLibraryName() && cellName == getCurrentGroupName()) {
        if (!findEmSetupViewItem()) {
            registerCreatedView(libName, cellName, QStringLiteral("emsetup"),
                                QDir::toNativeSeparators(emRoot));
        } else if (QTreeWidgetItem *emItem = findEmSetupViewItem()) {
            emItem->setData(0, RoleEmSetupPath, QDir::toNativeSeparators(emRoot));
            refreshEmSetupVariantItems(emItem);
            emItem->setExpanded(true);
        }
    }
    return true;
}

/*!*********************************************************************************************************************
 * \brief Returns \p preferred if free under \p emRoot, otherwise preferred_2, preferred_3, …
 **********************************************************************************************************************/
QString MainWindow::uniqueEmSetupVariantName(const QString &emRoot, const QString &preferred) const
{
    QString base = sanitizeEmSetupVariantName(preferred);
    if (base.isEmpty()) {
        base = emSetupDefaultVariantName();
    }
    QString name = base;
    int n = 2;
    while (QDir(QDir(emRoot).filePath(name)).exists() && n < 1000) {
        name = QStringLiteral("%1_%2").arg(base).arg(n);
        ++n;
    }
    return name;
}

/*!*********************************************************************************************************************
 * \brief Renames the selected emsetup variant folder on disk and refreshes the View tree.
 **********************************************************************************************************************/
void MainWindow::renameEmSetupVariant()
{
    QList<QTreeWidgetItem *> items = m_ui->listViews->selectedItems();
    if (items.count() != 1 || !items.first()) {
        return;
    }
    QTreeWidgetItem *item = items.first();
    const int type = item->data(0, RoleType).toInt();
    QTreeWidgetItem *parent = item->parent();
    const bool isVariant = type == ItemEmSetupVariant
            || (parent
                && (parent->data(0, RoleType).toInt() == ItemViewEmSetup
                    || isEmSetupViewName(parent->text(0))));
    if (!isVariant) {
        return;
    }

    const QString oldPath = item->data(0, RoleEmSetupPath).toString();
    if (oldPath.isEmpty() || !QDir(oldPath).exists()) {
        error(tr("emsetup variant folder not found."), false);
        return;
    }

    bool ok = false;
    QString newName = QInputDialog::getText(this,
                                            tr("Rename EM Setup Variant"),
                                            tr("New name:"),
                                            QLineEdit::Normal,
                                            item->text(0),
                                            &ok);
    if (!ok) {
        return;
    }
    newName = sanitizeEmSetupVariantName(newName);
    if (newName.isEmpty()) {
        error(tr("Invalid variant name."), false);
        return;
    }
    if (newName == item->text(0)) {
        return;
    }

    const QString newPath = QDir(QFileInfo(oldPath).absolutePath()).filePath(newName);
    if (QDir(newPath).exists()) {
        error(tr("A variant named '%1' already exists.").arg(newName), false);
        return;
    }
    if (!QDir().rename(oldPath, newPath)) {
        error(tr("Failed to rename:\n%1\nto\n%2").arg(oldPath, newPath), false);
        return;
    }

    item->setText(0, newName);
    item->setData(0, RoleEmSetupPath, QFileInfo(newPath).absoluteFilePath());
    setStateChanged();
    info(tr("Renamed emsetup variant to '%1'.").arg(newName));
}

/*!*********************************************************************************************************************
 * \brief Copies selected emsetup root or variant into a chosen library/cell.
 *
 * Root copy asks for library + cell and copies the whole <cell>.emsetup folder.
 * Variant copy asks for library + cell and places the variant under that cell's emsetup.
 **********************************************************************************************************************/
void MainWindow::copyEmSetupSelection()
{
    QList<QTreeWidgetItem *> items = m_ui->listViews->selectedItems();
    if (items.isEmpty()) {
        return;
    }

    bool rootSelected = false;
    QList<QTreeWidgetItem *> variants;
    for (QTreeWidgetItem *it : items) {
        if (!it) {
            continue;
        }
        const int type = it->data(0, RoleType).toInt();
        QTreeWidgetItem *parent = it->parent();
        const bool underEm = parent
                && (parent->data(0, RoleType).toInt() == ItemViewEmSetup
                    || isEmSetupViewName(parent->text(0)));
        if (type == ItemEmSetupVariant || underEm) {
            variants << it;
        } else if (type == ItemViewEmSetup || isEmSetupViewName(it->text(0))) {
            rootSelected = true;
        }
    }

    QString targetLib;
    QString targetCell;
    const QString title = rootSelected && variants.isEmpty()
            ? tr("Copy EM Setup")
            : tr("Copy EM Setup Variant");
    if (!pickTargetLibraryAndCell(&targetLib, &targetCell, title)) {
        return;
    }

    const QString libRoot = getLibraryPath(targetLib);
    if (libRoot.isEmpty() || !QFileInfo(libRoot).exists()) {
        error(tr("Library path not found for '%1'.").arg(targetLib), false);
        return;
    }

    const QString cellPath = QDir::toNativeSeparators(libRoot + "/" + targetCell);
    QDir().mkpath(cellPath);
    const QString destEmRoot = emSetupDirPath(cellPath, targetCell);

    if (rootSelected && variants.isEmpty()) {
        QTreeWidgetItem *rootItem = items.first();
        const QString srcRoot = rootItem->data(0, RoleEmSetupPath).toString();
        if (srcRoot.isEmpty() || !QDir(srcRoot).exists()) {
            error(tr("Source emsetup folder not found."), false);
            return;
        }
        if (QFileInfo(srcRoot).absoluteFilePath() == QFileInfo(destEmRoot).absoluteFilePath()) {
            info(tr("Source and destination emsetup are the same."));
            return;
        }
        if (QDir(destEmRoot).exists()) {
            if (!askUserForAction(tr("Destination already has an emsetup. Merge/overwrite variants?"))) {
                return;
            }
        } else if (!QDir().mkpath(destEmRoot)) {
            error(tr("Failed to create '%1'.").arg(destEmRoot), false);
            return;
        }

        const QFileInfoList srcVariants =
            QDir(srcRoot).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &fi : srcVariants) {
            const QString destVariant = QDir(destEmRoot).filePath(fi.fileName());
            if (QDir(destVariant).exists()) {
                QDir(destVariant).removeRecursively();
            }
            copyDir(fi.absoluteFilePath(), destVariant);
        }
        ensureEmSetupViewRegistered(targetLib, targetCell, destEmRoot);
        info(tr("Copied emsetup to %1/%2").arg(targetLib, targetCell));
        return;
    }

    if (variants.isEmpty()) {
        return;
    }

    if (!QDir(destEmRoot).exists() && !QDir().mkpath(destEmRoot)) {
        error(tr("Failed to create '%1'.").arg(destEmRoot), false);
        return;
    }

    for (QTreeWidgetItem *item : variants) {
        if (!item) {
            continue;
        }
        const QString srcPath = item->data(0, RoleEmSetupPath).toString();
        if (srcPath.isEmpty() || !QDir(srcPath).exists()) {
            error(tr("Variant folder not found:\n%1").arg(srcPath), false);
            continue;
        }

        QString destName = uniqueEmSetupVariantName(destEmRoot, item->text(0));
        if (destName != item->text(0)
            || QDir(QDir(destEmRoot).filePath(item->text(0))).exists()) {
            bool ok = false;
            QString asked = QInputDialog::getText(this,
                                                  tr("Copy EM Setup Variant"),
                                                  tr("Name under %1/%2:").arg(targetLib, targetCell),
                                                  QLineEdit::Normal,
                                                  destName,
                                                  &ok);
            if (!ok) {
                return;
            }
            asked = sanitizeEmSetupVariantName(asked);
            if (asked.isEmpty()) {
                error(tr("Invalid variant name."), false);
                return;
            }
            destName = asked;
        }

        const QString destPath = QDir(destEmRoot).filePath(destName);
        if (QDir(destPath).exists()) {
            if (!askUserForAction(tr("Replace existing variant '%1'?").arg(destName))) {
                continue;
            }
            QDir(destPath).removeRecursively();
        }
        copyDir(srcPath, destPath);
        if (!QDir(destPath).exists()) {
            error(tr("Failed to copy variant to '%1'.").arg(destPath), false);
            continue;
        }
        info(tr("Copied emsetup/%1 to %2/%3").arg(destName, targetLib, targetCell));
    }

    ensureEmSetupViewRegistered(targetLib, targetCell, destEmRoot);
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

    // emsetup/<variant> children vs top-level views (emsetup root is a directory).
    QList<QTreeWidgetItem *> variantItems;
    QStringList viewNames;
    bool emSetupRootSelected = false;
    for(QTreeWidgetItem *item : items) {
        if(!item) {
            continue;
        }
        const int type = item->data(0, RoleType).toInt();
        QTreeWidgetItem *parent = item->parent();
        const bool underEmSetup = parent
                && (parent->data(0, RoleType).toInt() == ItemViewEmSetup
                    || isEmSetupViewName(parent->text(0)));
        if(type == ItemEmSetupVariant || underEmSetup) {
            if(!emSetupRootSelected) {
                variantItems << item;
            }
            continue;
        }
        if(parent) {
            continue;
        }
        const QString viewName = item->text(0).trimmed();
        if(viewName.isEmpty()) {
            continue;
        }
        if(isEmSetupViewName(viewName) || type == ItemViewEmSetup) {
            emSetupRootSelected = true;
            variantItems.clear(); // whole emsetup folder goes away
        }
        if(!viewNames.contains(viewName)) {
            viewNames << viewName;
        }
    }

    if(viewNames.isEmpty() && variantItems.isEmpty()) {
        return;
    }

    bool deleteFiles = false;
    if(!promptDeleteChoice(&deleteFiles)) {
        return;
    }

    bool changed = false;

    // Subviews under emsetup (nominal, palace_cmim, …): delete the variant folder / tree row.
    for(QTreeWidgetItem *item : variantItems) {
        if(!item) {
            continue;
        }
        const QString path = item->data(0, RoleEmSetupPath).toString();
        if(deleteFiles && !path.isEmpty() && QFileInfo::exists(path)) {
            info(tr("Removing emsetup variant '%1'").arg(path));
            const QFileInfo fi(path);
            if(fi.isDir()) {
                QDir(path).removeRecursively();
            } else {
                QFile::remove(path);
            }
        }
        if(QTreeWidgetItem *parent = item->parent()) {
            delete parent->takeChild(parent->indexOfChild(item));
        }
        changed = true;
    }

    for(const QString &viewName : viewNames) {
        const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
        const QString viewPath = getViewPath(libName, groupName, viewName);

        if(deleteFiles && !viewPath.isEmpty() && QFileInfo(viewPath).exists()) {
            info(QString("Removing view '%1'").arg(viewPath));
            const QFileInfo fi(viewPath);
            if(fi.isDir()) {
                QDir(viewPath).removeRecursively();
            } else {
                QFile::remove(viewPath);
            }
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

    const QString extra = isRoomViewName(viewName) ? coreLockInfoExtraLines(viewPath) : QString();
    showFolderInfo("View", viewName, viewPath, true, extra);
}

QString MainWindow::siblingRoomJsonPath(const QString &roomPath)
{
    QString path = QFileInfo(roomPath).absoluteFilePath();
    if(path.endsWith(QStringLiteral(".room"), Qt::CaseInsensitive)) {
        path.chop(5);
        return path + QStringLiteral(".json");
    }
    if(path.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
        return path;
    }
    return path + QStringLiteral(".json");
}

bool MainWindow::convertRoomViewToJson(const QString &roomPath)
{
#ifndef LIBMAN_NO_ROOM
    const QString absRoom = QFileInfo(roomPath).absoluteFilePath();
    if(absRoom.isEmpty() || !QFileInfo::exists(absRoom)) {
        error(tr("ROOM file not found: %1").arg(roomPath), false);
        return false;
    }
    const room::RoomJsonIo::Result result =
        room::RoomJsonIo::convertInPlaceToJson(absRoom.toStdString());
    if(!result.ok) {
        error(tr("Convert to JSON failed: %1").arg(QString::fromStdString(result.message)), false);
        return false;
    }
    // Drop legacy sibling *.json if present (format now lives inside *.room).
    const QString legacyJson = siblingRoomJsonPath(absRoom);
    if(legacyJson.compare(absRoom, Qt::CaseInsensitive) != 0 && QFileInfo::exists(legacyJson)) {
        QFile::remove(legacyJson);
    }
    info(tr("Wrote JSON into %1").arg(QDir::toNativeSeparators(absRoom)), false);
    return true;
#else
    Q_UNUSED(roomPath);
    error(tr("Convert to JSON requires ROOM support."), false);
    return false;
#endif
}

bool MainWindow::convertJsonSiblingToRoom(const QString &roomPath)
{
#ifndef LIBMAN_NO_ROOM
    const QString absRoom = QFileInfo(roomPath).absoluteFilePath();
    if(absRoom.isEmpty() || !QFileInfo::exists(absRoom)) {
        error(tr("ROOM file not found: %1").arg(roomPath), false);
        return false;
    }
    if(!room::RoomJsonIo::isJsonRoom(absRoom.toStdString())) {
        error(tr("File is already binary ROOM: %1").arg(QDir::toNativeSeparators(absRoom)), false);
        return false;
    }
    const room::RoomJsonIo::Result result =
        room::RoomJsonIo::convertInPlaceToBinary(absRoom.toStdString());
    if(!result.ok) {
        error(tr("Convert to ROOM failed: %1").arg(QString::fromStdString(result.message)), false);
        return false;
    }
    info(tr("Wrote binary ROOM into %1").arg(QDir::toNativeSeparators(absRoom)), false);
    return true;
#else
    Q_UNUSED(roomPath);
    error(tr("Convert to ROOM requires ROOM support."), false);
    return false;
#endif
}

bool MainWindow::ensureWorkingRoomFromJson(const QString &roomPath)
{
    // Editors load JSON-in-*.room via RoomJsonIo::isJsonRoom / loadRoomOrJson.
    // No convert-before-launch; path name stays *.room either way.
    return QFileInfo::exists(roomPath);
}

void MainWindow::convertSelectedViewToJson()
{
    const QString libName = getCurrentLibraryName();
    const QString groupName = getCurrentGroupName();
    const QString viewName = getCurrentViewName();
    if(libName.isEmpty() || groupName.isEmpty() || viewName.isEmpty()) {
        return;
    }
    if(!isSchematicLikeView(viewName)) {
        error(tr("Convert to JSON is only for schematic/symbol views."), false);
        return;
    }
    const QString viewPath = getViewPath(libName, groupName, viewName);
    convertRoomViewToJson(viewPath);
}

void MainWindow::convertSelectedViewToRoom()
{
    const QString libName = getCurrentLibraryName();
    const QString groupName = getCurrentGroupName();
    const QString viewName = getCurrentViewName();
    if(libName.isEmpty() || groupName.isEmpty() || viewName.isEmpty()) {
        return;
    }
    if(!isSchematicLikeView(viewName)) {
        error(tr("Convert to ROOM is only for schematic/symbol views."), false);
        return;
    }
    const QString viewPath = getViewPath(libName, groupName, viewName);
    convertJsonSiblingToRoom(viewPath);
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
    else if(type == ItemViewCore && isLayoutRoomViewName(item->text(0))) {
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
    const QStringList parts = tool.trimmed().split(QLatin1Char(' '), Qt::SkipEmptyParts);
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

    m_klayoutTools.startXorProcess(program,
                                   args,
                                   scriptPath,
                                   outputPath,
                                   m_xorFirst.klayoutPath,
                                   m_xorFirst.cellName);
    m_xorFirst = XorSelection();
}
