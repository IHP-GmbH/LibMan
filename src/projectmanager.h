#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QDialog>
#include <QVariant>

class Properties;
class MainWindow;

class QtProperty;
class QtTreePropertyBrowser;
class QtVariantPropertyManager;

namespace Ui {
    class ProjectManager;
}

/*!*********************************************************************************************************************
 * \brief The ProjectManager class creates a dialog window to allow user controlling LibMan projects.
 **********************************************************************************************************************/
class ProjectManager : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProjectManager(QWidget *parent, Properties *properties);
    ~ProjectManager();

private slots:
    void                                closeEvent(QCloseEvent *);
    void                                settingsChanged(QtProperty*, QVariant);    

    void                                on_btnOk_clicked();
    void                                on_btnCancel_clicked();

private:
    void                                init();

    bool                                isStateChanged() const;
    void                                setStateSaved();
    void                                setStateChanged();
    
private:
    Ui::ProjectManager                  *m_ui; /*!< A pointer to acess ProjectManager graphic items. */

    MainWindow                          *m_mw; /*!< A pointer to acess MainWindow items as it's a friend class. */

    bool                                m_isStateChanged; /*!< Keeps the state of the ProjectManager dialog window session. */

    Properties                          *m_properties; /*!< Container to store LibMan settings. */

    QtTreePropertyBrowser*              m_pbSettings; /*!< A property browser framework enabling the user to edit a set of properties. */
    QtVariantPropertyManager*           m_vmSettings; /*!< Each property type, the framework provides a property manager associated with editor factory. */
};

//*********************************************************************************************************************
// ProjectManager::isStateChanged
//*********************************************************************************************************************
inline bool ProjectManager::isStateChanged() const
{
    return m_isStateChanged;
}

#endif // PROJECTMANAGER_H
