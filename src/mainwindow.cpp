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
#include <QGuiApplication>

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
#include "projectmanager.h"

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
    m_currentCopyState(NONE)
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

    m_ui->treeLibs->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->listViews->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->listGroups->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->listCategories->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui->treeLibs, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showLibraryMenu(const QPoint &)));    
    connect(m_ui->listViews, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showViewMenu(const QPoint &)));
    connect(m_ui->listGroups, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showGroupMenu(const QPoint &)));
    connect(m_ui->listCategories, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCategoryMenu(const QPoint &)));

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
}

/*!*******************************************************************************************************************
 * \brief Destroys the main window.
 *********************************************************************************************************************/
MainWindow::~MainWindow()
{
    delete m_ui;
    delete m_properties;
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

    m_properties->set("PdfReader", pdfReader);

    settings.endGroup();
}

/*!*******************************************************************************************************************
 * \brief Returns list of the currently loaded projects (libraries).
 **********************************************************************************************************************/
QMap<QString, QString> MainWindow::getCurrentLibraries() const
{
    QMap<QString, QString> libMap;

    QMap<QString, PropertyItem*> propItems = m_properties->getMap();
    QMap<QString, PropertyItem*>::const_iterator it;
    for(it = propItems.constBegin(); it != propItems.constEnd(); ++it) {
        QString key = it.key();
        if(key.toUpper().startsWith(getLibraryKeyPrefix())) {
            QString libName = key;
            libName.remove(getLibraryKeyPrefix());
            QString libPath = m_properties->get<QString>(key);

            if(QFileInfo(libPath).exists()) {
                libMap[libName] = libPath;
            }
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
    QSettings settings(getSettingsHeaderName());
    settings.beginGroup("RecentProjects");

    QStringList files = settings.value("RecentProjList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
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

    int numRecentFiles = qMin(files.size(), (int) PROJ_MAX_COUNT);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString fileName = files[i];
        if(!QFileInfo(fileName).exists()) {
            continue;
        }

        QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
        m_recentProjects[i]->setText(text);
        m_recentProjects[i]->setData(files[i]);
        m_recentProjects[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < PROJ_MAX_COUNT; ++j) {
        m_recentProjects[j]->setVisible(false);
    }
}

/*!*******************************************************************************************************************
 * \brief Returns current working directory from where the project file has been loaded, otherwise user home path.
 **********************************************************************************************************************/
QString MainWindow::getCurrentWorkingDir() const
{
    if(m_ui->actionRecent1 && !m_ui->actionRecent1->text().isEmpty()) {
        QString workDir = QFileInfo(m_ui->actionRecent1->text()).absolutePath();
        if(QFileInfo(workDir).exists()) {
            return workDir;
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
    QString libPath;
    QString key = getLibraryKeyPrefix() + libName;
    if(m_properties->exists(key)) {
        libPath = m_properties->get<QString>(key);
    }

    return libPath;
}

/*!*******************************************************************************************************************
 * \brief Launches a dialog window to select a project file to load.
 **********************************************************************************************************************/
void MainWindow::on_actionOpen_triggered()
{
    QString workDir = getCurrentWorkingDir();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open file(s)"),
                                                    workDir,
                                                    tr("Project (*.projects);; All (*)"));

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
    views<<"gds"<<"cdl"<<"spice"<<"verilog";
    return views;
}

/*!*******************************************************************************************************************
 * \brief Returns specified by user tool for displaying views based on view name.
 * \param viewName     Name of the view to return an appropriate tool.
 **********************************************************************************************************************/
QString MainWindow::getToolByView(const QString &viewName) const
{
    QString tool;

    QStringList tools = m_properties->get<QString>("ToolList").split(",");
    foreach(const QString &name, tools) {
        if(m_properties->exists(name + "Views")) {
            QStringList views = m_properties->get<QString>(name + "Views").remove(" ").split(",");
            if(views.contains(viewName)) {
                if(m_properties->exists(name)) {
                    return(m_properties->get<QString>(name));
                }
            }
        }
    }

    return tool;
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
 * \brief Returns absolute path of the view based on given project/group/view (library/cell/view) information.
 * \param libName       Name of the project (library).
 * \param groupName     Name of the group (cell).
 * \param viewName      Name of the view.
 **********************************************************************************************************************/
QString MainWindow::getViewPath(const QString &libName, const QString &groupName, const QString &viewName) const
{
    QString viewPath = QDir::toNativeSeparators(libName + "/" + viewName + "/" + groupName + "." + viewName);
    return(viewPath);
}

/*!*******************************************************************************************************************
 * \brief Returns absolute path of the view for currently selected project/group (library/cell).
 * \param viewName     Name of the view.
 **********************************************************************************************************************/
QString MainWindow::getCurrentViewFilePath(const QString &viewName) const
{
    QString viewPath;

    QList<QTreeWidgetItem*> libItems = m_ui->treeLibs->selectedItems();
    if(!libItems.count()) {
        return viewPath;
    }

    QTreeWidgetItem *libItem = libItems.first();
    if(!libItem) {
        return viewPath;
    }

    QList<QListWidgetItem*> groupItems = m_ui->listGroups->selectedItems();
    if(!groupItems.count()) {
        return viewPath;
    }

    QListWidgetItem *groupItem = groupItems.first();
    if(!groupItem) {
        return viewPath;
    }

    QString key = getLibraryKeyPrefix() + libItem->text(0);
    QString libPath = m_properties->get<QString>(key);

    if(!QFileInfo(libPath).exists()) {
        return viewPath;
    }

    viewPath = QDir::toNativeSeparators(libPath + "/" + viewName + "/" + groupItem->text() + "." + viewName);

    if(QFileInfo(viewPath).exists()) {
        return viewPath;
    }

    return "";
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

    if(projId->text(0).isEmpty()) {
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
    QString viewName;

    QList<QListWidgetItem*> viewItems = m_ui->listViews->selectedItems();
    if(!viewItems.count()) {
        return viewName;
    }

    QListWidgetItem *viewId = viewItems.first();
    if(!viewId) {
        return viewName;
    }
    if(viewId->text().isEmpty()) {
        return viewName;
    }

    viewName = viewId->text();

    return viewName;
}

/*!*******************************************************************************************************************
 * \brief MainWindow::getCurrentGroupPath
 * \param viewName
 * \param toBeCreated
 * \return
 **********************************************************************************************************************/
QString MainWindow::getCurrentGroupPath(const QString &viewName, bool toBeCreated)
{
    QString groupPath;
    QString libPath = getCurrentLibraryPath();
    if(!QFileInfo(libPath).exists()) {
        return groupPath;
    }

    QString groupName = getCurrentGroupName();
    if(groupName.isEmpty()) {
        return groupPath;
    }

    groupPath = QDir::toNativeSeparators(libPath + "/" + viewName);
    if(!QFileInfo(groupPath).isDir()) {
        if(toBeCreated) {
            QDir dir;
            dir.mkpath(groupPath);
            if(!QFileInfo(groupPath).isDir()) {
                error(QString("Failed to create a group '%1'").arg(groupPath));
                return QString("");
            }
        }
        else {
            return QString("");
        }
    }

    return groupPath;
}

/*!*******************************************************************************************************************
 * \brief Returns absolute path of the currently selected project (library)
 **********************************************************************************************************************/
QString MainWindow::getCurrentLibraryPath() const
{
    QString libPath;

    QList<QTreeWidgetItem*> libItems = m_ui->treeLibs->selectedItems();
    if(!libItems.count()) {
        return libPath;
    }

    QTreeWidgetItem *libItem = libItems.first();
    if(!libItem) {
        return libPath;
    }

    QString key = getLibraryKeyPrefix() + libItem->text(0);
    libPath = m_properties->get<QString>(key);

    if(!QFileInfo(libPath).exists()) {
        return libPath;
    }

    return libPath;
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
 * \brief Returns project group (cell) list.
 * \param libPath      Path to the library, where group (cell) is located.
 **********************************************************************************************************************/
QStringList MainWindow::getCurrentGroups(const QString &libPath) const
{
    QStringList groups;

    if(!QFileInfo(libPath).isDir()) {
        return groups;
    }

    QStringList views = getValidViewList();

    QDir libDir = QDir(libPath);
    groups = libDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

    groups.removeDuplicates();

    return(groups);
}

/*!*******************************************************************************************************************
 * \brief Returns valid view paths for the carent project/ group (library/cell).
 * \param libPath      Path to the library, where group (cell) is located.
 * \param groupName    Name of group (cell) to read for its view(s).
 **********************************************************************************************************************/
QStringList MainWindow::getCurrentViews(const QString &libPath, const QString &groupName) const
{
    QStringList groupViews;

    QStringList views = getValidViewList();
    foreach(const QString viewName, views) {
        QString viewPath = QDir::toNativeSeparators(libPath + "/" + viewName + "/" + groupName + "." + viewName);
        if(QFileInfo(viewPath).exists()) {
            groupViews<<viewName;
        }
    }

    groupViews.removeDuplicates();

    return(groupViews);
}

/*!*******************************************************************************************************************
 * \brief Adds documents for the given project (library) into the document tree widget.
 * \param libPath      Path to the library, where documentation is located.
 **********************************************************************************************************************/
void MainWindow::loadDocuments(const QString &libPath)
{
    m_ui->listDocumentation->clear();

    QString docPath = QDir::toNativeSeparators(libPath + "/doc");
    if(!QFileInfo(docPath).isDir()) {
        return;
    }

    QStringList formats;
    formats<<"*.txt"<<"*.pdf"<<"*.doc"<<"*.celllist";

    QDir docDir(docPath);
    docDir.setNameFilters(formats);

    QStringList fileList = docDir.entryList();
    foreach(QString docName, fileList) {
        QTreeWidgetItem *docItem = new QTreeWidgetItem;
        docItem->setText(0, docName);
        if(QFileInfo(docName).completeSuffix().toLower() == "pdf") {
            docItem->setIcon(0, QIcon(":pdf"));
        }
        else {
            docItem->setIcon(0, QIcon(":new"));
        }

        m_ui->listDocumentation->addTopLevelItem(docItem);
    }

    m_ui->listDocumentation->sortColumn();
    m_ui->listDocumentation->resizeColumnToContents(0);
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
 * \brief Adds group (cell) for the given project (library) into the group list widget.
 * \param libPath      Path to the library, where group (cell) is located.
 **********************************************************************************************************************/
void MainWindow::loadGroups(const QString &libPath)
{
    m_ui->listGroups->clear();
    m_ui->listViews->clear();

    QStringList groups;

    QStringList views = getValidViewList();
    foreach(const QString viewName, views) {
        QString groupPath = QDir::toNativeSeparators(libPath + "/" + viewName);

        QDir groupDir(groupPath);
        groupDir.setNameFilters(QStringList()<<"*." + viewName);

        QStringList fileList = groupDir.entryList();
        foreach(QString groupName, fileList) {
            groupName.remove(QString(".") + viewName);
            groups<<groupName;
        }
    }

    groups.removeDuplicates();

    foreach(const QString &groupName, groups) {
        QListWidgetItem *groupItem = new QListWidgetItem;
        groupItem->setText(groupName);
        groupItem->setFlags(groupItem->flags() | Qt::ItemIsEditable);
        m_ui->listGroups->addItem(groupItem);
    }

    m_ui->listGroups->sortItems();
}

/*!*******************************************************************************************************************
 * \brief Adds views for the given group (cell) into the view list widget.
 * \param libPath      Path to the library, where group (cell) is located.
 * \param groupName    Name of group (cell) to load its view(s).
 **********************************************************************************************************************/
void MainWindow::loadViews(const QString &libPath, const QString &groupName)
{
    m_ui->listViews->clear();

    QStringList groupViews;

    QStringList views = getValidViewList();
    foreach(const QString viewName, views) {
        QString viewPath = QDir::toNativeSeparators(libPath + "/" + viewName + "/" + groupName + "." + viewName);
        if(QFileInfo(viewPath).exists()) {
            groupViews<<viewName;
        }
    }

    groupViews.removeDuplicates();

    foreach(const QString &viewName, groupViews) {
        QListWidgetItem *viewItem = new QListWidgetItem;
        viewItem->setText(viewName);
        m_ui->listViews->addItem(viewItem);
    }

    m_ui->listViews->sortItems();
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
 * \brief Slot to load all groups (cells) for selected project (library) into the group (cell) list widget.
 * \param item       Pointer to list item group (cell).
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

    m_ui->txtLibSearch->setText(item->text(0));

    QString key = getLibraryKeyPrefix() + item->text(0);
    QString libPath = m_properties->get<QString>(key);

    if(QFileInfo(libPath).exists()) {
        loadGroups(libPath);        
        loadDocuments(libPath);
        loadCategories(libPath);
    }

    m_ui->actionGroup->setEnabled(true);
    m_ui->actionUnion->setEnabled(false);
    m_ui->actionCategory->setEnabled(true);
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

    QList<QTreeWidgetItem*> libItems = m_ui->treeLibs->selectedItems();
    if(!libItems.count()) {
        return;
    }

    QTreeWidgetItem *libItem = libItems.first();
    if(!libItem) {
        return;
    }

    m_ui->txtCellSearch->setText(item->text());


    QString key = getLibraryKeyPrefix() + libItem->text(0);
    QString libPath = m_properties->get<QString>(key);

    if(QFileInfo(libPath).exists()) {
        loadViews(libPath, item->text());
    }

    m_ui->actionUnion->setEnabled(true);
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
 * \brief Slot to execute a tool based on the selected item (schematic, layout, cdl, spice, etc.).
 * \param item       Pointer to list item view.
 **********************************************************************************************************************/
void MainWindow::on_listViews_itemDoubleClicked(QListWidgetItem *item)
{
    if(!item) {
        return;
    }

    m_itemText = item->text();

    m_ui->txtViewSearch->setText(item->text());

    QString viewName = item->text();
    if(viewName.isEmpty()) {
        return;
    }

    QString viewPath = getCurrentViewFilePath(viewName);
    if(!QFileInfo(viewPath).exists()) {
        error(QString("Failed to find view '%1'").arg(viewPath));
        return;
    }

    qDebug()<<viewName;

    QString tool = getToolByView(viewName);
    if(tool.isEmpty()) {
        error(QString("Please specify tool first."));
        return;
    }

    QProcess proc;
    QStringList args;
    args<<viewPath;

    proc.startDetached(tool, args);
}

/*!*******************************************************************************************************************
 * \brief Sets the name of the selected item into the view filter line edit.
 * \param item       Pointer to list item view.
 **********************************************************************************************************************/
void MainWindow::on_listViews_itemClicked(QListWidgetItem *item)
{
    if(!item) {
        return;
    }

    m_itemText = item->text();

    m_ui->txtViewSearch->setText(item->text());
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
void MainWindow::on_listDocumentation_itemDoubleClicked(QTreeWidgetItem *item)
{
    if(!item) {
        return;
    }

    m_itemText = item->text(0);

    QString docName = item->text(0);
    if(docName.isEmpty()) {
        return;
    }

    QString docPath = getCurrentDocumentFilePath(docName);
    if(!QFileInfo(docPath).exists()) {
        error(QString("Failed to find document '%1'").arg(docPath));
        return;
    }


    QString tool = getDocumentTool(docName);
    if(tool.isEmpty()) {
        error(QString("Please specify tool first."));
        return;
    }

    QProcess proc;
    QStringList args;
    args<<docPath;

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
 * \brief Pops up dialog box where user can specify the file to save the project libraries.
 **********************************************************************************************************************/
void MainWindow::on_actionSave_As_triggered()
{
    QString workDir = getCurrentWorkingDir();
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Project File As.."),
                                                    workDir,
                                                    tr("Project (*.projects);; All (*)"));
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
 * \brief Searches for *.projects-files in the specified directory and returns the first found one.
 * \param dirName     Name of folder to search for project file.
 **********************************************************************************************************************/
QString MainWindow::getProjectFileFromDir(const QString &dirName) const
{
    QString projFile;

    QStringList formats;
    formats<<"*.projects";

    QDir projDir(dirName);
    projDir.setNameFilters(formats);

    QStringList fileList = projDir.entryList();
    foreach(QString projName, fileList) {
        QString projPath = QDir::toNativeSeparators(dirName + "/" + projName);
        if(QFileInfo(projPath).isFile()) {
            QFile file(projPath);
            if(!file.open(QFile::ReadOnly | QFile::Text)) {
                return projFile;
            }

            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().remove("^\\s+").remove("\\s+$");

                if(line.startsWith("#")) {
                    continue;
                }

                if(line.contains("PROJECT")) {
                    #if QT_VERSION >= 0x050000
                        QStringList words = line.split(" ", Qt::SkipEmptyParts);
                    #else
                        QStringList words = line.split(" ", QString::SkipEmptyParts);
                    #endif
                    if(words.count() == 3) {
                        projFile = projPath;
                        break;
                    }
                }
                else if(line.contains("GROUP")) {
                    #if QT_VERSION >= 0x050000
                        QStringList words = line.split(" ", Qt::SkipEmptyParts);
                    #else
                        QStringList words = line.split(" ", QString::SkipEmptyParts);
                    #endif
                    if(words.count() > 1) {
                        projFile = projPath;
                        break;
                    }
                }
            }

            file.close();

            if(!projFile.isEmpty()) {
                return projFile;
            }
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
 * \brief Filters views based on user input.
 * \param filter     Text string used to filter views.
 **********************************************************************************************************************/
void MainWindow::on_txtViewSearch_textEdited(const QString &filter)
{
    hideListItem(m_ui->listViews, filter);
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
