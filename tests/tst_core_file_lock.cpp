#include "tst_core_file_lock.h"

#include "core/core_file_lock.h"

#include <QFile>
#include <QTemporaryDir>

void CoreFileLockTest::lockPath_appendsLckSuffix()
{
    QCOMPARE(lockFilePathForCore(QStringLiteral("/tmp/cell.schematic.core")),
             QStringLiteral("/tmp/cell.schematic.core.lck"));
}

void CoreFileLockTest::readLockFile_missingReturnsNotPresent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString corePath = dir.filePath(QStringLiteral("inv.schematic.core"));
    const CoreFileLockInfo info = readCoreLockFile(corePath);

    QVERIFY(!info.present);
    QCOMPARE(info.lockPath, lockFilePathForCore(corePath));
}

void CoreFileLockTest::readLockFile_parsesHolderMetadata()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString corePath = dir.filePath(QStringLiteral("inv.schematic.core"));
    const QString lockPath = lockFilePathForCore(corePath);

    QFile lockFile(lockPath);
    QVERIFY(lockFile.open(QIODevice::WriteOnly | QIODevice::Text));
    lockFile.write(R"({
  "version": 1,
  "corePath": "inv.schematic.core",
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
