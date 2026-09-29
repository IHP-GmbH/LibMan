#include "projecteditor.h"
#include "ui_projecteditor.h"

#include "mainwindow.h"
#include "libfileparser.h"
#include "libdefine_utils.h"

#include "extension/variantfactory.h"
#include "extension/variantmanager.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"

#include <QBrush>
#include <QDir>
#include <QFontMetrics>
#include <QSizePolicy>
#include <QStyle>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QTreeWidget>

ProjectEditor::ProjectEditor(MainWindow *parent)
    : QWidget(parent)
    , m_ui(new Ui::ProjectEditor)
    , m_mainWindow(parent)
{
    m_ui->setupUi(this);
    m_ui->labelTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_ui->labelHelp->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_ui->labelHelp->setWordWrap(false);
    m_ui->verticalLayout->setStretch(m_ui->verticalLayout->indexOf(m_ui->editorSplitter), 1);
    m_ui->editorSplitter->setStretchFactor(0, 1);
    m_ui->editorSplitter->setStretchFactor(1, 2);
    m_ui->editorSplitter->setSizes({180, 360});
    for (QPushButton *button : {m_ui->btnAddLibrary, m_ui->btnRemoveLibrary, m_ui->btnAddPath,
                                m_ui->btnRemovePath, m_ui->btnSave, m_ui->btnClose}) {
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    m_browser = new QtTreePropertyBrowser(m_ui->browserHost);
    m_browser->setResizeMode(QtTreePropertyBrowser::Interactive);
    m_browser->setPropertiesWithoutValueMarked(true);
    m_browser->setHeaderVisible(true);
    if (QTreeWidget *tree = m_browser->findChild<QTreeWidget *>()) {
        tree->setHeaderLabels({tr("Name"), tr("Path")});
        tree->header()->setStretchLastSection(true);
    }
    m_ui->browserLayout->addWidget(m_browser);

    m_manager = new VariantManager(m_browser);
    QtVariantEditorFactory *factory = new VariantFactory(m_browser);
    m_browser->setFactoryForManager(static_cast<QtVariantPropertyManager *>(m_manager), factory);

    m_paths = m_manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Paths"));
    m_browser->addProperty(m_paths);

    connect(m_ui->libraryTree, &QTreeWidget::itemSelectionChanged, this, &ProjectEditor::onLibrarySelectionChanged);
    connect(m_manager, &VariantManager::valueChanged, this, &ProjectEditor::onPathValueChanged);
}

ProjectEditor::~ProjectEditor()
{
    delete m_ui;
}

void ProjectEditor::reloadFromDisk()
{
    m_filePath = m_mainWindow ? m_mainWindow->getCurrentProjectFile() : QString();

    QVector<LibraryEntry> libraries;
    if (!m_filePath.isEmpty()) {
        LibFileParser parser;
        if (parser.parseFile(m_filePath)) {
            for (const LibDefinition &def : parser.data().definitions) {
                const QString libName = def.name.trimmed();
                const QString path = def.path.trimmed();
                if (libName.isEmpty() || path.isEmpty()) {
                    continue;
                }

                int index = -1;
                for (int i = 0; i < libraries.size(); ++i) {
                    if (libraries.at(i).name == libName) {
                        index = i;
                        break;
                    }
                }
                if (index < 0) {
                    LibraryEntry entry;
                    entry.name = libName;
                    libraries.append(entry);
                    index = libraries.size() - 1;
                }
                libraries[index].paths.append(path);
            }
        }
    }

    setLibraries(libraries);
    m_modified = false;
    if (!m_filePath.isEmpty()) {
        m_ui->labelTitle->setText(tr("Project Editor — %1").arg(QFileInfo(m_filePath).fileName()));
    }
    else {
        m_ui->labelTitle->setText(tr("Project Editor"));
    }
}

bool ProjectEditor::confirmHide()
{
    if (!m_modified) {
        return true;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Project Editor"),
        tr("The project has been modified.\nDo you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (answer == QMessageBox::Save) {
        on_btnSave_clicked();
        return !m_modified;
    }
    if (answer == QMessageBox::Discard) {
        m_modified = false;
        return true;
    }
    return false;
}

void ProjectEditor::setLibraries(const QVector<LibraryEntry> &libraries)
{
    m_loading = true;
    m_currentLibrary = -1;
    m_libraries = libraries;
    m_ui->libraryTree->clear();

    for (int i = 0; i < m_libraries.size(); ++i) {
        auto *item = new QTreeWidgetItem(m_ui->libraryTree);
        item->setData(0, Qt::UserRole, i);
        refreshLibraryLabel(i);
    }

    m_loading = false;
    if (m_ui->libraryTree->topLevelItemCount() > 0) {
        m_ui->libraryTree->setCurrentItem(m_ui->libraryTree->topLevelItem(0));
    }
    else {
        showLibrary(-1);
    }
}

void ProjectEditor::flushCurrentPaths()
{
    if (m_currentLibrary < 0 || m_currentLibrary >= m_libraries.size() || !m_paths) {
        return;
    }

    QStringList paths;
    const QList<QtProperty *> properties = m_paths->subProperties();
    for (QtProperty *property : properties) {
        const QString path = m_manager->value(property).toString().trimmed();
        if (!path.isEmpty()) {
            paths.append(path);
        }
    }
    m_libraries[m_currentLibrary].paths = paths;
}

void ProjectEditor::showLibrary(int index)
{
    m_loading = true;
    const QList<QtProperty *> existing = m_paths->subProperties();
    for (QtProperty *property : existing) {
        m_paths->removeSubProperty(property);
        delete property;
    }

    m_currentLibrary = index;
    if (index >= 0 && index < m_libraries.size()) {
        m_paths->setPropertyName(m_libraries.at(index).name);
        for (const QString &path : m_libraries.at(index).paths) {
            addPathProperty(path);
        }
    }
    else {
        m_paths->setPropertyName(tr("Paths"));
    }

    const QList<QtBrowserItem *> top = m_browser->topLevelItems();
    for (QtBrowserItem *item : top) {
        m_browser->setExpanded(item, true);
    }
    fitNameColumn();
    m_loading = false;
}

void ProjectEditor::fitNameColumn()
{
    QTreeWidget *tree = m_browser ? m_browser->findChild<QTreeWidget *>() : nullptr;
    if (!tree) {
        return;
    }

    QFontMetrics metrics(tree->font());
    int textWidth = metrics.horizontalAdvance(tr("Paths"));
    for (const LibraryEntry &library : m_libraries) {
        textWidth = qMax(textWidth, metrics.horizontalAdvance(library.name));
    }

    const int extra = tree->indentation()
        + tree->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) * 2
        + 8;
    if (tree->header()->count() > 0) {
        m_browser->setSplitterPosition(textWidth + extra);
    }
}

void ProjectEditor::refreshLibraryLabel(int index)
{
    if (index < 0 || index >= m_libraries.size()) {
        return;
    }

    int missing = 0;
    for (const QString &path : m_libraries.at(index).paths) {
        if (!pathExists(path)) {
            ++missing;
        }
    }

    QString label = m_libraries.at(index).name;
    if (missing > 0) {
        label += tr(" (%1 missing)").arg(missing);
    }

    for (int row = 0; row < m_ui->libraryTree->topLevelItemCount(); ++row) {
        QTreeWidgetItem *item = m_ui->libraryTree->topLevelItem(row);
        if (item && item->data(0, Qt::UserRole).toInt() == index) {
            item->setText(0, label);
            item->setForeground(0, missing > 0 ? QBrush(Qt::red) : QBrush());
            break;
        }
    }
}

QtVariantProperty *ProjectEditor::addPathProperty(const QString &path)
{
    const bool wildcard = libdefine::isWildcardDefinePath(path);
    QtVariantProperty *item = m_manager->addProperty(VariantManager::filePathTypeId(), pathLabel(path));
    item->setWhatsThis(wildcard ? QStringLiteral("folder") : QStringLiteral("file"));
    item->setAttribute(QStringLiteral("filter"), viewFileFilter());
    item->setValue(QDir::toNativeSeparators(path));
    item->setToolTip(pathExists(path) ? tr("Path exists") : tr("Path is in the project file but was not loaded"));
    item->setStatusTip(wildcard ? QStringLiteral("wildcard") : QString());
    m_paths->addSubProperty(item);
    return item;
}

QString ProjectEditor::pathLabel(const QString &path) const
{
    QString label = QFileInfo(path).fileName();
    if (label.isEmpty()) {
        label = tr("Path");
    }
    if (!pathExists(path)) {
        label += tr(" (missing)");
    }
    return label;
}

bool ProjectEditor::pathExists(const QString &path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    if (libdefine::isWildcardDefinePath(trimmed)) {
        if (m_filePath.isEmpty()) {
            return false;
        }
        return !libdefine::wildcardScanRoot(QFileInfo(m_filePath).absolutePath(), trimmed).isEmpty();
    }

    return QFileInfo(trimmed).exists();
}

void ProjectEditor::refreshPathLabel(QtProperty *property)
{
    if (!property || m_manager->propertyType(property) != VariantManager::filePathTypeId()) {
        return;
    }

    const QString path = m_manager->value(property).toString();
    const QString label = pathLabel(path);
    if (property->propertyName() != label) {
        property->setPropertyName(label);
    }
    property->setToolTip(pathExists(path)
                             ? tr("Path exists")
                             : tr("Path is in the project file but was not loaded"));
}

QList<QPair<QString, QString>> ProjectEditor::collectEntries() const
{
    QList<QPair<QString, QString>> entries;
    for (const LibraryEntry &library : m_libraries) {
        for (const QString &path : library.paths) {
            if (library.name.trimmed().isEmpty() || path.trimmed().isEmpty()) {
                continue;
            }
            entries.append(qMakePair(library.name.trimmed(), QDir::toNativeSeparators(path.trimmed())));
        }
    }
    return entries;
}

QString ProjectEditor::viewFileFilter() const
{
    return tr("Library files (*.room *.sch *.sym *.gds *.oas *.lstr);;All files (*)");
}

bool ProjectEditor::saveToFile(const QString &filePath)
{
    if (!m_mainWindow || filePath.isEmpty()) {
        return false;
    }

    flushCurrentPaths();
    const QList<QPair<QString, QString>> entries = collectEntries();
    m_mainWindow->m_ignoreProjectFileChange = true;
    const bool saved = m_mainWindow->saveProjectEntriesToFile(filePath, entries, true);
    if (saved) {
        QTimer::singleShot(100, m_mainWindow, [mw = m_mainWindow]() {
            mw->m_ignoreProjectFileChange = false;
        });
    }
    else {
        m_mainWindow->m_ignoreProjectFileChange = false;
        return false;
    }

    m_filePath = QFileInfo(filePath).absoluteFilePath();
    m_mainWindow->loadProjectFile(m_filePath);
    m_modified = false;
    reloadFromDisk();
    return true;
}

void ProjectEditor::on_btnAddLibrary_clicked()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this,
                                                tr("Add Library"),
                                                tr("Library name:"),
                                                QLineEdit::Normal,
                                                QString(),
                                                &ok).trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    flushCurrentPaths();
    LibraryEntry entry;
    entry.name = name;
    m_libraries.append(entry);
    const int index = m_libraries.size() - 1;

    auto *item = new QTreeWidgetItem(m_ui->libraryTree);
    item->setData(0, Qt::UserRole, index);
    refreshLibraryLabel(index);
    m_ui->libraryTree->setCurrentItem(item);
    m_modified = true;
}

