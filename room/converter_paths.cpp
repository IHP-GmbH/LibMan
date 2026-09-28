#include "room/converter_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace {

QString converterFileName(const QString &baseName)
{
    QString name = baseName.trimmed();
#ifdef Q_OS_WIN
    if (!name.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
        name += QStringLiteral(".exe");
    }
#endif
    return name;
}

} // namespace

QStringList coreImportConverterNames()
{
    return {
        QStringLiteral("gds_to_room"),
        QStringLiteral("xschem_to_room"),
        QStringLiteral("qucs_to_room"),
        QStringLiteral("oas_to_room"),
    };
}

QStringList coreExportConverterNames()
{
    return {
        QStringLiteral("room_to_gds"),
        QStringLiteral("room_to_xschem"),
        QStringLiteral("room_to_qucs"),
    };
}

QString findCoreConverterExecutable(const QString &baseName)
{
    if (baseName.trimmed().isEmpty()) {
        return {};
    }

    QStringList searchDirs;
    const QString envDir = qEnvironmentVariable("LIBMAN_CONVERTER_DIR").trimmed();
    if (!envDir.isEmpty()) {
        searchDirs << QDir(envDir).absolutePath();
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    searchDirs << appDir;
    searchDirs << QDir(appDir).filePath(QStringLiteral("tools"));
    searchDirs << QDir(appDir).filePath(QStringLiteral("converters"));

    const QString fileName = converterFileName(baseName);
    for (const QString &dirPath : searchDirs) {
        const QString candidate = QDir(dirPath).filePath(fileName);
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return {};
}
