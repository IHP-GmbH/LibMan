#include <QMenu>
#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QGuiApplication>
#include <QListWidgetItem>

#if QT_VERSION >= 0x050000
#include <QScreen>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "property.h"

/*!******************************************************************************************************************
 * \brief Deletes folder recursevly.
 * \param dirName     Name of the folder to be deleted.
 *******************************************************************************************************************/
bool MainWindow::removeDir(const QString &dirName) const
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  |
                                                    QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }

        result = dir.rmdir(dirName);
    }

    return result;
}

/*!*****************************************************************************************************************
 * \brief Copies folder recursevly.
 * \param sourceFolder     Name of the source folder.
 * \param destFolder       Name of the target folder.
 ******************************************************************************************************************/
void MainWindow::copyDir(const QString &sourceFolder, const QString &destFolder) const
{
    QDir sourceDir(sourceFolder);
    if(!sourceDir.exists())
        return;

    QDir destDir(destFolder);
    if(!destDir.exists()) {
        destDir.mkdir(destFolder);
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        QFile::copy(srcName, destName);
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        copyDir(srcName, destName);
    }
}

/*!*****************************************************************************************************************
 * \brief Displays a dialog box to confirm user action. For ex., removing folder or file.
 * \param title     Title to be display for user with action description.
 ******************************************************************************************************************/
bool MainWindow::askUserForAction(const QString &title) const
{
    QMessageBox msgBox;
    msgBox.setText(title);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QScreen* pScreen = QGuiApplication::screenAt(this->mapToGlobal(QPoint(this->width() / 2, 0)));
    QRect screenRect = pScreen ? pScreen->availableGeometry() : QRect();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QScreen* pScreen = QGuiApplication::screenAt(this->mapToGlobal(QPoint(this->width() / 2, 0)));
    QRect screenRect = pScreen ? pScreen->availableGeometry() : QRect();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QDesktopWidget desktop;
    QRect screenRect = desktop.screenGeometry(this);
#else
    QRect screenRect = QDesktopWidget().screen()->rect();
#endif


    msgBox.move(QPoint(screenRect.width()/2, screenRect.height()/2));

    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            return(true);
            break;
        case QMessageBox::No:
            return(false);
            break;
        default:
            break;
    }

    return(false);
}
/*!******************************************************************************************************************
 * \brief Displays a dialog box to confirm file replacement.
 *******************************************************************************************************************/
bool MainWindow::askForFileReplacement() const
{
    return(askUserForAction("File already exists. Would you like to replace it?"));
}

/*!*******************************************************************************************************************
 * \brief Displays a dialog box to confirm permanent removing of file.
 * \return
 ********************************************************************************************************************/
bool MainWindow::askForPermanentDelete() const
{
    return(askUserForAction("Would you like to delete it permanently?"));
}

/*!*******************************************************************************************************************
 * \brief Creates and displays menu for project group (nested projects).
 ********************************************************************************************************************/
void MainWindow::createProjectUnionMenu()
{
    QMenu *menu = new QMenu(this);

    QAction *proj = new QAction(tr("&Remove"), this);
    proj->setStatusTip(tr("Remove group"));
    connect(proj, SIGNAL(triggered()), this, SLOT(removeGroupUnion()));
    menu->addAction(proj);

    QMenu *menuGroup = menu->addMenu("Add To Group");

    QString curProjName = getCurrentUnionName();
    if(curProjName.isEmpty()) {
        return;
    }

    for(int i = 0; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_ui->treeLibs->topLevelItem(i);
        if(!item) {
            continue;
        }

        if(item->childCount()) {
            continue;
        }

        QString projName = item->text(0);
        if(projName.isEmpty()) {
            continue;
        }

        if(projName == curProjName) {
            continue;
        }

        QAction *projId = new QAction(projName, this);
        projId->setStatusTip(projName);
        connect(projId, SIGNAL(triggered()), this, SLOT(mergeProjectIntoGroup()));
        menuGroup->addAction(projId);
    }

    menu->addMenu(menuGroup);

    menu->popup(QCursor::pos());
    menu->exec();

    delete menu;
}