void ProjectEditor::on_btnRemoveLibrary_clicked()
{
    QTreeWidgetItem *item = m_ui->libraryTree->currentItem();
    if (!item) {
        return;
    }

    const int index = item->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= m_libraries.size()) {
        return;
    }

    m_currentLibrary = -1;
    m_libraries.removeAt(index);
    setLibraries(m_libraries);
    m_modified = true;
}

void ProjectEditor::on_btnAddPath_clicked()
{
    if (m_currentLibrary < 0) {
        QMessageBox::information(this, tr("Project Editor"), tr("Select a library first."));
        return;
    }

    addPathProperty(QString());
    const QList<QtBrowserItem *> top = m_browser->topLevelItems();
    for (QtBrowserItem *item : top) {
        m_browser->setExpanded(item, true);
    }
    m_modified = true;
}

void ProjectEditor::on_btnRemovePath_clicked()
{
    QtBrowserItem *current = m_browser->currentItem();
    if (!current) {
        return;
    }

    QtProperty *property = current->property();
    if (!property || property == m_paths) {
        return;
    }
    if (m_manager->propertyType(property) != VariantManager::filePathTypeId()) {
        return;
    }

    m_paths->removeSubProperty(property);
    delete property;
    flushCurrentPaths();
    refreshLibraryLabel(m_currentLibrary);
    m_modified = true;
}

