#ifndef TST_TOOLMANAGER_H
#define TST_TOOLMANAGER_H

#include <QObject>

class ToolManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void addCustomTool_addsNewTab();
    void addCustomTool_duplicateDoesNotAddSecondTab();
    void addCustomTool_readsStoredValues();
    void getTabNames_includesCustomTools();
    void on_btnOk_clicked_savesToolProperties();
};

#endif // TST_TOOLMANAGER_H
