#ifndef KEYBINDINGSDIALOG_H
#define KEYBINDINGSDIALOG_H

#include "keybindings.h"

#include <QDialog>
#include <QMap>

class QTableWidget;

/*!*********************************************************************************************************************
 * \brief Dialog to view and edit LibMan menu key bindings, with Restore Defaults.
 **********************************************************************************************************************/
class KeyBindingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyBindingsDialog(const QMap<QString, QKeySequence> &current,
                               QWidget *parent = nullptr);

    QMap<QString, QKeySequence> bindings() const;

private slots:
    void restoreDefaults();

private:
    void fillTable(const QMap<QString, QKeySequence> &bindings);

    QTableWidget                   *m_table;
    QVector<KeyBindingEntry>        m_catalog;
};

#endif // KEYBINDINGSDIALOG_H
