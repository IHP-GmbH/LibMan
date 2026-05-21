#include "tst_lstream_writer.h"

#include <QFile>
#include <QTemporaryDir>

#include "lstream/lstreamcellwriter.h"

namespace
{

static QByteArray readFile(const QString &path)
{
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly)) {
        return {};
    }

    return f.readAll();
}

} // namespace

void LStreamWriterTest::write_invalidPath_returnsError()
{
    const QString badPath = "/definitely_not_existing_dir_xyz__/bad.lstr";

    const LStreamCellWriter::Result result =
        LStreamCellWriter::write(badPath,
                                 "lib",
                                 QStringList() << "TOP",
                                 "unknown",
                                 "LibMan");

    QVERIFY(!result.written);
    QVERIFY(!result.errors.isEmpty());
}

void LStreamWriterTest::write_singleCell_createsFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.path() + "/single.lstr";

    const LStreamCellWriter::Result result =
        LStreamCellWriter::write(path,
                                 "my_lib",
                                 QStringList() << "TOP",
                                 "unknown",
                                 "LibMan");

    QVERIFY2(result.written, qPrintable(result.errors.join("\n")));
    QVERIFY(result.errors.isEmpty());

    QFileInfo fi(path);
    QVERIFY(fi.exists());
    QVERIFY(fi.size() > 12);

    const QByteArray data = readFile(path);
    QVERIFY(data.startsWith(QByteArray("LStream_1.0\0", 12)));
}

void LStreamWriterTest::write_multipleCells_createsFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.path() + "/multi.lstr";

    const QStringList cells{"TOP", "INV", "NAND2"};

    const LStreamCellWriter::Result result =
        LStreamCellWriter::write(path,
                                 "my_lib",
                                 cells,
                                 "sg13g2",
                                 "LibManTest");

    QVERIFY2(result.written, qPrintable(result.errors.join("\n")));
    QVERIFY(result.errors.isEmpty());

    QFileInfo fi(path);
    QVERIFY(fi.exists());
    QVERIFY(fi.size() > 12);

    const QByteArray data = readFile(path);
    QVERIFY(data.startsWith(QByteArray("LStream_1.0\0", 12)));

    // Smoke-check: file should be noticeably larger for multiple cells
    QVERIFY(data.size() > 50);
}
