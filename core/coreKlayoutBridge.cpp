#include "core/coreKlayoutBridge.h"

#include "database.h"
#include "gds_exporter.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

QString coreLayoutPathForKLayout(const QString &viewPath, QStringList *errors)
{
    const QFileInfo fi(viewPath);
    if (!fi.exists() || !fi.isFile()) {
        if (errors) {
            *errors << QString("CORE file not found: %1").arg(viewPath);
        }
        return QString();
    }

    if (fi.suffix().compare(QStringLiteral("core"), Qt::CaseInsensitive) != 0) {
        return fi.absoluteFilePath();
    }

    const QString tempGds = QDir::temp().filePath(
        QStringLiteral("libman_%1_%2.gds")
            .arg(fi.completeBaseName())
            .arg(fi.lastModified().toSecsSinceEpoch()));

    try {
        const core::Database db = core::Database::loadFromFile(fi.absoluteFilePath().toStdString());
        core::GdsExporter exporter;
        exporter.exportFile(db, tempGds.toStdString());

        if (!exporter.errors().empty()) {
            if (errors) {
                for (const std::string &e : exporter.errors()) {
                    errors->append(QString::fromStdString(e));
                }
            }
            return QString();
        }

        if (!QFileInfo::exists(tempGds)) {
            if (errors) {
                *errors << QString("Failed to create temporary GDS for KLayout: %1").arg(tempGds);
            }
            return QString();
        }

        return QDir::toNativeSeparators(tempGds);
    }
    catch (const std::exception &e) {
        if (errors) {
            *errors << QString::fromUtf8(e.what());
        }
        return QString();
    }
}