/*!******************************************************************************************************************
 * \brief Creates and displays menu for project (library) tree widget. It is used for projects management.
 * \param pos       Point(x, y) where to display menu.
 *******************************************************************************************************************/
void MainWindow::showLibraryMenu(const QPoint &pos)
{
    QTreeWidgetItem *currentItem = m_ui->treeLibs->currentItem();
    if(currentItem && currentItem->childCount()) {
        createProjectUnionMenu();
        return;
    }

    QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&event);

    QMenu *menu = new QMenu(this);

    QAction *proj = new QAction(tr("&Add New..."), this);
    proj->setStatusTip(tr("Add new project."));
    connect(proj, SIGNAL(triggered()), this, SLOT(addNewProject()));
    menu->addAction(proj);

    QList<QTreeWidgetItem *> items = m_ui->treeLibs->selectedItems();
    if(items.count()) {
        QAction *copyProj = new QAction(tr("&Copy"), this);
        copyProj->setStatusTip(tr("Copy Project."));
        connect(copyProj, SIGNAL(triggered()), this, SLOT(copySelectedProject()));
        menu->addAction(copyProj);

        if(isProjectCopied()) {
            QAction *pasteProj = new QAction(tr("&Paste"), this);
            pasteProj->setStatusTip(tr("Paste Project."));
            connect(pasteProj, SIGNAL(triggered()), this, SLOT(pasteSelectedData()));
            menu->addAction(pasteProj);
        }

        QAction *delProj = new QAction(tr("&Delete"), this);
        delProj->setStatusTip(tr("Detele Project."));
        connect(delProj, SIGNAL(triggered()), this, SLOT(removeSelectedProject()));
        menu->addAction(delProj);

        QAction *projInfo = new QAction(tr("&Info"), this);
        projInfo->setStatusTip(tr("Detele Project."));
        connect(projInfo, SIGNAL(triggered()), this, SLOT(showProjectInfo()));
        menu->addAction(projInfo);

        QMap<QString, QString> projects = getCurrentLibraries();
        if(projects.count() && currentItem && !currentItem->parent()) {
            QMenu *menuGroup = menu->addMenu("Group with");

            QString curProjName = getCurrentLibraryName();
            if(curProjName.isEmpty()) {
                return;
            }

            QMap<QString, QString>::const_iterator it;
            for(it = projects.constBegin(); it != projects.constEnd(); ++it) {
                QString projName = it.key();

                if(projName.isEmpty()) {
                    continue;
                }

                if(projName == curProjName) {
                    continue;
                }

                QTreeWidgetItem *item = getTreeItemByName(projName);
                if(item) {
                    if(item->parent()) {
                        continue;
                    }
                }

                QAction *projId = new QAction(projName, this);
                projId->setStatusTip(projName);
                connect(projId, SIGNAL(triggered()), this, SLOT(mergeProjectIntoGroup()));
                menuGroup->addAction(projId);
            }

            menu->addMenu(menuGroup);
        }
        else if(currentItem && currentItem->parent()) {
            QAction *ungroupInfo = new QAction(tr("&Ungroup"), this);
            ungroupInfo->setStatusTip(tr("Detele Project."));
            connect(ungroupInfo, SIGNAL(triggered()), this, SLOT(removeFromGroup()));
            menu->addAction(ungroupInfo);
        }
    }

    QMenu *gitMenu = menu->addMenu("Git");

    QAction *gitStatus = new QAction(tr("Status"), this);
    connect(gitStatus, SIGNAL(triggered()), this, SLOT(gitShowStatus()));
    gitMenu->addAction(gitStatus);

    QAction *gitCommit = new QAction(tr("Commit"), this);
    connect(gitCommit, SIGNAL(triggered()), this, SLOT(gitCommitChanges()));
    gitMenu->addAction(gitCommit);

    QAction *gitLog = new QAction(tr("Log"), this);
    connect(gitLog, SIGNAL(triggered()), this, SLOT(gitShowLog()));
    gitMenu->addAction(gitLog);

    QAction *gitDiff = new QAction(tr("Diff"), this);
    connect(gitDiff, SIGNAL(triggered()), this, SLOT(gitShowDiff()));
    gitMenu->addAction(gitDiff);

    QAction *gitPull = new QAction(tr("Pull"), this);
    connect(gitPull, SIGNAL(triggered()), this, SLOT(gitPull()));
    gitMenu->addAction(gitPull);

    QAction *gitPush = new QAction(tr("Push"), this);
    connect(gitPush, SIGNAL(triggered()), this, SLOT(gitPush()));
    gitMenu->addAction(gitPush);

    QAction *gitCheckout = new QAction(tr("Checkout..."), this);
    connect(gitCheckout, SIGNAL(triggered()), this, SLOT(gitCheckout()));
    gitMenu->addAction(gitCheckout);

    menu->popup(QCursor::pos());
    menu->exec();

    delete menu;
}

