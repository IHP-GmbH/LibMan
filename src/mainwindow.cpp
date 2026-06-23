#include <QMenu>
#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QProcess>
#include <QVariant>
#include <QFileInfo>
#include <QSettings>
#include <QMouseEvent>
#include <QTextStream>
#include <QFileDialog>
#include <QDirIterator>
#include <QInputDialog>
#include <QTemporaryFile>
#include <QGuiApplication>
#include <QFileSystemWatcher>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include <QListWidgetItem>

#if QT_VERSION >= 0x050000
#include <QScreen>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "about.h"
#include "newview.h"
#include "property.h"
#include "toolmanager.h"
#include "libfileparser.h"
#include "projectmanager.h"
#include "projecteditor.h"
#include "core/core_path_utils.h"

/*!*******************************************************************************************************************
 * \brief Constructs a LibMan MainWindow object with the given arguments.
 * \param projFile      Path to the project file. Be default, it will be searched in the current folder.
 * \param runDir        Path to the directory, where application was executed.
 * \param parent        Parent widget, by default is NULL.
 *********************************************************************************************************************/
MainWindow::MainWindow(const QString &projFile, const QString &runDir, QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_properties(new Properties),
    m_isStateChanged(false),
    m_itemText(""),
    m_runDirectory(runDir),
    m_currentProjFile(QString("")),
    m_currentCopyState(NONE),
    m_projFileWatcher(new QFileSystemWatcher(this))
{
    m_ui->setupUi(this);

    m_ui->actionProjects->setVisible(false);
    m_ui->textMessages->setReadOnly(true);

    m_ui->groupCats->setVisible(false);
    m_ui->groupDocs->setVisible(false);
    m_ui->actionShow_Documents->setChecked(false);
    m_ui->actionShow_Categories->setChecked(false);

    m_ui->actionGroup->setEnabled(false);
    m_ui->actionUnion->setEnabled(false);
    m_ui->actionCategory->setEnabled(false);

    initRecentProjectMenu();

    loadSettings();

    setWindowIcon(QIcon(":logo"));
    initIcons();

    m_ui->treeLibs->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->listViews->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->listGroups->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->listCategories->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui->treeLibs, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showLibraryMenu(const QPoint &)));
    connect(m_ui->listViews, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showViewMenu(const QPoint &)));
    connect(m_ui->listGroups, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showGroupMenu(const QPoint &)));
    connect(m_ui->listCategories, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCategoryMenu(const QPoint &)));
    connect(m_ui->listViews, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(on_viewItemExpanded(QTreeWidgetItem*)));
    connect(m_ui->treeLibs, SIGNAL(itemSelectionChanged()), this, SLOT(on_treeLibs_itemSelectionChanged()));

    m_ui->listViews->viewport()->installEventFilter(this);

    setupDragAndDrop();

    setWindowTitle(getLibManTitle());

    if(QFileInfo(projFile).exists()) {
        loadProjectFile(projFile);
    }
    else {
        QString localProjFile = getProjectFileFromDir(runDir);
        if(QFileInfo(localProjFile).exists()) {
            loadProjectFile(localProjFile);
        }
    }

    connect(m_projFileWatcher,
            SIGNAL(fileChanged(QString)),
            this,
            SLOT(onProjectFileChanged(QString)));

    m_ui->listViews->setHeaderHidden(true);
}

/*!*******************************************************************************************************************
 * \brief Destroys the main window.
 *********************************************************************************************************************/
MainWindow::~MainWindow()
{
    delete m_ui;
    delete m_properties;
}

/*!********************************************************************************************************************
 * \brief Initializes all action icons in the main window.
 *
 * This function assigns SVG icons to all UI actions, ensuring a consistent
 * visual style across menus and toolbars. It should be called once after
 * the UI has been set up (e.g. in the constructor after setupUi()).
 *********************************************************************************************************************/
void MainWindow::initIcons()
{
    // File / Project
    m_ui->actionProject->setIcon(QIcon(":/icons/category.svg"));
    m_ui->actionGroup->setIcon(QIcon(":/icons/category.svg"));
    m_ui->actionUnion->setIcon(QIcon(":/icons/view.svg"));

    // Open / Save
    m_ui->actionOpen->setIcon(QIcon(":/icons/category.svg"));
    m_ui->actionSave->setIcon(QIcon(":/icons/save.svg"));
    m_ui->actionSave_As->setIcon(QIcon(":/icons/save.svg"));

    // Exit
    m_ui->actionExit->setIcon(QIcon(":/icons/exit.svg"));

    // View switching
    m_ui->actionShow_Documents->setIcon(QIcon(":/icons/show_documents.svg"));
    m_ui->actionShow_Categories->setIcon(QIcon(":/icons/show_categories.svg"));

    // Category
    m_ui->actionCategory->setIcon(QIcon(":/icons/category.svg"));

    // Session
    m_ui->actionSession->setIcon(QIcon(":/icons/view.svg"));

    // Reload
    m_ui->actionReload->setIcon(QIcon(":/icons/reload.svg"));

    // Tools / About
    m_ui->actionTools->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_ui->actionAbout->setIcon(QIcon(":/icons/info.svg"));

    // Recent
    QIcon recentIcon(":/icons/documents.svg");
    m_ui->actionRecent1->setIcon(recentIcon);
    m_ui->actionRecent2->setIcon(recentIcon);
    m_ui->actionRecent3->setIcon(recentIcon);
    m_ui->actionRecent4->setIcon(recentIcon);
    m_ui->actionRecent5->setIcon(recentIcon);

    // Clear recent
    m_ui->actionClear_Recent_File_Stack->setIcon(QIcon(":/icons/delete.svg"));
}

/*!*******************************************************************************************************************
 * \brief This event handler is called with the given event when Qt receives a window close request for a top-level
 * widget from the window system. The function is used to save the projects settings.
 * \param event       Variable is used to either accept to close the window or ignore to prevent closing it.
 **********************************************************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings(getSettingsHeaderName());
    settings.setDefaultFormat(QSettings::NativeFormat);

    settings.beginGroup("Appearance");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("WindowState", saveState());
    settings.setValue("ShowDocuments", m_ui->groupDocs->isVisible());
    settings.setValue("ShowCategories", m_ui->groupCats->isVisible());
    settings.endGroup();

    settings.beginGroup("Tools");

    QString schematic = "nedit";
    if(m_properties->exists("Schematic")) {
        schematic = m_properties->get<QString>("Schematic");
    }

    QString layout = "klayout";
    if(m_properties->exists("Layout")) {
        layout = m_properties->get<QString>("Layout");
    }

    QString editor = "nedit";
    if(m_properties->exists("Editor")) {
        editor = m_properties->get<QString>("Editor");
    }

    QString pdfReader = "";
    if(m_properties->exists("PdfReader")) {
        pdfReader = m_properties->get<QString>("PdfReader");
    }

    if(m_properties->exists("ToolList")) {
        QStringList tools = m_properties->get<QString>("ToolList").split(",");
        settings.setValue("ToolList", tools);
        foreach(const QString &name, tools) {
            if(m_properties->exists(name)) {
                settings.setValue(name, m_properties->get<QString>(name));
            }

            if(m_properties->exists(name + "Views")) {
                settings.setValue(name + "Views", m_properties->get<QString>(name + "Views"));
            }
        }
    }

    settings.setValue("Editor", editor);
    settings.setValue("PdfReader", pdfReader);
    settings.endGroup();

    checkAndSaveProjectData(event);

    QMainWindow::closeEvent(event);
}

/*!*******************************************************************************************************************
 * \brief If state of the LibMan has been changed the dialog window pops up to ask user for saving of the changes.
 * \param event       Variable is used to either accept to close the window or ignore to prevent closing it.
 **********************************************************************************************************************/
void MainWindow::checkAndSaveProjectData(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setText("The project settings have been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    QString windowTitle = this->windowTitle();
    if(windowTitle.contains("*")) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QScreen* pScreen = QGuiApplication::screenAt(this->mapToGlobal(QPoint(this->width() / 2, 0)));
        QRect screenRect = pScreen ? pScreen->availableGeometry() : QRect();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        QScreen* pScreen = QGuiApplication::screenAt(this->mapToGlobal(QPoint(this->width() / 2, 0)));
        QRect screenRect = pScreen ? pScreen->availableGeometry() : QRect();
#else
        QRect screenRect = QApplication::desktop()->screenGeometry(this);
#endif

        msgBox.move(QPoint(screenRect.width()/2, screenRect.height()/2));

        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                on_actionSave_triggered();
                event->accept();
                break;
            case QMessageBox::Discard:
                event->accept();
                break;
            case QMessageBox::Cancel:
                event->ignore();
                break;
            default:
                break;
        }
    }
}

/*!*******************************************************************************************************************
 * \brief Loads LibMan settings (geometry, tools, widget states etc.) and aplies them accordingly.
 **********************************************************************************************************************/
void MainWindow::loadSettings()
{
    QSettings settings(getSettingsHeaderName());

    settings.beginGroup("Appearance");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());

    m_ui->groupDocs->setVisible(settings.value("ShowDocuments", false).toBool());
    m_ui->groupCats->setVisible(settings.value("ShowCategories", false).toBool());
    m_ui->actionShow_Documents->setChecked(settings.value("ShowDocuments", false).toBool());
    m_ui->actionShow_Categories->setChecked(settings.value("ShowCategories", false).toBool());

    settings.endGroup();

    settings.beginGroup("Tools");

    QString editor = "nedit";
    if(settings.contains("Editor")) {
        editor = settings.value("Editor").toString();
    }
    m_properties->set("Editor", editor);

    QString pdfReader = "";
    if(settings.contains("PdfReader")) {
        pdfReader = settings.value("PdfReader").toString();
    }

    if(settings.contains("ToolList")) {
        QStringList tools = settings.value("ToolList").toStringList();
        if(tools.count()) {
            m_properties->set("ToolList", tools.join(","));
            foreach(const QString name, tools) {
                if(settings.contains(name)) {
                    QString tool = settings.value(name).toString();
                    QString views = settings.value(name + "Views").toString();

                    m_properties->set(name, tool);
                    m_properties->set(name + "Views", views);
                }
            }
        }
    }
    else {
        m_properties->set("ToolList", "Schematic,Layout");
        m_properties->set("Schematic", "nedit");
        m_properties->set("SchematicViews", "cdl,spice,schematic,symbol");
        m_properties->set("Layout", "klayout");
        m_properties->set("LayoutViews", "gds,oas,lstr,layout");
    }

    m_properties->set("PdfReader", pdfReader);

    settings.endGroup();
}

/*!*******************************************************************************************************************
 * \brief Returns list of the currently loaded projects (libraries).
 **********************************************************************************************************************/
QMap<QString, QString> MainWindow::getCurrentLibraries() const
{
    QMap<QString, QString> libMap;

    const QMap<QString, PropertyItem*> propItems = m_properties->getMap();

    for(auto it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        const QString key = it.key();
        if(!key.toUpper().startsWith(getLibraryKeyPrefix())) {
            continue;
        }

        QString tail = key;
        tail.remove(getLibraryKeyPrefix());

        const int pos = tail.indexOf('/');
        const QString libName = (pos >= 0) ? tail.left(pos).trimmed() : tail.trimmed();
        if(libName.isEmpty()) {
            continue;
        }

        const QString filePath = m_properties->get<QString>(key).trimmed();
        QFileInfo fi(filePath);
        if(!fi.exists() || !fi.isFile()) {
            continue;
        }

        QDir libDir = fi.dir();
        if(!libDir.cdUp()) {
            continue;
        }

        const QString rootPath = QDir::toNativeSeparators(libDir.absolutePath());

        if(!libMap.contains(libName)) {
            libMap[libName] = rootPath;
        }
    }

    return libMap;
}

/*!*******************************************************************************************************************
 * \brief Triggers execution of ToolManager dialog window.
 **********************************************************************************************************************/
