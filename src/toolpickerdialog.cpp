#include "toolpickerdialog.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

ToolPickerDialog::ToolPickerDialog(const QVector<ViewToolEntry> &entries, QWidget *parent)
    : QDialog(parent)
    , m_entries(entries)
{
    setWindowTitle(tr("Choose Tool"));
    resize(520, 280);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Select the tool to open this view:"), this));

    m_list = new QListWidget(this);
    for (const ViewToolEntry &entry : m_entries) {
        const QString displayName = entry.name.trimmed().isEmpty() ? QFileInfo(entry.path).fileName() : entry.name;
        auto *item = new QListWidgetItem(displayName, m_list);
        item->setToolTip(entry.path);
        item->setData(Qt::UserRole, entry.path);
    }
    if (m_list->count() > 0) {
        m_list->setCurrentRow(0);
    }
    layout->addWidget(m_list);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        m_selectedIndex = m_list ? m_list->currentRow() : -1;
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) {
        m_selectedIndex = m_list ? m_list->currentRow() : -1;
        accept();
    });
    layout->addWidget(buttons);
}

QString ToolPickerDialog::selectedPath() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_entries.size()) {
        return m_entries.at(m_selectedIndex).path;
    }
    return {};
}
