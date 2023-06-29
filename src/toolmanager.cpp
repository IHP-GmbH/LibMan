#include <QEvent>
#include <QDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDialogButtonBox>

#include "extension/variantmanager.h"
#include "extension/variantfactory.h"
#include "QtPropertyBrowser/qteditorfactory.h"
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

    init();

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
    m_ui->toolLayout->addWidget(m_pbSettings);

    m_vmSettings = new VariantManager(m_pbSettings);

    QtVariantProperty *item, *subitem;

    item = m_vmSettings->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Tools"));
    m_pbSettings->addProperty( item );

    subitem = m_vmSettings->addProperty(VariantManager::filePathTypeId(), "Schematic");
    subitem->setWhatsThis("file");
    subitem->setToolTip("Please, provide schematic engine path...");

    if(m_properties->exists("Schematic")) {
        QString schematic = m_properties->get<QString>("Schematic");
        subitem->setValue(schematic);
    }
    else {
        subitem->setValue("nedit");
    }

    item->addSubProperty(subitem);

    subitem = m_vmSettings->addProperty(VariantManager::filePathTypeId(), "Layout");
    subitem->setWhatsThis("file");
    subitem->setToolTip("Please, provide layout engine path...");

    if(m_properties->exists("Layout")) {
        QString schematic = m_properties->get<QString>("Layout");
        subitem->setValue(schematic);
    }
    else {
        subitem->setValue("klayout");
    }

    item->addSubProperty(subitem);

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

    close();
}

/*!**********************************************************************************************************************
 * \brief closes ToolManager window.
 ***********************************************************************************************************************/
void ToolManager::on_btnCancel_clicked()
{
    close();
}

