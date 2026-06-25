#ifndef VIEWTOOLSTABLEWIDGET_H
#define VIEWTOOLSTABLEWIDGET_H

#include "view_tools.h"

#include <QTableWidget>

class ViewToolsTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit ViewToolsTableWidget(QWidget *parent = nullptr);

    void setEntries(const QVector<ViewToolEntry> &entries);
    QVector<ViewToolEntry> entries() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void addToolRow();
    void deleteSelectedToolRow();
    void browsePathForCurrentRow();
    void onDefaultItemChanged(QTableWidgetItem *item);

private:
    void setupTable();
    void setDefaultRow(int row);
    int pathColumn() const { return 2; }

    bool m_updatingDefaults = false;
};

#endif // VIEWTOOLSTABLEWIDGET_H
