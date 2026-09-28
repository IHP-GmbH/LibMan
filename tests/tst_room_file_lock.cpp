#include "tst_room_file_lock.h"

#include "room/room_file_lock.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

void CoreFileLockTest::lockPath_appendsLckSuffix()
{
    const QString corePath = QStringLiteral("/tmp/cell.schematic.room");
    const QString lockPath = lockFilePathForCore(corePath);
    QCOMPARE(QFileInfo(lockPath).fileName(), QStringLiteral("cell.schematic.room.lck"));
    QVERIFY(lockPath.endsWith(QStringLiteral(".room.lck")));
}

void CoreFileLockTest::readLockFile_missingReturnsNotPresent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString corePath = dir.filePath(QStringLiteral("inv.schematic.room"));
    const CoreFileLockInfo info = readCoreLockFile(corePath);

    QVERIFY(!info.present);
    QCOMPARE(info.lockPath, lockFilePathForCore(corePath));
}

void CoreFileLockTest::readLockFile_parsesHolderMetadata()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString corePath = dir.filePath(QStringLiteral("inv.schematic.room"));
    const QString lockPath = lockFilePathForCore(corePath);

    QFile lockFile(lockPath);
    QVERIFY(lockFile.open(QIODevice::WriteOnly | QIODevice::Text));
    lockFile.write(R"({
  "version": 1,
  "corePath": "inv.schematic.room",
  "holder": {
    "user": "anton",
    "host": "workstation-01",
    "pid": 4242,
    "tool": "Qucs-S",
    "toolVersion": "26.1.1"
  },
  "createdAt": "2026-09-02T14:30:12Z"
})");
    lockFile.close();

    const CoreFileLockInfo info = readCoreLockFile(corePath);
    QVERIFY(info.present);
    QVERIFY(info.parseOk);
    QCOMPARE(info.user, QStringLiteral("anton"));
    QCOMPARE(info.host, QStringLiteral("workstation-01"));
    QCOMPARE(info.tool, QStringLiteral("Qucs-S"));
    QCOMPARE(info.createdAt, QStringLiteral("2026-09-02T14:30:12Z"));
    QCOMPARE(info.pid, 4242);

    const QString formatted = formatCoreLockInfoBlock(info);
    QVERIFY(formatted.contains(QStringLiteral("Lock: active")));
    QVERIFY(formatted.contains(QStringLiteral("Locked by: anton @ workstation-01")));
    QVERIFY(formatted.contains(QStringLiteral("Tool: Qucs-S")));
}
