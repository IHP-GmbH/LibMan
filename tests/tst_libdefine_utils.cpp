#include "tst_libdefine_utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTextStream>

#include "libdefine_utils.h"

namespace
{

bool writeTextFile(const QString &path, const QString &text)
{
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << text;
    return true;
}

} // namespace

void LibDefineUtilsTest::wildcardDefine_expandsLibraryPattern()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString projectDir = dir.path();
    QVERIFY(writeTextFile(projectDir + "/analogLib/R/R.symbol.core", "core"));
    QVERIFY(writeTextFile(projectDir + "/analogLib/C/C.symbol.core", "core"));
    QVERIFY(writeTextFile(projectDir + "/analogLib/notes.txt", "skip"));

    const QStringList expanded =
        libdefine::expandWildcardDefinePath(projectDir, QStringLiteral("analogLib/*"));
    QCOMPARE(expanded.size(), 2);
    QVERIFY(expanded.at(0).endsWith(QStringLiteral("R.symbol.core")));
    QVERIFY(expanded.at(1).endsWith(QStringLiteral("C.symbol.core")));
}

void LibDefineUtilsTest::wildcardDefine_expandsCellPattern()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString projectDir = dir.path();
    QVERIFY(writeTextFile(projectDir + "/analogLib/MyDev/MyDev.symbol.core", "core"));
    QVERIFY(writeTextFile(projectDir + "/analogLib/MyDev/MyDev.schematic.core", "core"));

    const QStringList expanded =
        libdefine::expandWildcardDefinePath(projectDir, QStringLiteral("analogLib/MyDev/*"));
    QCOMPARE(expanded.size(), 2);
}

void LibDefineUtilsTest::wildcardDefine_libraryRootUsesTopLevelDirectory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString projectDir = dir.path();
    QDir().mkpath(projectDir + "/analogLib/MyDev");

    const QString root =
        libdefine::wildcardLibraryRoot(projectDir, QStringLiteral("analogLib/*"));
    QCOMPARE(QDir::fromNativeSeparators(root),
             QDir::fromNativeSeparators(projectDir + "/analogLib"));
}

void LibDefineUtilsTest::wildcardDefine_libraryRootUsesParentForCellPattern()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString projectDir = dir.path();
    QDir().mkpath(projectDir + "/analogLib/MyDev");

    const QString root =
        libdefine::wildcardLibraryRoot(projectDir, QStringLiteral("analogLib/MyDev/*"));
    QCOMPARE(QDir::fromNativeSeparators(root),
             QDir::fromNativeSeparators(projectDir + "/analogLib"));
}

void LibDefineUtilsTest::wildcardDefine_coversExpandedFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString projectDir = dir.path();
    const QString corePath = projectDir + "/analogLib/R/R.symbol.core";
    QVERIFY(writeTextFile(corePath, "core"));

    QVERIFY(libdefine::isPathCoveredByWildcardDefine(projectDir,
                                                     QStringLiteral("analogLib/*"),
                                                     corePath));
    QVERIFY(!libdefine::isPathCoveredByWildcardDefine(projectDir,
                                                      QStringLiteral("analogLib/R/*"),
                                                      corePath));
}

QTEST_APPLESS_MAIN(LibDefineUtilsTest)