/*!******************************************************************************************************************
 * \brief Copies selected project to the buffer.
 *******************************************************************************************************************/
void MainWindow::copySelectedProject()
{
    QList<QTreeWidgetItem *> items = m_ui->treeLibs->selectedItems();
    if(!items.count()) {
        return;
    }

    clearCurrentCopyState();

    foreach(QTreeWidgetItem *item, items) {
        if(!item->text(0).isEmpty()) {
            addProjectToBeCopied(item->text(0));
        }
    }
}

/*!******************************************************************************************************************
 * \brief Pastes selected data (could be project, group or file.
 *******************************************************************************************************************/
void MainWindow::pasteSelectedData()
{
    if(!m_copyData.count()) {
        return;
    }

    if(m_currentCopyState == NONE) {
        return;
    }

    if(isProjectCopied()) {
        foreach(const QString &projName, m_copyData) {
            QString key = getLibraryKeyPrefix() + projName;
            QString projPath = m_properties->get<QString>(key);

            if(QFileInfo(projPath).isDir()) {
                QString targetName = generateCopyName(projName, QFileInfo(projPath).absolutePath());

                QString libName = QFileInfo(targetName).completeBaseName();
                info(QString("Coping '%1'' to '%2'...").arg(projName).arg(libName), false);

                copyDir(projPath, targetName);

                QTreeWidgetItem *item = new QTreeWidgetItem;
                item->setText(0, libName);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                m_ui->treeLibs->addTopLevelItem(item);

                key = getLibraryKeyPrefix() + libName;
                m_properties->set(key, targetName);

                setStateChanged();
            }
        }
    }
    else if(isGroupCopied()) {
        if(m_copyData.count() != 2) {
            return;
        }

        QString groupName = m_copyData[0];
        QString groupPath = m_copyData[1];

        if(!QFileInfo(groupPath).isDir()) {
            return;
        }

        QStringList viewsToBeCopied;

        QStringList views = getValidViewList();
        foreach(const QString &viewName, views) {
            QString viewPath = QDir::toNativeSeparators(groupPath + "/" + viewName + "/" + groupName + "." + viewName);
            if(QFileInfo(viewPath).exists()) {
                viewsToBeCopied<<viewPath;
            }
        }

        QString tarLibPath = getCurrentLibraryPath();
        if(!QFileInfo(tarLibPath).isDir()) {
            return;
        }

        QMap<QString, QString> copyMap;

        bool askForReplacement = false;
        foreach(const QString &viewPath, viewsToBeCopied) {
            QString tarViewName = QFileInfo(viewPath).completeSuffix();
            QString tarGroupName = QFileInfo(viewPath).completeBaseName();
            QString tarViewPath = QDir::toNativeSeparators(tarLibPath + "/" + tarViewName + "/" + tarGroupName + "." + tarViewName);
            copyMap[viewPath] = tarViewPath;

            if(QFileInfo(tarViewPath).exists()) {
                askForReplacement = true;
            }
        }

        if(askForReplacement) {
            if(!askForFileReplacement()) {
                return;
            }
        }

        QMap<QString, QString>::const_iterator it;
        for(it = copyMap.constBegin(); it != copyMap.constEnd(); ++it) {
            QString src = it.key();
            QString tar = it.value();

            if(!QFileInfo(src).exists()) {
                continue;
            }

            QString tarPath = QFileInfo(tar).absolutePath();
            if(!QFileInfo(tarPath).isDir()) {
                QDir dir;
                dir.mkpath(tarPath);
            }

            info(QString("Coping view '%1' to '%2'...").arg(src).arg(tar));

            QFile::copy(src, tar);

            if(QFileInfo(tar).exists()) {
                QString viewName = QFileInfo(tar).completeSuffix();
                QString groupName = QFileInfo(tar).completeBaseName();

                if(viewName.isEmpty() || groupName.isEmpty()) {
                    continue;
                }

                QListWidgetItem *itemGroup = new QListWidgetItem;
                itemGroup->setText(groupName);
                itemGroup->setFlags(itemGroup->flags() | Qt::ItemIsEditable);
                m_ui->listGroups->addItem(itemGroup);
                m_ui->listGroups->setCurrentItem(itemGroup);

                QTreeWidgetItem *itemView = new QTreeWidgetItem(m_ui->listViews);
                itemView->setText(0, viewName);

                if(viewName == "gds") {
                    itemView->setData(0, RoleType, ItemViewGds);
                    itemView->setData(0, RoleGdsPath, tar);
                    itemView->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                }

                setStateChanged();
            }
        }
    }
    else if(isViewCopied()) {
        foreach(const QString &viewPath, m_copyData) {
            if(QFileInfo(viewPath).isFile()) {
                QString viewName = QFileInfo(viewPath).completeSuffix();
                if(viewName.isEmpty()) {
                    continue;
                }

                QString groupName = getCurrentGroupName();
                if(groupName.isEmpty()) {
                    continue;
                }

                QString groupPath = getCurrentGroupPath(viewName, true);
                if(!QFileInfo(groupPath).isDir()) {
                    continue;
                }

                QString newViewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + "." + viewName);
                if(QFileInfo(newViewPath).exists()) {
                    if(!askForFileReplacement()) {
                        continue;
                    }
                    else {
                        QFile::remove(newViewPath);
                    }
                }

                QFile::copy(viewPath, newViewPath);
                if(QFileInfo(newViewPath).exists()) {
                    info(QString("Coping view '%1' to '%2'").arg(viewPath).arg(newViewPath));
                }
                else {
                    error(QString("Failed to copy view '%1' to '%2'").arg(viewPath).arg(newViewPath));
                    continue;
                }

                QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->listViews);
                item->setText(0, viewName);

                if(viewName == "gds") {
                    item->setData(0, RoleType, ItemViewGds);
                    item->setData(0, RoleGdsPath, newViewPath);
                    item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                }

                setStateChanged();
            }
        }
    }

    m_copyData.clear();
    m_currentCopyState = NONE;
}

