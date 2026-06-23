#include "projecteditor.h"
#include "ui_projecteditor.h"

#include "mainwindow.h"
#include "libfileparser.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QSet>
#include <QTimer>
#include <algorithm>
#include <QTableWidgetItem>

namespace {

bool rowHasContent(QTableWidget *table, int row)
{
    if (!table || row < 0 || row >= table->rowCount()) {
        return false;
    }

    const QTableWidgetItem *libItem = table->item(row, 0);
    const QTableWidgetItem *pathItem = table->item(row, 1);
    return (libItem && !libItem->text().trimmed().isEmpty())
        || (pathItem && !pathItem->text().trimmed().isEmpty());
}

} // namespace

ProjectEditor::ProjectEditor(MainWindow *parent)
    : QDialog(parent)
    , m_ui(new Ui::ProjectEditor)
    , m_mainWindow(parent)
{
    m_ui->setupUi(this);
    m_filePath = m_mainWindow ? m_mainWindow->getCurrentProjectFile() : QString();

    initTable();
    loadEntries();
    updateWindowTitle();

    connect(m_ui->actionSave, &QAction::triggered, this, &ProjectEditor::on_actionSave_triggered);
    connect(m_ui->actionSaveAs, &QAction::triggered, this, &ProjectEditor::on_actionSaveAs_triggered);
    connect(m_ui->actionClose, &QAction::triggered, this, &ProjectEditor::on_actionClose_triggered);
    connect(m_ui->tableEntries, &QTableWidget::customContextMenuRequested,
            this, &ProjectEditor::on_tableEntries_customContextMenuRequested);
    connect(m_ui->tableEntries, &QTableWidget::cellChanged,
            this, &ProjectEditor::on_tableEntries_cellChanged);
    connect(m_ui->tableEntries, &QTableWidget::cellDoubleClicked,
            this, &ProjectEditor::on_tableEntries_cellDoubleClicked);
}

ProjectEditor::~ProjectEditor()
{
    delete m_ui;
}

void ProjectEditor::initTable()
{
    QTableWidget *table = m_ui->tableEntries;
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({tr("Library"), tr("Path")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(true);
    table->setAlternatingRowColors(true);
}

void ProjectEditor::loadEntries()
{
    m_loadingTable = true;
    m_ui->tableEntries->setRowCount(0);

    if (m_mainWindow) {
        const QList<QPair<QString, QString>> rows = m_mainWindow->projectEntriesForEditor();
        for (const auto &row : rows) {
            const int index = m_ui->tableEntries->rowCount();
            m_ui->tableEntries->insertRow(index);
            m_ui->tableEntries->setItem(index, 0, new QTableWidgetItem(row.first));
            m_ui->tableEntries->setItem(index, 1, new QTableWidgetItem(row.second));
        }
    }

    appendEmptyRow();
    m_loadingTable = false;
    setDocumentModified(false);
}

void ProjectEditor::appendEmptyRow()
{
    const int row = m_ui->tableEntries->rowCount();
    m_ui->tableEntries->insertRow(row);
    m_ui->tableEntries->setItem(row, 0, new QTableWidgetItem());
    m_ui->tableEntries->setItem(row, 1, new QTableWidgetItem());
}

void ProjectEditor::ensureTrailingEmptyRow()
{
    if (m_ui->tableEntries->rowCount() == 0) {
        appendEmptyRow();
        return;
    }

    const int lastRow = m_ui->tableEntries->rowCount() - 1;
    if (rowHasContent(m_ui->tableEntries, lastRow)) {
        appendEmptyRow();
    }
}

QList<QPair<QString, QString>> ProjectEditor::collectEntries() const
{
    QList<QPair<QString, QString>> entries;
    QTableWidget *table = m_ui->tableEntries;

    for (int row = 0; row < table->rowCount(); ++row) {
        const QTableWidgetItem *libItem = table->item(row, 0);
        const QTableWidgetItem *pathItem = table->item(row, 1);
        const QString libName = libItem ? libItem->text().trimmed() : QString();
        const QString path = pathItem ? pathItem->text().trimmed() : QString();

        if (libName.isEmpty() && path.isEmpty()) {
            continue;
        }

        if (libName.isEmpty() || path.isEmpty()) {
            continue;
        }

        entries.append(qMakePair(libName, path));
    }

    return entries;
}

void ProjectEditor::setDocumentModified(bool modified)
{
    m_modified = modified;
    updateWindowTitle();
}

void ProjectEditor::updateWindowTitle()
{
    QString title = tr("Project Editor");
    if (!m_filePath.isEmpty()) {
        title += QStringLiteral(": %1").arg(m_filePath);
    }
    if (m_modified) {
        title += QStringLiteral(" *");
    }
    setWindowTitle(title);
}

QString ProjectEditor::projectFileFilter() const
{
    return tr("LibMan project (*.projects *.lib);;All files (*)");
}

bool ProjectEditor::saveToFile(const QString &filePath)
{
    if (!m_mainWindow || filePath.isEmpty()) {
        return false;
    }

    const QList<QPair<QString, QString>> entries = collectEntries();
    m_mainWindow->m_ignoreProjectFileChange = true;
    const bool saved = m_mainWindow->saveProjectEntriesToFile(filePath, entries);
    if (saved) {
        QTimer::singleShot(100, m_mainWindow, [mw = m_mainWindow]() {
            mw->m_ignoreProjectFileChange = false;
        });
    } else {
        m_mainWindow->m_ignoreProjectFileChange = false;
    }
    if (!saved) {
        return false;
    }

    m_filePath = QFileInfo(filePath).absoluteFilePath();
    m_mainWindow->loadProjectFile(m_filePath);
    setDocumentModified(false);
    return true;
}

bool ProjectEditor::confirmDiscardChanges()
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
        on_actionSave_triggered();
        return !m_modified;
    }
    if (answer == QMessageBox::Discard) {
        return true;
    }
    return false;
}

