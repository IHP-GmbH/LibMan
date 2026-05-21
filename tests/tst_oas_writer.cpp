#include "tst_oas_writer.h"

#include <QFile>
#include <QTemporaryDir>

#include "oas/oasReader.h"

namespace
{

/*!********************************************************************************************************
 * \brief Reads whole file into QByteArray.
 *********************************************************************************************************/
static QByteArray readFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    return file.readAll();
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies that oasCreate() creates a minimal valid OAS file.
 *********************************************************************************************************/
void OasWriterTest::oasCreate_createsMinimalValidFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.path() + "/test.oas";

    oasReader reader(path);
    reader.oasCreate("TOP");

    const QStringList errors = reader.getErrors();
    QVERIFY2(errors.isEmpty(), qPrintable(errors.join("\n")));

    const QByteArray data = readFile(path);
    QVERIFY2(!data.isEmpty(), "Created OAS file is empty.");

    QVERIFY2(data.startsWith("%SEMI-OASIS\r\n"),
             "OAS file does not start with expected magic header.");

    QVERIFY2(data.contains("TOP"),
             "OAS file does not contain expected cell name.");

    QVERIFY2(data.contains(QByteArray(1, char(1))),
             "OAS file does not contain START record id.");

    QVERIFY2(data.contains(QByteArray(1, char(3))),
             "OAS file does not contain CELLNAME record id.");

    QVERIFY2(data.contains(QByteArray(1, char(13))),
             "OAS file does not contain CELL record id.");

    QVERIFY2(data.contains(QByteArray(1, char(2))),
             "OAS file does not contain END record id.");
}

/*!********************************************************************************************************
 * \brief Verifies that oasCreate() reports an error for an invalid path.
 *********************************************************************************************************/
void OasWriterTest::oasCreate_invalidPath_reportsError()
{
    const QString badPath = "/definitely_not_existing_dir_xyz__/bad.oas";

    oasReader reader(badPath);
    reader.oasCreate("TOP");

    QVERIFY(!reader.getErrors().isEmpty());
}

void OasWriterTest::oasCreate_secondTopCell_exercisesWriterEncoding()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.path() + "/multi.oas";

    oasReader reader(path);
    reader.oasCreate("TOP_A");
    reader.oasCreate("TOP_B");

    const QStringList errors = reader.getErrors();
    QVERIFY2(errors.isEmpty(), qPrintable(errors.join("\n")));

    QVERIFY(QFileInfo::exists(path));
    QVERIFY(QFile(path).size() > 32);
}
