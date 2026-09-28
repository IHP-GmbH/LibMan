#ifndef ROOM_PATH_UTILS_H
#define ROOM_PATH_UTILS_H

#include <QString>

struct RoomViewIdentity {
    QString cellName;
    QString viewName;
    bool    valid = false;
};

RoomViewIdentity parseRoomViewIdentity(const QString &filePath);
bool isRoomViewName(const QString &viewName);
bool isLayoutRoomViewName(const QString &viewName);
QString roomViewFileName(const QString &cellName, const QString &viewName);
QString roomViewFilePath(const QString &directory, const QString &cellName, const QString &viewName);

#endif // ROOM_PATH_UTILS_H
