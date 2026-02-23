#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gds/gdsreader.h"
#include "oas/oasReader.h"

#include <QProcess>
#include <QMainWindow>
#include <QHash>
#include <QTimer>
#include <memory>

class Properties;
class QTreeWidget;
class QListWidget;
class QListWidgetItem;
class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}

/*!*********************************************************************************************************************
 * \brief The MainWindow class is responsible for the main LibMan UI and controls all slots and signals.
 **********************************************************************************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class NewView;
    friend class ProjectManager;

    /*!
     * \brief The RECENT_FILES enum specifies maximum number of recent project files.
     */
    enum RECENT_FILES {
        PROJ_MAX_COUNT           = 5
    };

    /*!
     * \brief The COPY_STATE enum specifies different states used for copying project/group/view (library/cell/view).
     */
    enum COPY_STATE {
        NONE                     = 1000,
        PROJECT,
        GROUP,
        VIEW
    };

    /*!
     * \brief Custom roles used in the views tree widget items.
     */
    enum VIEW_ITEM_ROLE {
        RoleType                 = Qt::UserRole + 1,    /*!< Item type: view root / cell node / etc. */
        RoleGdsPath              = Qt::UserRole + 2,    /*!< Absolute path to GDS file for "gds" view node. */
        RoleCellName             = Qt::UserRole + 3,    /*!< Cell name for hierarchy nodes. */
        RoleOasPath              = Qt::UserRole + 4     /*!< Absolute path to OASIS file for "oas" view node. */
    };

    /*!
     * \brief Item types stored in RoleType.
     */
    enum VIEW_ITEM_TYPE {
        ItemViewGds              = 1,                   /*!< "gds" view root node. */
        ItemCell                 = 2,                   /*!< Cell node in hierarchy (GDS/OASIS). */
        ItemViewOas              = 3                    /*!< "oas" view root node. */
    };

public:
    explicit MainWindow(const QString &projFile, const QString &runDir, QWidget *parent = 0);
    ~MainWindow();

    /*!
     * \brief Cache entry storing parsed GDS hierarchy for a given file.
     */
    struct GdsCacheEntry
    {
        bool                                loaded   = false;    /*!< True if hierarchy is successfully loaded. */
        bool                                loading  = false;    /*!< True while async loading is in progress. */
        QString                             path;                /*!< Absolute path to the GDS file. */
        GdsReader::GdsHierarchy             hierarchy;            /*!< Parsed hierarchy (cells + references). */
        QStringList                         errors;              /*!< Errors collected during parsing. */
    };

    /*!
     * \brief Cache entry storing parsed OASIS hierarchy for a given file.
     */
    struct OasCacheEntry
    {
        bool                                loaded   = false;    /*!< True if hierarchy is successfully loaded. */
        bool                                loading  = false;    /*!< True while async loading is in progress. */
        QString                             path;                /*!< Absolute path to the OASIS file. */
        LayoutHierarchy                     hierarchy;            /*!< Parsed hierarchy (cells + placements). */
        QStringList                         errors;              /*!< Errors collected during parsing. */
    };

    /*!
     * \brief Per-item spinner animation state.
     */
    struct SpinnerState
    {
        QTimer                             *timer    = nullptr;  /*!< Timer driving icon rotation. Owned by MainWindow. */
        int                                 angleDeg = 0;        /*!< Current rotation angle (0..359). */
    };

private slots:
    void                                closeEvent(QCloseEvent *event) override;
    bool                                eventFilter(QObject *obj, QEvent *event) override;

    void                                showViewMenu(const QPoint &pos);
    void                                showGroupMenu(const QPoint &pos);
    void                                showLibraryMenu(const QPoint &pos);
    void                                showCategoryMenu(const QPoint &pos);

    void                                on_viewItemExpanded(QTreeWidgetItem *item);

    void                                addNewGroup();
    void                                addNewProject();
    void                                addNewCategory();
    void                                addNewSpiceView();
    void                                addNewLayoutView();
    void                                addNewSchematicView();

    void                                removeSelectedView();
    void                                removeSelectedGroup();
    void                                removeSelectedProject();
    void                                removeSelectedCategory();

    void                                showViewInfo();
    void                                showGroupInfo();
    void                                showProjectInfo();
    void                                showCategoryInfo();

    void                                removeFromGroup();
    void                                removeGroupUnion();
    void                                showFolderInfo(const QString &, const QString &, const QString &, bool clear = true);
    void                                mergeProjectIntoGroup();

    void                                pasteSelectedData();
    void                                copySelectedView();
    void                                copySelectedGroup();
    void                                copySelectedProject();
    void                                clearCurrentCopyState();
    void                                addViewToBeCopied(const QString &);
    void                                addProjectToBeCopied(const QString &);
    void                                addGroupToBeCopied(const QString &, const QString &);

    void                                loadRecentProject();
    void                                updateRecentProjectActions();
    void                                loadProjectFile(const QString &);
    void                                saveProjectFile(const QString &);
    void                                setRecentProject(const QString &);

    void                                gitShowStatus();
    void                                gitCommitChanges();
    void                                gitShowLog();
    void                                gitShowDiff();
    void                                gitPull();
    void                                gitPush();
    void                                gitCheckout();

    void                                on_actionSave_triggered();
    void                                on_actionSave_As_triggered();
    void                                on_actionExit_triggered();
    void                                on_actionShow_Categories_toggled(bool);
    void                                on_actionShow_Documents_toggled(bool);

    void                                on_actionTools_triggered();
    void                                on_actionProjects_triggered();
    void                                on_actionOpen_triggered();
    void                                on_actionClear_Recent_File_Stack_triggered();

    void                                on_treeLibs_itemClicked(QTreeWidgetItem *item, int column);
    void                                on_listViews_itemDoubleClicked(QTreeWidgetItem *item, int);
    void                                on_listDocumentation_itemDoubleClicked(QTreeWidgetItem *item);
    void                                on_listGroups_itemClicked(QListWidgetItem *item);
    void                                on_listCategories_itemClicked(QTreeWidgetItem *item);
    void                                on_treeLibs_itemChanged(QTreeWidgetItem *item, int column);
    void                                on_listCategories_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void                                on_txtLibSearch_textEdited(const QString &arg1);
    void                                on_txtCatSearch_textEdited(const QString &arg1);
    void                                on_txtCellSearch_textEdited(const QString &arg1);
    void                                on_txtViewSearch_textEdited(const QString &arg1);

    void                                on_actionAbout_triggered();
    void                                on_actionProject_triggered();
    void                                on_actionGroup_triggered();
    void                                on_actionUnion_triggered();
    void                                on_actionCategory_triggered();
    void                                on_actionSession_triggered();

