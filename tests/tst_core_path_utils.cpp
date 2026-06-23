#include <QtTest>

#include "core/core_path_utils.h"

class CorePathUtilsTest : public QObject
{
    Q_OBJECT

private slots:
    void layoutCorePath_parsesCellAndView();
    void schematicCorePath_parsesCellAndView();
    void legacyCorePath_defaultsToLayout();
};

void CorePathUtilsTest::layoutCorePath_parsesCellAndView()
{
    const CoreViewIdentity identity =
        parseCoreViewIdentity(QStringLiteral("sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.layout.core"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("sg13g2_stdcell"));
    QCOMPARE(identity.viewName, QStringLiteral("layout"));
}

void CorePathUtilsTest::schematicCorePath_parsesCellAndView()
{
    const CoreViewIdentity identity =
        parseCoreViewIdentity(QStringLiteral("sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.schematic.core"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("sg13g2_stdcell"));
    QCOMPARE(identity.viewName, QStringLiteral("schematic"));
}

void CorePathUtilsTest::legacyCorePath_defaultsToLayout()
{
    const CoreViewIdentity identity = parseCoreViewIdentity(QStringLiteral("top.core"));
    QVERIFY(identity.valid);
    QCOMPARE(identity.cellName, QStringLiteral("top"));
    QCOMPARE(identity.viewName, QStringLiteral("layout"));
}

#include "tst_core_path_utils.moc"