void MainWindow::on_actionTools_triggered()
{
    ToolManager(this, m_properties).exec();
}

/*!*******************************************************************************************************************
 * \brief Triggers execution of ProjectManager dialog window.
 *********************************************************************************************************************/
void MainWindow::on_actionProjects_triggered()
{
    ProjectManager(this, m_properties).exec();
}

void MainWindow::on_actionEditProject_triggered()
{
    ProjectEditor editor(this);
    editor.exec();
}

/*!*******************************************************************************************************************
 * \brief Initializes recent menu and assigns menu items with assotiated project files.
 ********************************************************************************************************************/
void MainWindow::initRecentProjectMenu()
{
    m_recentProjects.push_back(m_ui->actionRecent1);
    m_recentProjects.push_back(m_ui->actionRecent2);
    m_recentProjects.push_back(m_ui->actionRecent3);
    m_recentProjects.push_back(m_ui->actionRecent4);
    m_recentProjects.push_back(m_ui->actionRecent5);

    for(int i = 0; i < m_recentProjects.count(); ++i) {
        m_recentProjects[i]->setVisible(false);
        connect(m_recentProjects[i],
                SIGNAL(triggered()),
                this,
                SLOT(loadRecentProject()));
    }

    updateRecentProjectActions();
}

/*!*******************************************************************************************************************
 * \brief Loads the most recent project file into LibMan environment.
 **********************************************************************************************************************/
void MainWindow::loadRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString projFile = action->data().toString();
        if(!QFileInfo(projFile).exists()) {
            QMessageBox::critical(this,
                                  "Open Project Error",
                                  QString("Can not open project '%1'.").arg(projFile));
            return;
        }

        loadProjectFile(projFile);
    }
}

/*!*******************************************************************************************************************
 * \brief Sets the name of the last loaded project to the upper recent file list.
 * \param fileName     Name of the project file.
 **********************************************************************************************************************/
void MainWindow::setRecentProject(const QString &fileName)
{
    if(fileName.isEmpty()) {
        return;
    }

    const QString absFileName = QFileInfo(fileName).absoluteFilePath();

    QSettings settings(getSettingsHeaderName());
    settings.beginGroup("RecentProjects");

    QStringList files = settings.value("RecentProjList").toStringList();
    for(int i = files.size() - 1; i >= 0; --i) {
        if(QFileInfo(files[i]).absoluteFilePath() == absFileName) {
            files.removeAt(i);
        }
    }
    files.prepend(absFileName);
    while (files.size() > PROJ_MAX_COUNT) {
        files.removeLast();
    }

    settings.setValue("RecentProjList", files);
    settings.endGroup();

    updateRecentProjectActions();
}

/*!*******************************************************************************************************************
 * \brief Updates File->Recent menu items.
 **********************************************************************************************************************/
void MainWindow::updateRecentProjectActions()
{
    QSettings settings(getSettingsHeaderName());
    settings.beginGroup("RecentProjects");
    QStringList files = settings.value("RecentProjList").toStringList();
    settings.endGroup();

    int slot = 0;
    for(int i = 0; i < files.size() && slot < PROJ_MAX_COUNT; ++i) {
        const QString absFileName = QFileInfo(files[i]).absoluteFilePath();
        if(!QFileInfo(absFileName).exists()) {
            continue;
        }

        const QString text = tr("&%1 %2").arg(slot + 1).arg(absFileName);
        m_recentProjects[slot]->setText(text);
        m_recentProjects[slot]->setData(absFileName);
        m_recentProjects[slot]->setVisible(true);
        ++slot;
    }

    for(int j = slot; j < PROJ_MAX_COUNT; ++j) {
        m_recentProjects[j]->setVisible(false);
    }
}

/*!*******************************************************************************************************************
 * \brief Returns current working directory from where the project file has been loaded, otherwise user home path.
 **********************************************************************************************************************/
QString MainWindow::getCurrentWorkingDir() const
{
    if(!m_currentProjFile.isEmpty()) {
        const QString projDir = QFileInfo(m_currentProjFile).absolutePath();
        if(QFileInfo(projDir).exists()) {
            return projDir;
        }
    }

    if(m_ui->actionRecent1 && m_ui->actionRecent1->isVisible()) {
        const QString recentFile = m_ui->actionRecent1->data().toString();
        if(!recentFile.isEmpty()) {
            const QString workDir = QFileInfo(recentFile).absolutePath();
            if(QFileInfo(workDir).exists()) {
                return workDir;
            }
        }
    }

    return m_runDirectory;
}

/*!*******************************************************************************************************************
 * \brief Returns library path based on the project (library) name.
 * \param libName      Name of the library.
 **********************************************************************************************************************/
QString MainWindow::getLibraryPath(const QString &libName) const
{
    if(libName.isEmpty()) {
        return QString();
    }

    const QMap<QString, PropertyItem*> propItems = m_properties->getMap();
    const QString prefix = getLibraryKeyPrefix() + libName + "/";

    for(auto it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        const QString key = it.key();
        if(!key.startsWith(prefix)) {
            continue;
        }

        const QString filePath = m_properties->get<QString>(key).trimmed();
        if(filePath.isEmpty()) {
            continue;
        }

        QFileInfo fi(filePath);
        if(!fi.exists() || !fi.isFile()) {
            continue;
        }

        QDir viewDir = fi.dir();
        if(!viewDir.cdUp()) {
            continue;
        }

        return QDir::toNativeSeparators(viewDir.absolutePath());
    }

    const QString rootKey = getLibraryKeyPrefix() + libName;
    if(m_properties->exists(rootKey)) {
        const QString rootPath = m_properties->get<QString>(rootKey).trimmed();
        QFileInfo rootInfo(rootPath);
        if(rootInfo.exists() && rootInfo.isDir()) {
            return QDir::toNativeSeparators(rootInfo.absoluteFilePath());
        }
    }

    return QString();
}

/*!*******************************************************************************************************************
 * \brief Stores the root directory for a library created without layout files.
 **********************************************************************************************************************/
void MainWindow::setLibraryRootDirectory(const QString &libName, const QString &dirPath)
{
    if(libName.isEmpty() || dirPath.isEmpty()) {
        return;
    }

    const QString key = getLibraryKeyPrefix() + libName;
    m_properties->set(key, QDir::toNativeSeparators(dirPath));
}

/*!*******************************************************************************************************************
 * \brief Launches a dialog window to select a lib file to load.
 **********************************************************************************************************************/
void MainWindow::on_actionOpen_triggered()
{
    QString workDir = getCurrentWorkingDir();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open file(s)"),
                                                    workDir,
                                                    tr("KLayout Lib (*.lib);; All (*)"));

    if(!fileName.isEmpty()) {
        loadProjectFile(fileName);
    }
}

/*!*******************************************************************************************************************
 * \brief Displays basic message in the output text widget of LibMan.
 * \param msg         Message to output.
 * \param clear       Flag to clear the output text widget of LibMan before displaying the message.
 **********************************************************************************************************************/
void MainWindow::info(const QString &msg, bool clear)
{
    if(clear) {
        m_ui->textMessages->clear();
    }

    m_ui->textMessages->setTextColor(Qt::black);
    m_ui->textMessages->insertPlainText("[INFO] " + msg + "\n");
    m_ui->textMessages->setTextColor(Qt::black);
}

/*!*******************************************************************************************************************
 * \brief Displays error message in the output text widget of LibMan.
 * \param msg         Message to output.
 * \param clear       Flag to clear the output text widget of LibMan before displaying the message.
 **********************************************************************************************************************/
void MainWindow::error(const QString &msg, bool clear)
{
    if(clear) {
        m_ui->textMessages->clear();
    }

    m_ui->textMessages->setTextColor(Qt::red);
    m_ui->textMessages->insertPlainText("[ERROR] " + msg);
    m_ui->textMessages->setTextColor(Qt::black);
}

/*!*******************************************************************************************************************
 * \brief Returns list of valid view names.
 **********************************************************************************************************************/
QStringList MainWindow::getValidViewList() const
{
    QStringList views;
    views<<"oas"<<"gds"<<"cdl"<<"spice"<<"verilog";
    return views;
}

/*!*******************************************************************************************************************
 * \brief Returns specified by user tool for displaying views based on view name.
 * \param viewName     Name of the view to return an appropriate tool.
 **********************************************************************************************************************/
QString MainWindow::getToolByView(const QString &viewName) const
{
    const QString view = viewName.trimmed().toLower();
    if(view.isEmpty()) {
        return QString();
    }

    const QStringList tools = m_properties->get<QString>("ToolList").split(",", QString::SkipEmptyParts);
    for(const QString &name : tools) {
        const QString toolName = name.trimmed();
        if(toolName.isEmpty()) {
            continue;
        }

        if(layoutViewsForTool(toolName).contains(view)) {
            if(m_properties->exists(toolName)) {
                return m_properties->get<QString>(toolName);
            }
        }
    }

    return QString();
}

QStringList MainWindow::layoutViewsForTool(const QString &toolName) const
{
    const QString trimmedTool = toolName.trimmed();
    const QString key = trimmedTool + QStringLiteral("Views");
    if(!m_properties->exists(key)) {
        return {};
    }

    QStringList views =
        m_properties->get<QString>(key).remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts);

    for(QString &view : views) {
        view = view.trimmed().toLower();
    }
    views.removeAll(QString());

    if(trimmedTool == QStringLiteral("Layout") && !views.contains(QStringLiteral("layout"))) {
        views << QStringLiteral("layout");
    }

    views.sort();
    return views;
}

QString MainWindow::layoutPathForKLayout(const QString &viewName,
                                         const QString &viewPath,
                                         QStringList *errors) const
{
    if(isLayoutCoreViewName(viewName)) {
        return coreLayoutPathForKLayout(viewPath, errors);
    }

    return QFileInfo(viewPath).absoluteFilePath();
}

namespace {

LayoutHierarchySnapshot snapshotFromGds(const GdsReader::GdsHierarchy &hierarchy)
{
    LayoutHierarchySnapshot snapshot;
    snapshot.topCells = hierarchy.topCells;
    snapshot.allCells = hierarchy.allCells;
    return snapshot;
}

LayoutHierarchySnapshot snapshotFromOas(const LayoutHierarchy &hierarchy)
{
    LayoutHierarchySnapshot snapshot;
    snapshot.topCells = hierarchy.topCells;
    snapshot.allCells = hierarchy.allCells;
    return snapshot;
}

LayoutHierarchySnapshot snapshotFromCore(const CoreCellReader::CoreHierarchy &hierarchy)
{
    LayoutHierarchySnapshot snapshot;
    snapshot.topCells = hierarchy.topCells;
    snapshot.allCells = hierarchy.allCells;
    return snapshot;
}

LayoutHierarchySnapshot snapshotFromLStream(const QStringList &cellNames)
{
    LayoutHierarchySnapshot snapshot;
    snapshot.topCells = cellNames;
    snapshot.topCells.sort();
    for (const QString &name : cellNames) {
        snapshot.allCells.insert(name);
    }
    return snapshot;
}

} // namespace