void ProjectEditor::on_actionSave_triggered()
{
    if (m_filePath.isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }

    saveToFile(m_filePath);
}

void ProjectEditor::on_actionSaveAs_triggered()
{
    QString initialDir;
    if (!m_filePath.isEmpty()) {
        initialDir = QFileInfo(m_filePath).absolutePath();
    }
    else if (m_mainWindow) {
        initialDir = m_mainWindow->getCurrentProjectDirectory();
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save project file"),
        initialDir,
        projectFileFilter());

    if (filePath.isEmpty()) {
        return;
    }

    saveToFile(filePath);
}

void ProjectEditor::on_actionClose_triggered()
{
    if (!confirmDiscardChanges()) {
        return;
    }
    reject();
}

void ProjectEditor::closeEvent(QCloseEvent *event)
{
    if (!confirmDiscardChanges()) {
        event->ignore();
        return;
    }
    event->accept();
}

void ProjectEditor::on_tableEntries_customContextMenuRequested(const QPoint &pos)
{
    QTableWidget *table = m_ui->tableEntries;
    const int row = table->rowAt(pos.y());

    QMenu menu(this);
    QAction *addAction = menu.addAction(tr("Add Library..."), this, &ProjectEditor::addLibraryRow);
    addAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+I")));

    QAction *deleteAction = menu.addAction(tr("Delete"), this, &ProjectEditor::deleteSelectedRows);
    deleteAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+D")));

    if (row >= 0) {
        menu.addSeparator();
        menu.addAction(tr("Browse path..."), [this, row]() { browsePathForRow(row); });
    }

    menu.exec(table->viewport()->mapToGlobal(pos));
}

void ProjectEditor::on_tableEntries_cellChanged(int row, int column)
{
    Q_UNUSED(column);

    if (m_loadingTable || row < 0) {
        return;
    }

    ensureTrailingEmptyRow();
    setDocumentModified(true);
}

void ProjectEditor::on_tableEntries_cellDoubleClicked(int row, int column)
{
    if (column == 1 && row >= 0) {
        browsePathForRow(row);
    }
}

void ProjectEditor::addLibraryRow()
{
    int row = m_ui->tableEntries->currentRow();
    if (row < 0) {
        row = m_ui->tableEntries->rowCount() - 1;
    }
    else {
        ++row;
    }

    m_ui->tableEntries->insertRow(row);
    m_ui->tableEntries->setItem(row, 0, new QTableWidgetItem());
    m_ui->tableEntries->setItem(row, 1, new QTableWidgetItem());
    m_ui->tableEntries->setCurrentCell(row, 0);
    m_ui->tableEntries->editItem(m_ui->tableEntries->item(row, 0));
    setDocumentModified(true);
    ensureTrailingEmptyRow();
}

void ProjectEditor::deleteSelectedRows()
{
    const QList<QTableWidgetItem *> selected = m_ui->tableEntries->selectedItems();
    if (selected.isEmpty()) {
        return;
    }

    QSet<int> rows;
    for (QTableWidgetItem *item : selected) {
        rows.insert(item->row());
    }

    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

    for (int row : sortedRows) {
        m_ui->tableEntries->removeRow(row);
    }

    if (m_ui->tableEntries->rowCount() == 0) {
        appendEmptyRow();
    }
    else {
        ensureTrailingEmptyRow();
    }

    setDocumentModified(true);
}

void ProjectEditor::browsePathForRow(int row)
{
    if (row < 0 || row >= m_ui->tableEntries->rowCount()) {
        return;
    }

    QString initialDir;
    if (!m_filePath.isEmpty()) {
        initialDir = QFileInfo(m_filePath).absolutePath();
    }
    else if (m_mainWindow) {
        initialDir = m_mainWindow->getCurrentProjectDirectory();
    }

    QTableWidgetItem *pathItem = m_ui->tableEntries->item(row, 1);
    const QString currentPath = pathItem ? pathItem->text().trimmed() : QString();
    if (!currentPath.isEmpty()) {
        const QFileInfo currentFi(currentPath);
        if (currentFi.isAbsolute() && currentFi.exists()) {
            initialDir = currentFi.absolutePath();
        }
        else if (!m_filePath.isEmpty()) {
            const QString resolved = QFileInfo(QFileInfo(m_filePath).absoluteDir(), currentPath).absolutePath();
            if (QFileInfo(resolved).exists()) {
                initialDir = resolved;
            }
        }
    }

    const QString chosen = QFileDialog::getOpenFileName(
        this,
        tr("Select library view file"),
        initialDir,
        tr("All files (*)"));

    if (chosen.isEmpty()) {
        return;
    }

    QTableWidgetItem *libItem = m_ui->tableEntries->item(row, 0);
    if (!libItem || libItem->text().trimmed().isEmpty()) {
        for (int r = row - 1; r >= 0; --r) {
            const QTableWidgetItem *prevLib = m_ui->tableEntries->item(r, 0);
            if (!prevLib || prevLib->text().trimmed().isEmpty()) {
                continue;
            }
            if (!libItem) {
                libItem = new QTableWidgetItem();
                m_ui->tableEntries->setItem(row, 0, libItem);
            }
            libItem->setText(prevLib->text());
            break;
        }
    }

    const QString storedPath =
        QDir::toNativeSeparators(QFileInfo(chosen).absoluteFilePath());

    if (!pathItem) {
        pathItem = new QTableWidgetItem();
        m_ui->tableEntries->setItem(row, 1, pathItem);
    }
    pathItem->setText(storedPath);
    ensureTrailingEmptyRow();
    setDocumentModified(true);
}
