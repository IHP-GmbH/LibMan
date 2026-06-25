#ifndef TOOLPICKERDIALOG_H
#define TOOLPICKERDIALOG_H

#include "view_tools.h"

#include <QDialog>

class QListWidget;

class ToolPickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ToolPickerDialog(const QVector<ViewToolEntry> &entries, QWidget *parent = nullptr);

    QString selectedPath() const;

private:
    QVector<ViewToolEntry> m_entries;
    QListWidget           *m_list = nullptr;
    int                    m_selectedIndex = -1;
};

#endif // TOOLPICKERDIALOG_H