void ProjectEditor::on_btnClose_clicked()
{
    if (!confirmHide()) {
        return;
    }
    hide();
}

void ProjectEditor::on_btnSave_clicked()
{
    if (m_filePath.isEmpty()) {
        const QString initialDir = m_mainWindow ? m_mainWindow->getCurrentProjectDirectory() : QString();
        const QString filePath = QFileDialog::getSaveFileName(
            this,
            tr("Save project file"),
            initialDir,
            tr("LibMan project (*.projects *.lib);;All files (*)"));
        if (filePath.isEmpty()) {
            return;
        }
        saveToFile(filePath);
        return;
    }

    saveToFile(m_filePath);
}

void ProjectEditor::onLibrarySelectionChanged()
{
    if (m_loading) {
        return;
    }

    flushCurrentPaths();
    if (m_currentLibrary >= 0) {
        refreshLibraryLabel(m_currentLibrary);
    }

    QTreeWidgetItem *item = m_ui->libraryTree->currentItem();
    if (!item) {
        showLibrary(-1);
        return;
    }

    showLibrary(item->data(0, Qt::UserRole).toInt());
}

void ProjectEditor::onPathValueChanged(QtProperty *property, const QVariant &value)
{
    Q_UNUSED(value);
    if (m_loading) {
        return;
    }

    refreshPathLabel(property);
    flushCurrentPaths();
    refreshLibraryLabel(m_currentLibrary);
    m_modified = true;
}
