/************************************************************************
 *  LibMan – GUI Test Suite
 *
 *  This test suite verifies GUI behavior of LibMan including
 *  project loading, library and cell navigation,
 *  view handling (gds/oas/etc), and interaction logic.
 *
 *  Tests simulate user interaction via QtTest without
 *  starting external tools.
 ************************************************************************/

#ifndef TST_LIBMAN_GUI_GDS_H
#define TST_LIBMAN_GUI_GDS_H

#include <QObject>

class LibManGui : public QObject
{
    Q_OBJECT

private slots:
    void loadProject_clickLib_clickCell_hasGdsView();
};

#endif // TST_LIBMAN_GUI_GDS_H