QString MainWindow::preferredKLayoutCellForRoot(const QString &layoutPath,
                                              const QString &groupName) const
{
    const QString key = QFileInfo(layoutPath).absoluteFilePath();
    LayoutHierarchySnapshot snapshot;
    bool haveSnapshot = false;

    const auto useSnapshot = [&](const LayoutHierarchySnapshot &candidate) {
        if (!candidate.allCells.isEmpty() || !candidate.topCells.isEmpty()) {
            snapshot = candidate;
            haveSnapshot = true;
        }
    };

    if (const auto it = m_gdsCache.constFind(key); it != m_gdsCache.cend()) {
        const std::shared_ptr<GdsCacheEntry> &entry = it.value();
        if (entry && entry->loaded) {
            useSnapshot(snapshotFromGds(entry->hierarchy));
        }
    }
    if (!haveSnapshot) {
        if (const auto it = m_oasCache.constFind(key); it != m_oasCache.cend()) {
            const std::shared_ptr<OasCacheEntry> &entry = it.value();
            if (entry && entry->loaded) {
                useSnapshot(snapshotFromOas(entry->hierarchy));
            }
        }
    }
    if (!haveSnapshot) {
        if (const auto it = m_coreCache.constFind(key); it != m_coreCache.cend()) {
            const std::shared_ptr<CoreCacheEntry> &entry = it.value();
            if (entry && entry->loaded) {
                useSnapshot(snapshotFromCore(entry->hierarchy));
            }
        }
    }
    if (!haveSnapshot) {
        if (const auto it = m_lstreamCache.constFind(key); it != m_lstreamCache.cend()) {
            const std::shared_ptr<LStreamCacheEntry> &entry = it.value();
            if (entry && entry->loaded) {
                useSnapshot(snapshotFromLStream(entry->cellNames));
            }
        }
    }

    if (!haveSnapshot) {
        QStringList errors;
        haveSnapshot = loadLayoutHierarchySnapshot(layoutPath, snapshot, &errors);
        Q_UNUSED(errors);
    }

    if (!haveSnapshot) {
        return QString();
    }

    return resolveKLayoutRootCell(snapshot, groupName);
}

bool MainWindow::isLayoutViewTreeItem(QTreeWidgetItem *item) const
{
    if (!item) {
        return false;
    }

    const int type = item->data(0, RoleType).toInt();

    if (type == ItemViewGds || type == ItemViewOas || type == ItemViewLStream) {
        return true;
    }

    if (type == ItemViewCore && isLayoutCoreViewName(item->text(0))) {
        return true;
    }

    if (type != ItemCell) {
        return false;
    }

    QTreeWidgetItem *root = item;
    while (root->parent()) {
        root = root->parent();
    }

    const int rootType = root->data(0, RoleType).toInt();
    if (rootType == ItemViewGds || rootType == ItemViewOas || rootType == ItemViewLStream) {
        return true;
    }

    return rootType == ItemViewCore && isLayoutCoreViewName(root->text(0));
}

/*!*******************************************************************************************************************
 * \brief Returns specified by user tool for viewing documents based on doument name.
 * \param documentName     Name of the document to return an appropriate tool.
 **********************************************************************************************************************/
QString MainWindow::getDocumentTool(const QString &documentName) const
{
    QString tool;

    if(QFileInfo(documentName).completeSuffix().toLower() == "pdf") {
        tool = m_properties->get<QString> ("PdfReader");
    }
    else {
        tool = m_properties->get<QString> ("Editor");
    }

    return tool;
}

/*!*******************************************************************************************************************
 * \brief Returns absolute path of the view based on given library/cell/view information.
 *
 * In KLayout .lib based workflow a library points directly to a layout file,
 * therefore the returned path is the library file itself. The groupName and
 * viewName arguments are currently unused and kept only for API compatibility.
 *
 * \param libName       Name of the library.
 * \param groupName     Name of the group (cell).
 * \param viewName      Name of the view.
 **********************************************************************************************************************/
QString MainWindow::getViewPath(const QString &libName,
                                const QString &groupName,
                                const QString &viewName) const
{
    if(libName.isEmpty() || groupName.isEmpty() || viewName.isEmpty()) {
        return QString();
    }

    const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
    if(!m_properties->exists(key)) {
        return QString();
    }

    return m_properties->get<QString>(key).trimmed();
}

/*!*******************************************************************************************************************
 * \brief Returns absolute path of the currently selected view file.
 *
 * In KLayout .lib based workflow the library itself points to the layout file,
 * so for layout views such as gds/oas/lstr this function returns the current
 * library file path. For non-layout views an empty string is returned.
 *
 * \param viewName     Name of the view.
 **********************************************************************************************************************/
QString MainWindow::getCurrentViewFilePath(const QString &viewName) const
{
    const QString v = viewName.trimmed().toLower();

    if(v != "gds" && v != "oas" && v != "oasis" && v != "lstr" && !isLayoutCoreViewName(v)) {
        return QString();
    }

    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists() || !QFileInfo(libPath).isFile()) {
        return QString();
    }

    return libPath;
}

/*!*******************************************************************************************************************
 * \brief Returns currently selected project union (nested projects) name.
 **********************************************************************************************************************/
QString MainWindow::getCurrentUnionName() const
{
    QString projName;

    QList<QTreeWidgetItem*> projects = m_ui->treeLibs->selectedItems();
    if(!projects.count()) {
        return projName;
    }

    QTreeWidgetItem *projId = projects.first();
    if(!projId) {
        return projName;
    }

    if(!projId->childCount()) {
        return projName;
    }

    if(projId->text(0).isEmpty()) {
        return projName;
    }

    projName = projId->text(0);

    return projName;
}

/*!*******************************************************************************************************************
 * \brief Returns currently selected name of category.
 **********************************************************************************************************************/
QString MainWindow::getCurrentCategoryName() const
{
    QString catName;

    QList<QTreeWidgetItem*> catItems = m_ui->listCategories->selectedItems();
    if(!catItems.count()) {
        return catName;
    }

    QTreeWidgetItem *catId = catItems.first();
    if(!catId) {
        return catName;
    }

    if(catId->text(0).isEmpty()) {
        return catName;
    }

    catName = catId->text(0);

    return catName;
}

/*!*******************************************************************************************************************
 * \brief Returns currently selected project (library) name.
 **********************************************************************************************************************/
QString MainWindow::getCurrentLibraryName() const
{
    QString projName;

    QList<QTreeWidgetItem*> projects = m_ui->treeLibs->selectedItems();
    if(!projects.count()) {
        return projName;
    }

    QTreeWidgetItem *projId = projects.first();
    if(!projId) {
        return projName;
    }

    if(projId->text(0).isEmpty() || projId->childCount() > 0) {
        return projName;
    }

    projName = projId->text(0);

    return projName;
}

/*!*******************************************************************************************************************
 * \brief Returns currently selected group (cell) name.
 **********************************************************************************************************************/
QString MainWindow::getCurrentGroupName() const
{
    QString groupName;

    QList<QListWidgetItem*> groupItems = m_ui->listGroups->selectedItems();
    if(!groupItems.count()) {
        return groupName;
    }

    QListWidgetItem *groupId = groupItems.first();
    if(!groupId) {
        return groupName;
    }

    if(groupId->text().isEmpty()) {
        return groupName;
    }

    groupName = groupId->text();

    return groupName;
}

/*!*******************************************************************************************************************
 * \brief Returns currently selected view name.
 **********************************************************************************************************************/
QString MainWindow::getCurrentViewName() const
{
    QList<QTreeWidgetItem*> items = m_ui->listViews->selectedItems();
    if (items.isEmpty()) {
        return QString();
    }

    QTreeWidgetItem *item = items.first();
    if (!item) {
        return QString();
    }

    return item->text(0);
}

/*!*******************************************************************************************************************
 * \brief Returns path associated with the currently selected group.
 *
 * In KLayout .lib based workflow groups are cells inside a layout file rather than
 * directories on disk. Therefore this function returns the current library file path.
 *
 * \param viewName        Name of the view.
 * \param toBeCreated     Unused for file-based libraries.
 *
 * \return Absolute path to the current library file or empty string on failure.
 **********************************************************************************************************************/
QString MainWindow::getCurrentGroupPath(const QString &viewName, bool toBeCreated)
{
    Q_UNUSED(viewName);
    Q_UNUSED(toBeCreated);

    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists() || !QFileInfo(libPath).isFile()) {
        return QString();
    }

    return libPath;
}

/*!*******************************************************************************************************************
 * \brief Returns absolute path of the currently selected library file.
 **********************************************************************************************************************/
QString MainWindow::getCurrentLibraryPath() const
{
    QList<QTreeWidgetItem*> libItems = m_ui->treeLibs->selectedItems();
    if(!libItems.count()) {
        return QString();
    }

    QTreeWidgetItem *libItem = libItems.first();
    if(!libItem) {
        return QString();
    }

    return getLibraryPath(libItem->text(0));
}

/*!*******************************************************************************************************************
 * \brief Returns absolute documentation file path based on the given library path and document name.
 * \param docName      Name of the document.
 **********************************************************************************************************************/
QString MainWindow::getCurrentDocumentFilePath(const QString &docName) const
{
    QString libPath = getCurrentLibraryPath();
    QString docPath = QDir::toNativeSeparators(libPath + "/doc/" + docName);

    if(QFileInfo(docPath).exists()) {
        return docPath;
    }

    return "";
}

/*!*******************************************************************************************************************
 * \brief Returns group (cell) list for the given library file.
 *
 * In file-based KLayout .lib workflow the library points to a specific view file,
 * therefore the group name is derived from the file base name.
 *
 * Example:
 *   ./sg13g2_stdcell/gds/sg13g2_stdcell.gds -> group "sg13g2_stdcell"
 *
 * \param libPath      Path to the library view file.
 *
 * \return List containing a single group name or empty list on failure.
 **********************************************************************************************************************/
QStringList MainWindow::getCurrentGroups(const QString &libName) const
{
    QStringList groups;

    if(libName.isEmpty()) {
        return groups;
    }

    const QMap<QString, PropertyItem*> propItems = m_properties->getMap();
    const QString prefix = getLibraryKeyPrefix() + libName + "/";

    for(auto it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        const QString key = it.key();
        if(!key.startsWith(prefix)) {
            continue;
        }

        const QString tail = key.mid(prefix.length()).trimmed();
        if(tail.isEmpty()) {
            continue;
        }

        const int pos = tail.indexOf('/');
        if(pos < 0) {
            continue;
        }

        const QString groupName = tail.left(pos).trimmed();
        if(groupName.isEmpty()) {
            continue;
        }

        if(!groups.contains(groupName)) {
            groups << groupName;
        }
    }

    groups.sort();
    return groups;
}

/*!*******************************************************************************************************************
 * \brief Returns valid views for the given library/cell.
 *
 * In KLayout .lib based workflow a library is backed by a single layout file,
 * so the available view is derived from the file extension.
 *
 * \param libPath      Path to the layout file.
 * \param groupName    Name of group (cell) to query views for.
 **********************************************************************************************************************/
QStringList MainWindow::getCurrentViews(const QString &libName, const QString &groupName) const
{
    QStringList views;

    if(libName.isEmpty() || groupName.isEmpty()) {
        return views;
    }

    const QMap<QString, PropertyItem*> propItems = m_properties->getMap();
    const QString prefix = getLibraryKeyPrefix() + libName + "/" + groupName + "/";

    for(auto it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        const QString key = it.key();
        if(!key.startsWith(prefix)) {
            continue;
        }

        const QString viewName = key.mid(prefix.length()).trimmed();
        if(viewName.isEmpty()) {
            continue;
        }

        if(!views.contains(viewName)) {
            views << viewName;
        }
    }

    views.sort();
    return views;
}

/*!*******************************************************************************************************************
 * \brief Returns absolute directory path of the currently loaded project file.
 *
 * \return Absolute project directory path or empty string if no project file is loaded.
 **********************************************************************************************************************/
QString MainWindow::getCurrentProjectDirectory() const
{
    if(m_currentProjFile.isEmpty()) {
        return QString();
    }

    QFileInfo fi(m_currentProjFile);
    if(!fi.exists() || !fi.isFile()) {
        return QString();
    }

    return QDir::toNativeSeparators(fi.absolutePath());
}

/*!*******************************************************************************************************************
 * \brief Searches recursively for PDF documents inside the given project directory.
 * \param projectDir      Absolute path to the project directory.
 *
 * \return List of absolute PDF file paths.
 **********************************************************************************************************************/
