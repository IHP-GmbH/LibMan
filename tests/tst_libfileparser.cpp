#include "tst_libfileparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStringList>
#include <QVariant>

#include "libfileparser.h"

namespace
{

/*!********************************************************************************************************
 * \brief Writes text file content.
 *********************************************************************************************************/
static bool writeTextFile(const QString &path, const QString &text)
{
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << text;
    file.close();
    return true;
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies parser reports missing file.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_missingFile_returnsError()
{
    LibFileParser parser;

    const bool ok = parser.parseFile("definitely_missing_file_123456.lib");

    QVERIFY(!ok);
    QVERIFY(!parser.errorString().isEmpty());
    QVERIFY(parser.data().definitions.isEmpty());
    QVERIFY(parser.data().includes.isEmpty());
}

/*!********************************************************************************************************
 * \brief Verifies define(name, path) form is parsed correctly.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_singleDefineWithExplicitName_parsesDefinition()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile  = dir.path() + "/root.lib";
    const QString viewFile = dir.path() + "/my_layout.gds";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"my_lib\", \"my_layout.gds\");\n"));

    LibFileParser parser;
    QVERIFY2(parser.parseFile(libFile), qPrintable(parser.errorString()));

    const LibFileData &data = parser.data();
    QCOMPARE(data.definitions.size(), 1);
    QCOMPARE(data.includes.size(), 0);

    const LibDefinition &def = data.definitions.first();
    QCOMPARE(def.name, QString("my_lib"));
    QCOMPARE(QDir::fromNativeSeparators(def.path),
             QDir::fromNativeSeparators(QFileInfo(viewFile).absoluteFilePath()));
    QCOMPARE(def.hasExplicitName, true);
}

/*!********************************************************************************************************
 * \brief Verifies define(path) infers library name from file path.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_singleDefineWithoutExplicitName_infersNameFromPath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile  = dir.path() + "/root.lib";
    const QString viewFile = dir.path() + "/abc_layout.oas";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"abc_layout.oas\");\n"));

    LibFileParser parser;
    QVERIFY2(parser.parseFile(libFile), qPrintable(parser.errorString()));

    const LibFileData &data = parser.data();
    QCOMPARE(data.definitions.size(), 1);

    const LibDefinition &def = data.definitions.first();
    QCOMPARE(def.name, QString("abc_layout"));
    QCOMPARE(QDir::fromNativeSeparators(def.path),
             QDir::fromNativeSeparators(QFileInfo(viewFile).absoluteFilePath()));
    QCOMPARE(def.hasExplicitName, false);
}

/*!********************************************************************************************************
 * \brief Verifies include() recursively parses referenced file.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_include_parsesIncludedFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString rootFile     = dir.path() + "/root.lib";
    const QString includedFile = dir.path() + "/child.lib";
    const QString viewFile     = dir.path() + "/child.gds";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(includedFile,
                          "define(\"child_lib\", \"child.gds\");\n"));
    QVERIFY(writeTextFile(rootFile,
                          "include(\"child.lib\");\n"));

    LibFileParser parser;
    QVERIFY2(parser.parseFile(rootFile), qPrintable(parser.errorString()));

    const LibFileData &data = parser.data();
    QCOMPARE(data.includes.size(), 1);
    QCOMPARE(data.definitions.size(), 1);

    const LibInclude &inc = data.includes.first();
    QCOMPARE(QDir::fromNativeSeparators(inc.path),
             QDir::fromNativeSeparators(QFileInfo(includedFile).absoluteFilePath()));

    const LibDefinition &def = data.definitions.first();
    QCOMPARE(def.name, QString("child_lib"));
}

/*!********************************************************************************************************
 * \brief Verifies recursive include detection.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_recursiveInclude_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString aFile = dir.path() + "/a.lib";
    const QString bFile = dir.path() + "/b.lib";

    QVERIFY(writeTextFile(aFile, "include(\"b.lib\");\n"));
    QVERIFY(writeTextFile(bFile, "include(\"a.lib\");\n"));

    LibFileParser parser;
    const bool ok = parser.parseFile(aFile);

    QVERIFY(!ok);
    QVERIFY(parser.errorString().contains("Recursive include"));
}

/*!********************************************************************************************************
 * \brief Verifies invalid statement syntax is rejected.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_invalidStatement_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    QVERIFY(writeTextFile(libFile,
                          "define \"broken\";\n"));

    LibFileParser parser;
    const bool ok = parser.parseFile(libFile);

    QVERIFY(!ok);
    QVERIFY(parser.errorString().contains("Invalid statement syntax"));
}

/*!********************************************************************************************************
 * \brief Verifies unknown function is rejected.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_unknownFunction_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    QVERIFY(writeTextFile(libFile,
                          "foobar(\"x\");\n"));

    LibFileParser parser;
    const bool ok = parser.parseFile(libFile);

    QVERIFY(!ok);
    QVERIFY(parser.errorString().contains("Unknown function"));
}

/*!********************************************************************************************************
 * \brief Verifies define options are parsed correctly.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_defineWithOptions_parsesOptions()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile  = dir.path() + "/opts.lib";
    const QString viewFile = dir.path() + "/chip.gds";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"chip_lib\", \"chip.gds\", "
                          "technology=\"SG13G2\", "
                          "technologies=[\"A\",\"B\"], "
                          "replicate=true, "
                          "owner=\"Anton\");\n"));

    LibFileParser parser;
    QVERIFY2(parser.parseFile(libFile), qPrintable(parser.errorString()));

    const LibFileData &data = parser.data();
    QCOMPARE(data.definitions.size(), 1);

    const LibDefinition &def = data.definitions.first();

    QCOMPARE(def.name, QString("chip_lib"));
    QCOMPARE(def.technology, QString("SG13G2"));
    QCOMPARE(def.technologies, QStringList() << "A" << "B");
    QCOMPARE(def.hasReplicate, true);
    QCOMPARE(def.replicate, true);

    QVERIFY(def.options.contains("technology"));
    QVERIFY(def.options.contains("technologies"));
    QVERIFY(def.options.contains("replicate"));
    QVERIFY(def.options.contains("owner"));

    QCOMPARE(def.options.value("owner").toString(), QString("Anton"));
}

/*!********************************************************************************************************
 * \brief Verifies duplicate named option is rejected.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_duplicateNamedOption_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile  = dir.path() + "/dup.lib";
    const QString viewFile = dir.path() + "/x.gds";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"x\", \"x.gds\", technology=\"A\", technology=\"B\");\n"));

    LibFileParser parser;
    const bool ok = parser.parseFile(libFile);

    QVERIFY(!ok);
    QVERIFY(parser.errorString().contains("Duplicate named option"));
}

/*!********************************************************************************************************
 * \brief Verifies invalid technologies list is rejected.
 *********************************************************************************************************/
void LibFileParserTest::parseFile_invalidStringList_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile  = dir.path() + "/badlist.lib";
    const QString viewFile = dir.path() + "/x.gds";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"x\", \"x.gds\", technologies=[\"A\", true]);\n"));

    LibFileParser parser;
    const bool ok = parser.parseFile(libFile);

    QVERIFY(!ok);
    QVERIFY(parser.errorString().contains("technologies"));
}

