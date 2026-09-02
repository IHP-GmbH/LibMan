#ifndef TST_CORE_FILE_LOCK_H
#define TST_CORE_FILE_LOCK_H

#include <QtTest/QtTest>

class CoreFileLockTest : public QObject
{
    Q_OBJECT

private slots:
    void lockPath_appendsLckSuffix();
    void readLockFile_missingReturnsNotPresent();
    void readLockFile_parsesHolderMetadata();
};

#endif // TST_CORE_FILE_LOCK_H
