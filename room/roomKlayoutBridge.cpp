#include "room/roomKlayoutBridge.h"
#include "room/room_path_utils.h"

#include <QDir>
#include <QFileInfo>

QString roomLayoutPathForKLayout(const QString &viewPath, QStringList *errors)
{
    const QFileInfo fi(viewPath);
    if (!fi.exists() || !fi.isFile()) {
        if (errors) {
            *errors << QString("ROOM file not found: %1").arg(viewPath);
        }
        return QString();
    }

    const RoomViewIdentity identity = parseRoomViewIdentity(viewPath);
    if (identity.valid && !isLayoutRoomViewName(identity.viewName)) {
        if (errors) {
            *errors << QString("ROOM file is not a layout view: %1").arg(viewPath);
        }
        return QString();
    }

    const bool isCoreLayout = identity.valid
        || fi.suffix().compare(QStringLiteral("core"), Qt::CaseInsensitive) == 0;

    if (isCoreLayout) {
        // KLayout opens *.layout.room natively via the mcore streamer plugin.
        return QDir::toNativeSeparators(fi.absoluteFilePath());
    }

    if (errors) {
        *errors << QString("Not a ROOM layout file: %1").arg(viewPath);
    }
    return QString();
}
