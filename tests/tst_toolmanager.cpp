#include <QtTest/QtTest>

#include <QTabWidget>
#include <QWidget>
#include <QStringList>
#include <QVariant>
#include <QDir>

#define private public
#include "mainwindow.h"
#include "toolmanager.h"
#include "tst_toolmanager.h"
#undef private

#include "property.h"
#include "view_tools.h"
#include "viewtoolstablewidget.h"

/*!*********************************************************************************************************************
 * \brief Verifies that addCustomTool() adds a new tab for a new tool.
 **********************************************************************************************************************/
void ToolManagerTest::addCustomTool_addsNewTab()
{
    Properties props;

    ToolManager dlg(nullptr, &props);
    dlg.show();

    QTabWidget *tabs = dlg.findChild<QTabWidget*>("tabTools");
    QVERIFY2(tabs, "tabTools widget not found");

    const int beforeCount = tabs->count();

    dlg.addCustomTool("MyTool");

    QCOMPARE(tabs->count(), beforeCount + 1);

    bool found = false;
    for(int i = 0; i < tabs->count(); ++i) {
        if(tabs->tabText(i) == "MyTool") {
            found = true;
            break;
        }
    }

    QVERIFY2(found, "New custom tool tab was not added");
}

/*!*********************************************************************************************************************
 * \brief Verifies that addCustomTool() does not add duplicate tabs for the same tool.
 **********************************************************************************************************************/
void ToolManagerTest::addCustomTool_duplicateDoesNotAddSecondTab()
{
    Properties props;

    ToolManager dlg(nullptr, &props);
    dlg.show();

    QTabWidget *tabs = dlg.findChild<QTabWidget*>("tabTools");
    QVERIFY2(tabs, "tabTools widget not found");

    dlg.addCustomTool("MyTool");
    const int countAfterFirstAdd = tabs->count();

    dlg.addCustomTool("MyTool");
    QCOMPARE(tabs->count(), countAfterFirstAdd);
}

/*!*********************************************************************************************************************
 * \brief Verifies that addCustomTool() reads stored tool path and view names from Properties.
 **********************************************************************************************************************/
void ToolManagerTest::addCustomTool_readsStoredValues()
{
    Properties props;
    props.set("MyTool", "/usr/bin/mytool");
    props.set("MyToolViews", "gds,oas,lstr");

    ToolManager dlg(nullptr, &props);
    dlg.show();

    dlg.addCustomTool("MyTool");

    QVERIFY2(dlg.m_custToolsTableMap.contains("MyTool"),
             "Tool table for MyTool was not created");
    QVERIFY2(dlg.m_custViewsEditMap.contains("MyTool"),
             "View suffix editor for MyTool was not created");

    QCOMPARE(dlg.m_custViewsEditMap.value("MyTool")->text(),
             QString("gds,oas,lstr"));

    const QVector<ViewToolEntry> entries =
        dlg.m_custToolsTableMap.value("MyTool")->entries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).path, QString("/usr/bin/mytool"));
    QVERIFY(entries.at(0).isDefault);
}

/*!*********************************************************************************************************************
 * \brief Verifies getTabNames() returns added custom tool tabs.
 **********************************************************************************************************************/
void ToolManagerTest::getTabNames_includesCustomTools()
{
    Properties props;
    ToolManager dlg(nullptr, &props);
    dlg.show();

    dlg.addCustomTool("MyTool");

    QStringList tabNames = dlg.getTabNames();

    QVERIFY(tabNames.contains("MyTool"));
}

/*!*********************************************************************************************************************
 * \brief Verifies on_btnOk_clicked() saves tool properties for custom tools.
 **********************************************************************************************************************/
void ToolManagerTest::on_btnOk_clicked_savesToolProperties()
{
    Properties props;
    MainWindow mainWindow("", QDir::currentPath());
    ToolManager dlg(&mainWindow, &props);
    dlg.show();

    dlg.addCustomTool("MyTool");

    QVERIFY2(dlg.m_custViewsEditMap.contains("MyTool"), "View editor not found");
    QVERIFY2(dlg.m_custToolsTableMap.contains("MyTool"), "Tool table not found");

    dlg.m_custViewsEditMap.value("MyTool")->setText("gds,oas");

    QVector<ViewToolEntry> entries;
    ViewToolEntry entry;
    entry.name = "MyTool";
    entry.path = "/usr/bin/mytool";
    entry.isDefault = true;
    entries.push_back(entry);
    dlg.m_custToolsTableMap.value("MyTool")->setEntries(entries);

    dlg.on_btnOk_clicked();

    QStringList savedTools = props.get<QString>("ToolList").split(",", Qt::SkipEmptyParts);
    QVERIFY(savedTools.contains("MyTool"));
    QCOMPARE(props.get<QString>("MyToolViews"), QString("gds,oas"));

    const QVector<ViewToolEntry> loaded = loadViewTools(&props, "MyTool");
    QCOMPARE(loaded.size(), 1);
    QCOMPARE(loaded.at(0).path, QString("/usr/bin/mytool"));
    QCOMPARE(loaded.at(0).name, QString("MyTool"));
    QVERIFY(loaded.at(0).isDefault);
}
