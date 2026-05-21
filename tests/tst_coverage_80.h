#ifndef TST_COVERAGE_80_H
#define TST_COVERAGE_80_H

#include <QObject>

class Coverage80MainWindowDeepTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void expandOasAndLstrRoots_populateHierarchy();
    void hooks_documents_categories_filtersAndCombinedLibs();
    void dialogs_about_toolsAndProjectManager_autoClose();
    void documentationTree_doubleClick_smoke();
    void nullItemSlots_noCrash();
};

class Coverage80LStreamReaderTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void read_missingFile_reportsError();
    void read_emptyFile_reportsError();
    void read_badPrefix_reportsError();
    void read_unterminatedHeader_reportsError();
    void read_truncatedPayload_reportsCapnpError();
};

class Coverage80GdsExtraTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void readHierarchy_secondStdcellGds_fixture();
};

#endif // TST_COVERAGE_80_H
