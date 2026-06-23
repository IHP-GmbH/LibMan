#ifndef PROJECTEDITOR_H
#define PROJECTEDITOR_H

#include <QDialog>
#include <QPair>
#include <QString>
#include <QList>

class MainWindow;

namespace Ui {
class ProjectEditor;
}

class ProjectEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectEditor(MainWindow *parent);
    ~ProjectEditor() override;

private slots:
    void                                on_actionSave_triggered();
    void                                on_actionSaveAs_triggered();
    void                                on_actionClose_triggered();
    void                                on_tableEntries_customContextMenuRequested(const QPoint &pos);
    void                                on_tableEntries_cellChanged(int row, int column);
    void                                on_tableEntries_cellDoubleClicked(int row, int column);

    void                                addLibraryRow();
    void                                deleteSelectedRows();
    void                                browsePathForRow(int row);

private:
    void                                initTable();
    void                                loadEntries();
    void                                appendEmptyRow();
    void                                ensureTrailingEmptyRow();
    QList<QPair<QString, QString>>      collectEntries() const;
    void                                setDocumentModified(bool modified);
    void                                updateWindowTitle();
    bool                                confirmDiscardChanges();
    bool                                saveToFile(const QString &filePath);
    QString                             projectFileFilter() const;

    void                                closeEvent(QCloseEvent *event) override;

private:
    Ui::ProjectEditor                    *m_ui = nullptr;
    MainWindow                           *m_mainWindow = nullptr;
    QString                               m_filePath;
    bool                                  m_modified = false;
    bool                                  m_loadingTable = false;
};

#endif // PROJECTEDITOR_H