void LibFileParserTest::parseFile_pathIsDirectory_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LibFileParser parser;
    QVERIFY(!parser.parseFile(dir.path()));
    QVERIFY(parser.errorString().contains("not a file") || parser.errorString().contains("Path is not a file"));
}

void LibFileParserTest::parseFile_secondParse_skipsAlreadyParsedFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString rootFile = dir.path() + "/root.lib";
    const QString childFile = dir.path() + "/child.lib";
    const QString viewFile  = dir.path() + "/cell.gds";

    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(childFile, "define(\"lib\", \"cell.gds\");\n"));
    QVERIFY(writeTextFile(rootFile,
                          "include(\"child.lib\");\n"
                          "include(\"child.lib\");\n"));

    LibFileParser parser;
    QVERIFY2(parser.parseFile(rootFile), qPrintable(parser.errorString()));
    QCOMPARE(parser.data().definitions.size(), 1);
}

void LibFileParserTest::parseFile_defineEmptyArgs_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    QVERIFY(writeTextFile(libFile, "define();\n"));

    LibFileParser parser;
    QVERIFY(!parser.parseFile(libFile));
    QVERIFY(parser.errorString().contains("define() expects"));
}

void LibFileParserTest::parseFile_defineTooManyPositionals_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    const QString g1 = dir.path() + "/a.gds";
    const QString g2 = dir.path() + "/b.gds";
    QVERIFY(writeTextFile(g1, QString()));
    QVERIFY(writeTextFile(g2, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"n1\", \"a.gds\", \"b.gds\");\n"));

    LibFileParser parser;
    QVERIFY(!parser.parseFile(libFile));
    QVERIFY(parser.errorString().contains("positional"));
}

void LibFileParserTest::parseFile_trailingTokensAfterStatement_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    const QString viewFile = dir.path() + "/x.gds";
    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"x\", \"x.gds\") garbage;\n"));

    LibFileParser parser;
    QVERIFY(!parser.parseFile(libFile));
    QVERIFY(parser.errorString().contains("Unexpected trailing"));
}

void LibFileParserTest::parseFile_technologyNonString_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    const QString viewFile = dir.path() + "/x.gds";
    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"x\", \"x.gds\", technology=123);\n"));

    LibFileParser parser;
    QVERIFY(!parser.parseFile(libFile));
    QVERIFY(parser.errorString().contains("technology"));
}

void LibFileParserTest::parseFile_replicateFalse_parses()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/ok.lib";
    const QString viewFile = dir.path() + "/x.gds";
    QVERIFY(writeTextFile(viewFile, QString()));
    QVERIFY(writeTextFile(libFile,
                          "define(\"x\", \"x.gds\", replicate=false);\n"));

    LibFileParser parser;
    QVERIFY2(parser.parseFile(libFile), qPrintable(parser.errorString()));
    QCOMPARE(parser.data().definitions.size(), 1);
    QVERIFY(parser.data().definitions.first().hasReplicate);
    QVERIFY(!parser.data().definitions.first().replicate);
}

void LibFileParserTest::parseFile_includeWrongArgCount_returnsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString libFile = dir.path() + "/bad.lib";
    QVERIFY(writeTextFile(libFile, "include(\"a.lib\", \"b.lib\");\n"));

    LibFileParser parser;
    QVERIFY(!parser.parseFile(libFile));
    QVERIFY(parser.errorString().contains("include() expects"));
}
