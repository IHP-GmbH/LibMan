#ifndef TST_LIBMAN_LAYOUTVIEW_CREATE_H
#define TST_LIBMAN_LAYOUTVIEW_CREATE_H

#include <QtTest/QtTest>

class LibManLayoutViewCreateTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void addNewGdsView_createsFileAndRegistersView();
    void addNewOasView_createsFileAndRegistersView();
    void addNewLStreamView_createsFileAndRegistersView();
};

#endif // TST_LIBMAN_LAYOUTVIEW_CREATE_H
