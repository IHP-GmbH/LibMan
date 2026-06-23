#ifndef TST_CORE_PATH_UTILS_H
#define TST_CORE_PATH_UTILS_H

#include <QObject>

class CorePathUtilsTest : public QObject
{
    Q_OBJECT

private slots:
    void layoutCorePath_parsesCellAndView();
    void schematicCorePath_parsesCellAndView();
    void legacyCorePath_defaultsToLayout();
};

#endif
