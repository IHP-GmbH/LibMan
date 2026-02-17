#include <QMenu>
#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QProcess>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QInputDialog>
#include <QGuiApplication>
#include <QListWidgetItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "gds/gdsreader.h"

/*!*********************************************************************************************************************
 * \brief Displays menu for view widget.
 * \param pos       Point(x, y) where menu will be displayed.
 **********************************************************************************************************************/
void MainWindow::showViewMenu(const QPoint &pos)
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&event);

    QMenu *menu = new QMenu(this);

    QMenu *menuViews = menu->addMenu("New");

    QAction *schematic = new QAction(tr("&Schematic"), this);
    schematic->setStatusTip(tr("Schematic"));
    connect(schematic, SIGNAL(triggered()), this, SLOT(addNewSchematicView()));
    menuViews->addAction(schematic);

    QAction *layout = new QAction(tr("&Layout"), this);
    layout->setStatusTip(tr("Layout"));
    connect(layout, SIGNAL(triggered()), this, SLOT(addNewLayoutView()));
    menuViews->addAction(layout);

    QAction *spice = new QAction(tr("&Spice"), this);
    spice->setStatusTip(tr("Spice"));
    connect(spice, SIGNAL(triggered()), this, SLOT(addNewSpiceView()));
    menuViews->addAction(spice);

    menu->addMenu(menuViews);

    if(isViewCopied() && m_copyData.count()) {
        QStringList views = getCurrentViews(libPath, groupName);
        QString viewPath = m_copyData.first();
        QString viewName = QFileInfo(viewPath).completeSuffix();

        if(!views.contains(viewName)) {
            QAction *pasteView = new QAction(tr("&Paste"), this);
            pasteView->setStatusTip(tr("Paste Project."));
            connect(pasteView, SIGNAL(triggered()), this, SLOT(pasteSelectedData()));
            menu->addAction(pasteView);
        }
    }

    QList<QTreeWidgetItem *> items = m_ui->listViews->selectedItems();
    if(items.count()) {
        QAction *copyView = new QAction(tr("&Copy"), this);
        copyView->setStatusTip(tr("Copy view."));
        connect(copyView, SIGNAL(triggered()), this, SLOT(copySelectedView()));
        menu->addAction(copyView);

        QAction *delView = new QAction(tr("&Delete"), this);
        delView->setStatusTip(tr("Detele view."));
        connect(delView, SIGNAL(triggered()), this, SLOT(removeSelectedView()));
        menu->addAction(delView);

        QAction *viewInfo = new QAction(tr("&Info"), this);
        viewInfo->setStatusTip(tr("Detele view."));
        connect(viewInfo, SIGNAL(triggered()), this, SLOT(showViewInfo()));
        menu->addAction(viewInfo);
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

/*!*********************************************************************************************************************
 * \brief Creates new spice model view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewSpiceView()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QStringList views = getCurrentViews(libPath, groupName);
    if(views.contains("spice")) {
        return;
    }

    QString groupPath = getCurrentGroupPath("spice", true);
    if(!QFileInfo(groupPath).isDir()) {
        return;
    }

    QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".spice");
    if(createNewFile(viewPath)) {
        QTreeWidgetItem *viewItem = new QTreeWidgetItem(m_ui->listViews);
        viewItem->setText(0, "spice");
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
}

/*!*********************************************************************************************************************
 * \brief Creates new layout view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewLayoutView()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QStringList views = getCurrentViews(libPath, groupName);
    if(views.contains("gds")) {
        return;
    }

    QString groupPath = getCurrentGroupPath("gds", true);
    if(!QFileInfo(groupPath).isDir()) {
        return;
    }

    QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".gds");
    if(QFileInfo(viewPath).exists()) {
        return;
    }

    GdsReader gdsReader(viewPath);
    gdsReader.gdsCreate(groupName);

    QStringList errors = gdsReader.getErrors();
    if(errors.count()) {
        foreach(const QString &explain, errors) {
            error(explain, false);
        }
        return;
    }

    if(QFileInfo(viewPath).exists()) {
        QTreeWidgetItem *viewItem = new QTreeWidgetItem(m_ui->listViews);
        viewItem->setText(0, "gds");
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
}

/*!*********************************************************************************************************************
 * \brief Creates new schematic view and adds it to the list widget.
 **********************************************************************************************************************/
void MainWindow::addNewSchematicView()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QStringList views = getCurrentViews(libPath, groupName);
    if(views.contains("cdl")) {
        return;
    }

    QString groupPath = getCurrentGroupPath("cdl", true);
    if(!QFileInfo(groupPath).isDir()) {
        return;
    }

    QString viewPath = QDir::toNativeSeparators(groupPath + "/" + groupName + ".cdl");
    if(createNewFile(viewPath)) {
        QTreeWidgetItem *viewItem = new QTreeWidgetItem(m_ui->listViews);
        viewItem->setText(0, "cdl");
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
}

/*!*********************************************************************************************************************
 * \brief Adds selected view to the buffer for coping.
 **********************************************************************************************************************/
void MainWindow::copySelectedView()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QString viewName = getCurrentViewName();
    if(viewName.isEmpty()) {
        return;
    }

    QString viewPath = getViewPath(libPath, groupName, viewName);
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
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QList<QTreeWidgetItem*> items = m_ui->listViews->selectedItems();
    if(!items.count()) {
        return;
    }

    bool deleteFiles = askForPermanentDelete();

    for(int i = 0; i < items.count(); ++i) {
        QTreeWidgetItem *item = items[i];
        if(!item) {
            continue;
        }

        QString refText = item->text(0);
        if(refText.isEmpty()) {
            continue;
        }

        if(deleteFiles) {
            QString viewPath = getViewPath(libPath, groupName, refText);
            if(QFileInfo(viewPath).exists()) {
                info(QString("Removing view '%1'").arg(viewPath));
                QFile::remove(viewPath);
            }
        }

        QTreeWidgetItem *parent = item->parent();
        if(parent) {
            parent->removeChild(item);
            delete item;
        }
        else {
            int idx = m_ui->listViews->indexOfTopLevelItem(item);
            if(idx >= 0) {
                QTreeWidgetItem *taken = m_ui->listViews->takeTopLevelItem(idx);
                delete taken;
            }
            else {
                delete item;
            }
        }
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
}

/*!*********************************************************************************************************************
 * \brief Prints view file Unix information into the MainWindow output window.
 **********************************************************************************************************************/
void MainWindow::showViewInfo()
{
    QString viewName = getCurrentViewName();
    if(viewName.isEmpty()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QString viewPath = getViewPath(libPath, groupName, viewName);

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