/*!*****************************************************************************************************************
 * \brief Creates new project (library) and adds it to the tree widget.
 ******************************************************************************************************************/
void MainWindow::addNewProject()
{
    QString workDir = getCurrentWorkingDir();
    QString projName = QFileDialog::getExistingDirectory(this,
                                                         tr("Choose or create a new project directory."),
                                                         workDir);

    if(!projName.isEmpty()) {
        QString libName = QFileInfo(projName).completeBaseName();

        bool addProj = true;
        for(int i = 0; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = m_ui->treeLibs->topLevelItem(i);
            if(item) {
                if(item->text(0) == libName) {
                    addProj = false;
                    break;
                }
            }
        }

        if(addProj) {
            QTreeWidgetItem *item = new QTreeWidgetItem;
            item->setText(0, libName);
            m_ui->treeLibs->addTopLevelItem(item);

            QString key = getLibraryKeyPrefix() + libName;
            m_properties->set(key, projName);

            setStateChanged();
        }
    }

#if QT_VERSION >= 0x050000
    m_ui->treeLibs->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->treeLibs->sortByColumn(0);
#endif
}

/*!******************************************************************************************************************
 * \brief Removes selected project (library).
 *******************************************************************************************************************/
void MainWindow::removeSelectedProject()
{
    QList<QTreeWidgetItem *> items = m_ui->treeLibs->selectedItems();
    if(!items.count()) {
        return;
    }

    bool deleteProject = askForPermanentDelete();

    for(int i = 0; i < items.count(); ++i) {
        QString refText = items[i]->text(0);
        for(int j = 0; j < m_ui->treeLibs->topLevelItemCount(); ++j) {
            QTreeWidgetItem *item = m_ui->treeLibs->topLevelItem(j);
            if(refText == item->text(0)) {
                m_ui->treeLibs->takeTopLevelItem(j);

                QString key = getLibraryKeyPrefix() + refText;
                if(m_properties->exists(key)) {
                    QString libPath = m_properties->get<QString>(key);
                    m_properties->remove(key);
                    if(deleteProject) {
                        removeDir(libPath);
                    }
                }

                setStateChanged();
                break;
            }
            else if(item->childCount()) {
                for(int k = 0; k < item->childCount(); ++k) {
                    QTreeWidgetItem *child = item->child(k);
                    if(refText == child->text(0)) {
                        item->takeChild(k);

                        QString key = getLibraryKeyPrefix() + refText;
                        if(m_properties->exists(key)) {
                            QString libPath = m_properties->get<QString>(key);
                            m_properties->remove(key);
                            if(deleteProject) {
                                removeDir(libPath);
                            }
                        }

                        setStateChanged();
                        break;
                    }
                }
            }
        }
    }

#if QT_VERSION >= 0x050000
    m_ui->treeLibs->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->treeLibs->sortByColumn(0);
#endif
}

