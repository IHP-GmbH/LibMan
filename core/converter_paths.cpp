#include "core/converter_paths.h"

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
        QStringLiteral("gds_to_core"),
        QStringLiteral("xschem_to_core"),
        QStringLiteral("qucs_to_core"),
        QStringLiteral("oas_to_core"),
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
