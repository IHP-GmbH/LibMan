#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "core_export_service.h"

#include <QDialog>

class MainWindow;

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(MainWindow *parent);
    ~ExportDialog() override;

private slots:
    void on_btnBrowseSource_clicked();
    void on_btnBrowseDestination_clicked();
    void on_btnExport_clicked();
    void on_btnClose_clicked();
    void on_radioSingleFile_toggled(bool checked);
    void on_radioFolder_toggled(bool checked);

private:
    void populateLibraries();
    void populateFormats();
    void loadSettings();
    void saveSettings() const;
    void appendLogLine(const QString &line);
    CoreExportService::Format currentFormat() const;
    QStringList collectSourceFiles() const;
    QString defaultBrowseDirectory() const;

    Ui::ExportDialog *m_ui = nullptr;
    MainWindow       *m_mainWindow = nullptr;
};

#endif // EXPORTDIALOG_H
