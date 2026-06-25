#include "viewtoolstablewidget.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>

ViewToolsTableWidget::ViewToolsTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    setupTable();
    connect(this, &QTableWidget::itemChanged, this, &ViewToolsTableWidget::onDefaultItemChanged);
    connect(this, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        if (column == pathColumn()) {
            setCurrentCell(row, column);
            browsePathForCurrentRow();
        }
    });
}

void ViewToolsTableWidget::setupTable()
{
    setColumnCount(3);
    setHorizontalHeaderLabels({tr("Default"), tr("Name"), tr("Path")});
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

void ViewToolsTableWidget::setEntries(const QVector<ViewToolEntry> &entries)
{
    m_updatingDefaults = true;
    setRowCount(0);

    int row = 0;
    for (const ViewToolEntry &entry : entries) {
        insertRow(row);

        auto *defaultItem = new QTableWidgetItem();
        defaultItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        defaultItem->setCheckState(entry.isDefault ? Qt::Checked : Qt::Unchecked);
        setItem(row, 0, defaultItem);

        auto *nameItem = new QTableWidgetItem(entry.name);
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        setItem(row, 1, nameItem);

        auto *pathItem = new QTableWidgetItem(entry.path);
        pathItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        pathItem->setToolTip(entry.path);
        setItem(row, 2, pathItem);

        ++row;
    }

    m_updatingDefaults = false;
}

QVector<ViewToolEntry> ViewToolsTableWidget::entries() const
{
    QVector<ViewToolEntry> result;
    result.reserve(rowCount());

    for (int row = 0; row < rowCount(); ++row) {
        const QTableWidgetItem *defaultItem = item(row, 0);
        const QTableWidgetItem *nameItem = item(row, 1);
        const QTableWidgetItem *pathItem = item(row, 2);
        if (!nameItem || !pathItem) {
            continue;
        }

        ViewToolEntry entry;
        entry.isDefault = defaultItem && defaultItem->checkState() == Qt::Checked;
        entry.name = nameItem->text().trimmed();
        entry.path = pathItem->text().trimmed();
        result.push_back(entry);
    }

    return result;
}

void ViewToolsTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *addAction = menu.addAction(tr("Add Tool"));
    QAction *deleteAction = menu.addAction(tr("Delete Tool"));
    deleteAction->setEnabled(currentRow() >= 0);

    QAction *chosen = menu.exec(event->globalPos());
    if (chosen == addAction) {
        addToolRow();
    } else if (chosen == deleteAction) {
        deleteSelectedToolRow();
    }
}

void ViewToolsTableWidget::addToolRow()
{
    const int row = rowCount();
    insertRow(row);

    auto *defaultItem = new QTableWidgetItem();
    defaultItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    defaultItem->setCheckState(row == 0 && rowCount() == 1 ? Qt::Checked : Qt::Unchecked);
    setItem(row, 0, defaultItem);

    auto *nameItem = new QTableWidgetItem(tr("New Tool"));
    nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    setItem(row, 1, nameItem);

    auto *pathItem = new QTableWidgetItem();
    pathItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    setItem(row, 2, pathItem);

    setCurrentCell(row, 1);
    editItem(nameItem);
}

void ViewToolsTableWidget::deleteSelectedToolRow()
{
    const int row = currentRow();
    if (row < 0) {
        return;
    }
    removeRow(row);
}

void ViewToolsTableWidget::browsePathForCurrentRow()
{
    const int row = currentRow();
    if (row < 0) {
        return;
    }

    QTableWidgetItem *pathItem = item(row, pathColumn());
    if (!pathItem) {
        return;
    }

    const QString startPath = pathItem->text().trimmed();
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Tool Executable"),
        startPath,
        tr("Executables (*.exe *.bat *.cmd *.sh);;All files (*)"));
    if (!filePath.isEmpty()) {
        pathItem->setText(QDir::toNativeSeparators(filePath));
        pathItem->setToolTip(filePath);
    }
}

void ViewToolsTableWidget::onDefaultItemChanged(QTableWidgetItem *item)
{
    if (m_updatingDefaults || !item || item->column() != 0) {
        return;
    }

    if (item->checkState() == Qt::Checked) {
        setDefaultRow(item->row());
    }
}

void ViewToolsTableWidget::setDefaultRow(int row)
{
    m_updatingDefaults = true;
    for (int i = 0; i < rowCount(); ++i) {
        QTableWidgetItem *defaultItem = this->item(i, 0);
        if (!defaultItem) {
            continue;
        }
        defaultItem->setCheckState(i == row ? Qt::Checked : Qt::Unchecked);
    }
    m_updatingDefaults = false;
}
