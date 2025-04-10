#include <QEvent>
#include <QScreen>
#include <QDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QDialogButtonBox>

#include "extension/variantmanager.h"
#include "extension/variantfactory.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"

#include "toolmanager.h"
#include "ui_toolmanager.h"

#include "property.h"
#include "mainwindow.h"

/*!**********************************************************************************************************************
 * \brief Adds a new custom tool as a tab in the Tool Manager.
 *
 * This function dynamically creates a new tab for the given tool name. Each tab contains a property browser that allows
 * the user to configure settings related to the tool. It checks if the tool already exists before adding a new tab.
 *
 * \param toolName The name of the tool to be added.
 ***********************************************************************************************************************/
void ToolManager::addCustomTool(const QString &toolName)
{
    if (!m_properties)
        return;

    if (m_custPropertyMap.contains(toolName) || m_custVariantMngrMap.contains(toolName)) {
        return;
    }

    QWidget *newTab = new QWidget(this);
    QVBoxLayout *tabLayout = new QVBoxLayout(newTab);
    newTab->setLayout(tabLayout);

    QHBoxLayout *toolLayout = new QHBoxLayout();
    QWidget *toolWidget = new QWidget(this);
    toolWidget->setLayout(toolLayout);

    QtTreePropertyBrowser *pbCustTool = new QtTreePropertyBrowser(this);
    pbCustTool->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
    pbCustTool->setPropertiesWithoutValueMarked(true);
    pbCustTool->setHeaderVisible(false);

    toolLayout->addWidget(pbCustTool);
    tabLayout->addWidget(toolWidget);  // Add to the new tab's layout

    QtVariantPropertyManager *vmCustTool = new VariantManager(pbCustTool);
    QtVariantProperty *item, *subitem;

    item = vmCustTool->addProperty(QtVariantPropertyManager::groupTypeId(), tr("View"));
    pbCustTool->addProperty(item);

    subitem = vmCustTool->addProperty(VariantManager::filePathTypeId(), toolName);
    subitem->setWhatsThis("file");
    subitem->setToolTip("Please, provide schematic engine path...");

    if (m_properties->exists(toolName)) {
        QString schematic = m_properties->get<QString>(toolName);
        subitem->setValue(schematic);
    }

    item->addSubProperty(subitem);

    subitem = vmCustTool->addProperty(VariantManager::filePathTypeId(), "Name(s)");
    subitem->setToolTip("Please, provide view names...");

    if (m_properties->exists(toolName + "Views")) {
        QString tool = m_properties->get<QString>(toolName + "Views");
        subitem->setValue(tool);
    }

    item->addSubProperty(subitem);

    QtVariantEditorFactory *vf = new VariantFactory();
    pbCustTool->setFactoryForManager(vmCustTool, vf);

    m_custPropertyMap[toolName] = pbCustTool;
    m_custVariantMngrMap[toolName] = vmCustTool;

    connect(vmCustTool,
            SIGNAL(valueChanged(QtProperty*, QVariant)),
            this,
            SLOT(settingsChanged(QtProperty*, QVariant)));

    m_ui->tabTools->addTab(newTab, toolName);
}
