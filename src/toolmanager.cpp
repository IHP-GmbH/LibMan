#include <QMap>
#include <QDebug>
#include <QEvent>
#include <QScreen>
#include <QDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QInputDialog>
#include <QGuiApplication>
#include <QDialogButtonBox>

#include "extension/variantmanager.h"
#include "extension/variantfactory.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"

#include "toolmanager.h"
#include "ui_toolmanager.h"

#include "property.h"
#include "mainwindow.h"

/*!*********************************************************************************************************************
 * \brief Creates ToolManager object to give user an opportunity to choose tools for working with different view types.
 * \param parent       Parent widget, by default is NULL.
 * \param properties   Container of LibMan settings.
 **********************************************************************************************************************/
ToolManager::ToolManager(QWidget *parent, Properties *properties) :
    QDialog(parent),
    m_ui(new Ui::ToolManager),
    m_properties(properties)
{
    m_ui->setupUi(this);

    m_mw = static_cast<MainWindow*> (parent);

    //m_ui->btnAdd->setVisible(false);
    //m_ui->btnDelete->setVisible(false);

    init();

    if(m_properties->exists("ToolList")) {
        QStringList tools = m_properties->get<QString>("ToolList").split(",");
        foreach(const QString &name, tools) {
            addCustomTool(name);
        }
    }
    else {
        addCustomTool("Schematic");
        addCustomTool("Layout");
    }

    m_ui->tabTools->setCurrentIndex(0);

    setWindowTitle("Tool Manager");
}

/*!*********************************************************************************************************************
 * \brief Destructs ToolManager obejct and clears up UI members.
 *********************************************************************************************************************/
ToolManager::~ToolManager()
{
    delete m_ui;
}

/*!*********************************************************************************************************************
 * \brief Initializes QtTreePropertyBrowser and creates its properties. It's used to provide a list of tools
 * that can be chosen by user to work with different view, documentation etc.
 **********************************************************************************************************************/
void ToolManager::init()
{
    if( !m_properties )
        return;

    m_pbSettings = new QtTreePropertyBrowser(this);
    m_pbSettings->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
    m_pbSettings->setPropertiesWithoutValueMarked( true );
    m_pbSettings->setHeaderVisible(false);
    m_ui->toolSettings->addWidget(m_pbSettings);

    m_vmSettings = new VariantManager(m_pbSettings);

    QtVariantProperty *item, *subitem;

    item = m_vmSettings->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Tools"));
    m_pbSettings->addProperty( item );

    subitem = m_vmSettings->addProperty(VariantManager::filePathTypeId(), "Schematic");
    subitem->setWhatsThis("file");
    subitem->setToolTip("Please, provide schematic engine path...");

    subitem = m_vmSettings->addProperty(VariantManager::filePathTypeId(), "Editor");
    subitem->setWhatsThis("file");
    subitem->setToolTip("Please, provide editor path...");

    if(m_properties->exists("Editor")) {
        QString schematic = m_properties->get<QString>("Editor");
        subitem->setValue(schematic);
    }
    else {
        subitem->setValue("nedit");
    }

    item->addSubProperty(subitem);

    subitem = m_vmSettings->addProperty(VariantManager::filePathTypeId(), "PDF Reader");
    subitem->setWhatsThis("file");
    subitem->setToolTip("Please, provide PDF reader path...");

    if(m_properties->exists("PdfReader")) {
        QString schematic = m_properties->get<QString>("PdfReader");
        subitem->setValue(schematic);
    }
    else {
        subitem->setValue("");
    }

    item->addSubProperty(subitem);

    QtVariantEditorFactory *vf = new VariantFactory();
    m_pbSettings->setFactoryForManager(m_vmSettings, vf);

    connect(m_vmSettings,
            SIGNAL(valueChanged(QtProperty*, QVariant)),
            this,
            SLOT(settingsChanged(QtProperty*, QVariant)));
}

/*!*********************************************************************************************************************
 * \brief The slot is triggered once user changes tool settings.
 **********************************************************************************************************************/
void ToolManager::settingsChanged(QtProperty *, QVariant)
{

}

/*!**********************************************************************************************************************
 * \brief Retrieves the names of all tool tabs except the first one.
 *
 * This function iterates through the tabs in m_ui->tabTools and collects their names into a QStringList.
 * It skips the first tab (index 0) to ensure the primary tab remains untouched.
 *
 * \return A QStringList containing the names of all custom tool tabs.
 ***********************************************************************************************************************/
