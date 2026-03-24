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
#include <QRegularExpression>

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

    for(const LibDefinition& def : data.definitions) {

        const QString libName = def.name.trimmed();
        const QString libPath = expandShellVariables(def.path.trimmed());

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

        QString key = getLibraryKeyPrefix() + libName + "/" + viewName;
        m_properties->set(key, fi.absoluteFilePath());
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
    const QDir baseDir = QFileInfo(fileName).absoluteDir();

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

        QFileInfo fi(libPath);

        if(!fi.exists()) {
            error(QString("Skipping library '%1': file does not exist: %2")
                      .arg(libName, libPath));
            continue;
        }

        if(!fi.isFile()) {
            error(QString("Skipping library '%1': path is not a file: %2")
                      .arg(libName, libPath));
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
