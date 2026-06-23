#include "core/core_path_utils.h"

#include <QDir>
#include <QFileInfo>

namespace {

bool endsWithCore(const QString &name)
{
    return name.endsWith(QStringLiteral(".core"), Qt::CaseInsensitive);
}

QString normalizedViewSuffix(const QString &suffix)
{
    const QString lower = suffix.trimmed().toLower();
    if (lower == QStringLiteral("sch")) {
        return QStringLiteral("schematic");
    }
    if (lower == QStringLiteral("sym")) {
        return QStringLiteral("symbol");
    }
    if (lower == QStringLiteral("abs")) {
        return QStringLiteral("abstract");
    }
    return lower;
}

bool isKnownCoreView(const QString &viewName)
{
    return viewName == QStringLiteral("layout")
        || viewName == QStringLiteral("schematic")
        || viewName == QStringLiteral("symbol")
        || viewName == QStringLiteral("abstract");
}

} // namespace

CoreViewIdentity parseCoreViewIdentity(const QString &filePath)
{
    CoreViewIdentity identity;
    const QFileInfo fi(filePath);
    const QString baseName = fi.fileName();
    if (!endsWithCore(baseName)) {
        return identity;
    }

    const QString stem = baseName.left(baseName.size() - QStringLiteral(".core").size());
    const int dot = stem.lastIndexOf(QLatin1Char('.'));
    if (dot <= 0) {
        identity.cellName = stem.trimmed();
        identity.viewName = QStringLiteral("layout");
        identity.valid = !identity.cellName.isEmpty();
        return identity;
    }

    const QString viewName = normalizedViewSuffix(stem.mid(dot + 1));
    if (!isKnownCoreView(viewName)) {
        return identity;
    }

    identity.cellName = stem.left(dot).trimmed();
    identity.viewName = viewName;
    identity.valid = !identity.cellName.isEmpty();
    return identity;
}

bool isCoreViewName(const QString &viewName)
{
    const QString normalized = normalizedViewSuffix(viewName);
    return isKnownCoreView(normalized) || normalized == QStringLiteral("core");
}

bool isLayoutCoreViewName(const QString &viewName)
{
    const QString normalized = normalizedViewSuffix(viewName);
    return normalized == QStringLiteral("layout") || normalized == QStringLiteral("core");
}

QString coreViewFileName(const QString &cellName, const QString &viewName)
{
    const QString normalized = normalizedViewSuffix(viewName);
    if (normalized == QStringLiteral("core")) {
        return cellName + QStringLiteral(".layout.core");
    }
    return cellName + QLatin1Char('.') + normalized + QStringLiteral(".core");
}

QString coreViewFilePath(const QString &directory, const QString &cellName, const QString &viewName)
{
    return QFileInfo(QDir(directory).filePath(coreViewFileName(cellName, viewName))).absoluteFilePath();
}
