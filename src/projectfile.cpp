#include <QMenu>
#include <QFile>
#include <QProcess>
#include <QVariant>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QListWidgetItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "property.h"

/*!******************************************************************************************************************
 * \brief Expands shell-style environment variables in a given path string.
 *
 * This function replaces variables like $VAR and ${VAR} with their actual values
 * from the current system environment (e.g. $HOME -> /home/user).
 *
 * \param path         Input path containing shell-style variables.
 * \return             A string with all environment variables expanded.
 *******************************************************************************************************************/
QString MainWindow::expandShellVariables(const QString& path) const
{
    QString result = path;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Handle ${VAR} and $VAR
    QRegularExpression re(R"(\$(\{([^}]+)\}|([A-Za-z_][A-Za-z0-9_]*)))");
    QRegularExpressionMatchIterator it = re.globalMatch(result);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = match.captured(2).isEmpty() ? match.captured(3) : match.captured(2);
        QString varValue = env.value(varName);
        result.replace(match.captured(0), varValue);
    }

    return result;
}

/*!******************************************************************************************************************
 * \brief Loads project file contence into LibMan.
 * \param fileName     Path to the file to be loaded.
 *******************************************************************************************************************/
void MainWindow::loadProjectFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not read file '%1':\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        error("Can not read file '" + fileName + "'.");
        return;
    }

    QMap<QString, QStringList> combinedLibs;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().remove("^\\s+").remove("\\s+$");

        if(line.startsWith("#")) {
            continue;
        }

        if(line.contains("GROUP")) {
            #if QT_VERSION >= 0x050000
                QStringList words = line.split(" ", Qt::SkipEmptyParts);
            #else
                QStringList words = line.split(" ", QString::SkipEmptyParts);
            #endif
            if(words.count() > 1) {
                QString groupName = words[1];
                QStringList groupItems;
                for(int i = 2; i < words.count(); ++i) {
                    groupItems<<words[i];
                }

                combinedLibs[groupName] = groupItems;
            }
        }
        else if(line.contains("PROJECT")) {
            #if QT_VERSION >= 0x050000
                QStringList words = line.split(" ", Qt::SkipEmptyParts);
            #else
                QStringList words = line.split(" ", QString::SkipEmptyParts);
            #endif
            if(words.count() == 3) {
                QString libName = words[1];
                QString libPath = expandShellVariables(words[2]);

                if(!libName.isEmpty() && QFileInfo(libPath).exists() && QFileInfo(libPath).isDir()) {
                    QString key = getLibraryKeyPrefix() + libName;
                    m_properties->set(key, libPath);
                }
            }
        }
    }

    file.close();

    m_ui->treeLibs->clear();
    m_ui->listGroups->clear();
    m_ui->listViews->clear();

    m_ui->txtLibSearch->clear();
    m_ui->txtCatSearch->clear();
    m_ui->txtCellSearch->clear();
    m_ui->txtViewSearch->clear();

    loadLibraries();
    loadCombinedLibs(combinedLibs);

    setRecentProject(fileName);

    m_currentProjFile = fileName;

    QString fileTitle = QFileInfo(m_currentProjFile).completeBaseName();
    QString fileSuffix = QFileInfo(m_currentProjFile).completeSuffix();

    if(!fileSuffix.isEmpty()) {
        fileTitle += "." + fileSuffix;
    }


    setWindowTitle(getLibManTitle() + " (" + fileTitle + ")");
    setStateSaved();
}

/*!******************************************************************************************************************
 * \brief Saves LibMan Library/Cell/View data into project file.
 * \param fileName     Path to the file to be saved.
 *******************************************************************************************************************/
void MainWindow::saveProjectFile(const QString &fileName)
{
    if(fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not write to file '%1':\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        error("Can not write to file '" + fileName + "'.");
        return;
    }

    QTextStream out(&file);

    bool insertEmptyLine = false;
    for(int i = 0 ; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_ui->treeLibs->topLevelItem(i);
        if(!item) {
            continue;
        }

        QString groupName = item->text(0);
        if(groupName.isEmpty()) {
            continue;
        }

        if(item->childCount()) {
            QStringList childs;
            for(int j = 0 ; j < item->childCount(); ++j) {
                QTreeWidgetItem *child = item->child(j);
                if(!child) {
                    continue;
                }

                QString libName = child->text(0);
                if(libName.isEmpty()) {
                    continue;
                }

                childs<<libName;
            }

            if(childs.count()) {
                insertEmptyLine = true;
                out<<"GROUP "<<groupName<<" "<<childs.join(" ")<<"\n";
            }
        }
    }

    if(insertEmptyLine) {
        out<<"\n";
    }

    QMap<QString, QString> projs = getCurrentLibraries();
    QMap<QString, QString>::const_iterator it;

    for(it = projs.constBegin(); it != projs.constEnd(); ++it) {
        QString projName = it.key();
        QString projPath = it.value();

        if(QFileInfo(projPath).isDir()) {
            out<<"PROJECT "<<projName<<" "<<projPath<<"\n";
        }
    }

    file.close();

    info(QString("Project '%1' has been saved.").arg(fileName));

    setStateSaved();
}

/*!******************************************************************************************************************
 * \brief Creates an empty project file.
 * \param fileName     Path to the file to be created.
 *******************************************************************************************************************/
bool MainWindow::createNewFile(const QString &fileName)
{
    if(fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not write to file '%1':\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        error("Can not write to file '" + fileName + "'.");
        return false;
    }

    file.close();

    return true;
}
