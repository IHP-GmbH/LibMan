#include <QMenu>
#include <QFile>
#include <QScreen>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QGuiApplication>
#include <QListWidgetItem>
#include <QSet>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "property.h"
#include "gds/gdsreader.h"
#include "oas/oasReader.h"

/*!*********************************************************************************************************************
 * \brief Displays menu for group (cell) widget.
 * \param pos       Point(x, y) where menu will be displayed.
 **********************************************************************************************************************/
void MainWindow::showGroupMenu(const QPoint &pos)
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libPath = getLibraryPath(libName);
    if(libPath.isEmpty() || !QFileInfo(libPath).exists()) {
        return;
    }

    QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&event);

    QMenu *menu = new QMenu(this);

    QAction *group = new QAction(tr("&Add New..."), this);
    group->setIcon(QIcon(":/icons/view.svg"));
    group->setStatusTip(tr("Add new cell."));
    connect(group, &QAction::triggered, this, &MainWindow::addNewGroup);
    menu->addAction(group);

    if(isGroupCopied() && m_copyData.count() > 1) {
        const QString groupName = m_copyData[0];
        if(!groupName.isEmpty()) {
            bool addPasteMenu = true;

            for(int i = 0; i < m_ui->listGroups->count(); ++i) {
                QListWidgetItem *item = m_ui->listGroups->item(i);
                if(item && item->text() == groupName) {
                    addPasteMenu = false;
                    break;
                }
            }

            if(addPasteMenu) {
                QAction *pasteGroup = new QAction(tr("&Paste"), this);
                pasteGroup->setIcon(QIcon(":/icons/paste.svg"));
                pasteGroup->setShortcut(QKeySequence::Paste);
                pasteGroup->setStatusTip(tr("Paste cell."));
                pasteGroup->setShortcutContext(Qt::WidgetShortcut);
                connect(pasteGroup, &QAction::triggered, this, &MainWindow::pasteSelectedData);
                menu->addAction(pasteGroup);
                addAction(pasteGroup);
            }
        }
    }

    QList<QListWidgetItem *> items = m_ui->listGroups->selectedItems();
    if(items.count()) {
        menu->addSeparator();

        QAction *copyGroup = new QAction(tr("&Copy"), this);
        copyGroup->setIcon(QIcon(":/icons/copy.svg"));
        copyGroup->setShortcut(QKeySequence::Copy);
        copyGroup->setStatusTip(tr("Copy cell."));
        copyGroup->setShortcutContext(Qt::WidgetShortcut);
        connect(copyGroup, &QAction::triggered, this, &MainWindow::copySelectedGroup);
        menu->addAction(copyGroup);
        addAction(copyGroup);

        QAction *delGroup = new QAction(tr("&Delete"), this);
        delGroup->setIcon(QIcon(":/icons/delete.svg"));
        delGroup->setShortcut(QKeySequence::Delete);
        delGroup->setStatusTip(tr("Delete cell."));
        delGroup->setShortcutContext(Qt::WidgetShortcut);
        connect(delGroup, &QAction::triggered, this, &MainWindow::removeSelectedGroup);
        menu->addAction(delGroup);
        addAction(delGroup);

        QAction *groupInfo = new QAction(tr("&Info"), this);
        groupInfo->setIcon(QIcon(":/icons/info.svg"));
        groupInfo->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Return));
        groupInfo->setStatusTip(tr("Show cell information."));
        groupInfo->setShortcutContext(Qt::WidgetShortcut);
        connect(groupInfo, &QAction::triggered, this, &MainWindow::showGroupInfo);
        menu->addAction(groupInfo);
        addAction(groupInfo);
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
 * \brief Creates new group (cell) and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewGroup()
{
    QListWidgetItem *groupId = new QListWidgetItem;
    groupId->setText("CellName");
    groupId->setFlags(groupId->flags() | Qt::ItemIsEditable);
    m_ui->listGroups->addItem(groupId);
    m_ui->listGroups->sortItems();
    m_ui->listViews->clear();
    groupId->setSelected(true);
    updateLibraryActionStates();
}

/*!*********************************************************************************************************************
 * \brief Removes selected group (cell).
 **********************************************************************************************************************/
