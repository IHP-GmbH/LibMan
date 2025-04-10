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
 * \brief The ToolManager class creates a dialog window to allow users to control third-party tools executed by LibMan.
 *
 * This class provides a graphical interface for managing external tools used within LibMan. It allows users to add,
 * configure, and remove tools through dynamically created tabs and property browsers.
 **********************************************************************************************************************/
class ToolManager : public QDialog
{
    Q_OBJECT

public:
    /*!*****************************************************************************************************************
     * \brief Constructs the ToolManager dialog.
     * \param parent       The parent widget (typically the main window).
     * \param properties   A pointer to the properties container storing LibMan settings.
     ******************************************************************************************************************/
    explicit ToolManager(QWidget *parent, Properties *properties);

    /*!*****************************************************************************************************************
     * \brief Destroys the ToolManager object and releases allocated resources.
     ******************************************************************************************************************/
    ~ToolManager();

private slots:
    /*!*****************************************************************************************************************
     * \brief Handles changes in tool settings.
     * \param property     The property that was changed.
     * \param value        The new value of the property.
     ******************************************************************************************************************/
    void                                        settingsChanged(QtProperty*, QVariant);

    /*!*****************************************************************************************************************
     * \brief Saves the tool settings and closes the dialog.
     ******************************************************************************************************************/
    void                                        on_btnOk_clicked();

    /*!*****************************************************************************************************************
     * \brief Closes the ToolManager dialog without saving changes.
     ******************************************************************************************************************/
    void                                        on_btnCancel_clicked();

    /*!*****************************************************************************************************************
     * \brief Opens a dialog to add a new custom tool and creates a new tab for it.
     ******************************************************************************************************************/
    void                                        on_btnAdd_clicked();

    /*!*****************************************************************************************************************
     * \brief Deletes the currently selected tool tab, except for the first one.
     *
     * This function prompts the user for confirmation before deleting the tab. It also removes the tool's settings
     * from the property container and deletes its associated UI elements.
     ******************************************************************************************************************/
    void                                        on_btnDelete_clicked();

private:
    /*!*****************************************************************************************************************
     * \brief Initializes the property browser and loads stored tool settings.
     ******************************************************************************************************************/
    void                                        init();

    /*!*****************************************************************************************************************
     * \brief Retrieves a list of tool names from the existing tabs.
     * \return A QStringList containing the names of all tool tabs (excluding the first tab).
     ******************************************************************************************************************/
    QStringList                                 getTabNames() const;

    /*!*****************************************************************************************************************
     * \brief Adds a new custom tool by creating a tab and populating it with a property browser.
     * \param toolName The name of the tool to be added.
     ******************************************************************************************************************/
    void                                        addCustomTool(const QString &toolName);

private:
    Ui::ToolManager                             *m_ui; /*!< Pointer to access ToolManager UI components. */

    MainWindow                                  *m_mw; /*!< Pointer to access MainWindow, allowing interaction between ToolManager and the main interface. */
    Properties                                  *m_properties; /*!< Container to store LibMan settings and configurations. */

    QtTreePropertyBrowser*                      m_pbSettings; /*!< Property browser for editing tool settings. */
    QtVariantPropertyManager*                   m_vmSettings; /*!< Property manager for handling different property types and associated UI editors. */

    QMap<QString, QtTreePropertyBrowser*>       m_custPropertyMap; /*!< Maps tool names to their respective property browsers. */
    QMap<QString, QtVariantPropertyManager*>    m_custVariantMngrMap; /*!< Maps tool names to their variant property managers. */
};



#endif // TOOLMANAGER_H
