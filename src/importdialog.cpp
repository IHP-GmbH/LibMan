#include "importdialog.h"
#include "ui_importdialog.h"

#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QEventLoop>
#include <QSettings>
#include <QSet>

namespace {

constexpr auto kSettingsGroup = "Import";

} // namespace

ImportDialog::ImportDialog(MainWindow *parent)
    : QDialog(parent)
    , m_ui(new Ui::ImportDialog)
    , m_mainWindow(parent)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Import"));

    populateFormats();
    populateLibraries();
    loadSettings();
}

ImportDialog::~ImportDialog()
{
    saveSettings();
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
    m_ui->comboLibrary->addItem(QString());
    if (!m_mainWindow) {
        return;
    }

    QSet<QString> seen;
    for (const QString &libraryName : m_mainWindow->registeredLibraryNames()) {
        if (libraryName.isEmpty() || seen.contains(libraryName)) {
            continue;
        }
        seen.insert(libraryName);
        m_ui->comboLibrary->addItem(libraryName);
    }

    m_ui->comboLibrary->setCurrentIndex(0);
}

void ImportDialog::loadSettings()
{
    if (!m_mainWindow) {
        return;
    }

    QSettings settings(m_mainWindow->getSettingsHeaderName());
    settings.beginGroup(kSettingsGroup);

    const int formatIndex = m_ui->comboFormat->findData(settings.value(QStringLiteral("Format")).toInt());
    if (formatIndex >= 0) {
        m_ui->comboFormat->setCurrentIndex(formatIndex);
    }

    const QString libraryName = settings.value(QStringLiteral("Library")).toString().trimmed();
    if (!libraryName.isEmpty()) {
        const int libraryIndex = m_ui->comboLibrary->findText(libraryName);
        if (libraryIndex >= 0) {
            m_ui->comboLibrary->setCurrentIndex(libraryIndex);
        } else {
            m_ui->comboLibrary->setCurrentIndex(0);
        }
    } else {
        m_ui->comboLibrary->setCurrentIndex(0);
    }

    if (settings.contains(QStringLiteral("FolderMode"))) {
        const bool folderMode = settings.value(QStringLiteral("FolderMode")).toBool();
        m_ui->radioFolder->setChecked(folderMode);
        m_ui->radioSingleFile->setChecked(!folderMode);
    }

    m_ui->editSourcePath->setText(settings.value(QStringLiteral("SourcePath")).toString());
    m_ui->checkOverwrite->setChecked(settings.value(QStringLiteral("OverwriteExisting"), false).toBool());

    settings.endGroup();
}

void ImportDialog::saveSettings() const
{
    if (!m_mainWindow) {
        return;
    }

    QSettings settings(m_mainWindow->getSettingsHeaderName());
    settings.beginGroup(kSettingsGroup);
    settings.setValue(QStringLiteral("Format"), m_ui->comboFormat->currentData().toInt());
    settings.setValue(QStringLiteral("Library"), m_ui->comboLibrary->currentText().trimmed());
    settings.setValue(QStringLiteral("FolderMode"), m_ui->radioFolder->isChecked());
    settings.setValue(QStringLiteral("SourcePath"), m_ui->editSourcePath->text().trimmed());
    settings.setValue(QStringLiteral("OverwriteExisting"), m_ui->checkOverwrite->isChecked());
    settings.endGroup();
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
    const QString startDir = m_ui->editSourcePath->text().trimmed();

    if (m_ui->radioSingleFile->isChecked()) {
        const QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("Select source file"),
            startDir,
            tr("Source files (%1);;All files (*)").arg(filterLine));
        if (!filePath.isEmpty()) {
            m_ui->editSourcePath->setText(QDir::toNativeSeparators(filePath));
        }
        return;
    }

    const QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select source folder"),
        startDir);
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

    saveSettings();

    m_ui->btnImport->setEnabled(false);
    m_ui->plainLog->clear();
    appendLogLine(tr("Importing %1 file(s) into library '%2'...")
                      .arg(sourceFiles.size())
                      .arg(libraryName));

    CoreImportService service(m_mainWindow);
    const CoreImportService::LogCallback logCallback = [this](const QString &line) {
        appendLogLine(line);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    };

    const QVector<CoreImportService::ImportItemResult> results =
        service.importFiles(currentFormat(),
                            libraryName,
                            sourceFiles,
                            m_ui->checkOverwrite->isChecked(),
                            logCallback);

    int successCount = 0;
    for (const CoreImportService::ImportItemResult &result : results) {
        if (result.success) {
            ++successCount;
        }
    }

    appendLogLine(tr("Finished: %1 succeeded, %2 failed.")
                      .arg(successCount)
                      .arg(results.size() - successCount));
    m_ui->btnImport->setEnabled(true);
}

void ImportDialog::on_btnClose_clicked()
{
    saveSettings();
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
