#include "core_import_service.h"

#include "mainwindow.h"
#include "core/converter_paths.h"
#include "core/core_path_utils.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QProcess>

namespace {

QString cellNameFromSource(const QString &sourcePath)
{
    return QFileInfo(sourcePath).completeBaseName().trimmed();
}

QString viewNameForXschemSource(const QString &sourcePath)
{
    const QString suffix = QFileInfo(sourcePath).suffix().trimmed().toLower();
    if (suffix == QStringLiteral("sym")) {
        return QStringLiteral("symbol");
    }
    return QStringLiteral("schematic");
}

} // namespace

CoreImportService::CoreImportService(MainWindow *mainWindow)
    : m_mainWindow(mainWindow)
{
}

QString CoreImportService::formatDisplayName(Format format)
{
    switch (format) {
    case Format::Gds:
        return QStringLiteral("GDS");
    case Format::Xschem:
        return QStringLiteral("Xschem");
    case Format::Qucs:
        return QStringLiteral("Qucs");
    case Format::Oas:
        return QStringLiteral("OAS");
    }
    return {};
}

QStringList CoreImportService::sourceNameFilters(Format format)
{
    switch (format) {
    case Format::Gds:
        return {QStringLiteral("*.gds"), QStringLiteral("*.gds2")};
    case Format::Xschem:
        return {QStringLiteral("*.sch"), QStringLiteral("*.sym")};
    case Format::Qucs:
        return {QStringLiteral("*.sch")};
    case Format::Oas:
        return {QStringLiteral("*.oas"), QStringLiteral("*.oas.gz")};
    }
    return {};
}

QString CoreImportService::converterBaseName(Format format)
{
    switch (format) {
    case Format::Gds:
        return QStringLiteral("gds_to_core");
    case Format::Xschem:
        return QStringLiteral("xschem_to_core");
    case Format::Qucs:
        return QStringLiteral("qucs_to_core");
    case Format::Oas:
        return QStringLiteral("oas_to_core");
    }
    return {};
}

QVector<CoreImportService::ImportItemResult> CoreImportService::importFiles(Format format,
                                                                              const QString &libraryName,
                                                                              const QStringList &sourceFiles) const
{
    QVector<ImportItemResult> results;
    results.reserve(sourceFiles.size());

    for (const QString &sourcePath : sourceFiles) {
        results.push_back(importOne(format, libraryName, sourcePath));
    }

    return results;
}

QString CoreImportService::libraryRootPath(const QString &libraryName) const
{
    if (!m_mainWindow) {
        return {};
    }
    return m_mainWindow->getLibraryPath(libraryName);
}

QString CoreImportService::destinationCorePath(const QString &libraryRoot,
                                               const QString &cellName,
                                               const QString &viewName) const
{
    const QString cellDirPath = QDir(libraryRoot).filePath(cellName);
    return coreViewFilePath(cellDirPath, cellName, viewName);
}

