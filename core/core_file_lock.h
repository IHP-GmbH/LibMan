#ifndef CORE_FILE_LOCK_H
#define CORE_FILE_LOCK_H

#include <QString>

struct CoreFileLockInfo
{
    bool    present   = false;
    bool    parseOk   = false;
    QString corePath;
    QString lockPath;
    QString user;
    QString host;
    QString tool;
    QString toolVersion;
    QString createdAt;
    int     pid       = 0;
    QString parseError;
};

QString lockFilePathForCore(const QString &corePath);

CoreFileLockInfo readCoreLockFile(const QString &corePath);

bool isStaleCoreLock(const CoreFileLockInfo &info);

// If lock is stale (holder process gone), delete the .lck and return empty info.
CoreFileLockInfo readActiveCoreLockFile(const QString &corePath);

QString formatCoreLockInfoBlock(const CoreFileLockInfo &info);

#endif // CORE_FILE_LOCK_H
