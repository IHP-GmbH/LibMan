#ifndef TST_LIBDEFINE_UTILS_H
#define TST_LIBDEFINE_UTILS_H

#include <QtTest/QtTest>

class LibDefineUtilsTest : public QObject
{
    Q_OBJECT

private slots:
    void wildcardDefine_expandsLibraryPattern();
    void wildcardDefine_expandsCellPattern();
    void wildcardDefine_libraryRootUsesTopLevelDirectory();
    void wildcardDefine_libraryRootUsesParentForCellPattern();
    void wildcardDefine_coversExpandedFiles();
};

#endif // TST_LIBDEFINE_UTILS_H