QStringList MainWindow::findProjectPdfDocuments(const QString &projectDir) const
{
    QStringList result;

    if(projectDir.isEmpty() || !QFileInfo(projectDir).isDir()) {
        return result;
    }

    QDirIterator it(projectDir,
                    QStringList() << "*.pdf",
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while(it.hasNext()) {
        const QString filePath = QDir::toNativeSeparators(it.next());
        if(!result.contains(filePath, Qt::CaseInsensitive)) {
            result << filePath;
        }
    }

    result.sort();
    return result;
}

/*!*******************************************************************************************************************
 * \brief Adds documents for the given project (library) into the document tree widget.
 *        In addition to the local "doc" folder, PDF files are searched recursively
 *        inside the current project directory.
 *
 * \param libPath      Path to the library, where documentation is located.
 **********************************************************************************************************************/
void MainWindow::loadDocuments(const QString &libPath)
{
    m_ui->listDocumentation->clear();

    QSet<QString> addedPaths;

    const QString docPath = QDir::toNativeSeparators(libPath + "/doc");
    if(QFileInfo(docPath).isDir()) {
        QStringList formats;
        formats<<"*.pdf";

        QDir docDir(docPath);
        docDir.setNameFilters(formats);
        docDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        const QStringList fileList = docDir.entryList(QDir::Files, QDir::Name);
        foreach(const QString &docName, fileList) {
            const QString absPath = QDir::toNativeSeparators(docDir.absoluteFilePath(docName));
            if(addedPaths.contains(absPath)) {
                continue;
            }

            QTreeWidgetItem *docItem = new QTreeWidgetItem;
            docItem->setText(0, docName);
            docItem->setData(0, RoleDocumentPath, absPath);

            const QString suffix = QFileInfo(absPath).suffix().toLower();
            if(suffix == "pdf") {
                docItem->setIcon(0, QIcon(":/icons/pdf.svg"));
            }
            else {
                docItem->setIcon(0, QIcon(":/icons/info.svg"));
            }

            m_ui->listDocumentation->addTopLevelItem(docItem);
        }
    }

    const QString projectDir = getCurrentProjectDirectory();
    const QStringList pdfFiles = findProjectPdfDocuments(projectDir);

    foreach(const QString &pdfPath, pdfFiles) {
        if(addedPaths.contains(pdfPath)) {
            continue;
        }

        QFileInfo fi(pdfPath);

        QTreeWidgetItem *docItem = new QTreeWidgetItem;
        docItem->setText(0, fi.fileName());
        docItem->setToolTip(0, pdfPath);
        docItem->setData(0, RoleDocumentPath, pdfPath);
        docItem->setIcon(0, QIcon(":/icons/pdf.svg"));

        m_ui->listDocumentation->addTopLevelItem(docItem);
        addedPaths.insert(pdfPath);
    }

#if QT_VERSION >= 0x050000
    m_ui->listDocumentation->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->listDocumentation->sortByColumn(0);
#endif

    m_ui->listDocumentation->resizeColumnToContents(0);
}

/*!*******************************************************************************************************************
 * \brief Clears all UI widgets that depend on currently selected library.
 *
 * This function is used when no library is selected anymore, for example after
 * clicking on an empty area in the library tree widget. It resets lists,
 * search fields and related action states so that stale data is not shown.
 **********************************************************************************************************************/
void MainWindow::clearLibrarySelectionDependentViews()
{
    m_itemText.clear();

    m_ui->txtLibSearch->clear();
    m_ui->txtCatSearch->clear();
    m_ui->txtCellSearch->clear();
    m_ui->txtViewSearch->clear();

    m_ui->listGroups->clear();
    m_ui->listViews->clear();
    m_ui->listDocumentation->clear();
    m_ui->listCategories->clear();

    updateLibraryActionStates();
}

/*!*******************************************************************************************************************
 * \brief Enables toolbar actions based on the current library/cell selection.
 **********************************************************************************************************************/
void MainWindow::updateLibraryActionStates()
{
    const QString libName = getCurrentLibraryName();
    const bool hasLibrary = !libName.isEmpty();
    const QString libRoot = hasLibrary ? getLibraryPath(libName) : QString();
    const bool hasLibraryRoot = !libRoot.isEmpty() && QFileInfo(libRoot).exists();
    const bool hasCell = !getCurrentGroupName().isEmpty();

    m_ui->actionGroup->setEnabled(hasLibrary);
    m_ui->actionUnion->setEnabled(hasLibrary && hasLibraryRoot && hasCell);
    m_ui->actionCategory->setEnabled(hasLibrary && hasLibraryRoot);
}

/*!*******************************************************************************************************************
 * \brief Enables drag-and-drop of supported layout files onto library and cell panes.
 **********************************************************************************************************************/
void MainWindow::setupDragAndDrop()
{
    m_ui->treeLibs->setAcceptDrops(true);
    m_ui->treeLibs->viewport()->setAcceptDrops(true);
    m_ui->listGroups->setAcceptDrops(true);
    m_ui->listGroups->viewport()->setAcceptDrops(true);

    m_ui->treeLibs->viewport()->installEventFilter(this);
    m_ui->listGroups->viewport()->installEventFilter(this);
}

/*!*******************************************************************************************************************
 * \brief Handles library tree selection changes.
 *
 * If the current library selection becomes empty, all dependent widgets on the
 * right side are cleared to avoid showing stale groups, views, categories and
 * documents from previously selected library.
 **********************************************************************************************************************/
void MainWindow::on_treeLibs_itemSelectionChanged()
{
    const QList<QTreeWidgetItem*> items = m_ui->treeLibs->selectedItems();
    if(items.isEmpty()) {
        clearLibrarySelectionDependentViews();
        return;
    }

    QTreeWidgetItem *item = items.first();
    if(!item) {
        clearLibrarySelectionDependentViews();
        return;
    }

    if(item->childCount()) {
        clearLibrarySelectionDependentViews();
        return;
    }

    updateLibraryActionStates();
}

/*!*******************************************************************************************************************
 * \brief Adds categories for the given project (library) into the category tree widget.
 * \param libPath      Path to the library, where category is located.
 **********************************************************************************************************************/
void MainWindow::loadCategories(const QString &libPath)
{
    m_ui->listCategories->clear();

    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QStringList formats;
    formats<<"*.group";

    QDir catDir(libPath);
    catDir.setNameFilters(formats);

    QStringList fileList = catDir.entryList();
    foreach(QString catName, fileList) {
        QTreeWidgetItem *catItem = new QTreeWidgetItem;
        catItem->setText(0, QFileInfo(catName).completeBaseName());
        m_ui->listCategories->addTopLevelItem(catItem);
    }

#if QT_VERSION >= 0x050000
    m_ui->listDocumentation->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->listDocumentation->sortByColumn(0);
#endif

    m_ui->listDocumentation->resizeColumnToContents(0);
}

/*!*******************************************************************************************************************
 * \brief Adds group (cell) for the given library into the group list widget.
 *
 * In file-based KLayout .lib workflow the group name is derived from the file base name.
 *
 * \param libPath      Path to the library view file.
 **********************************************************************************************************************/
void MainWindow::loadGroups(const QString &libName)
{
    m_ui->listGroups->clear();
    m_ui->listViews->clear();

    QStringList groups = getCurrentGroups(libName);

    foreach(const QString &groupName, groups) {
        QListWidgetItem *groupItem = new QListWidgetItem;
        groupItem->setText(groupName);
        groupItem->setFlags(groupItem->flags() | Qt::ItemIsEditable);
        m_ui->listGroups->addItem(groupItem);
    }

    m_ui->listGroups->sortItems();
}

/*!*******************************************************************************************************************
 * \brief Adds views for the given group (cell) into the view tree widget.
 *
 * In KLayout .lib based workflow the library is represented by a single layout file.
 * Therefore the view type is derived from the file extension and the selected cell
 * is opened from that same file.
 *
 * \param libPath      Path to the library layout file.
 * \param groupName    Name of group (cell) to load its view(s).
 **********************************************************************************************************************/
void MainWindow::loadViews(const QString &libName, const QString &groupName)
{
    m_ui->listViews->clear();
    m_ui->listViews->setHeaderHidden(true);
    m_ui->listViews->setRootIsDecorated(true);

    const QStringList views = getCurrentViews(libName, groupName);

    foreach(const QString &viewName, views) {
        const QString viewPath = getViewPath(libName, groupName, viewName);
        if(viewPath.isEmpty()) {
            continue;
        }

        QTreeWidgetItem *viewItem = new QTreeWidgetItem(m_ui->listViews);
        viewItem->setText(0, viewName);

        if(viewName == "gds") {
            viewItem->setData(0, RoleType, ItemViewGds);
            viewItem->setData(0, RoleGdsPath, viewPath);
            viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
        else if(viewName == "lstr") {
            viewItem->setData(0, RoleType, ItemViewLStream);
            viewItem->setData(0, RoleLStreamPath, viewPath);
            viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
        else if(viewName == "oas" || viewName == "oasis") {
            viewItem->setData(0, RoleType, ItemViewOas);
            viewItem->setData(0, RoleOasPath, viewPath);
            viewItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
        else if(isCoreViewName(viewName)) {
            configureCoreViewTreeItem(viewItem, viewName, viewPath);
        }
    }

    m_ui->listViews->sortItems(0, Qt::AscendingOrder);
}

/*!*******************************************************************************************************************
 * \brief Adds project (libraries) into the project tree widget from the LibMan settings.
 **********************************************************************************************************************/
void MainWindow::loadLibraries()
{
    m_ui->treeLibs->clear();

    QMap<QString, QString> libraries = getCurrentLibraries();
    QMap<QString, QString>::const_iterator it;

    for(it = libraries.constBegin(); it != libraries.constEnd(); ++it) {
        QString libName = it.key();
        if(libName.isEmpty()) {
            continue;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, libName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        m_ui->treeLibs->addTopLevelItem(item);
    }

#if QT_VERSION >= 0x050000
    m_ui->treeLibs->sortByColumn(0, Qt::AscendingOrder);
#else
    m_ui->treeLibs->sortByColumn(0);
#endif
}

void MainWindow::populateLibraryBrowser(const QString &libName)
{
    if (libName.isEmpty()) {
        return;
    }

    m_itemText = libName;
    m_ui->txtLibSearch->setText(libName);

    loadGroups(libName);

    const QString libPath = getLibraryPath(libName);
    loadDocuments(libPath);
    loadCategories(libPath);
    updateLibraryActionStates();
}

/*!*******************************************************************************************************************
 * \brief Adds given nested projects (libraries) into the project tree widget.
 * \param combinedLibs     Map of combined libraries, where key - is name of the library, values - is library path.
 **********************************************************************************************************************/
void MainWindow::loadCombinedLibs(const QMap<QString, QStringList> &combinedLibs)
{
    QMap<QString, QStringList>::const_iterator it;
    for(it = combinedLibs.constBegin(); it != combinedLibs.constEnd(); it++) {
        QString groupName = it.key();

        QTreeWidgetItem *groupId = new QTreeWidgetItem;
        groupId->setText(0, groupName);
        groupId->setFlags(groupId->flags() | Qt::ItemIsEditable);
        m_ui->treeLibs->addTopLevelItem(groupId);

        QStringList groupLibs = it.value();
        foreach(const QString libName, groupLibs) {
            for(int i = 0; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = m_ui->treeLibs->topLevelItem(i);
                if(!item) {
                    continue;
                }

                QString curProjName = item->text(0);

                if(curProjName == libName) {
                    QTreeWidgetItem *childId = new QTreeWidgetItem;
                    childId->setText(0, libName);
                    childId->setFlags(childId->flags() | Qt::ItemIsEditable);

                    m_ui->treeLibs->takeTopLevelItem(i);

                    groupId->addChild(childId);
                }
            }
        }
    }
}

/*!*******************************************************************************************************************
 * \brief Slot to load all groups (cells) for selected library into the group list widget.
 * \param item       Pointer to list item library.
 **********************************************************************************************************************/
void MainWindow::on_treeLibs_itemClicked(QTreeWidgetItem *item, int)
{
    m_itemText = "";

    if(!item) {
        return;
    }

    if(item->childCount()) {
        return;
    }

    m_itemText = item->text(0);

    m_ui->txtLibSearch->clear();
    m_ui->txtCatSearch->clear();
    m_ui->txtCellSearch->clear();
    m_ui->txtViewSearch->clear();

    populateLibraryBrowser(item->text(0));
}

/*!*******************************************************************************************************************
 * \brief Slot to load all views for selected group (cell) into the view list widget.
 * \param item       Pointer to list item group.
 **********************************************************************************************************************/
void MainWindow::on_listGroups_itemClicked(QListWidgetItem *item)
{
    if(!item) {
        return;
    }

    m_itemText = item->text();

    m_ui->txtCellSearch->clear();
    m_ui->txtViewSearch->clear();
    m_ui->listViews->clear();

    QList<QTreeWidgetItem*> libItems = m_ui->treeLibs->selectedItems();
    if(!libItems.count()) {
        return;
    }

    QTreeWidgetItem *libItem = libItems.first();
    if(!libItem) {
        return;
    }

    m_ui->txtCellSearch->setText(item->text());

    loadViews(libItem->text(0), item->text());

    updateLibraryActionStates();
}

/*!*******************************************************************************************************************
 * \brief Slot to execute a tool to view ctegory content.
 * \param item       Pointer to tree item category.
 * \param column     Column where double click event happened.
 **********************************************************************************************************************/
void MainWindow::on_listCategories_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(!item) {
        return;
    }

    m_itemText = item->text(column);

    m_ui->txtCatSearch->setText(m_itemText);

    QString tool = m_properties->get<QString>("Editor");
    if(tool.isEmpty()) {
        error(QString("Please specify tool first."));
        return;
    }

    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return;
    }

    QString catFile = QDir::toNativeSeparators(libPath + "/" + m_itemText + ".group");
    if(!QFileInfo(catFile).exists()) {
        return;
    }

    QProcess proc;
    QStringList args;
    args<<catFile;

    proc.startDetached(tool, args);
}

/*!*******************************************************************************************************************
 * \brief Creates a temporary KLayout python script and opens a given cell from a GDS file.
 *        The script also performs delayed "zoom fit" (GUI ready check) using a timer.
 * \param gdsPath     Absolute path to the GDS file.
 * \param cellName    Cell name to open (top cell).
 * \return Absolute path to created script file, or empty string on failure.
 **********************************************************************************************************************/
QString MainWindow::createKLayoutOpenScript(const QString &gdsPath,
                                            const QString &cellName) const
{
    auto pyRaw = [](const QString &s) -> QString {
        QString t = QDir::toNativeSeparators(s);
        t.replace("\\", "\\\\");
        t.replace("'", "\\'");
        return QString("r'%1'").arg(t);
    };

    QTemporaryFile tf(QDir::tempPath() + QDir::separator() + "libman_klayout_open_cell_XXXXXX.py");
    tf.setAutoRemove(false);

    if(!tf.open()) {
        return QString();
    }

    QTextStream out(&tf);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif

    out <<
        R"(# -*- coding: utf-8 -*-
import pya
import os
import os.path

_app = pya.Application.instance()
_mw  = _app.main_window() if _app is not None else None


#==============================================================================
def libman_cmp_paths(p1, p2):
    p1 = os.path.normcase(os.path.normpath(p1))
    p2 = os.path.normcase(os.path.normpath(p2))
    return p1 == p2


#==============================================================================
# Delayed zoom_fit (wait until the view is fully ready)
def libman_fit_view_to_window():
    global _app, _mw
    if _app is None:
        _app = pya.Application.instance()
    if _mw is None and _app is not None:
        _mw = _app.main_window()
    if _mw is None:
        return

    global _libman_fit_timer
    try:
        _libman_fit_timer
    except NameError:
        _libman_fit_timer = None

    if _libman_fit_timer is None:
        t = pya.QTimer(_mw)
        t.setSingleShot(True)

        def _on_timeout():
            app2 = pya.Application.instance()
            mw2  = app2.main_window() if app2 is not None else None
            lv2  = mw2.current_view() if mw2 is not None else None

            ready = (lv2 is not None) and (lv2.cellviews() > 0) and (lv2.active_cellview() is not None)

            if ready:
                try:
                    lv2.zoom_fit()
                except Exception:
                    pass
            else:
                t.start(200)

        t.timeout(_on_timeout)
        _libman_fit_timer = t

    if _libman_fit_timer.isActive():
        _libman_fit_timer.stop()
    _libman_fit_timer.start(200)


#==============================================================================
class LibManRequest:
    def open_cell(self, file_name, cell_name):
        if _mw is None:
            return

        # Ensure layout is loaded
        if not os.path.exists(file_name):
            # Only meaningful to create a layout when a cell name is given
            if not cell_name:
                return
            (lv, cv, lv_idx, cv_idx, need_save) = self.libman_create_layout(file_name, cell_name)
        else:
            (lv, cv, lv_idx, cv_idx, need_save) = self.libman_open_layout(file_name)

        if lv is None or cv is None or cv_idx < 0:
            return

        _mw.select_view(lv_idx)

        # If no cell requested: just open file (zoom_fit will be applied later)
        if not cell_name:
            return

        # Select requested cell if it exists
        try:
            top_cell = cv.layout().cell_by_name(cell_name)
            if top_cell is not None:
                lv.select_cell(top_cell, cv_idx)
        except Exception:
            pass


    def libman_create_layout(self, file_name, cell_name):
        # Create a new layout in a new view.
        cv = _mw.create_layout(1)

        # Add cell.
        cv.layout().add_cell(cell_name)

        # Save file.
        (lv, cv_idx) = self.libman_get_view_and_index(cv)
        lv.save_as(cv_idx, file_name, False, pya.SaveLayoutOptions())

        (lv, cv, lv_idx, cv_idx) = self.libman_find_view_for_file(file_name)
        return (lv, cv, lv_idx, cv_idx, False)  # do not save


    def libman_open_layout(self, file_name):
        (lv, cv, lv_idx, cv_idx) = self.libman_find_view_for_file(file_name)
        if cv_idx == -1:
            # Load into existing view (same-view mode)
            _mw.load_layout(file_name, 1)
            (lv, cv, lv_idx, cv_idx) = self.libman_find_view_for_file(file_name)
        return (lv, cv, lv_idx, cv_idx, False)


    def libman_get_view_and_index(self, cell_view):
        num_views = _mw.views()
        for lv_idx in range(num_views):
            lv = _mw.view(lv_idx)
            cv_idx = self.libman_cellview_index(lv, cell_view)
            if cv_idx != -1:
                return (lv, cv_idx)
        return (None, -1)


    def libman_cellview_index(self, layout_view, cell_view):
        n = layout_view.cellviews()
        for i in range(n):
            cv = layout_view.cellview(i)
            if cv == cell_view:
                return i
        return -1


    def libman_find_view_for_file(self, file_name):
        num_views = _mw.views()
        for lv_idx in range(num_views):
            lv = _mw.view(lv_idx)
            (cv, cv_idx) = self.libman_find_cellview(lv, file_name)
            if cv_idx != -1:
                return (lv, cv, lv_idx, cv_idx)
        return (None, None, -1, -1)


    def libman_find_cellview(self, layout_view, file_name):
        n = layout_view.cellviews()
        for i in range(n):
            cv = layout_view.cellview(i)
            fn = cv.filename()
            if libman_cmp_paths(fn, file_name):
                return (cv, i)
        return (None, -1)


#==============================================================================
# Call
req = LibManRequest()
)";

    out << "req.open_cell(" << pyRaw(gdsPath) << ", " << pyRaw(cellName) << ")\n";
    out << "libman_fit_view_to_window()\n";

    out.flush();
    tf.close();

    return tf.fileName();
}

/*!*******************************************************************************************************************
 * \brief Starts external tool with a temporary script file and removes the script after the tool finishes.
 * \param tool        Tool executable (e.g. "klayout").
 * \param args        Arguments for the tool.
 * \param scriptPath  Temporary script file to remove after tool exit.
 **********************************************************************************************************************/
void MainWindow::startToolWithTempScript(const QString &tool, const QStringList &args, const QString &scriptPath)
{
    QProcess *p = new QProcess(this);

    connect(p,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [p, scriptPath](int, QProcess::ExitStatus)
            {
                if(!scriptPath.isEmpty() && QFileInfo(scriptPath).exists()) {
                    QFile::remove(scriptPath);
                }
                p->deleteLater();
            });

    connect(p,
            &QProcess::errorOccurred,
            this,
            [scriptPath](QProcess::ProcessError)
            {
                if(!scriptPath.isEmpty() && QFileInfo(scriptPath).exists()) {
                    QFile::remove(scriptPath);
                }
            });

    p->start(tool, args);
}

/*!*******************************************************************************************************************
 * \brief Opens selected view using configured external tool.
 *        For GDS it generates a temporary KLayout script and opens either the whole file or a specific cell.
 * \param item       Pointer to a clicked tree item.
 * \param            Column (unused).
 **********************************************************************************************************************/
void MainWindow::on_listViews_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if(!item) {
        return;
    }

    m_itemText = item->text(0);
    m_ui->txtViewSearch->setText(item->text(0));

    const int type = item->data(0, RoleType).toInt();

    QString viewName;   // "gds" / "oas" / "cdl" / ...
    QString viewPath;   // file path for gds/oas or normal view file
    QString cellName;   // for hierarchy nodes

    // ------------------------------------------------------------
    // Root items: "gds" / "oas" / "lstr"
    // ------------------------------------------------------------
    if(type == ItemViewGds && item->text(0) == "gds") {
        viewName = "gds";
        viewPath = item->data(0, RoleGdsPath).toString();
    }
    else if(type == ItemViewOas && (item->text(0) == "oas" || item->text(0) == "oasis")) {
        viewName = "oas";
        viewPath = item->data(0, RoleOasPath).toString();
    }
    else if(type == ItemViewLStream && (item->text(0) == "lstr" || item->text(0) == "lstream")) {
        viewName = "lstr";
        viewPath = item->data(0, RoleLStreamPath).toString();
    }
    else if(type == ItemViewCore) {
        viewName = item->text(0);
        viewPath = item->data(0, RoleCorePath).toString();
    }
    else if(type == ItemCell) {

        // find top view root ("gds" or "oas" or "lstr")
        QTreeWidgetItem *p = item->parent();
        while(p && p->parent()) {
            p = p->parent();
        }
        if(!p) {
            return;
        }

        const QString rootName = p->text(0);

        if(rootName == "gds") {
            viewName = "gds";
            viewPath = p->data(0, RoleGdsPath).toString();
        }
        else if(rootName == "oas" || rootName == "oasis") {
            viewName = "oas";
            viewPath = p->data(0, RoleOasPath).toString();
        }
        else if(rootName == "lstr" || rootName == "lstream") {
            viewName = "lstr";
            viewPath = p->data(0, RoleLStreamPath).toString();
        }
        else if(p->data(0, RoleType).toInt() == ItemViewCore) {
            viewName = rootName;
            viewPath = p->data(0, RoleCorePath).toString();
        }
        else {
            return;
        }

        cellName = item->data(0, RoleCellName).toString();

        if(viewPath.isEmpty() || cellName.isEmpty()) {
            return;
        }
    }
    // ------------------------------------------------------------
    // Normal views: cdl/spice/verilog/...
    // ------------------------------------------------------------
    else {
        viewName = item->text(0);
        if(viewName.isEmpty()) {
            return;
        }

        viewPath = getViewPath(getCurrentLibraryName(), getCurrentGroupName(), viewName);
    }

    if(viewPath.isEmpty() || !QFileInfo(viewPath).exists()) {
        error(QString("Failed to find view '%1'").arg(viewPath));
        return;
    }

    QString tool = getToolByView(viewName);
    if(tool.isEmpty()) {
        error(QString("Please specify tool first."));
        return;
    }

    // ------------------------------------------------------------
    // Layout handling via KLayout server: GDS + OAS (same behavior)
    // ------------------------------------------------------------
    if(viewName == "gds" || viewName == "oas" || viewName == "lstr" || isLayoutCoreViewName(viewName)) {

        QStringList bridgeErrors;
        const QString klayoutPath = layoutPathForKLayout(viewName, viewPath, &bridgeErrors);
        if(klayoutPath.isEmpty()) {
            if(!bridgeErrors.isEmpty()) {
                error(bridgeErrors.join(QLatin1Char('\n')), false);
            }
            else {
                error(QString("Failed to prepare layout file for KLayout: %1").arg(viewPath), false);
            }
            return;
        }

        // Root item: "gds", "oas", "lstr", or layout CORE view
        if(type == ItemViewGds || type == ItemViewOas || type == ItemViewLStream || type == ItemViewCore) {

            const QString groupName = getCurrentGroupName();
            const QString rootCell = preferredKLayoutCellForRoot(klayoutPath, groupName);
            const bool serverWasRunning = isKLayoutServerRunning();

            if(serverWasRunning) {
                if(!rootCell.isEmpty()) {
                    sendKLayoutSelectRequest(klayoutPath, rootCell);
                }
                return;
            }

            if(!ensureKLayoutServerRunning(tool)) {
                return;
            }

            sendKLayoutOpenRequest(klayoutPath, rootCell);
            return;
        }

        if(!ensureKLayoutServerRunning(tool)) {
            return;
        }

        sendKLayoutOpenRequest(klayoutPath, cellName);
        return;
    }

    // ------------------------------------------------------------
    // Non-layout views: original behavior
    // ------------------------------------------------------------
    QProcess::startDetached(tool, QStringList() << viewPath);
}

/*!*******************************************************************************************************************
 * \brief Filters events for the Views tree widget to suppress default expand/collapse behavior on double click.
 *
 * This event filter prevents QTreeWidget from automatically expanding or collapsing
 * layout view nodes (gds/oas/lstr/layout) and their hierarchy cell nodes on double click.
 * Double click is reserved exclusively for opening the corresponding view or cell in KLayout.
 *
 * \param obj      Object which received the event.
 * \param event    Event instance.
 *
 * \return true if the event was handled and should not be propagated further,
 *         false to allow default event processing.
 **********************************************************************************************************************/
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_ui->listViews->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        auto *mouseEvent = static_cast<QMouseEvent*>(event);
        QTreeWidgetItem *item = m_ui->listViews->itemAt(mouseEvent->pos());
        if (isLayoutViewTreeItem(item)) {
            on_listViews_itemDoubleClicked(item, m_ui->listViews->columnAt(mouseEvent->pos().x()));
            return true;
        }
    }

    const bool isDropTarget =
        obj == m_ui->treeLibs->viewport() ||
        obj == m_ui->listGroups->viewport();

    if(isDropTarget) {
        if(event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
            auto *dragEvent = static_cast<QDragEnterEvent*>(event);
            if(isSupportedViewDrop(dragEvent->mimeData())) {
                dragEvent->acceptProposedAction();
            }
            else {
                dragEvent->ignore();
            }
            return true;
        }

        if(event->type() == QEvent::Drop) {
            handleViewFileDrop(static_cast<QDropEvent*>(event));
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

/*!*******************************************************************************************************************
 * \brief Recursively hides/shows items based on filter, without populating any lazy data.
 *
 * Works only on items that already exist in the tree (already expanded/filled).
 * If any child matches, its parents become visible too.
 *
 * \param item     Item to process.
 * \param filter   Search text.
 * \return true if item or any of its existing children matches.
 **********************************************************************************************************************/
bool MainWindow::filterViewsTreeItemNoPopulate(QTreeWidgetItem *item, const QString &filter)
{
    if (!item) {
        return false;
    }

    const bool match = filter.isEmpty() || item->text(0).contains(filter, Qt::CaseInsensitive);

    bool childMatch = false;
    for (int i = 0; i < item->childCount(); ++i) {
        if (filterViewsTreeItemNoPopulate(item->child(i), filter)) {
            childMatch = true;
        }
    }

    const bool visible = filter.isEmpty() || match || childMatch;
    item->setHidden(!visible);

    // optional UX: auto-expand only already-existing branches that have matches
    if (!filter.isEmpty() && childMatch) {
        item->setExpanded(true);
    }

    return (match || childMatch);
}

/*!*******************************************************************************************************************
 * \brief Handles click on an item in the Views tree widget.
 *        Stores clicked item text and updates the View filter line edit.
 * \param item       Pointer to clicked tree item.
 * \param column     Column index where click happened.
 **********************************************************************************************************************/
void MainWindow::on_txtViewSearch_textEdited(const QString &filter)
{
    for (int i = 0; i < m_ui->listViews->topLevelItemCount(); ++i) {
        QTreeWidgetItem *top = m_ui->listViews->topLevelItem(i);
        filterViewsTreeItemNoPopulate(top, filter);
    }
}

/*!*******************************************************************************************************************
 * \brief Slot is to load groups (cells) located into the specified category file.
 * \param item       Pointer to tree item category.
 **********************************************************************************************************************/
void MainWindow::on_listCategories_itemClicked(QTreeWidgetItem *item)
{
    if(!item) {
        return;
    }

    m_itemText = item->text(0);

    QString libPath = getCurrentLibraryPath();
    QStringList groups = readLibraryCategories(libPath, item->text(0));

    m_ui->txtCatSearch->setText(item->text(0));

    m_ui->listGroups->clear();
    m_ui->listViews->clear();

    foreach(const QString &groupName, groups) {
        QListWidgetItem *groupItem = new QListWidgetItem;
        groupItem->setText(groupName);
        groupItem->setFlags(groupItem->flags() | Qt::ItemIsEditable);
        m_ui->listGroups->addItem(groupItem);
    }

    m_ui->listGroups->sortItems();
}

/*!*******************************************************************************************************************
 * \brief Slot is to execute document viewer with specified documentation.
 * \param item       Pointer to tree item document.
 **********************************************************************************************************************/
/*!*******************************************************************************************************************
 * \brief Slot is to execute document viewer with specified documentation.
 * \param item       Pointer to tree item document.
 **********************************************************************************************************************/
void MainWindow::on_listDocumentation_itemDoubleClicked(QTreeWidgetItem *item)
{
    if(!item) {
        return;
    }

    m_itemText = item->text(0);

    const QString docName = item->text(0);
    if(docName.isEmpty()) {
        return;
    }

    const QString docPath = item->data(0, RoleDocumentPath).toString();
    if(docPath.isEmpty() || !QFileInfo(docPath).exists()) {
        error(QString("Failed to find document '%1'.").arg(docPath), false);
        return;
    }

    const QString tool = getDocumentTool(docName);
    if(tool.isEmpty()) {
        error(QString("Please specify tool first."), false);
        return;
    }

    QProcess proc;
    QStringList args;
    args << docPath;

    proc.startDetached(tool, args);
}

/*!*******************************************************************************************************************
 * \brief Set LibMan state saved and updates the window titel accordingly.
 **********************************************************************************************************************/
void MainWindow::setStateSaved()
{
    m_isStateChanged = false;

    QString title = windowTitle();
    title.remove("*");
    setWindowTitle(title);
}

/*!*******************************************************************************************************************
 * \brief Set LibMan state changed and updates the window titel accordingly.
 ********************************************************************************************************************/
void MainWindow::setStateChanged()
{
    m_isStateChanged = true;

    QString title = windowTitle();
    if(!title.contains("*")) {
        setWindowTitle(title + "*");
    }
}

/*!*******************************************************************************************************************
 * \brief Saves the current project file or executes on_actionSave_As_triggered if no current file specified.
 **********************************************************************************************************************/
void MainWindow::on_actionSave_triggered()
{
    QString projFile = getCurrentProjectFile();
    if(projFile.isEmpty()) {
        on_actionSave_As_triggered();
    }
    else {
        saveProjectFile(projFile);
    }
}

/*!*******************************************************************************************************************
 * \brief Pops up dialog box where user can specify the file to save the library definition.
 **********************************************************************************************************************/
void MainWindow::on_actionSave_As_triggered()
{
    QString workDir = getCurrentWorkingDir();
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Lib File As.."),
                                                    workDir,
                                                    tr("KLayout Lib (*.lib);; All (*)"));
    if(!fileName.isEmpty()) {
        saveProjectFile(fileName);
    }
}

/*!*******************************************************************************************************************
 * \brief Closes LibMan.
 **********************************************************************************************************************/
void MainWindow::on_actionExit_triggered()
{
    close();
}

/*!*******************************************************************************************************************
 * \brief Slot is triggered to show or hide the categories tree widget.
 * \param state      Sate to show (true) or hide (false) the categories tree widget.
 **********************************************************************************************************************/
void MainWindow::on_actionShow_Categories_toggled(bool state)
{
    m_ui->groupCats->setVisible(state);
}

/*!*******************************************************************************************************************
 * \brief Slot is triggered to show or hide the documentation tree widget.
 * \param state      Sate to show (true) or hide (false) the documentation tree widget.
 **********************************************************************************************************************/
void MainWindow::on_actionShow_Documents_toggled(bool state)
{
    m_ui->groupDocs->setVisible(state);
}

/*!*******************************************************************************************************************
 * \brief Generates unexisting name based on the given information. Used when coping an item.
 * \param name       Name of the item (project, cell, view).
 * \param path       Path to the folder where the item (project, cell, view) exists.
 * \param suffix     Suffix of the output file (used for view name generation).
 **********************************************************************************************************************/
QString MainWindow::generateCopyName(const QString &name, const QString &path, const QString &suffix) const
{
    QString copyName = name + "_copy";

    QString copyPath = QDir::toNativeSeparators(path + "/" + copyName + suffix);
    if(!QFileInfo(copyPath).exists()) {
        return copyPath;
    }

    int index = 1;
    bool nameExists = true;
    while(nameExists) {
        copyName = name + "_copy" + QString("%1").arg(index);
        copyPath = QDir::toNativeSeparators(path + "/" + copyName + suffix);
        if(!QFileInfo(copyPath).exists()) {
            break;
        }

        index++;
    }

    return copyPath;
}

/*!*******************************************************************************************************************
 * \brief Slot is triggered when tree item is changed. Used for storing the project names after renaming.
 * \param item       Pointer to item which has been changed.
 * \param column     Column where item which has been changed.
 **********************************************************************************************************************/
void MainWindow::on_treeLibs_itemChanged(QTreeWidgetItem *item, int column)
{
    if(!item) {
        return;
    }

    if(column != 0) {
        return;
    }

    if(item->childCount()) {
        return;
    }

    if(m_itemText.isEmpty()) {
        return;
    }

    QString projName = item->text(column);
    QString key = getLibraryKeyPrefix() + m_itemText;
    QString libPath = m_properties->get<QString> (key);

    if(!QFileInfo(libPath).isDir()) {
        return;
    }

    QString targetPath = QDir::toNativeSeparators(QFileInfo(libPath).absolutePath() + "/" + projName);
    if(QFileInfo(targetPath).exists()) {
        error(QString("Directory '%1' aleardy exists.").arg(targetPath));
        item->setText(column, m_itemText);
        return;
    }

    QDir dir;
    dir.rename(libPath, targetPath);

    m_properties->remove(key);

    key = getLibraryKeyPrefix() + projName;
    m_properties->set(key, targetPath);

    setStateChanged();
}

/*!*******************************************************************************************************************
 * \brief Returns pointer to tree item.
 * \param name       Name of the item to look for.
 **********************************************************************************************************************/
QTreeWidgetItem* MainWindow::getTreeItemByName(const QString &name)
{
    for(int i = 0; i < m_ui->treeLibs->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_ui->treeLibs->topLevelItem(i);
        if(!item) {
            continue;
        }

        if(item->text(0) == name) {
            return item;
        }

        for(int j = 0; j < item->childCount(); ++j) {
            QTreeWidgetItem *child = item->child(j);
            if(!child) {
                continue;
            }

            if(child->text(0) == name) {
                return child;
            }
        }
    }

    return 0;
}

/*!*******************************************************************************************************************
 * \brief Searches for *.lib files in the specified directory and returns the first valid one.
 * \param dirName     Name of folder to search for lib file.
 **********************************************************************************************************************/
QString MainWindow::getProjectFileFromDir(const QString &dirName) const
{
    QString projFile;

    QStringList formats;
    formats << "*.lib";

    QDir projDir(dirName);
    projDir.setNameFilters(formats);

    QStringList fileList = projDir.entryList();
    foreach(QString projName, fileList) {
        QString projPath = QDir::toNativeSeparators(dirName + "/" + projName);
        if(!QFileInfo(projPath).isFile()) {
            continue;
        }

        LibFileParser parser;
        if(parser.parseFile(projPath)) {
            return projPath;
        }
    }

    return projFile;
}

/*!*******************************************************************************************************************
 * \brief Hides tree item during filtering.
 * \param tree       Tree pointer used for filtering.
 * \param filter     Text string used to filter tree items.
 **********************************************************************************************************************/
void MainWindow::hideTreeItem(QTreeWidget *tree, const QString &filter)
{
    if(!tree) {
        return;
    }

    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        if(!item) {
            continue;
        }

        if(filter.isEmpty()) {
            item->setHidden(false);
            continue;
        }

        item->setHidden(item->text(0).contains(filter) ? false : true);
    }
}

