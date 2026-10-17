#include "keybindingsdialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

/*!*********************************************************************************************************************
 * \brief Builds the Key Bindings editor table from \p current shortcuts.
 **********************************************************************************************************************/
KeyBindingsDialog::KeyBindingsDialog(const QMap<QString, QKeySequence> &current, QWidget *parent)
    : QDialog(parent)
    , m_table(nullptr)
    , m_catalog(KeyBindings::catalog())
{
    setWindowTitle(tr("Key Bindings"));
    setModal(true);
    resize(640, 480);

    auto *root = new QVBoxLayout(this);
    auto *hint = new QLabel(
        tr("Click a shortcut and press the new key combination. "
           "Use Restore Defaults to reset all bindings."),
        this);
    hint->setWordWrap(true);
    root->addWidget(hint);

    m_table = new QTableWidget(this);
    m_table->setObjectName(QStringLiteral("keyBindingsTable"));
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({tr("Category"), tr("Action"), tr("Shortcut")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    root->addWidget(m_table, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *defaultsBtn = buttons->addButton(tr("Restore Defaults"), QDialogButtonBox::ResetRole);
    connect(defaultsBtn, &QPushButton::clicked, this, &KeyBindingsDialog::restoreDefaults);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);

    fillTable(current);
}

/*!*********************************************************************************************************************
 * \brief Returns the edited shortcut map (id → sequence).
 **********************************************************************************************************************/
QMap<QString, QKeySequence> KeyBindingsDialog::bindings() const
{
    QMap<QString, QKeySequence> map;
    for (int r = 0; r < m_table->rowCount() && r < m_catalog.size(); ++r) {
        auto *edit = qobject_cast<QKeySequenceEdit *>(m_table->cellWidget(r, 2));
        const QKeySequence seq = edit ? edit->keySequence() : QKeySequence();
        map.insert(m_catalog.at(r).id, seq);
    }
    return map;
}

/*!*********************************************************************************************************************
 * \brief Resets every row to the built-in default shortcut.
 **********************************************************************************************************************/
void KeyBindingsDialog::restoreDefaults()
{
    fillTable(KeyBindings::defaults());
}

/*!*********************************************************************************************************************
 * \brief Rebuilds table rows and QKeySequenceEdit widgets from \p bindings.
 **********************************************************************************************************************/
void KeyBindingsDialog::fillTable(const QMap<QString, QKeySequence> &bindings)
{
    m_table->setRowCount(m_catalog.size());
    for (int r = 0; r < m_catalog.size(); ++r) {
        const KeyBindingEntry &e = m_catalog.at(r);
        m_table->setItem(r, 0, new QTableWidgetItem(e.category));
        m_table->setItem(r, 1, new QTableWidgetItem(e.label));
        auto *edit = new QKeySequenceEdit(bindings.value(e.id, e.defaultShortcut), m_table);
        m_table->setCellWidget(r, 2, edit);
    }
}
