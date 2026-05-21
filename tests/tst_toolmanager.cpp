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
#include "extension/variantmanager.h"
#include "extension/variantfactory.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"

template <typename Container>
static QtProperty *findPropertyByName(const Container &props, const QString &name)
{
    for(QtProperty *prop : props) {
        if(prop && prop->propertyName() == name) {
            return prop;
        }
    }
    return nullptr;
}

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

    QVERIFY2(dlg.m_custVariantMngrMap.contains("MyTool"),
             "Variant manager for MyTool was not created");

    QtVariantPropertyManager *vm = dlg.m_custVariantMngrMap.value("MyTool");
    QVERIFY2(vm, "Variant manager is null");

    const QSet<QtProperty*> propsSet = vm->properties();
    QVERIFY2(!propsSet.isEmpty(), "No properties were created in the custom tool manager");

    bool foundToolPath = false;
    bool foundViews = false;

    foreach(QtProperty *groupProp, propsSet) {
        if(!groupProp) {
            continue;
        }

        const QList<QtProperty*> subProps = groupProp->subProperties();
        foreach(QtProperty *subProp, subProps) {
            if(!subProp) {
                continue;
            }

            const QString name = subProp->propertyName();

            if(name == "MyTool") {
                const QVariant v = vm->value(subProp);
                QCOMPARE(v.toString(), QString("/usr/bin/mytool"));
                foundToolPath = true;
            }

            if(name == "Name(s)") {
                const QVariant v = vm->value(subProp);
                QCOMPARE(v.toString(), QString("gds,oas,lstr"));
                foundViews = true;
            }
        }
    }

    QVERIFY2(foundToolPath, "Stored tool path property was not found");
    QVERIFY2(foundViews, "Stored tool views property was not found");
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

    QtProperty *toolsGroup = findPropertyByName(dlg.m_pbSettings->properties(), "Tools");
    QVERIFY2(toolsGroup, "Tools group not found");

    QtProperty *myToolProperty = nullptr;
    QtProperty *myToolViewsProperty = nullptr;

    QtVariantPropertyManager *customManager = dlg.m_custVariantMngrMap.value("MyTool");
    QVERIFY2(customManager, "Custom tool manager not found");

    QtProperty *viewGroup = findPropertyByName(customManager->properties(), "View");
    QVERIFY2(viewGroup, "View group not found");

    myToolProperty = findPropertyByName(viewGroup->subProperties(), "MyTool");
    myToolViewsProperty = findPropertyByName(viewGroup->subProperties(), "Name(s)");
    QVERIFY2(myToolProperty, "Custom tool path property not found");
    QVERIFY2(myToolViewsProperty, "Custom tool names property not found");

    customManager->setValue(myToolProperty, QVariant("/usr/bin/mytool"));
    customManager->setValue(myToolViewsProperty, QVariant("gds,oas"));

    dlg.on_btnOk_clicked();

    QStringList savedTools = props.get<QString>("ToolList").split(",", Qt::SkipEmptyParts);
    QVERIFY(savedTools.contains("MyTool"));
    QCOMPARE(props.get<QString>("MyTool"), QString("/usr/bin/mytool"));
    QCOMPARE(props.get<QString>("MyToolViews"), QString("gds,oas"));
}