void MainWindow::removeSelectedGroup()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QString libPath = getLibraryPath(libName);
    if(libPath.isEmpty() || !QFileInfo(libPath).isDir()) {
        return;
    }

    QList<QListWidgetItem *> items = m_ui->listGroups->selectedItems();
    if(!items.count()) {
        return;
    }

    bool deleteGroup = false;
    if(!promptDeleteChoice(&deleteGroup)) {
        return;
    }

    QSet<QString> deletedCells;
    for(QListWidgetItem *item : items) {
        if(!item) {
            continue;
        }

        const QString groupName = item->text().trimmed();
        if(groupName.isEmpty()) {
            continue;
        }

        if(deleteGroup) {
            const QString cellDir = QDir::toNativeSeparators(libPath + "/" + groupName);
            if(QFileInfo(cellDir).isDir()) {
                info(QString("Removing cell '%1'").arg(cellDir), false);
                removeDir(cellDir);
            }
            else {
                const QStringList views = getCurrentViews(libName, groupName);
                foreach(const QString &viewName, views) {
                    const QString viewPath = getViewPath(libName, groupName, viewName);
                    if(QFileInfo(viewPath).exists()) {
                        info(QString("Removing view '%1'").arg(viewPath), false);
                        QFile::remove(viewPath);
                    }
                }
            }
        }

        removeCellPropertyKeys(libName, groupName);
        deletedCells.insert(groupName);
    }

    for(int j = m_ui->listGroups->count() - 1; j >= 0; --j) {
        QListWidgetItem *item = m_ui->listGroups->item(j);
        if(item && deletedCells.contains(item->text())) {
            delete m_ui->listGroups->takeItem(j);
        }
    }

    if(!m_itemText.isEmpty() && deletedCells.contains(m_itemText)) {
        m_ui->listViews->clear();
        m_itemText.clear();
    }

    setStateChanged();
    if(!m_currentProjFile.isEmpty()) {
        saveProjectFile(m_currentProjFile);
    }
}

/*!*********************************************************************************************************************
 * \brief Prints group folder Unix information into the MainWindow output window.
 **********************************************************************************************************************/
void MainWindow::showGroupInfo()
{
    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QStringList views = getCurrentViews(libPath, groupName);

    foreach(const QString &viewName, views) {
        QString groupPath = QDir::toNativeSeparators(libPath + "/" + viewName + "/" + groupName + "." + viewName);
        showFolderInfo("Cell", groupName, groupPath, false);
    }
}

/*!*********************************************************************************************************************
 * \brief Adds selected group to the buffer for coping.
 **********************************************************************************************************************/
void MainWindow::copySelectedGroup()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString guiGroupName = getCurrentGroupName();
    if(guiGroupName.isEmpty()) {
        return;
    }

    QStringList groups = getCurrentGroups(libPath);

    if(groups.count()) {
        m_copyData.clear();
        addGroupToBeCopied(guiGroupName, libPath);
    }
}

/*!*********************************************************************************************************************
 * \brief Adds group to the buffer for later coping.
 * \param groupName     Name of the group to copy.
 * \param groupPath     Path of the group from where to copy.
 **********************************************************************************************************************/
void MainWindow::addGroupToBeCopied(const QString &groupName, const QString &groupPath)
{
    m_copyData<<groupName<<groupPath;
    m_currentCopyState = GROUP;
}

/*!*******************************************************************************************************************
 * \brief Handles expansion of a view item in the view tree widget.
 *
 * This slot is triggered when a tree item is expanded by the user.
 * It lazily loads GDS hierarchy for the "gds" view:
 *  - parses the GDS file only on first expansion,
 *  - inserts top-level GDS cells as child items,
 *  - marks cells with children as expandable.
 *
 * Non-GDS items and already populated items are ignored.
 *
 * \param item     Pointer to the expanded tree widget item.
 **********************************************************************************************************************/