/*!*********************************************************************************************************************
 * \brief Prints folder Unix information into the MainWindow output window.
 * \param title       Title to print (for ex., Library, Cell, Category, View).
 * \param alias       If file is aliased then it prints the real (aliased) name.
 * \param folderPath  Path to the folder which is requested for printing its information.
 * \param clear       Clears MainWindow output window before printing message.
 **********************************************************************************************************************/
void MainWindow::showFolderInfo(const QString &title, const QString &alias, const QString &folderPath, bool clear)
{
    if(!QFileInfo(folderPath).exists()) {
        return;
    }

    QString owner = QFileInfo(folderPath).owner();
    QString group = QFileInfo(folderPath).group();
    QString lastModify = QFileInfo(folderPath).lastModified().toString("dd:mm:yyyy");

    QString permissions = "";
    if(QFileInfo(folderPath).isReadable()) {
        permissions += "r";
    }
    else {
        permissions += "-";
    }

    if(QFileInfo(folderPath).isWritable()) {
        permissions += "w";
    }
    else {
        permissions += "-";
    }

    if(QFileInfo(folderPath).isExecutable()) {
        permissions += "x";
    }
    else {
        permissions += "-";
    }

    QString msg = title + ": \n";
    msg += "\tName: " + alias + "\n";
    msg += "\tRead Path: " + folderPath + "\n";
    msg += "\tOwner: " + owner + "\n";
    msg += "\tGroup: " + group + "\n";
    msg += "\tLast Modify: " + lastModify + "\n";
    msg += "\tPermissions: " + permissions + "\n";

    info(msg, clear);
}

/*!*********************************************************************************************************************
 * \brief Prints library folder Unix information into the MainWindow output window.
 **********************************************************************************************************************/
void MainWindow::showProjectInfo()
{
    QList<QTreeWidgetItem *> items = m_ui->treeLibs->selectedItems();
    if(!items.count()) {
        return;
    }

    QTreeWidgetItem *projId = items.first();
    QString projName = projId->text(0);
    if(projName.isEmpty()) {
        return;
    }

    QString libPath = getLibraryPath(projName);
    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    showFolderInfo("Project", projName, libPath);
}

/*!******************************************************************************************************************
 * \brief Clears current buffer used for coping of data.
 *******************************************************************************************************************/
void MainWindow::clearCurrentCopyState()
{
    m_copyData.clear();
    m_currentCopyState = NONE;
}

/*!******************************************************************************************************************
 * \brief Adds project to the buffer for later coping.
 * \param projName     Name of the project (library) to be copied.
 *******************************************************************************************************************/
void MainWindow::addProjectToBeCopied(const QString &projName)
{
    m_copyData<<projName;
    m_currentCopyState = PROJECT;
}

/*!******************************************************************************************************************
 * \brief Merges project into a nested group.
 *******************************************************************************************************************/
