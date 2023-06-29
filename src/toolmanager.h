#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QDialog>
#include <QVariant>

class Properties;
class MainWindow;

class QtProperty;
class QtTreePropertyBrowser;
class QtVariantPropertyManager;

namespace Ui {
class ToolManager;
}

/*!*********************************************************************************************************************
 * \brief The ToolManager class creates a dialog window to allow user controlling 3-d party tools executed by LibMan.
 **********************************************************************************************************************/
class ToolManager : public QDialog
{
    Q_OBJECT
    
public:
    explicit ToolManager(QWidget *parent, Properties *properties);
    ~ToolManager();

private slots:
    void                                settingsChanged(QtProperty*, QVariant);

    void                                on_btnOk_clicked();
    void                                on_btnCancel_clicked();

private:
    void                                init();

private:
    Ui::ToolManager                     *m_ui; /*!< A pointer to acess ToolManager graphic items. */

    MainWindow                          *m_mw; /*!< A pointer to acess MainWindow items as it's a friend class. */
    Properties                          *m_properties; /*!< Container to store LibMan settings. */

    QtTreePropertyBrowser*              m_pbSettings; /*!< A property browser framework enabling the user to edit a set of properties. */
    QtVariantPropertyManager*           m_vmSettings; /*!< Each property type, the framework provides a property manager associated with editor factory. */
};


#endif // TOOLMANAGER_H