/*!*******************************************************************************************************************
 * \brief Hides list item during filtering.
 * \param list       List pointer used for filtering.
 * \param filter     Text string used to filter list items.
 **********************************************************************************************************************/
void MainWindow::hideListItem(QListWidget *list, const QString &filter)
{
    if(!list) {
        return;
    }

    for(int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        if(!item) {
            continue;
        }

        if(filter.isEmpty()) {
            item->setHidden(false);
            continue;
        }

        item->setHidden(item->text().contains(filter) ? false : true);
    }
}

/*!*******************************************************************************************************************
 * \brief Recursively hides or shows tree items based on filter string.
 * \param item       Tree item to process.
 * \param filter     Text string used to filter items.
 * \return true if item or any of its children matches the filter.
 **********************************************************************************************************************/
bool MainWindow::filterTreeItem(QTreeWidgetItem *item, const QString &filter)
{
    if (!item) {
        return false;
    }

    bool match = item->text(0).contains(filter, Qt::CaseInsensitive);

    bool childMatch = false;
    for (int i = 0; i < item->childCount(); ++i) {
        if (filterTreeItem(item->child(i), filter)) {
            childMatch = true;
        }
    }

    bool visible = filter.isEmpty() || match || childMatch;
    item->setHidden(!visible);

    if (childMatch && !filter.isEmpty()) {
        item->setExpanded(true);
    }

    return match || childMatch;
}

