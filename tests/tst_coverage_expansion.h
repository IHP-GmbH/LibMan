#ifndef TST_COVERAGE_EXPANSION_H
#define TST_COVERAGE_EXPANSION_H

#include <QObject>

class FormatReadersCoverageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void gdsReader_readHierarchy_fixtureFile();
    void oasReader_readHierarchy_fixtureFile();
    void lstreamReader_read_fixtureFile();
};

class MainWindowCoverageExpansionTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void hooks_removeDirAndCopyDir();
    void hooks_expandShellVariables_detectView_generateCopyName();
    void hooks_getViewPath_andRepresentativeFile();

    void mainWindow_searches_toggles_gitStatus_and_infoSlots();
    void newView_dialogInvokesMainWindowCreatePaths();
    void mainWindow_expandGdsAndViewSearchFilter();
    void mainWindow_save_supportedViews_session_category();
};

class MainWindowSlotSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void invokeSafeSlots_withPartialSelection();
};

#endif // TST_COVERAGE_EXPANSION_H
