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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "property.h"
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

    QList<QListWidgetItem *> items = m_ui->listViews->selectedItems();
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
        QListWidgetItem *viewId = new QListWidgetItem;
        viewId->setText("spice");
        m_ui->listViews->addItem(viewId);
    }

    m_ui->listViews->sortItems();
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
        QListWidgetItem *viewId = new QListWidgetItem;
        viewId->setText("gds");
        m_ui->listViews->addItem(viewId);
    }

    m_ui->listViews->sortItems();
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
        QListWidgetItem *viewId = new QListWidgetItem;
        viewId->setText("cdl");
        m_ui->listViews->addItem(viewId);
    }

    m_ui->listViews->sortItems();
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

    QList<QListWidgetItem *> items = m_ui->listViews->selectedItems();
    if(!items.count()) {
        return;
    }

    bool deleteFiles = askForPermanentDelete();

    for(int i = 0; i < items.count(); ++i) {
        QString refText = items[i]->text();
        for(int j = 0; j < m_ui->listViews->count(); ++j) {
            QListWidgetItem *item = m_ui->listViews->item(j);
            if(refText == item->text()) {
                if(deleteFiles) {
                    QString viewPath = getViewPath(libPath, groupName, refText);
                    if(QFileInfo(viewPath).exists()) {
                        info(QString("Removing view '%1'").arg(viewPath));
                        QFile::remove(viewPath);
                    }
                }

                m_ui->listViews->takeItem(j);
                break;
            }
        }
    }

    m_ui->listViews->sortItems();
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

