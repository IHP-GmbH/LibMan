#include "room/roomcellreader.h"
#include "room/roomKlayoutBridge.h"
#include "src/mainwindow.h"

#include <QFileInfo>

RoomCellReader::RoomCellReader(const QString &fileName)
    : m_fileName(fileName)
{
}

void RoomCellReader::coreCreate(const QString &cellName, const QString &viewName)
{
    Q_UNUSED(cellName);
    Q_UNUSED(viewName);
    m_errorList << QStringLiteral("ROOM support is not built (CONFIG+=no_room).");
}

bool RoomCellReader::readHierarchy(CoreHierarchy &out)
{
    Q_UNUSED(out);
    m_errorList << QStringLiteral("ROOM support is not built (CONFIG+=no_room).");
    return false;
}

QString roomLayoutPathForKLayout(const QString &viewPath, QStringList *errors)
{
    const QFileInfo fi(viewPath);
    if(!fi.exists() || !fi.isFile()) {
        if(errors) {
            *errors << QStringLiteral("ROOM file not found: %1").arg(viewPath);
        }
        return QString();
    }

    return fi.absoluteFilePath();
}

void MainWindow::loadRoomHierarchyAsync(const QString &corePath,
                                        const std::shared_ptr<CoreCacheEntry> &entry,
                                        QTreeWidgetItem *targetItem,
                                        const QString &requestedCellName)
{
    Q_UNUSED(corePath);
    Q_UNUSED(targetItem);
    Q_UNUSED(requestedCellName);

    if(!entry) {
        return;
    }

    entry->loading = false;
    entry->loaded = false;
    entry->errors << QStringLiteral("ROOM support is not built (CONFIG+=no_room).");
}
