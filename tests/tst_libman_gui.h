#ifndef TST_LIBMAN_GUI_H
#define TST_LIBMAN_GUI_H

#include <QObject>

class LibManGui : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void loadProject_hasLibrariesGroupsAndViews();
    void expandGdsView_populatesHierarchy();
    void expandOasView_populatesHierarchy();
    void expandLstrView_populatesHierarchy();

    void cleanupTestCase();
};

#endif // TST_LIBMAN_GUI_H
