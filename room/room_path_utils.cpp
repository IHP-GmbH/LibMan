#include "room/room_path_utils.h"

#include <QDir>
#include <QFileInfo>

namespace {

bool endsWithCore(const QString &name)
{
    return name.endsWith(QStringLiteral(".room"), Qt::CaseInsensitive);
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

RoomViewIdentity parseRoomViewIdentity(const QString &filePath)
{
    RoomViewIdentity identity;
    const QFileInfo fi(filePath);
    const QString baseName = fi.fileName();
    if (!endsWithCore(baseName)) {
        return identity;
    }

    const QString stem = baseName.left(baseName.size() - QStringLiteral(".room").size());
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

bool isRoomViewName(const QString &viewName)
{
    const QString normalized = normalizedViewSuffix(viewName);
    return isKnownCoreView(normalized) || normalized == QStringLiteral("core");
}

bool isLayoutRoomViewName(const QString &viewName)
{
    const QString normalized = normalizedViewSuffix(viewName);
    return normalized == QStringLiteral("layout") || normalized == QStringLiteral("core");
}

QString roomViewFileName(const QString &cellName, const QString &viewName)
{
    const QString normalized = normalizedViewSuffix(viewName);
    if (normalized == QStringLiteral("core")) {
        return cellName + QStringLiteral(".layout.room");
    }
    return cellName + QLatin1Char('.') + normalized + QStringLiteral(".room");
}

QString roomViewFilePath(const QString &directory, const QString &cellName, const QString &viewName)
{
    return QFileInfo(QDir(directory).filePath(roomViewFileName(cellName, viewName))).absoluteFilePath();
}

bool isEmSetupViewName(const QString &viewName)
{
    return viewName.trimmed().toLower() == QStringLiteral("emsetup");
}

bool isEmSetupDirName(const QString &dirName)
{
    return dirName.endsWith(QStringLiteral(".emsetup"), Qt::CaseInsensitive);
}

RoomViewIdentity parseEmSetupIdentity(const QString &dirPath)
{
    RoomViewIdentity identity;
    const QString name = QFileInfo(dirPath).fileName();
    if (!isEmSetupDirName(name)) {
        return identity;
    }
    const QString stem = name.left(name.size() - QStringLiteral(".emsetup").size()).trimmed();
    if (stem.isEmpty()) {
        return identity;
    }
    identity.cellName = stem;
    identity.viewName = QStringLiteral("emsetup");
    identity.valid = true;
    return identity;
}

QString emSetupDirName(const QString &cellName)
{
    return cellName.trimmed() + QStringLiteral(".emsetup");
}

QString emSetupDirPath(const QString &cellDirectory, const QString &cellName)
{
    return QFileInfo(QDir(cellDirectory).filePath(emSetupDirName(cellName))).absoluteFilePath();
}

QString emSetupDefaultVariantName()
{
    return QStringLiteral("nominal");
}
