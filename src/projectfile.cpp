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
#include "core/core_path_utils.h"

/*!******************************************************************************************************************
 * \brief Detects view name from file suffix.
 *******************************************************************************************************************/
QString MainWindow::detectViewFromPath(const QString& filePath) const
{
    QString groupName;
    QString viewName;
    if (resolveCellViewFromPath(filePath, &groupName, &viewName)) {
        return viewName;
    }
    return QString();
}

bool MainWindow::resolveCellViewFromPath(const QString &filePath,
                                         QString *groupName,
                                         QString *viewName) const
{
    if (!groupName || !viewName) {
        return false;
    }

    const CoreViewIdentity coreIdentity = parseCoreViewIdentity(filePath);
    if (coreIdentity.valid) {
        *groupName = coreIdentity.cellName;
        *viewName = coreIdentity.viewName;
        return true;
    }

    const QFileInfo fi(filePath);
    const QString suffix = fi.suffix().trimmed().toLower();
    if (suffix.isEmpty()) {
        return false;
    }

    const QString cellName = fi.completeBaseName().trimmed();
    if (cellName.isEmpty()) {
        return false;
    }

    *groupName = cellName;
    *viewName = suffix;
    return true;
}

void MainWindow::configureCoreViewTreeItem(QTreeWidgetItem *viewItem,
                                           const QString &viewName,
                                           const QString &viewPath) const
{
    if (!viewItem || !isCoreViewName(viewName)) {
        return;
    }

    viewItem->setData(0, RoleType, ItemViewCore);
    viewItem->setData(0, RoleCorePath, viewPath);
    if (isLayoutCoreViewName(viewName)) {
        viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
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
    const QString selectedLib = getCurrentLibraryName();
    const QString selectedGroup = getCurrentGroupName();

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

    clearLibraryViewProperties();

    QSet<QString> loadedLibraries;

    for(const LibDefinition& def : data.definitions) {

        const QString libName = def.name.trimmed();
        QString libPath = expandShellVariables(QDir::fromNativeSeparators(def.path.trimmed()));

        if (!QDir::isAbsolutePath(libPath)) {
            const QDir projectDir = QFileInfo(fileName).absoluteDir();
            libPath = QDir::toNativeSeparators(projectDir.absoluteFilePath(libPath));
        }

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

        QString groupName;
        QString viewName;
        if (!resolveCellViewFromPath(fi.absoluteFilePath(), &groupName, &viewName)) {
            error(QString("Skipping file with unknown view type: %1").arg(fi.absoluteFilePath()));
            continue;
        }

        if (groupName.isEmpty()) {
            error(QString("Skipping file with empty group name: %1").arg(fi.absoluteFilePath()));
            continue;
        }

        const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
        m_properties->set(key, fi.absoluteFilePath());

        loadedLibraries.insert(libName);
    }

    loadLibraries();

    if (!selectedLib.isEmpty()) {
        for (int i = 0; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
            QTreeWidgetItem *libItem = m_ui->treeLibs->topLevelItem(i);
            if (!libItem || libItem->text(0) != selectedLib) {
                continue;
            }
            m_ui->treeLibs->setCurrentItem(libItem);
            populateLibraryBrowser(selectedLib);
            break;
        }
    }

    if (!selectedLib.isEmpty() && !selectedGroup.isEmpty()) {
        for (int i = 0; i < m_ui->listGroups->count(); ++i) {
            QListWidgetItem *groupItem = m_ui->listGroups->item(i);
            if (!groupItem || groupItem->text() != selectedGroup) {
                continue;
            }
            m_ui->listGroups->setCurrentItem(groupItem);
            loadViews(selectedLib, selectedGroup);
            break;
        }
    }

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

void MainWindow::clearLibraryViewProperties()
{
    if (!m_properties) {
        return;
    }

    const QString prefix = getLibraryKeyPrefix();
    const QMap<QString, PropertyItem *> props = m_properties->getMap();
    QStringList keysToRemove;

    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
        if (it.key().startsWith(prefix, Qt::CaseInsensitive)) {
            keysToRemove.append(it.key());
        }
    }

    for (const QString &key : keysToRemove) {
        m_properties->remove(key);
    }
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

QList<QPair<QString, QString>> MainWindow::projectEntriesForEditor() const
{
    if (!m_currentProjFile.isEmpty()) {
        LibFileParser parser;
        if (parser.parseFile(m_currentProjFile)) {
            QList<QPair<QString, QString>> rows;
            for (const LibDefinition &def : parser.data().definitions) {
                rows.append(qMakePair(def.name.trimmed(), def.path.trimmed()));
            }
            return rows;
        }
    }

    QList<QPair<QString, QString>> rows;
    const QList<QPair<QString, QString>> absoluteEntries = getCurrentProjectEntries();
    for (const auto &entry : absoluteEntries) {
        rows.append(qMakePair(entry.first,
                              QDir::toNativeSeparators(QFileInfo(entry.second).absoluteFilePath())));
    }

    return rows;
}

bool MainWindow::saveProjectEntriesToFile(const QString &fileName,
                                          const QList<QPair<QString, QString>> &entries)
{
    if (fileName.isEmpty()) {
        return false;
    }

    const QString absFileName = QFileInfo(fileName).absoluteFilePath();

    m_ignoreProjectFileChange = true;

    if (m_projFileWatcher) {
        m_projFileWatcher->removePath(absFileName);
    }

    QFile file(absFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (m_projFileWatcher && QFileInfo(absFileName).exists()) {
            m_projFileWatcher->addPath(absFileName);
        }
        m_ignoreProjectFileChange = false;

        QMessageBox::warning(this, tr("LibManager"),
                             tr("Can not write to file '%1':\n%2.")
                                 .arg(absFileName)
                                 .arg(file.errorString()));
        error("Can not write to file '" + absFileName + "'.");
        return false;
    }

    QTextStream out(&file);
    const QDir baseDir = QFileInfo(absFileName).absoluteDir();

    out << "# KLayout library definition file\n";
    out << "# Generated by LibMan\n\n";

    for (const auto &entry : entries) {
        const QString libName = entry.first.trimmed();
        QString filePath = entry.second.trimmed();

        if (libName.isEmpty() || filePath.isEmpty()) {
            continue;
        }

        const QString resolvedPath = QDir::isAbsolutePath(filePath)
            ? filePath
            : baseDir.absoluteFilePath(filePath);
        const QFileInfo fi(resolvedPath);
        if (!fi.exists() || !fi.isFile()) {
            error(QString("Skipping library entry '%1': file does not exist: %2")
                      .arg(libName, resolvedPath));
            continue;
        }

        const QString storedPath = QDir::isAbsolutePath(filePath)
            ? QDir::toNativeSeparators(baseDir.relativeFilePath(fi.absoluteFilePath()))
            : QDir::toNativeSeparators(filePath);

        out << "define("
            << toLibStringLiteral(libName)
            << ", "
            << toLibStringLiteral(storedPath)
            << ");\n";
    }

    file.close();

    if (m_projFileWatcher && QFileInfo(absFileName).exists()) {
        m_projFileWatcher->addPath(absFileName);
    }

    m_ignoreProjectFileChange = false;
    return true;
}

/*!******************************************************************************************************************
 * \brief Saves LibMan libraries into KLayout .lib file.
 *******************************************************************************************************************/
void MainWindow::saveProjectFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return;
    }

    if (!saveProjectEntriesToFile(fileName, getCurrentProjectEntries())) {
        return;
    }

    const QString absFileName = QFileInfo(fileName).absoluteFilePath();
    m_currentProjFile = absFileName;

    QString fileTitle = QFileInfo(absFileName).completeBaseName();
    const QString fileSuffix = QFileInfo(absFileName).completeSuffix();
    if (!fileSuffix.isEmpty()) {
        fileTitle += "." + fileSuffix;
    }

    setWindowTitle(getLibManTitle() + " (" + fileTitle + ")");
    setupProjectFileWatcher(absFileName);
    setRecentProject(absFileName);

    info(QString("Project '%1' has been saved.").arg(absFileName));
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
