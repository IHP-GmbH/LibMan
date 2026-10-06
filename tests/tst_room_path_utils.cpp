#include <QtTest>

#include "room/room_path_utils.h"
#include "tst_room_path_utils.h"

void CorePathUtilsTest::layoutCorePath_parsesCellAndView()
{
    const RoomViewIdentity identity =
        parseRoomViewIdentity(QStringLiteral("sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.layout.room"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("sg13g2_stdcell"));
    QCOMPARE(identity.viewName, QStringLiteral("layout"));
}

void CorePathUtilsTest::schematicCorePath_parsesCellAndView()
{
    const RoomViewIdentity identity =
        parseRoomViewIdentity(QStringLiteral("sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.schematic.room"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("sg13g2_stdcell"));
    QCOMPARE(identity.viewName, QStringLiteral("schematic"));
}

void CorePathUtilsTest::legacyCorePath_defaultsToLayout()
{
    const RoomViewIdentity identity = parseRoomViewIdentity(QStringLiteral("top.room"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("top"));
    QCOMPARE(identity.viewName, QStringLiteral("layout"));
}

void CorePathUtilsTest::emSetupPath_parsesCellAndView()
{
    QVERIFY(isEmSetupViewName(QStringLiteral("emsetup")));
    QCOMPARE(emSetupDirName(QStringLiteral("Test")), QStringLiteral("Test.emsetup"));
    QCOMPARE(emSetupDefaultVariantName(), QStringLiteral("nominal"));

    const RoomViewIdentity identity =
        parseEmSetupIdentity(QStringLiteral("lib/Test/Test.emsetup"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("Test"));
    QCOMPARE(identity.viewName, QStringLiteral("emsetup"));
}
