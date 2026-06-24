#include "importdialog.h"
#include "ui_importdialog.h"

#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

ImportDialog::ImportDialog(MainWindow *parent)
    : QDialog(parent)
    , m_ui(new Ui::ImportDialog)
    , m_mainWindow(parent)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Import"));

    populateFormats();
    populateLibraries();

    if (m_mainWindow) {
        const QString currentLibrary = m_mainWindow->getCurrentLibraryName();
        const int index = m_ui->comboLibrary->findText(currentLibrary);
        if (index >= 0) {
            m_ui->comboLibrary->setCurrentIndex(index);
        }
    }
}

ImportDialog::~ImportDialog()
{
    delete m_ui;
}

void ImportDialog::populateFormats()
{
    m_ui->comboFormat->clear();
    m_ui->comboFormat->addItem(CoreImportService::formatDisplayName(CoreImportService::Format::Gds),
                               static_cast<int>(CoreImportService::Format::Gds));
    m_ui->comboFormat->addItem(CoreImportService::formatDisplayName(CoreImportService::Format::Xschem),
                               static_cast<int>(CoreImportService::Format::Xschem));
    m_ui->comboFormat->addItem(CoreImportService::formatDisplayName(CoreImportService::Format::Qucs),
                               static_cast<int>(CoreImportService::Format::Qucs));
    m_ui->comboFormat->addItem(CoreImportService::formatDisplayName(CoreImportService::Format::Oas),
                               static_cast<int>(CoreImportService::Format::Oas));
}

void ImportDialog::populateLibraries()
{
    m_ui->comboLibrary->clear();
    if (!m_mainWindow) {
        return;
    }

    const QList<QPair<QString, QString>> entries = m_mainWindow->projectEntriesForEditor();
    for (const auto &entry : entries) {
        const QString libraryName = entry.first.trimmed();
        if (libraryName.isEmpty()) {
            continue;
        }
        if (m_ui->comboLibrary->findText(libraryName) < 0) {
            m_ui->comboLibrary->addItem(libraryName);
        }
    }
}

void ImportDialog::appendLogLine(const QString &line)
{
    m_ui->plainLog->appendPlainText(line);
}

CoreImportService::Format ImportDialog::currentFormat() const
{
    return static_cast<CoreImportService::Format>(m_ui->comboFormat->currentData().toInt());
}

QStringList ImportDialog::collectSourceFiles() const
{
    const QString path = m_ui->editSourcePath->text().trimmed();
    if (path.isEmpty()) {
        return {};
    }

    const CoreImportService::Format format = currentFormat();
    const QStringList filters = CoreImportService::sourceNameFilters(format);
    const QFileInfo pathInfo(path);

    if (m_ui->radioSingleFile->isChecked()) {
        if (!pathInfo.isFile()) {
            return {};
        }
        return {pathInfo.absoluteFilePath()};
    }

    if (!pathInfo.isDir()) {
        return {};
    }

    const QDir dir(pathInfo.absoluteFilePath());
    QStringList names = dir.entryList(filters, QDir::Files, QDir::Name);
    QStringList files;
    for (const QString &name : names) {
        files << dir.absoluteFilePath(name);
    }
    return files;
}

void ImportDialog::on_btnBrowse_clicked()
{
    const CoreImportService::Format format = currentFormat();
    const QStringList filters = CoreImportService::sourceNameFilters(format);
    const QString filterLine = filters.join(QStringLiteral(" "));

    if (m_ui->radioSingleFile->isChecked()) {
        const QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("Select source file"),
            m_ui->editSourcePath->text(),
            tr("Source files (%1);;All files (*)").arg(filterLine));
        if (!filePath.isEmpty()) {
            m_ui->editSourcePath->setText(QDir::toNativeSeparators(filePath));
        }
        return;
    }

    const QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select source folder"),
        m_ui->editSourcePath->text());
    if (!dirPath.isEmpty()) {
        m_ui->editSourcePath->setText(QDir::toNativeSeparators(dirPath));
    }
}

void ImportDialog::on_btnImport_clicked()
{
    const QString libraryName = m_ui->comboLibrary->currentText().trimmed();
    if (libraryName.isEmpty()) {
        QMessageBox::warning(this, tr("Import"), tr("Select a target library."));
        return;
    }

    const QStringList sourceFiles = collectSourceFiles();
    if (sourceFiles.isEmpty()) {
        QMessageBox::warning(this, tr("Import"), tr("Select a valid source file or folder."));
        return;
    }

    m_ui->btnImport->setEnabled(false);
    m_ui->plainLog->clear();
    appendLogLine(tr("Importing %1 file(s) into library '%2'...")
                      .arg(sourceFiles.size())
                      .arg(libraryName));

    CoreImportService service(m_mainWindow);
    const QVector<CoreImportService::ImportItemResult> results =
        service.importFiles(currentFormat(), libraryName, sourceFiles);

    int successCount = 0;
    for (const CoreImportService::ImportItemResult &result : results) {
        const QString sourceName = QFileInfo(result.sourcePath).fileName();
        if (result.success) {
            ++successCount;
            appendLogLine(tr("[OK] %1 — %2").arg(sourceName, result.message));
        } else {
            appendLogLine(tr("[FAIL] %1 — %2").arg(sourceName, result.message));
        }
    }

    appendLogLine(tr("Finished: %1 succeeded, %2 failed.")
                      .arg(successCount)
                      .arg(results.size() - successCount));
    m_ui->btnImport->setEnabled(true);
}

void ImportDialog::on_btnClose_clicked()
{
    reject();
}

void ImportDialog::on_radioSingleFile_toggled(bool checked)
{
    if (checked) {
        m_ui->labelPath->setText(tr("File"));
    }
}

void ImportDialog::on_radioFolder_toggled(bool checked)
{
    if (checked) {
        m_ui->labelPath->setText(tr("Folder"));
    }
}