void MainWindow::on_viewItemExpanded(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }

    const int type = item->data(0, RoleType).toInt();

    // ------------------------------------------------------------
    // GDS root ("gds")
    // ------------------------------------------------------------
    if (item->text(0) == "gds") {

        if (item->childCount() > 0) {
            return;
        }

        const QString gdsPath = item->data(0, RoleGdsPath).toString();
        if (gdsPath.isEmpty()) {
            return;
        }

        auto entry = ensureGdsLoaded(gdsPath);

        if (entry->loaded) {
            populateGdsTopLevel(item, entry);
            return;
        }

        if (entry->loading) {
            return;
        }

        loadGdsHierarchyAsync(entry->path, entry, item);
        return;
    }

    // ------------------------------------------------------------
    // OAS root ("oas"/"oasis")
    // ------------------------------------------------------------
    if (item->text(0) == "oas" || item->text(0) == "oasis") {

        if (item->childCount() > 0) {
            return;
        }

        const QString oasPath = item->data(0, RoleOasPath).toString();
        if (oasPath.isEmpty()) {
            return;
        }

        auto entry = ensureOasLoaded(oasPath);

        if (entry->loaded) {
            populateOasTopLevel(item, entry);
            return;
        }

        if (entry->loading) {
            return;
        }

        loadOasHierarchyAsync(entry->path, entry, item);
        return;
    }

    // ------------------------------------------------------------
    // LStream root ("lstr")
    // ------------------------------------------------------------
    if (item->text(0) == "lstr") {

        if (item->childCount() > 0) {
            return;
        }

        const QString path = item->data(0, RoleLStreamPath).toString();
        if (path.isEmpty()) {
            return;
        }

        auto entry = ensureLStreamLoaded(path);

        if (entry->loaded) {
            populateLStreamTopLevel(item, entry);
            return;
        }

        if (entry->loading) {
            return;
        }

        loadLStreamAsync(path, entry, item);
        return;
    }

    // ------------------------------------------------------------
    // CORE root (layout / schematic / symbol)
    // ------------------------------------------------------------
    if (type == ItemViewCore && !item->parent()) {

        if (item->childCount() > 0) {
            return;
        }

        const QString corePath = item->data(0, RoleCorePath).toString();
        if (corePath.isEmpty()) {
            return;
        }

        auto entry = ensureCoreLoaded(corePath);

        if (entry->loaded) {
            populateCoreTopLevel(item, entry);
            return;
        }

        if (entry->loading) {
            return;
        }

        loadCoreHierarchyAsync(entry->path, entry, item);
        return;
    }

    // ------------------------------------------------------------
    // Cell node (GDS, OAS, or CORE)
    // ------------------------------------------------------------
    if (type == ItemCell) {

        if (item->childCount() > 0) {
            return;
        }

        const QString cellName = item->data(0, RoleCellName).toString();
        if (cellName.isEmpty()) {
            return;
        }

        // Prefer GDS if path is present
        const QString gdsPath = item->data(0, RoleGdsPath).toString();
        if (!gdsPath.isEmpty()) {

            auto entry = ensureGdsLoaded(gdsPath);

            if (entry->loaded) {
                populateCellChildren(item, entry, cellName);
                return;
            }

            if (entry->loading) {
                return;
            }

            loadGdsHierarchyAsync(entry->path, entry, item, cellName);
            return;
        }

        // Otherwise OAS if path is present
        const QString oasPath = item->data(0, RoleOasPath).toString();
        if (!oasPath.isEmpty()) {

            auto entry = ensureOasLoaded(oasPath);

            if (entry->loaded) {
                populateOasCellChildren(item, entry, cellName);
                return;
            }

            if (entry->loading) {
                return;
            }

            loadOasHierarchyAsync(entry->path, entry, item, cellName);
            return;
        }

        const QString corePath = item->data(0, RoleCorePath).toString();
        if (!corePath.isEmpty()) {

            auto entry = ensureCoreLoaded(corePath);

            if (entry->loaded) {
                populateCoreCellChildren(item, entry, cellName);
                return;
            }

            if (entry->loading) {
                return;
            }

            loadCoreHierarchyAsync(entry->path, entry, item, cellName);
            return;
        }

        return;
    }
}

