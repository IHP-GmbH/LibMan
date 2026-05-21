#ifndef TST_LIBMAN_VIEWOPS_H
#define TST_LIBMAN_VIEWOPS_H

#include <QtTest/QtTest>

class LibManViewOpsTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void addNewSpiceView_createsFileAndRegistersView();
    void addNewSchematicView_createsFileAndRegistersView();

    void copySelectedView_smoke();
};

#endif // TST_LIBMAN_VIEWOPS_H