private:
    void                                loadSettings();

    void                                info(const QString &msg, bool clear = true);
    void                                error(const QString &msg, bool clear = true);

    void                                initRecentProjectMenu();

    bool                                createNewFile(const QString &);
    bool                                isStateChanged() const;

    void                                setStateSaved();
    void                                setStateChanged();

    bool                                filterViewsTreeItemNoPopulate(QTreeWidgetItem *item, const QString &filter);

    void                                loadGdsHierarchyAsync(const QString &gdsPath,
                                                              const std::shared_ptr<GdsCacheEntry> &entry,
                                                              QTreeWidgetItem *targetItem,
                                                              const QString &requestedCellName = QString());

    void                                loadOasHierarchyAsync(const QString &oasPath,
                                                              const std::shared_ptr<OasCacheEntry> &entry,
                                                              QTreeWidgetItem *targetItem,
                                                              const QString &requestedCellName = QString());

    void                                setLoadingSpinner(QTreeWidgetItem *item, bool on);

    std::shared_ptr<GdsCacheEntry>      ensureGdsLoaded(const QString &gdsPath);
    void                                populateGdsTopLevel(QTreeWidgetItem *gdsItem,
                                                            const std::shared_ptr<GdsCacheEntry> &entry);
    void                                populateCellChildren(QTreeWidgetItem *cellItem,
                                                             const std::shared_ptr<GdsCacheEntry> &entry,
                                                             const QString &cellName);

    std::shared_ptr<OasCacheEntry>      ensureOasLoaded(const QString &oasPath);
    void                                populateOasTopLevel(QTreeWidgetItem *oasItem,
                                                            const std::shared_ptr<OasCacheEntry> &entry);
    void                                populateOasCellChildren(QTreeWidgetItem *cellItem,
                                                                const std::shared_ptr<OasCacheEntry> &entry,
                                                                const QString &cellName);

    void                                checkAndSaveProjectData(QCloseEvent *);

    void                                createProjectUnionMenu();

    void                                loadLibraries();
    void                                loadGroups(const QString &libPath);
    void                                loadDocuments(const QString &libPath);
    void                                loadCategories(const QString &libPath);
    void                                loadCombinedLibs(const QMap<QString, QStringList> &);
    void                                loadViews(const QString &libPath, const QString &groupName);

    void                                hideTreeItem(QTreeWidget *, const QString &filter);
    void                                hideListItem(QListWidget *, const QString &filter);
    bool                                filterTreeItem(QTreeWidgetItem *item, const QString &filter);

    bool                                isViewCopied() const;
    bool                                isGroupCopied() const;
    bool                                isProjectCopied() const;

    bool                                askForFileReplacement() const;
    bool                                askForPermanentDelete() const;
    bool                                askUserForAction(const QString &title) const;

    bool                                removeDir(const QString &) const;
    void                                copyDir(const QString &, const QString &) const;

    QString                             getLibraryPath(const QString &) const;
    QString                             getLibraryKeyPrefix() const;

    QString                             getProjectFileFromDir(const QString &) const;
    QString                             expandShellVariables(const QString &path) const;

    QString                             generateCopyName(const QString &, const QString &, const QString &suffix = "") const;

    QString                             getViewPath(const QString &, const QString &, const QString &) const;

    QString                             getCurrentViewName() const;
    QString                             getCurrentGroupName() const;
    QString                             getCurrentUnionName() const;
    QString                             getCurrentWorkingDir() const;
    QString                             getCurrentLibraryName() const;
    QString                             getCurrentProjectFile() const;
    QString                             getCurrentLibraryPath() const;
    QString                             getCurrentCategoryName() const;

    QString                             getCurrentGitPathForItem() const;

    QString                             getCurrentViewFilePath(const QString &) const;
    QString                             getCurrentDocumentFilePath(const QString &) const;
    QString                             getCurrentGroupPath(const QString &viewName, bool toBeCreated = false);

    QString                             getSettingsHeaderName() const;
    QString                             getToolByView(const QString &) const;
    QString                             getDocumentTool(const QString &) const;

    QString                             getLibManTitle() const;

    QStringList                         getValidViewList() const;
    QStringList                         getCurrentGroups(const QString &) const;
    QStringList                         getCurrentViews(const QString &, const QString &) const;
    QStringList                         readLibraryCategories(const QString &, const QString &);

    QTreeWidgetItem*                    getTreeItemByName(const QString &name);

    QMap<QString, QString>              getCurrentLibraries() const;

    bool                                ensureKLayoutServerRunning(const QString &tool);
    bool                                sendKLayoutOpenRequest(const QString &gdsPath, const QString &cellName);
    QString                             createKLayoutServerScript(const QString &cmdFile) const;
    bool                                sendKLayoutSelectRequest(const QString &gdsPath, const QString &cellName);

    QString                             createKLayoutOpenScript(const QString &gdsPath, const QString &cellName) const;
    void                                startToolWithTempScript(const QString &tool,
                                                                const QStringList &args,
                                                                const QString &scriptPath);