void MainWindow::populateCellChildren(QTreeWidgetItem *cellItem,
                                      const std::shared_ptr<GdsCacheEntry> &entry,
                                      const QString &cellName)
{
    if (!cellItem || !entry) {
        return;
    }

    if (cellItem->childCount() > 0) {
        return;
    }

    const auto it = entry->hierarchy.children.find(cellName);
    if (it == entry->hierarchy.children.end()) {
        return;
    }

    const QStringList childs = it.value();
    for (const QString &ch : childs) {
        auto *chItem = new QTreeWidgetItem(cellItem);
        chItem->setText(0, ch);
        chItem->setData(0, RoleType, ItemCell);
        chItem->setData(0, RoleCellName, ch);
        chItem->setData(0, RoleGdsPath, entry->path);

        const auto it2 = entry->hierarchy.children.find(ch);
        if (it2 != entry->hierarchy.children.end() && !it2.value().isEmpty()) {
            chItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }
}

std::shared_ptr<MainWindow::GdsCacheEntry> MainWindow::ensureGdsLoaded(const QString &gdsPath)
{
    const QString key = QFileInfo(gdsPath).absoluteFilePath();

    auto it = m_gdsCache.find(key);
    if (it != m_gdsCache.end()) {
        return it.value();
    }

    auto entry = std::make_shared<GdsCacheEntry>();
    entry->path = key;

    m_gdsCache.insert(key, entry);
    return entry;
}

void MainWindow::populateGdsTopLevel(QTreeWidgetItem *gdsItem,
                                     const std::shared_ptr<GdsCacheEntry> &entry)
{
    if (!gdsItem || !entry) {
        return;
    }

    if (gdsItem->childCount() > 0) {
        return;
    }

    for (const QString &topCell : entry->hierarchy.topCells) {
        auto *cellItem = new QTreeWidgetItem(gdsItem);
        cellItem->setText(0, topCell);
        cellItem->setData(0, RoleType, ItemCell);
        cellItem->setData(0, RoleCellName, topCell);
        cellItem->setData(0, RoleGdsPath, entry->path);

        const auto it = entry->hierarchy.children.find(topCell);
        if (it != entry->hierarchy.children.end() && !it.value().isEmpty()) {
            cellItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }
}

std::shared_ptr<MainWindow::CoreCacheEntry> MainWindow::ensureCoreLoaded(const QString &corePath)
{
    const QString key = QFileInfo(corePath).absoluteFilePath();

    auto it = m_coreCache.find(key);
    if (it != m_coreCache.end()) {
        return it.value();
    }

    auto entry = std::make_shared<CoreCacheEntry>();
    entry->path = key;

    m_coreCache.insert(key, entry);
    return entry;
}

void MainWindow::populateCoreTopLevel(QTreeWidgetItem *coreItem,
                                      const std::shared_ptr<CoreCacheEntry> &entry)
{
    if (!coreItem || !entry) {
        return;
    }

    if (coreItem->childCount() > 0) {
        return;
    }

    auto addCoreCellItem = [&](const QString &cellName) {
        auto *cellItem = new QTreeWidgetItem(coreItem);
        cellItem->setText(0, cellName);
        cellItem->setData(0, RoleType, ItemCell);
        cellItem->setData(0, RoleCellName, cellName);
        cellItem->setData(0, RoleCorePath, entry->path);

        const auto childIt = entry->hierarchy.children.find(cellName);
        if (childIt != entry->hierarchy.children.end() && !childIt.value().isEmpty()) {
            cellItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    };

    const QStringList topCells = entry->hierarchy.topCells;

    // Single wrapper top cell (e.g. library root) — show its children directly.
    if (topCells.size() == 1) {
        const QString topCell = topCells.first();
        const auto topChildren = entry->hierarchy.children.find(topCell);
        if (topChildren != entry->hierarchy.children.end() && !topChildren.value().isEmpty()) {
            for (const QString &childCell : topChildren.value()) {
                addCoreCellItem(childCell);
            }
            return;
        }
    }

    for (const QString &topCell : topCells) {
        addCoreCellItem(topCell);
    }
}

void MainWindow::populateCoreCellChildren(QTreeWidgetItem *cellItem,
                                          const std::shared_ptr<CoreCacheEntry> &entry,
                                          const QString &cellName)
{
    if (!cellItem || !entry) {
        return;
    }

    if (cellItem->childCount() > 0) {
        return;
    }

    const auto it = entry->hierarchy.children.find(cellName);
    if (it == entry->hierarchy.children.end()) {
        return;
    }

    const QStringList childs = it.value();
    for (const QString &ch : childs) {
        auto *chItem = new QTreeWidgetItem(cellItem);
        chItem->setText(0, ch);
        chItem->setData(0, RoleType, ItemCell);
        chItem->setData(0, RoleCellName, ch);
        chItem->setData(0, RoleCorePath, entry->path);

        const auto childIt = entry->hierarchy.children.find(ch);
        if (childIt != entry->hierarchy.children.end() && !childIt.value().isEmpty()) {
            chItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }
}
