#include "core_export_service.h"

#include "mainwindow.h"
#include "core/converter_paths.h"
#include "core/core_path_utils.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

namespace {

QString normalizedViewName(const QString &viewName)
{
    const QString lower = viewName.trimmed().toLower();
    if (lower == QStringLiteral("core")) {
        return QStringLiteral("layout");
    }
    return lower;
}

} // namespace

CoreExportService::CoreExportService(MainWindow *mainWindow)
    : m_mainWindow(mainWindow)
{
}

QString CoreExportService::formatDisplayName(Format format)
{
    switch (format) {
    case Format::Gds:
        return QStringLiteral("GDS");
    case Format::Xschem:
        return QStringLiteral("Xschem");
    case Format::Qucs:
        return QStringLiteral("Qucs");
    }
    return {};
}

QStringList CoreExportService::sourceNameFilters()
{
    return {
        QStringLiteral("*.core"),
        QStringLiteral("*.layout.core"),
        QStringLiteral("*.schematic.core"),
        QStringLiteral("*.symbol.core"),
    };
}

QString CoreExportService::converterBaseName(Format format)
{
    switch (format) {
    case Format::Gds:
        return QStringLiteral("core_to_gds");
    case Format::Xschem:
        return QStringLiteral("core_to_xschem");
    case Format::Qucs:
        return QStringLiteral("core_to_qucs");
    }
    return {};
}

bool CoreExportService::coreViewMatchesFormat(Format format, const QString &viewName)
{
    const QString view = normalizedViewName(viewName);
    switch (format) {
    case Format::Gds:
        return view == QStringLiteral("layout");
    case Format::Xschem:
        return view == QStringLiteral("schematic") || view == QStringLiteral("symbol");
    case Format::Qucs:
        return view == QStringLiteral("schematic");
    }
    return false;
}

QVector<CoreExportService::ExportItemResult> CoreExportService::exportFiles(Format format,
                                                                            const QString &destinationDir,
                                                                            const QStringList &sourceFiles) const
{
    QVector<ExportItemResult> results;
    results.reserve(sourceFiles.size());

    for (const QString &sourcePath : sourceFiles) {
        results.push_back(exportOne(format, destinationDir, sourcePath));
    }

    return results;
}

QString CoreExportService::destinationFilePath(const QString &destinationDir,
                                               const QString &cellName,
                                               const QString &viewName) const
{
    const QString view = normalizedViewName(viewName);
    QString extension;
    if (view == QStringLiteral("layout")) {
        extension = QStringLiteral(".gds");
    } else if (view == QStringLiteral("symbol")) {
        extension = QStringLiteral(".sym");
    } else {
        extension = QStringLiteral(".sch");
    }

    return QFileInfo(QDir(destinationDir).filePath(cellName + extension)).absoluteFilePath();
}

CoreExportService::ExportItemResult CoreExportService::exportOne(Format format,
                                                                 const QString &destinationDir,
                                                                 const QString &sourcePath) const
{
    ExportItemResult result;
    result.sourcePath = sourcePath;

    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        result.message = QStringLiteral("Source file does not exist.");
        return result;
    }

    const CoreViewIdentity identity = parseCoreViewIdentity(sourceInfo.absoluteFilePath());
    if (!identity.valid) {
        result.message = QStringLiteral("Not a recognized CORE view file.");
        return result;
    }

    if (!coreViewMatchesFormat(format, identity.viewName)) {
        result.message = QStringLiteral("View '%1' cannot be exported as %2.")
                             .arg(identity.viewName, formatDisplayName(format));
        return result;
    }

    const QString destRoot = destinationDir.trimmed();
    if (destRoot.isEmpty()) {
        result.message = QStringLiteral("Select a destination folder.");
        return result;
    }

    if (!QDir().mkpath(destRoot)) {
        result.message = QStringLiteral("Failed to create destination folder '%1'.").arg(destRoot);
        return result;
    }

    const QString destinationPath = destinationFilePath(destRoot, identity.cellName, identity.viewName);
    result.destinationPath = destinationPath;

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
        arguments << QDir::toNativeSeparators(sourceInfo.absoluteFilePath())
                  << QDir::toNativeSeparators(destinationPath);
        break;
    case Format::Xschem:
    case Format::Qucs:
        arguments << QDir::toNativeSeparators(sourceInfo.absoluteFilePath())
                  << identity.cellName
                  << QDir::toNativeSeparators(destinationPath);
        break;
    }

    if (!runConverter(converterPath, arguments, &conversionError)) {
        result.message = conversionError;
        return result;
    }

    if (!QFileInfo::exists(destinationPath)) {
        result.message = QStringLiteral("Converter finished but output file is missing.");
        return result;
    }

    result.success = true;
    result.message = QFileInfo(destinationPath).fileName();
    return result;
}

bool CoreExportService::runConverter(const QString &program,
                                     const QStringList &arguments,
                                     QString *errorMessage) const
{
    Q_UNUSED(m_mainWindow);

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