CoreImportService::ImportItemResult CoreImportService::importOne(Format format,
                                                                 const QString &libraryName,
                                                                 const QString &sourcePath) const
{
    ImportItemResult result;
    result.sourcePath = sourcePath;

    if (!m_mainWindow) {
        result.message = QStringLiteral("Internal error: main window is not available.");
        return result;
    }

    if (libraryName.trimmed().isEmpty()) {
        result.message = QStringLiteral("Select a target library.");
        return result;
    }

    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        result.message = QStringLiteral("Source file does not exist.");
        return result;
    }

    QString libraryRoot = libraryRootPath(libraryName);
    if (libraryRoot.isEmpty()) {
        if (m_mainWindow->getCurrentProjectFile().isEmpty()) {
            result.message = QStringLiteral("Failed to resolve library path for '%1'.").arg(libraryName);
            return result;
        }
        libraryRoot = QFileInfo(m_mainWindow->getCurrentProjectFile()).absoluteDir().absolutePath();
        m_mainWindow->setLibraryRootDirectory(libraryName, libraryRoot);
    }

    const QString cellName = cellNameFromSource(sourcePath);
    if (cellName.isEmpty()) {
        result.message = QStringLiteral("Failed to determine cell name from '%1'.").arg(sourcePath);
        return result;
    }

    if (format == Format::Oas) {
        const QString converterPath = findCoreConverterExecutable(converterBaseName(format));
        if (converterPath.isEmpty()) {
            if (m_mainWindow->importCellViewFile(libraryName, sourcePath)) {
                result.success = true;
                result.message = QStringLiteral("Imported as native OAS layout view.");
            } else {
                result.message = QStringLiteral("Failed to import OAS layout file.");
            }
            return result;
        }
    }

    QString viewName;
    switch (format) {
    case Format::Gds:
    case Format::Oas:
        viewName = QStringLiteral("layout");
        break;
    case Format::Xschem:
        viewName = viewNameForXschemSource(sourcePath);
        break;
    case Format::Qucs:
        viewName = QStringLiteral("schematic");
        break;
    }

    const QString cellDirPath = QDir(libraryRoot).filePath(cellName);
    if (!QDir().mkpath(cellDirPath)) {
        result.message = QStringLiteral("Failed to create cell directory '%1'.").arg(cellDirPath);
        return result;
    }

    const QString destinationPath = destinationCorePath(libraryRoot, cellName, viewName);
    if (QFileInfo::exists(destinationPath)) {
        result.destinationPath = destinationPath;
        result.message = QStringLiteral("Cell view already exists.");
        return result;
    }

    const QString converterPath = findCoreConverterExecutable(converterBaseName(format));
    if (converterPath.isEmpty()) {
        result.message = QStringLiteral("Converter '%1' was not found next to LibMan.")
                             .arg(converterBaseName(format));
        return result;
    }

    QStringList arguments;
    QString conversionError;
    switch (format) {
    case Format::Gds:
        arguments << QStringLiteral("--all-cells")
                  << QDir::toNativeSeparators(sourceInfo.absoluteFilePath())
                  << QDir::toNativeSeparators(destinationPath)
                  << QDir::toNativeSeparators(QDir::temp().filePath(
                         QStringLiteral("libman_gds_%1.txt").arg(cellName)));
        break;
    case Format::Xschem:
    case Format::Qucs:
    case Format::Oas:
        arguments << QDir::toNativeSeparators(sourceInfo.absoluteFilePath())
                  << QDir::toNativeSeparators(destinationPath);
        break;
    }

    if (!runConverter(converterPath, arguments, &conversionError)) {
        QFile::remove(destinationPath);
        result.message = conversionError;
        return result;
    }

    if (!QFileInfo::exists(destinationPath)) {
        result.message = QStringLiteral("Converter finished but output file is missing.");
        return result;
    }

    if (!m_mainWindow->registerCellViewAtPath(libraryName, cellName, viewName, destinationPath)) {
        QFile::remove(destinationPath);
        result.message = QStringLiteral("Conversion succeeded but registration in the project failed.");
        return result;
    }

    result.destinationPath = destinationPath;
    result.success = true;
    result.message = QStringLiteral("Imported as %1/%2/%3.")
                         .arg(libraryName, cellName, viewName);
    return result;
}

bool CoreImportService::runConverter(const QString &program,
                                     const QStringList &arguments,
                                     QString *errorMessage) const
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();

    if (!process.waitForStarted(10000)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to start converter: %1").arg(process.errorString());
        }
        return false;
    }

    if (!process.waitForFinished(600000)) {
        process.kill();
        if (errorMessage) {
            *errorMessage = QStringLiteral("Converter timed out.");
        }
        return false;
    }

    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage) {
            const QString details = output.trimmed();
            if (details.isEmpty()) {
                *errorMessage = QStringLiteral("Converter failed with exit code %1.").arg(process.exitCode());
            } else {
                *errorMessage = QStringLiteral("Converter failed: %1").arg(details);
            }
        }
        return false;
    }

    return true;
}