void MainWindow::mergeProjectIntoGroup()
{
    QObject *object = QObject::sender();
    if(!object) {
        return;
    }

    QAction *action = static_cast<QAction*> (object);
    if(!action) {
        return;
    }

    QString targetProj = action->text();
    if(targetProj.isEmpty()) {
        return;
    }

    QTreeWidgetItem *targetId = getTreeItemByName(targetProj);
    if(!targetId) {
        return;
    }

    QTreeWidgetItem *sourceId = getTreeItemByName(!m_itemText.isEmpty() ? m_itemText : getCurrentUnionName());
    if(!sourceId) {
        return;
    }

    if(!sourceId->childCount() && targetId->childCount()) {
        std::swap(sourceId, targetId);
    }

    if(!sourceId->childCount()) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, "GroupName");
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        m_ui->treeLibs->takeTopLevelItem(m_ui->treeLibs->indexOfTopLevelItem(sourceId));
        m_ui->treeLibs->takeTopLevelItem(m_ui->treeLibs->indexOfTopLevelItem(targetId));

        QTreeWidgetItem *proj1 = new QTreeWidgetItem;
        proj1->setText(0, m_itemText);
        proj1->setFlags(proj1->flags() | Qt::ItemIsEditable);
        item->addChild(proj1);

        QTreeWidgetItem *proj2 = new QTreeWidgetItem;
        proj2->setText(0, targetProj);
        proj2->setFlags(proj2->flags() | Qt::ItemIsEditable);
        item->addChild(proj2);

        m_ui->treeLibs->addTopLevelItem(item);

        setStateChanged();
    }
    else {
        QTreeWidgetItem *proj2 = new QTreeWidgetItem;
        proj2->setText(0, targetProj);
        proj2->setFlags(proj2->flags() | Qt::ItemIsEditable);

        m_ui->treeLibs->takeTopLevelItem(m_ui->treeLibs->indexOfTopLevelItem(targetId));

        sourceId->addChild(proj2);

        setStateChanged();
    }

#if QT_VERSION >= 0x050000
    m_ui->treeLibs->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->treeLibs->sortByColumn(0);
#endif
}

/*!******************************************************************************************************************
 * \brief Eliminates project from the nested group.
 *******************************************************************************************************************/
void MainWindow::removeFromGroup()
{
    QString projName = getCurrentLibraryName();
    if(projName.isEmpty()) {
        return;
    }

    QTreeWidgetItem *projId = getTreeItemByName(projName);
    if(!projId) {
        return;
    }

    QTreeWidgetItem *parentId = projId->parent();
    if(parentId) {
        QTreeWidgetItem *newProjId = new QTreeWidgetItem;
        newProjId->setText(0, projName);
        newProjId->setFlags(newProjId->flags() | Qt::ItemIsEditable);
        m_ui->treeLibs->addTopLevelItem(newProjId);

        parentId->takeChild(parentId->indexOfChild(projId));

        setStateChanged();
    }

#if QT_VERSION >= 0x050000
    m_ui->treeLibs->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->treeLibs->sortByColumn(0);
#endif
}

/*!******************************************************************************************************************
 * \brief Delets group (nested projects) from the tree widget.
 *******************************************************************************************************************/
void MainWindow::removeGroupUnion()
{
    QString groupName = getCurrentUnionName();
    if(groupName.isEmpty()) {
        return;
    }

    for(int i = 0; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
        QTreeWidgetItem *groupId = m_ui->treeLibs->topLevelItem(i);
        if(!groupId) {
            continue;
        }

        QString curItemName = groupId->text(0);
        if(curItemName == groupName) {
            while(groupId->childCount()) {
                int childIndex = groupId->childCount()-1;
                QTreeWidgetItem *childId = groupId->child(childIndex);
                if(!childId) {
                    continue;
                }

                QString projName = childId->text(0);
                if(projName.isEmpty()) {
                    continue;
                }

                QTreeWidgetItem *newProjId = new QTreeWidgetItem;
                newProjId->setText(0, projName);
                newProjId->setFlags(newProjId->flags() | Qt::ItemIsEditable);
                m_ui->treeLibs->addTopLevelItem(newProjId);

                groupId->takeChild(childIndex);
            }

            m_ui->treeLibs->takeTopLevelItem(i);

            setStateChanged();
        }
    }

#if QT_VERSION >= 0x050000
    m_ui->treeLibs->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->treeLibs->sortByColumn(0);
#endif
}
