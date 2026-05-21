#include <QMenu>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QVariant>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QFileSystemWatcher>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "property.h"
#include "libfileparser.h"

/*!******************************************************************************************************************
 * \brief Detects view name from file suffix.
 *******************************************************************************************************************/
QString MainWindow::detectViewFromPath(const QString& filePath) const
{
    return(QFileInfo(filePath).suffix().toLower());
}

/*!******************************************************************************************************************
 * \brief Expands shell-style environment variables in a given path string.
 *******************************************************************************************************************/
QString MainWindow::expandShellVariables(const QString& path) const
{
    QString result = path;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

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
 * \brief Resolves absolute or relative PROJECT path.
 *******************************************************************************************************************/
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

    QSet<QString> loadedLibraries;

    for(const LibDefinition& def : data.definitions) {

        const QString libName = def.name.trimmed();
        const QString libPath = expandShellVariables(QDir::fromNativeSeparators(def.path.trimmed()));

        if(libName.isEmpty()) {
            error(QString("Skipping library with empty name from '%1'.").arg(def.sourceFile));
            continue;
        }

        if(libPath.isEmpty()) {
            error(QString("Skipping library '%1' with empty path.").arg(libName));
            continue;
        }

        QFileInfo fi(libPath);

        if(!fi.exists()) {
            error(QString("Library file does not exist for '%1': %2").arg(libName, libPath));
            continue;
        }

        if(!fi.isFile()) {
            error(QString("Library path is not a file for '%1': %2").arg(libName, libPath));
            continue;
        }

        const QString viewName = detectViewFromPath(fi.absoluteFilePath());
        if(viewName.isEmpty()) {
            error(QString("Skipping file with unknown view type: %1").arg(fi.absoluteFilePath()));
            continue;
        }

        const QString groupName = fi.completeBaseName().trimmed();
        if(groupName.isEmpty()) {
            error(QString("Skipping file with empty group name: %1").arg(fi.absoluteFilePath()));
            continue;
        }

        const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
        m_properties->set(key, fi.absoluteFilePath());

        loadedLibraries.insert(libName);
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

    setupProjectFileWatcher(fileName);

    info(QString("Lib file '%1' has been loaded. %2 libraries found.")
             .arg(fileName)
             .arg(loadedLibraries.count()));
}

/*!*********************************************************************************************************************
 * \brief Finds a representative view file for the specified library.
 *
 * This function searches all LibMan property entries belonging to the given library
 * and returns the absolute path of the first existing view file found.
 *
 * The returned file path can be used in the project file as a reference entry for
 * the library. During project loading, LibMan will restore the full library content
 * by scanning the library root directory.
 *
 * \param libName     Name of the library.
 *
 * \return Absolute path to an existing view file, or an empty string if none was found.
 **********************************************************************************************************************/
QString MainWindow::findRepresentativeLibraryFile(const QString &libName) const
{
    if(libName.isEmpty()) {
        return QString();
    }

    const QMap<QString, PropertyItem*> propItems = m_properties->getMap();
    const QString prefix = getLibraryKeyPrefix() + libName + "/";

    for(auto it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        const QString key = it.key();
        if(!key.startsWith(prefix)) {
            continue;
        }

        const QString filePath = m_properties->get<QString>(key).trimmed();
        QFileInfo fi(filePath);
        if(fi.exists() && fi.isFile()) {
            return fi.absoluteFilePath();
        }
    }

    return QString();
}

/*!*********************************************************************************************************************
 * \brief Returns all current project file entries derived from registered library views.
 *
 * This function iterates through all LibMan library property entries and collects
 * pairs of:
 *   - library name
 *   - absolute view file path
 *
 * Each returned pair corresponds to one define(...) line that can be written
 * to the project file.
 *
 * Unlike getCurrentLibraries(), this function preserves multiple entries for the
 * same library name.
 *
 * \return List of (library name, absolute file path) pairs.
 **********************************************************************************************************************/
QList<QPair<QString, QString>> MainWindow::getCurrentProjectEntries() const
{
    QList<QPair<QString, QString>> entries;

    const QMap<QString, PropertyItem*> propItems = m_properties->getMap();

    for(auto it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        const QString key = it.key();
        if(!key.toUpper().startsWith(getLibraryKeyPrefix())) {
            continue;
        }

        QString tail = key;
        tail.remove(getLibraryKeyPrefix());

        const int pos = tail.indexOf('/');
        const QString libName = (pos >= 0) ? tail.left(pos).trimmed() : tail.trimmed();
        if(libName.isEmpty()) {
            continue;
        }

        const QString filePath = m_properties->get<QString>(key).trimmed();
        QFileInfo fi(filePath);
        if(!fi.exists() || !fi.isFile()) {
            continue;
        }

        entries.append(qMakePair(libName, fi.absoluteFilePath()));
    }

    return entries;
}

/*!******************************************************************************************************************
 * \brief Saves LibMan libraries into KLayout .lib file.
 *******************************************************************************************************************/
void MainWindow::saveProjectFile(const QString &fileName)
{
    if(fileName.isEmpty()) {
        return;
    }

    m_ignoreProjectFileChange = true;

    if(m_projFileWatcher) {
        m_projFileWatcher->removePath(fileName);
    }

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {

        if(m_projFileWatcher && QFileInfo(fileName).exists()) {
            m_projFileWatcher->addPath(fileName);
        }

        m_ignoreProjectFileChange = false;

        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not write to file '%1':\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
        error("Can not write to file '" + fileName + "'.");
        return;
    }

    QTextStream out(&file);
    const QDir baseDir = QFileInfo(fileName).absoluteDir();

    out << "# KLayout library definition file\n";
    out << "# Generated by LibMan\n\n";

    const QList<QPair<QString, QString>> entries = getCurrentProjectEntries();

    for(const auto &entry : entries) {

        const QString libName = entry.first.trimmed();
        const QString filePath = entry.second.trimmed();

        if(libName.isEmpty() || filePath.isEmpty()) {
            continue;
        }

        QFileInfo fi(filePath);
        if(!fi.exists() || !fi.isFile()) {
            error(QString("Skipping library entry '%1': file does not exist: %2")
                      .arg(libName, filePath));
            continue;
        }

        const QString storedPath =
            QDir::toNativeSeparators(baseDir.relativeFilePath(fi.absoluteFilePath()));

        out << "define("
            << toLibStringLiteral(libName)
            << ", "
            << toLibStringLiteral(storedPath)
            << ");\n";
    }

    file.close();

    if(m_projFileWatcher && QFileInfo(fileName).exists()) {
        m_projFileWatcher->addPath(fileName);
    }

    m_ignoreProjectFileChange = false;

    info(QString("Project '%1' has been saved.").arg(fileName));

    setStateSaved();
}

/*!******************************************************************************************************************
 * \brief Converts QString into valid .lib string literal.
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
 * \brief Creates empty project file.
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