/*!*******************************************************************************************************************
 * \brief Filters projects (libraries) based on user input.
 * \param filter     Text string used to filter projects (libraries).
 **********************************************************************************************************************/
void MainWindow::on_txtLibSearch_textEdited(const QString &filter)
{
    hideTreeItem(m_ui->treeLibs, filter);
}

/*!*******************************************************************************************************************
 * \brief Filters categories based on user input.
 * \param filter     Text string used to filter categories.
 **********************************************************************************************************************/
void MainWindow::on_txtCatSearch_textEdited(const QString &filter)
{
    hideTreeItem(m_ui->listCategories, filter);
}

/*!*******************************************************************************************************************
 * \brief Filters groups (cells) based on user input.
 * \param filter     Text string used to filter groups (cells).
 **********************************************************************************************************************/
void MainWindow::on_txtCellSearch_textEdited(const QString &filter)
{
    hideListItem(m_ui->listGroups, filter);
}

/*!*******************************************************************************************************************
 * \brief Triggers execution of About dialog window.
 **********************************************************************************************************************/
void MainWindow::on_actionAbout_triggered()
{
    About(this).exec();
}

/*!*******************************************************************************************************************
 * \brief Triggers creation of new project (library).
 **********************************************************************************************************************/
void MainWindow::on_actionProject_triggered()
{
    addNewProject();
}

