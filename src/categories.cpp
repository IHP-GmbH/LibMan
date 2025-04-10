#include <QMenu>
#include <QFile>
#include <QScreen>
#include <QProcess>
#include <QVariant>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QGuiApplication>
#include <QListWidgetItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "property.h"
#include "toolmanager.h"
#include "projectmanager.h"

/*!*********************************************************************************************************************
 * \brief Reads library category and returns it's cells.
 * \param libPath     Path to the project library.
 * \param catName     Name of the category.
 **********************************************************************************************************************/
QStringList MainWindow::readLibraryCategories(const QString &libPath, const QString &catName)
{
    QStringList categories;

    QString fileName = QDir::toNativeSeparators(libPath + "/" + catName + ".group");
    if(!QFileInfo(fileName).exists()) {
        QMessageBox::critical(this, tr("LibManager"),
                              tr("Can not find category '%1'.").arg(fileName));
        error("Can not find category '" + fileName + "'.");
        return categories;
    }


    QFile file(fileName);
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::critical(this, tr("LibManager"),
                              tr("Can not read category '%1':\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
        error("Can not read category '" + fileName + "'.");
        return categories;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().remove("^\\s+").remove("\\s+$");
        #if QT_VERSION >= 0x050000
            QStringList words = line.split(" ", Qt::SkipEmptyParts);
        #else
            QStringList words = line.split(" ", QString::SkipEmptyParts);
        #endif
        foreach(const QString catName, words) {
            categories<<catName;
        }
    }

    categories.removeDuplicates();
    categories.sort();

    return categories;
}