QStringList ToolManager::getTabNames() const
{
    QStringList tabNames;

    int tabCount = m_ui->tabTools->count();
    for (int i = 1; i < tabCount; ++i) {
        tabNames.append(m_ui->tabTools->tabText(i));
    }

    return tabNames;
}

/*!**********************************************************************************************************************
 * \brief Saves current tool settings into properties of MainWindow object and closes the window.
 ***********************************************************************************************************************/
void ToolManager::on_btnOk_clicked()
{
    if(!m_mw) {
        return;
    }

    QList<QtProperty *> props = m_pbSettings->properties();
    QList<QtProperty *>::iterator it;
    for( it = props.begin(); it != props.end(); ++it )
    {
        QtProperty *q = *it;

        if( q->propertyName() == "Tools" )
        {
            QList<QtProperty *> l = q->subProperties();
            QList<QtProperty *>::iterator t;
            for( t = l.begin(); t != l.end(); ++t )
            {
                QtProperty *p = *t;

                QString toolName = p->propertyName();
                QString toolPath = p->valueText();

                if(toolName == "PDF Reader") {
                    toolName = "PdfReader";
                }

                m_properties->set(toolName, toolPath);
            }
        }
    }

    QStringList tabNames = getTabNames();
    if(tabNames.count()) {
        m_properties->set("ToolList", tabNames.join(","));
        foreach(const QString &name, tabNames) {
            QtTreePropertyBrowser *m_pbCustTool = m_custPropertyMap.value(name);
            if(m_pbCustTool) {
                QList<QtProperty *> custProps = m_pbCustTool->properties();
                QList<QtProperty *>::iterator it;
                for( it = custProps.begin(); it != custProps.end(); ++it )
                {
                    QtProperty *q = *it;

                    if( q->propertyName() == "View" )
                    {
                        QList<QtProperty *> l = q->subProperties();
                        QList<QtProperty *>::iterator t;
                        for( t = l.begin(); t != l.end(); ++t )
                        {
                            QtProperty *p = *t;

                            QString toolName = p->propertyName();
                            QString toolPath = p->valueText();

                            if(toolName == "Name(s)") {
                                toolName = name + "Views";
                            }

                            m_properties->set(toolName, toolPath);
                        }
                    }
                }
            }
        }
    }

    close();
}

/*!**********************************************************************************************************************
 * \brief closes ToolManager window.
 ***********************************************************************************************************************/
void ToolManager::on_btnCancel_clicked()
{
    close();
}

/*!**********************************************************************************************************************
 * \brief Prompts the user to enter a custom tool name and adds it as a new tab.
 *
 * This function opens a dialog box where the user can input a tool name. If the user confirms and provides a non-empty
 * name, it calls addCustomTool(toolName) to create the tool and its settings. After adding, it sets the newly created
 * tab as the active tab.
 ***********************************************************************************************************************/
void ToolManager::on_btnAdd_clicked()
{
    bool ok;
    QString toolName = QInputDialog::getText(this, tr("Add Custom Tool"),
                                             tr("Enter tool name:"), QLineEdit::Normal,
                                             "", &ok);

    if (ok && !toolName.isEmpty()) {
        addCustomTool(toolName);
        m_ui->tabTools->setCurrentIndex(m_ui->tabTools->count()-1);
    }
}

/*!**********************************************************************************************************************
 * \brief Deletes the currently selected tool tab, except the first one.
 *
 * This function prevents deletion of the first tab (index 0). It prompts the user for confirmation before deletion.
 * If confirmed, it removes the associated settings from m_properties, as well as entries in m_custPropertyMap and
 * m_custVariantMngrMap. The selected tab is then removed from the tab list.
 ***********************************************************************************************************************/
void ToolManager::on_btnDelete_clicked()
{
    int currentIndex = m_ui->tabTools->currentIndex();

    if (currentIndex == 0) {
        QMessageBox::warning(this, tr("Delete Tool"), tr("The first tab cannot be deleted."));
        return;
    }

    QString toolName = m_ui->tabTools->tabText(currentIndex);
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete Tool"),
                                  tr("Are you sure you want to delete '%1' tool?").arg(toolName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    if (m_properties->exists(toolName)) {
        m_properties->remove(toolName);
    }
    if (m_properties->exists(toolName + "Sufixes")) {
        m_properties->remove(toolName + "Sufixes");
    }

    if (m_custPropertyMap.contains(toolName)) {
        m_custPropertyMap.remove(toolName);
    }

    if (m_custVariantMngrMap.contains(toolName)) {
        m_custVariantMngrMap.remove(toolName);
    }

    m_ui->tabTools->removeTab(currentIndex);
}


