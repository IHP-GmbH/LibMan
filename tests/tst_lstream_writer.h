#ifndef TST_LSTREAM_WRITER_H
#define TST_LSTREAM_WRITER_H

#include <QtTest/QtTest>

class LStreamWriterTest : public QObject
{
    Q_OBJECT

private slots:
    void write_invalidPath_returnsError();
    void write_singleCell_createsFile();
    void write_multipleCells_createsFile();
};

#endif // TST_LSTREAM_WRITER_H
