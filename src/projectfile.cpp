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
#include "libfileparser.h"

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

/*!*******************************************************************************************************************
 * \brief Resolves absolute or relative PROJECT path.
 *
 * Relative paths are interpreted relative to the .projects file location.
 *
 * \param projectsFile Path to the loaded .projects file.
 * \param rawPath Path string from PROJECT entry.
 *
 * \return Absolute path.
 **********************************************************************************************************************/
QString MainWindow::resolveProjectPath(const QString& projectsFile, const QString& rawPath)
{
    const QString p = QDir::fromNativeSeparators(rawPath);

    if (QDir::isAbsolutePath(p)) {
        return QDir::toNativeSeparators(p);
    }

    const QDir baseDir = QFileInfo(projectsFile).absoluteDir();

    return QDir::toNativeSeparators(baseDir.absoluteFilePath(p));
}

/*!******************************************************************************************************************
 * \brief Loads KLayout .lib file content into LibMan.
 * \param fileName     Path to the file to be loaded.
 *******************************************************************************************************************/
void MainWindow::loadProjectFile(const QString &fileName)
{
    LibFileParser parser;
    if(!parser.parseFile(fileName)) {
        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not read lib file '%1':\n%2.")
                                 .arg(fileName)
                                 .arg(parser.errorString()));
        error("Can not read lib file '" + fileName + "'.");
        return;
    }

    const LibFileData& data = parser.data();

    m_ui->treeLibs->clear();
    m_ui->listGroups->clear();
    m_ui->listViews->clear();

    m_ui->txtLibSearch->clear();
    m_ui->txtCatSearch->clear();
    m_ui->txtCellSearch->clear();
    m_ui->txtViewSearch->clear();

    QMap<QString, QString> loadedLibs;

    for(const LibDefinition& def : data.definitions) {
        QString libName = def.name.trimmed();
        QString libPath = expandShellVariables(def.path.trimmed());

        if(libName.isEmpty()) {
            continue;
        }

        if(libPath.isEmpty()) {
            continue;
        }

        if(!QFileInfo(libPath).exists()) {
            error(QString("Library path does not exist for '%1': %2")
                      .arg(libName, libPath));
            continue;
        }

        QString key = getLibraryKeyPrefix() + libName;
        m_properties->set(key, libPath);
        loadedLibs[libName] = libPath;
    }

    loadLibraries();

    setRecentProject(fileName);

    m_currentProjFile = fileName;

    QString fileTitle = QFileInfo(m_currentProjFile).completeBaseName();
    QString fileSuffix = QFileInfo(m_currentProjFile).completeSuffix();

    if(!fileSuffix.isEmpty()) {
        fileTitle += "." + fileSuffix;
    }

    setWindowTitle(getLibManTitle() + " (" + fileTitle + ")");
    setStateSaved();

    info(QString("Lib file '%1' has been loaded. %2 libraries found.")
             .arg(fileName)
             .arg(data.definitions.count()));
}

/*!******************************************************************************************************************
 * \brief Saves LibMan libraries into KLayout .lib file.
 * \param fileName     Path to the file to be saved.
 *******************************************************************************************************************/
void MainWindow::saveProjectFile(const QString &fileName)
{
    if(fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not write to file '%1':\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
        error("Can not write to file '" + fileName + "'.");
        return;
    }

    QTextStream out(&file);

    out << "# KLayout library definition file\n";
    out << "# Generated by LibMan\n\n";

    QMap<QString, QString> projs = getCurrentLibraries();
    QMap<QString, QString>::const_iterator it;

    for(it = projs.constBegin(); it != projs.constEnd(); ++it) {
        const QString libName = it.key().trimmed();
        const QString libPath = it.value().trimmed();

        if(libName.isEmpty() || libPath.isEmpty()) {
            continue;
        }

        if(!QFileInfo(libPath).exists()) {
            error(QString("Skipping library '%1': path does not exist: %2")
                      .arg(libName, libPath));
            continue;
        }

        out << "define("
            << toLibStringLiteral(libName)
            << ", "
            << toLibStringLiteral(QDir::toNativeSeparators(libPath))
            << ");\n";
    }

    file.close();

    info(QString("Project '%1' has been saved.").arg(fileName));

    setStateSaved();
}

/*!******************************************************************************************************************
 * \brief Converts a QString into a valid KLayout .lib string literal.
 *
 * Escapes special characters such as backslash, double quotes and control characters
 * (\n, \r, \t) so that the resulting string can be safely used inside define(...)
 * and include(...) statements in a .lib file.
 *
 * The resulting string is always enclosed in double quotes.
 *
 * \param value Input string.
 *
 * \return Escaped string literal ready for .lib file output.
 *******************************************************************************************************************/
QString MainWindow::toLibStringLiteral(const QString& value)
{
    QString s = value;
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    s.replace("\n", "\\n");
    s.replace("\r", "\\r");
    s.replace("\t", "\\t");

    return "\"" + s + "\"";
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
