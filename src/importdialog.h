#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include "core_import_service.h"

#include <QDialog>

class MainWindow;

namespace Ui {
class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(MainWindow *parent);
    ~ImportDialog() override;

private slots:
    void on_btnBrowse_clicked();
    void on_btnImport_clicked();
    void on_btnClose_clicked();
    void on_radioSingleFile_toggled(bool checked);
    void on_radioFolder_toggled(bool checked);

private:
    void populateLibraries();
    void populateFormats();
    void appendLogLine(const QString &line);
    CoreImportService::Format currentFormat() const;
    QStringList collectSourceFiles() const;

    Ui::ImportDialog *m_ui = nullptr;
    MainWindow       *m_mainWindow = nullptr;
};

#endif // IMPORTDIALOG_H
