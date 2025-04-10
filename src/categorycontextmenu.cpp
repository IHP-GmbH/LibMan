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

/*!*********************************************************************************************************************
 * \brief Displays menu for category widget.
 * \param pos       Point(x, y) where menu will be displayed.
 **********************************************************************************************************************/
void MainWindow::showCategoryMenu(const QPoint &pos)
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mousePressEvent(&event);

    QMenu *menu = new QMenu(this);

    QAction *group = new QAction(tr("&Create..."), this);
    group->setStatusTip(tr("Add new cell."));
    connect(group, SIGNAL(triggered()), this, SLOT(addNewCategory()));
    menu->addAction(group);

    QList<QTreeWidgetItem *> items = m_ui->listCategories->selectedItems();
    if(items.count()) {
        QAction *delGroup = new QAction(tr("&Delete"), this);
        delGroup->setStatusTip(tr("Detele Project."));
        connect(delGroup, SIGNAL(triggered()), this, SLOT(removeSelectedCategory()));
        menu->addAction(delGroup);

        QAction *groupInfo = new QAction(tr("&Info"), this);
        groupInfo->setStatusTip(tr("Detele Project."));
        connect(groupInfo, SIGNAL(triggered()), this, SLOT(showCategoryInfo()));
        menu->addAction(groupInfo);
    }

    menu->popup(QCursor::pos());
    menu->exec();

    delete menu;
}

/*!*********************************************************************************************************************
 * \brief Creates new category and adds it to the tree widget.
 **********************************************************************************************************************/
void MainWindow::addNewCategory()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QString catPath = QDir::toNativeSeparators(libPath + "/Category.group");
    if(QFileInfo(catPath).exists()) {
        catPath = generateCopyName("Category", libPath, ".group");
    }

    if(!createNewFile(catPath)) {
        return;
    }

    QString catName = QFileInfo(catPath).completeBaseName();

    QTreeWidgetItem *catId = new QTreeWidgetItem;
    catId->setText(0, catName);
    m_ui->listCategories->addTopLevelItem(catId);

#if QT_VERSION >= 0x050000
    m_ui->listCategories->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->listCategories->sortByColumn(0);
#endif
}

/*!*********************************************************************************************************************
 * \brief Removes selected category.
 **********************************************************************************************************************/
void MainWindow::removeSelectedCategory()
{
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QList<QTreeWidgetItem *> items = m_ui->listCategories->selectedItems();
    if(!items.count()) {
        return;
    }

    bool deleteCats = askForPermanentDelete();

    for(int i = 0; i < items.count(); ++i) {
        QTreeWidgetItem *catId = items[i];
        if(!catId) {
            continue;
        }

        QString refText = catId->text(0);
        for(int j = 0; j < m_ui->listCategories->topLevelItemCount(); ++j) {
            QTreeWidgetItem *item = m_ui->listCategories->topLevelItem(j);
            if(refText == item->text(0)) {
                if(deleteCats) {
                    QString catPath = QDir::toNativeSeparators(libPath + "/" + refText + ".group");
                    if(QFileInfo(catPath).exists()) {
                        info(QString("Removing category '%1'").arg(catPath), false);
                        QFile::remove(catPath);
                    }
                }

                m_ui->listCategories->takeTopLevelItem(j);
                break;
            }
        }
    }
}

/*!*********************************************************************************************************************
 * \brief Prints category file Unix information into the MainWindow output window.
 **********************************************************************************************************************/
void MainWindow::showCategoryInfo()
{
    QString catName = getCurrentCategoryName();
    if(catName.isEmpty()) {
        return;
    }

    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QString catPath = QDir::toNativeSeparators(libPath + "/" + catName + ".group");
    if(!QFileInfo(catPath).exists()) {
        return;
    }

    showFolderInfo("Category", catName, catPath, true);
}
