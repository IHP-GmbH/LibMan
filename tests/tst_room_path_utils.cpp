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