/*!*******************************************************************************************************************
 * \brief Triggers creation of new group (cell).
 **********************************************************************************************************************/
void MainWindow::on_actionGroup_triggered()
{
    addNewGroup();
}

/*!*******************************************************************************************************************
 * \brief Triggers execution of NewView dialog window.
 **********************************************************************************************************************/
void MainWindow::on_actionUnion_triggered()
{
    QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return;
    }

    NewView(this, libName, groupName).exec();
}

/*!*******************************************************************************************************************
 * \brief Triggers creation of new category.
 **********************************************************************************************************************/
void MainWindow::on_actionCategory_triggered()
{
    addNewCategory();
}

/*!*******************************************************************************************************************
 * \brief Creates new session and clears up all current project settings.
 ********************************************************************************************************************/
void MainWindow::on_actionSession_triggered()
{
   if(isStateChanged()) {
       if(askUserForAction("Would you like to save your current session changes?")) {
           on_actionSave_triggered();
       }
   }

   m_ui->treeLibs->clear();
   m_ui->listViews->clear();
   m_ui->listGroups->clear();
   m_ui->listCategories->clear();
   m_ui->listDocumentation->clear();

   m_currentProjFile = "";

   if(m_properties) {
       delete m_properties;
   }

   m_properties = new Properties;

   loadSettings();

   setWindowTitle(getLibManTitle());
   setupProjectFileWatcher(QString());
   setStateSaved();
}

/*!*******************************************************************************************************************
 * \brief Removes all the project file items from the menu
 ********************************************************************************************************************/
void MainWindow::on_actionClear_Recent_File_Stack_triggered()
{
    for (int i = 0; i < PROJ_MAX_COUNT; ++i) {
        m_recentProjects[i]->setText("");
        m_recentProjects[i]->setData("");
        m_recentProjects[i]->setVisible(false);
    }

    QSettings settings(getSettingsHeaderName());
    settings.beginGroup("RecentProjects");
    settings.setValue("RecentProjList", QStringList());
    settings.endGroup();
}

/*!*******************************************************************************************************************
 * \brief Sets up watcher for the current project file to detect external modifications.
 * \param projFile     Absolute path to the current project file.
 **********************************************************************************************************************/
void MainWindow::setupProjectFileWatcher(const QString &projFile)
{
    if(!m_projFileWatcher) {
        return;
    }

    QStringList files = m_projFileWatcher->files();
    if(!files.isEmpty()) {
        m_projFileWatcher->removePaths(files);
    }

    if(projFile.isEmpty()) {
        return;
    }

    QFileInfo fi(projFile);
    if(fi.exists() && fi.isFile()) {
        m_projFileWatcher->addPath(fi.absoluteFilePath());
    }
}

/*!*******************************************************************************************************************
 * \brief Clears currently loaded project-related UI data and properties.
 **********************************************************************************************************************/
void MainWindow::clearCurrentProjectData()
{
    m_ui->treeLibs->clear();
    m_ui->listViews->clear();
    m_ui->listGroups->clear();
    m_ui->listCategories->clear();
    m_ui->listDocumentation->clear();

    m_itemText.clear();
    m_currentProjFile.clear();

    if(m_properties) {
        delete m_properties;
    }

    m_properties = new Properties;

    loadSettings();

    setWindowTitle(getLibManTitle());
    setStateSaved();
}

/*!*******************************************************************************************************************
 * \brief Reloads currently opened project file from disk.
 **********************************************************************************************************************/
void MainWindow::reloadProjectFileFromDisk()
{
    const QString projFile = m_currentProjFile;
    if(projFile.isEmpty()) {
        return;
    }

    if(!QFileInfo(projFile).exists()) {
        error(QString("Project file '%1' does not exist anymore.").arg(projFile), false);
        return;
    }

    clearCurrentProjectData();
    loadProjectFile(projFile);
    setupProjectFileWatcher(projFile);

    info(QString("Project file '%1' was reloaded.").arg(projFile), false);
}

/*!*******************************************************************************************************************
 * \brief Handles external project file modification detected by QFileSystemWatcher.
 * \param path        Changed project file path.
 **********************************************************************************************************************/
