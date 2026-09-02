#include "core_export_service.h"

#include "mainwindow.h"
#include "core/converter_paths.h"
#include "core/core_path_utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

#ifndef LIBMAN_NO_CORE
#include "cell_content.h"
#include "database.h"
#include "enums.h"

#include <unordered_set>
#endif

CoreExportService::CoreExportService(MainWindow *mainWindow)
    : m_mainWindow(mainWindow)
{
}

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

#ifndef LIBMAN_NO_CORE
void writeExportedXschemRc(const QString &destRoot)
{
    const QString path = QDir(destRoot).filePath(QStringLiteral("xschemrc"));
    if (QFileInfo::exists(path)) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    // Match XSchem-coredb/integrations/xschem-batch.rc: devices + PDK parent of sg13g2_pr.
    out << "# LibMan Xschem export — hierarchical netlist needs devices + PDK parent path\n"
        << "set xschem_execute_scripts yes\n"
        << "set share [file join $::env(HOME) .local share xschem]\n"
        << "if {[file exists [file join $share xschemrc]]} {\n"
        << "  source [file join $share xschemrc]\n"
        << "}\n"
        << "if {![info exists XSCHEM_LIBRARY_PATH]} { set XSCHEM_LIBRARY_PATH {} }\n"
        << "if {[file isdirectory [file join $share xschem_library]]} {\n"
        << "  append XSCHEM_LIBRARY_PATH :[file join $share xschem_library]\n"
        << "  append XSCHEM_LIBRARY_PATH :[file join $share xschem_library devices]\n"
        << "}\n"
        << "if {![info exists ::env(PDK_ROOT)] || $::env(PDK_ROOT) eq \"\"} {\n"
        << "  foreach pdk [list /mnt/c/Work/IHP [file join $::env(HOME) IHP-Open-PDK] "
           "/mnt/c/Users/anton/Documents/IHP-Open-PDK] {\n"
        << "    if {[file isdirectory [file join $pdk ihp-sg13g2]]} {\n"
        << "      set ::env(PDK_ROOT) $pdk\n"
        << "      break\n"
        << "    }\n"
        << "  }\n"
        << "}\n"
        << "if {![info exists ::env(PDK)] || $::env(PDK) eq \"\"} { set ::env(PDK) ihp-sg13g2 }\n"
        << "if {[info exists ::env(PDK_ROOT)] && $::env(PDK_ROOT) ne \"\"} {\n"
        << "  set pdk_xschem [file join $::env(PDK_ROOT) $::env(PDK) libs.tech xschem]\n"
        << "  if {[file isdirectory $pdk_xschem]} { append XSCHEM_LIBRARY_PATH :$pdk_xschem }\n"
        << "}\n"
        << "append XSCHEM_LIBRARY_PATH :[file dirname [info script]]\n";
}

void CoreExportService::exportXschemReferencedCells(const QString &converterPath, const QString &destRoot,
                                                    const QString &sourceCorePath, const QString &cellName) const
{
    if (m_mainWindow == nullptr) {
        return;
    }

    QStringList techLibs = m_mainWindow->getTechLibraryAttachList(m_mainWindow->getCurrentLibraryName());
    if (techLibs.isEmpty()) {
        const QString envTech = qEnvironmentVariable("LIBMAN_TECH_LIBRARY").trimmed();
        if (!envTech.isEmpty()) {
            techLibs = envTech.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        }
    }

    std::unordered_set<std::string> models;
    try {
        const core::Database db = core::Database::loadFromFile(sourceCorePath.toStdString());
        const core::Cell *cell = db.lib().findCell(cellName.toStdString());
        if (cell != nullptr) {
            if (const core::CellContent *content = cell->findContent(core::ViewType::Schematic)) {
                for (const core::Instance &inst : content->block().instances()) {
                    if (inst.cellName() != "Lib") {
                        continue;
                    }
                    for (const core::Property &prop : inst.properties()) {
                        if (prop.name == "param.1" && !prop.value.empty()) {
                            models.insert(prop.value);
                        }
                    }
                }
            }
        }
    } catch (...) {
        return;
    }

    for (const std::string &model : models) {
        const QString modelName = QString::fromStdString(model);
        for (const QString &viewStem : {QStringLiteral("schematic"), QStringLiteral("symbol")}) {
            const QString needle = modelName + QLatin1Char('.') + viewStem + QStringLiteral(".core");
            QString corePath;
            for (const QString &techLib : techLibs) {
                for (const QString &path : m_mainWindow->resolveTechLibraryCorePaths(techLib)) {
                    if (path.endsWith(needle, Qt::CaseInsensitive)) {
                        corePath = path;
                        break;
                    }
                }
                if (!corePath.isEmpty()) {
                    break;
                }
            }
            if (corePath.isEmpty()) {
                continue;
            }

            const QString extension = viewStem == QStringLiteral("symbol") ? QStringLiteral(".sym")
                                                                         : QStringLiteral(".sch");
            const QString destinationPath =
                QFileInfo(QDir(destRoot).filePath(modelName + extension)).absoluteFilePath();
            if (QFileInfo::exists(destinationPath)) {
                continue;
            }

            QProcess process;
            process.setProgram(converterPath);
            process.setArguments(
                {QDir::toNativeSeparators(corePath), modelName, QDir::toNativeSeparators(destinationPath)});
            process.setProcessChannelMode(QProcess::MergedChannels);
            process.start();
            if (process.waitForStarted(10000)) {
                process.waitForFinished(600000);
            }
        }
    }
}
#endif

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

#ifndef LIBMAN_NO_CORE
    if (format == Format::Xschem && identity.viewName.compare(QStringLiteral("schematic"), Qt::CaseInsensitive) == 0) {
        writeExportedXschemRc(destRoot);
        exportXschemReferencedCells(converterPath, destRoot, sourceInfo.absoluteFilePath(), identity.cellName);
    }
#endif

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