private:
    Ui::MainWindow                     *m_ui                   = nullptr;  /*!< Main window UI instance (Qt Designer). */
    Properties                         *m_properties           = nullptr;  /*!< Properties collection with all settings. */

    bool                                m_isStateChanged       = false;    /*!< True if project state differs from last saved state. */

    QString                             m_itemText;                        /*!< Name of the last selected item. */
    QString                             m_runDirectory;                    /*!< Directory where LibMan was started. */
    QString                             m_currentProjFile;                 /*!< Currently loaded project file path. */

    QList<QAction*>                     m_recentProjects;                  /*!< Recent project actions list (menu). */

    QStringList                         m_copyData;                        /*!< Copy buffer: [name, path, ...] depending on COPY_STATE. */
    COPY_STATE                          m_currentCopyState     = NONE;     /*!< Current copy state (project/group/view). */

    QProcess                           *m_klayoutProc          = nullptr;  /*!< Running KLayout process instance (optional). */
    QString                             m_klayoutCmdFile;                  /*!< Path to KLayout JSON command file. */
    QString                             m_klayoutServerScript;             /*!< Path to generated KLayout server script. */

    QHash<QTreeWidgetItem*, SpinnerState> m_spinnerStates;                 /*!< Spinner animation state per tree item. */

    QHash<QString, std::shared_ptr<GdsCacheEntry>> m_gdsCache;             /*!< GDS hierarchy cache: abs path -> entry. */
    QHash<QString, std::shared_ptr<OasCacheEntry>> m_oasCache;             /*!< OASIS hierarchy cache: abs path -> entry. */
};

/*!*******************************************************************************************************************
 * \brief Returns prefix for storing libraries in the LibMan settings collection.
 ********************************************************************************************************************/
inline QString MainWindow::getLibraryKeyPrefix() const
{
    return "LIBRARY_";
}

/*!*******************************************************************************************************************
 * \brief Returns header name used for storing LibMan settings.
 ********************************************************************************************************************/
inline QString MainWindow::getSettingsHeaderName() const
{
    return "LIBAMN";
}

/*!*******************************************************************************************************************
 * \brief Returns true if the LibMan window state has been changed.
 ********************************************************************************************************************/
inline bool MainWindow::isStateChanged() const
{
    return m_isStateChanged;
}

/*!*******************************************************************************************************************
 * \brief Returns path to the current project file.
 ********************************************************************************************************************/
inline QString MainWindow::getCurrentProjectFile() const
{
    return m_currentProjFile;
}

/*!*******************************************************************************************************************
 * \brief Returns true if a project copy operation is active.
 ********************************************************************************************************************/
inline bool MainWindow::isProjectCopied() const
{
    return (m_currentCopyState == PROJECT);
}

/*!*******************************************************************************************************************
 * \brief Returns true if a group (cell) copy operation is active.
 ********************************************************************************************************************/
inline bool MainWindow::isGroupCopied() const
{
    return (m_currentCopyState == GROUP);
}

/*!*******************************************************************************************************************
 * \brief Returns true if a view copy operation is active.
 ********************************************************************************************************************/
inline bool MainWindow::isViewCopied() const
{
    return (m_currentCopyState == VIEW);
}

/*!*******************************************************************************************************************
 * \brief Returns title of the LibMan.
 ********************************************************************************************************************/
inline QString MainWindow::getLibManTitle() const
{
    return QString("LibMan - Library Project Manager");
}

#endif // MAINWINDOW_H