void MainWindow::onProjectFileChanged(const QString &path)
{
    if(m_ignoreProjectFileChange) {
        setupProjectFileWatcher(path);
        return;
    }

    QFileInfo fi(path);

    // QFileSystemWatcher can drop the path after change/remove, so re-arm it.
    if(fi.exists() && fi.isFile()) {
        setupProjectFileWatcher(path);
    }

#ifdef LIBMAN_TESTING
    reloadProjectFileFromDisk();
    return;
#endif

    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Project File Changed");
    msgBox.setText(QString("The project file\n\n%1\n\nhas been modified outside LibMan.").arg(path));
    msgBox.setInformativeText("Would you like to reload it?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    const int ret = msgBox.exec();
    if(ret == QMessageBox::Yes) {
        reloadProjectFileFromDisk();
    }
    else {
        info(QString("Project file '%1' was modified externally. Reload skipped.").arg(path), false);
    }
}

/*!*******************************************************************************************************************
 * \brief Reloads currently opened project file from disk.
 *
 * This slot is triggered by the "Reload" action. It discards all current
 * in-memory project data (libraries, groups, views, categories, caches)
 * and loads the project file again from disk.
 *
 * If no project file is currently loaded or the file does not exist,
 * the operation is aborted and an error message is displayed.
 *
 * \note This function does not prompt for saving changes. It is assumed that
 *       the user intentionally wants to reload the project state from disk.
 **********************************************************************************************************************/
void MainWindow::on_actionReload_triggered()
{
    const QString projFile = getCurrentProjectFile();

    if(projFile.isEmpty()) {
        error("No project file loaded.", false);
        return;
    }

    if(!QFileInfo(projFile).exists()) {
        error(QString("Project file '%1' does not exist.").arg(projFile), false);
        return;
    }

    m_ignoreProjectFileChange = true;

    clearCurrentProjectData();
    loadProjectFile(projFile);
    setupProjectFileWatcher(projFile);

    m_ignoreProjectFileChange = false;

    info(QString("Project '%1' reloaded.").arg(projFile), false);
}

/*!*********************************************************************************************************************
 * \brief Renames the currently selected library.
 *
 * Prompts the user for a new library name using an input dialog. If confirmed,
 * all property keys associated with the selected library are updated to use
 * the new library name prefix.
 *
 * Only internal LibMan property mappings are updated. File system contents
 * (directories and view files) remain unchanged.
 *
 * After successful renaming, the library list is reloaded and the UI is updated.
 **********************************************************************************************************************/
void MainWindow::renameSelectedLibrary()
{
    QTreeWidgetItem *item = m_ui->treeLibs->currentItem();
    if(!item) {
        return;
    }

    const QString oldLibName = item->text(0).trimmed();
    if(oldLibName.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString newLibName = QInputDialog::getText(this,
                                                     tr("Rename Library"),
                                                     tr("Enter new library name:"),
                                                     QLineEdit::Normal,
                                                     oldLibName,
                                                     &ok).trimmed();

    if(!ok || newLibName.isEmpty() || newLibName == oldLibName) {
        return;
    }

    if(!updateProjectFileLibraryName(oldLibName, newLibName)) {
        error("Failed to update project file.", false);
        return;
    }

    const QString oldPrefix = getLibraryKeyPrefix() + oldLibName + "/";
    const QString newPrefix = getLibraryKeyPrefix() + newLibName + "/";

    QMap<QString, PropertyItem*> propItems = m_properties->getMap();
    QList<QString> keysToUpdate;

    for(auto it = propItems.begin(); it != propItems.end(); ++it) {
        const QString key = it.key();
        if(key.startsWith(oldPrefix)) {
            keysToUpdate << key;
        }
    }

    for(const QString &oldKey : keysToUpdate) {
        const QString value = m_properties->get<QString>(oldKey);

        QString newKey = oldKey;
        newKey.replace(oldPrefix, newPrefix);

        m_properties->set(newKey, value);
        m_properties->remove(oldKey);
    }

    loadLibraries();

    info(QString("Library '%1' renamed to '%2'.")
             .arg(oldLibName, newLibName), false);
}

/*!*********************************************************************************************************************
 * \brief Updates library name in the current project file.
 *
 * This function scans the loaded project file and replaces all occurrences
 * of the given old library name with the new one inside library definitions
 * (e.g. define("libName", "...")).
 *
 * The file is read entirely, modified in memory, and then rewritten.
 *
 * \param oldName    Original library name to be replaced.
 * \param newName    New library name to assign.
 *
 * \return True if the project file was successfully updated, false otherwise.
 **********************************************************************************************************************/
bool MainWindow::updateProjectFileLibraryName(const QString &oldName,
                                              const QString &newName)
{
    if(m_currentProjFile.isEmpty()) {
        return false;
    }

    QFile file(m_currentProjFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error("Failed to open project file for reading.", false);
        return false;
    }

    QStringList lines;
    QTextStream in(&file);
    while(!in.atEnd()) {
        lines << in.readLine();
    }
    file.close();

    bool changed = false;
    for(QString &line : lines) {
        if(line.contains(QString("\"%1\"").arg(oldName))) {
            line.replace(QString("\"%1\"").arg(oldName),
                         QString("\"%1\"").arg(newName));
            changed = true;
        }
    }

    if(!changed) {
        return false;
    }

    m_ignoreProjectFileChange = true;

    if(m_projFileWatcher) {
        m_projFileWatcher->removePath(m_currentProjFile);
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if(m_projFileWatcher) {
            m_projFileWatcher->addPath(m_currentProjFile);
        }
        m_ignoreProjectFileChange = false;
        error("Failed to open project file for writing.", false);
        return false;
    }

    QTextStream out(&file);
    for(const QString &line : lines) {
        out << line << "\n";
    }
    file.close();

    if(m_projFileWatcher) {
        m_projFileWatcher->addPath(m_currentProjFile);
    }

    m_ignoreProjectFileChange = false;
    return true;
}

/*!******************************************************************************************************************
 * \brief Retrieves supported view types grouped by tool.
 *
 * This function collects all tools defined in the "ToolList" property and extracts
 * their associated view types from corresponding "<ToolName>Views" properties.
 *
 * Each tool may define a comma-separated list of view suffixes (e.g. "gds,oas,lstr"),
 * which are parsed, normalized (trimmed and converted to lower case), and stored
 * as a list of supported views for that tool.
 *
 * The result is a map where:
 *   - key   = tool name
 *   - value = list of supported view suffixes for that tool
 *
 * Only tools with valid and non-empty view definitions are included.
 *
 * \return QMap mapping tool names to their supported view suffix lists.
 *******************************************************************************************************************/
QMap<QString, QStringList> MainWindow::getSupportedViewsByTool() const
{
    QMap<QString, QStringList> toolViewsMap;
    QStringList tools;

    if(m_properties->exists("ToolList")) {
        tools = m_properties->get<QString>("ToolList").split(",", QString::SkipEmptyParts);
    }

    for(QString toolName : tools) {
        toolName = toolName.trimmed();
        if(toolName.isEmpty()) {
            continue;
        }

        const QString key = toolName + QStringLiteral("Views");
        if(!m_properties->exists(key)) {
            continue;
        }

        QStringList views =
            m_properties->get<QString>(key).remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts);
        for(QString &view : views) {
            view = view.trimmed().toLower();
        }
        views.removeAll(QString());

        if(toolName == QStringLiteral("Layout") && !views.contains(QStringLiteral("layout"))) {
            views << QStringLiteral("layout");
        }

        if(views.isEmpty()) {
            continue;
        }

        toolViewsMap.insert(toolName, views);
    }

    return toolViewsMap;
}

/*!*********************************************************************************************************************
 * \brief Adds an existing cell view file to the currently selected library.
 *
 * Opens a file dialog filtered by supported view suffixes, copies the selected file
 * into a newly created cell directory inside the current library, and creates the
 * corresponding LibMan property entry so that the new cell/view becomes visible
 * in the UI.
 *
 * The cell name is derived from the selected file base name.
 **********************************************************************************************************************/
QStringList MainWindow::collectSupportedViewSuffixes() const
{
    QStringList allViews;
    const QMap<QString, QStringList> supportedViewsByTool = getSupportedViewsByTool();

    for(auto it = supportedViewsByTool.constBegin(); it != supportedViewsByTool.constEnd(); ++it) {
        for(const QString &view : it.value()) {
            if(!allViews.contains(view)) {
                allViews << view;
            }
        }
    }

    if(allViews.isEmpty()) {
        allViews = getValidViewList();
        if(!allViews.contains(QStringLiteral("lstr"))) {
            allViews << QStringLiteral("lstr");
        }
        if(!allViews.contains(QStringLiteral("layout"))) {
            allViews << QStringLiteral("layout");
        }
        if(!allViews.contains(QStringLiteral("schematic"))) {
            allViews << QStringLiteral("schematic");
        }
        if(!allViews.contains(QStringLiteral("symbol"))) {
            allViews << QStringLiteral("symbol");
        }
    }

    allViews.sort();
    return allViews;
}

/*!*********************************************************************************************************************
 * \brief Returns true if the drag-and-drop payload contains supported layout view files.
 **********************************************************************************************************************/
bool MainWindow::isSupportedViewDrop(const QMimeData *mimeData) const
{
    if(!mimeData || !mimeData->hasUrls()) {
        return false;
    }

    const QStringList supportedViews = collectSupportedViewSuffixes();
    if(supportedViews.isEmpty()) {
        return false;
    }

    for(const QUrl &url : mimeData->urls()) {
        const QString localPath = url.toLocalFile();
        if(localPath.isEmpty()) {
            continue;
        }

        const CoreViewIdentity coreIdentity = parseCoreViewIdentity(localPath);
        if(coreIdentity.valid && supportedViews.contains(coreIdentity.viewName)) {
            return true;
        }

        const QString viewName = QFileInfo(localPath).suffix().trimmed().toLower();
        if(supportedViews.contains(viewName)) {
            return true;
        }
    }

    return false;
}

/*!*********************************************************************************************************************
 * \brief Imports an existing layout file into the given library as a new cell/view pair.
 **********************************************************************************************************************/
bool MainWindow::importCellViewFile(const QString &libName, const QString &srcFilePath)
{
    if(libName.isEmpty() || srcFilePath.isEmpty()) {
        return false;
    }

    QFileInfo srcFi(srcFilePath);
    if(!srcFi.exists() || !srcFi.isFile()) {
        error(QString("Selected file does not exist: %1").arg(srcFilePath), false);
        return false;
    }

    QString libRoot = getLibraryPath(libName);
    if(libRoot.isEmpty()) {
        if(!m_currentProjFile.isEmpty()) {
            libRoot = QFileInfo(m_currentProjFile).absoluteDir().absolutePath();
            setLibraryRootDirectory(libName, libRoot);
        }
        else {
            error(QString("Failed to determine root path for library '%1'.").arg(libName), false);
            return false;
        }
    }

    const QStringList allViews = collectSupportedViewSuffixes();
    if(allViews.isEmpty()) {
        error("No supported view suffixes found.", false);
        return false;
    }

    QString groupName;
    QString viewName;
    if(!resolveCellViewFromPath(srcFilePath, &groupName, &viewName)) {
        error(QString("Failed to determine cell/view from '%1'.").arg(srcFilePath), false);
        return false;
    }

    if(groupName.isEmpty()) {
        error(QString("Failed to determine cell name from '%1'.").arg(srcFilePath), false);
        return false;
    }

    if(viewName.isEmpty()) {
        error(QString("Failed to determine view type from '%1'.").arg(srcFilePath), false);
        return false;
    }

    if(!allViews.contains(viewName)) {
        error(QString("View '%1' is not supported.").arg(viewName), false);
        return false;
    }

    const QString cellDirPath = QDir::toNativeSeparators(libRoot + "/" + groupName);
    QDir dir;
    if(!dir.mkpath(cellDirPath)) {
        error(QString("Failed to create cell directory '%1'.").arg(cellDirPath), false);
        return false;
    }

    const QString dstFilePath = QDir::toNativeSeparators(
        isCoreViewName(viewName)
            ? coreViewFilePath(cellDirPath, groupName, viewName)
            : cellDirPath + "/" + groupName + "." + viewName);
    if(QFileInfo(dstFilePath).exists()) {
        error(QString("Cell view already exists: %1").arg(dstFilePath), false);
        return false;
    }

    if(!QFile::copy(srcFilePath, dstFilePath)) {
        error(QString("Failed to copy '%1' to '%2'.").arg(srcFilePath, dstFilePath), false);
        return false;
    }

    const QString key = getLibraryKeyPrefix() + libName + "/" + groupName + "/" + viewName;
    m_properties->set(key, dstFilePath);

    loadGroups(libName);

    QList<QListWidgetItem*> items = m_ui->listGroups->findItems(groupName, Qt::MatchExactly);
    if(items.count()) {
        m_ui->listGroups->setCurrentItem(items.first());
        on_listGroups_itemClicked(items.first());
    }

    updateLibraryActionStates();

    if(!m_currentProjFile.isEmpty()) {
        saveProjectFile(m_currentProjFile);
    }
    else {
        setStateChanged();
    }

    info(QString("Added existing cell '%1' to library '%2'.")
             .arg(groupName)
             .arg(libName), false);

    return true;
}

/*!*********************************************************************************************************************
 * \brief Handles drag-and-drop of supported layout files onto library or cell panes.
 **********************************************************************************************************************/
void MainWindow::handleViewFileDrop(QDropEvent *event)
{
    if(!event || !event->mimeData()) {
        return;
    }

    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        error(tr("Select a library before dropping layout files."), false);
        event->ignore();
        return;
    }

    bool imported = false;
    for(const QUrl &url : event->mimeData()->urls()) {
        const QString localPath = url.toLocalFile();
        if(localPath.isEmpty()) {
            continue;
        }

        if(importCellViewFile(libName, localPath)) {
            imported = true;
        }
    }

    if(imported) {
        event->acceptProposedAction();
    }
    else {
        event->ignore();
    }
}

/*!*********************************************************************************************************************
 * \brief Opens a file dialog and imports an existing layout file into the selected library.
 **********************************************************************************************************************/
void MainWindow::addExistingCell()
{
    const QString libName = getCurrentLibraryName();
    if(libName.isEmpty()) {
        return;
    }

    const QStringList allViews = collectSupportedViewSuffixes();
    if(allViews.isEmpty()) {
        error("No supported views configured.", false);
        return;
    }

    QStringList filters;
    QStringList allPatterns;
    for(const QString &view : allViews) {
        allPatterns << QString("*.%1").arg(view);
    }
    filters << tr("Supported Views (%1)").arg(allPatterns.join(" "));

    for(const QString &view : allViews) {
        filters << QString("%1 (*.%2)").arg(view.toUpper(), view);
    }

    filters << tr("All Files (*.*)");

    const QString srcFilePath = QFileDialog::getOpenFileName(this,
                                                             tr("Add Existing Cell"),
                                                             QString(),
                                                             filters.join(";;"));
    if(srcFilePath.isEmpty()) {
        return;
    }

    importCellViewFile(libName, srcFilePath);
}
