#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "toolmanager.h"
#include "ui_toolmanager.h"
#include "view_tools.h"
#include "viewtoolstablewidget.h"

#include "property.h"

/*!**********************************************************************************************************************
 * \brief Adds a new custom tool group tab with a view-suffix list and a multi-tool table.
 ***********************************************************************************************************************/
void ToolManager::addCustomTool(const QString &toolName)
{
    if (!m_properties) {
        return;
    }

    if (m_custToolsTableMap.contains(toolName)) {
        return;
    }

    QWidget *newTab = new QWidget(this);
    auto *tabLayout = new QVBoxLayout(newTab);

    auto *viewsLayout = new QHBoxLayout();
    viewsLayout->addWidget(new QLabel(tr("View name(s):"), newTab));
    auto *viewsEdit = new QLineEdit(newTab);
    viewsEdit->setToolTip(tr("Comma-separated view suffixes handled by this tab, e.g. schematic,symbol,cdl"));
    const QString viewsKey = toolName + QStringLiteral("Views");
    if (m_properties->exists(viewsKey)) {
        viewsEdit->setText(m_properties->get<QString>(viewsKey));
    }
    viewsLayout->addWidget(viewsEdit);
    tabLayout->addLayout(viewsLayout);

    auto *table = new ViewToolsTableWidget(newTab);
    table->setEntries(loadViewTools(m_properties, toolName));
    tabLayout->addWidget(table);

    m_custViewsEditMap[toolName] = viewsEdit;
    m_custToolsTableMap[toolName] = table;

    m_ui->tabTools->addTab(newTab, toolName);
}
