#include <QEvent>
#include <QDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDialogButtonBox>

#if QT_VERSION >= 0x050000
#include <QScreen>
#endif

#include "extension/variantmanager.h"
#include "extension/variantfactory.h"
#include "QtPropertyBrowser/qteditorfactory.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"

#include "property.h"
#include "mainwindow.h"
#include "projectmanager.h"
#include "ui_projectmanager.h"

/*!*********************************************************************************************************************
 * \brief Creates ToolManager object to give user an opportunity to add, rename and remove projects from a dialog
 * window.
 * \param parent       Parent widget, by default is NULL.
 * \param properties   Container of LibMan settings.
 **********************************************************************************************************************/
ProjectManager::ProjectManager(QWidget *parent, Properties *properties) :
    QDialog(parent),
    m_ui(new Ui::ProjectManager),
    m_mw(0),
    m_isStateChanged(false),
    m_properties(properties)
{
    m_ui->setupUi(this);

    m_mw = static_cast<MainWindow*> (parent);

    init();

    m_ui->btnOk->setEnabled(false);

    setStateSaved();
    setWindowTitle("Project Manager");
}

/*!*********************************************************************************************************************
 * \brief Destructs ProjectManager obejct and clears up UI members.
 **********************************************************************************************************************/
ProjectManager::~ProjectManager()
{
    delete m_ui;
}

/*!*********************************************************************************************************************
 * \brief Initializes QtTreePropertyBrowser and creates its properties. It's used to manage a list of projects
 * that can be chosen by user for further work.
 **********************************************************************************************************************/
void ProjectManager::init()
{
    if( !m_properties )
        return;

    m_pbSettings = new QtTreePropertyBrowser(this);
    m_pbSettings->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
    m_pbSettings->setPropertiesWithoutValueMarked( true );
    m_pbSettings->setHeaderVisible(false);
    m_ui->projLayout->addWidget(m_pbSettings);

    m_vmSettings = new VariantManager(m_pbSettings);

    QtVariantProperty *item, *subitem;

    item = m_vmSettings->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Projects"));
    m_pbSettings->addProperty( item );

    subitem = m_vmSettings->addProperty(VariantManager::filePathTypeId(), "");
    subitem->setWhatsThis("folder");
    subitem->setToolTip("Please, provide project path...");
    item->addSubProperty(subitem);

    QtVariantEditorFactory *vf = new VariantFactory();
    m_pbSettings->setFactoryForManager(m_vmSettings, vf);

    connect(m_vmSettings,
            SIGNAL(valueChanged(QtProperty*, QVariant)),
            this,
            SLOT(settingsChanged(QtProperty*, QVariant)));
}

/*!*********************************************************************************************************************
 * \brief If state of the ProjectManager has been changed the dialog window pops up to ask user for saving of the
 * changes.
 * \param event       Variable is used to either accept to close the window or ignore to prevent closing it.
 **********************************************************************************************************************/
void ProjectManager::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setText("The project settings have been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    QString windowTitle = this->windowTitle();
    if(windowTitle.contains("*")) {

#if QT_VERSION >= 0x050000
        QScreen* pScreen = QGuiApplication::screenAt(this->mapToGlobal({this->width()/2,0}));
        QRect screenRect = pScreen->availableGeometry();
#else
        QRect screenRect = QDesktopWidget().screen()->rect();
#endif

        msgBox.move(QPoint(screenRect.width()/2, screenRect.height()/2));

        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                on_btnOk_clicked();
                event->accept();
                break;
            case QMessageBox::Discard:
                event->accept();
                break;
            case QMessageBox::Cancel:
                event->ignore();
                break;
            default:
                QDialog::closeEvent(event);
                break;
        }
    }
}

/*!*********************************************************************************************************************
 * \brief The slot is triggered once user changes projects (add, remove, rename).
 **********************************************************************************************************************/
void ProjectManager::settingsChanged(QtProperty *, QVariant)
{
    setStateChanged();

    QList<QtProperty *> props = m_pbSettings->properties();
    QList<QtProperty *>::iterator it;
    for( it = props.begin(); it != props.end(); ++it )
    {
        QtProperty *q = *it;

        if( q->propertyName() == "Projects" )
        {
            QList<QtProperty *> l = q->subProperties();
            QList<QtProperty *>::iterator t;
            for( t = l.begin(); t != l.end(); ++t )
            {
                QtProperty *p = *t;

                QString libPath = p->valueText();
                if(QFileInfo(libPath).exists()) {
                    QString libName = QFileInfo(libPath).completeBaseName();
                    p->setPropertyName(libName);
                }
            }
        }
    }
}

/*!*********************************************************************************************************************
 * \brief Set state of ProjectManager dialog window saved (unchanged).
 **********************************************************************************************************************/
void ProjectManager::setStateSaved()
{
    m_isStateChanged = false;

    QString title = windowTitle();
    title.remove("*");
    setWindowTitle(title);

    m_ui->btnOk->setEnabled(false);
}

/*!*********************************************************************************************************************
 * \brief Sets the state chaged if project(s) has(have) been modified.
 **********************************************************************************************************************/
void ProjectManager::setStateChanged()
{
    m_isStateChanged = true;
    m_ui->btnOk->setEnabled(true);

    QString title = windowTitle();
    if(!title.contains("*")) {
        setWindowTitle(title + "*");
    }
}

/*!*********************************************************************************************************************
 * \brief Saves current tool projects into properties of MainWindow object and closes the window.
 **********************************************************************************************************************/
void ProjectManager::on_btnOk_clicked()
{
    if(!m_mw) {
        return;
    }

    QList<QtProperty *> props = m_pbSettings->properties();
    QList<QtProperty *>::iterator it;
    for( it = props.begin(); it != props.end(); ++it )
    {
        QtProperty *q = *it;

        if( q->propertyName() == "Projects" )
        {
            QList<QtProperty *> l = q->subProperties();
            QList<QtProperty *>::iterator t;
            for( t = l.begin(); t != l.end(); ++t )
            {
                QtProperty *p = *t;

                QString libName = p->propertyName();
                QString libPath = p->valueText();

                QString alias = m_mw->getLibraryKeyPrefix() + libName;
                m_properties->set(alias, libPath);

                setStateChanged();
            }
        }
    }

    m_mw->loadLibraries();
    setStateSaved();

    close();
}

/*!*********************************************************************************************************************
 * \brief closes ProjectManager window.
 **********************************************************************************************************************/
void ProjectManager::on_btnCancel_clicked()
{
    close();
}
