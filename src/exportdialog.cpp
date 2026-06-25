#include "exportdialog.h"
#include "ui_exportdialog.h"

#include "mainwindow.h"
#include "core/core_path_utils.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSet>

namespace {

constexpr auto kSettingsGroup = "Export";

} // namespace

ExportDialog::ExportDialog(MainWindow *parent)
    : QDialog(parent)
    , m_ui(new Ui::ExportDialog)
    , m_mainWindow(parent)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Export"));

    populateFormats();
    populateLibraries();
    loadSettings();
}

ExportDialog::~ExportDialog()
{
    saveSettings();
    delete m_ui;
}

void ExportDialog::populateFormats()
{
    m_ui->comboFormat->clear();
    m_ui->comboFormat->addItem(CoreExportService::formatDisplayName(CoreExportService::Format::Gds),
                               static_cast<int>(CoreExportService::Format::Gds));
    m_ui->comboFormat->addItem(CoreExportService::formatDisplayName(CoreExportService::Format::Xschem),
                               static_cast<int>(CoreExportService::Format::Xschem));
    m_ui->comboFormat->addItem(CoreExportService::formatDisplayName(CoreExportService::Format::Qucs),
                               static_cast<int>(CoreExportService::Format::Qucs));
}

void ExportDialog::populateLibraries()
{
    m_ui->comboLibrary->clear();
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
}

void ExportDialog::loadSettings()
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
        }
    } else {
        const QString currentLibrary = m_mainWindow->getCurrentLibraryName();
        const int index = m_ui->comboLibrary->findText(currentLibrary);
        if (index >= 0) {
            m_ui->comboLibrary->setCurrentIndex(index);
        }
    }

    if (settings.contains(QStringLiteral("FolderMode"))) {
        const bool folderMode = settings.value(QStringLiteral("FolderMode")).toBool();
        m_ui->radioFolder->setChecked(folderMode);
        m_ui->radioSingleFile->setChecked(!folderMode);
    }

    m_ui->editSourcePath->setText(settings.value(QStringLiteral("SourcePath")).toString());
    m_ui->editDestinationPath->setText(settings.value(QStringLiteral("DestinationPath")).toString());

    settings.endGroup();
}

void ExportDialog::saveSettings() const
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
    settings.setValue(QStringLiteral("DestinationPath"), m_ui->editDestinationPath->text().trimmed());
    settings.endGroup();
}

void ExportDialog::appendLogLine(const QString &line)
{
    m_ui->plainLog->appendPlainText(line);
}

CoreExportService::Format ExportDialog::currentFormat() const
{
    return static_cast<CoreExportService::Format>(m_ui->comboFormat->currentData().toInt());
}

QString ExportDialog::defaultBrowseDirectory() const
{
    if (!m_mainWindow) {
        return {};
    }

    const QString libraryName = m_ui->comboLibrary->currentText().trimmed();
    if (!libraryName.isEmpty()) {
        const QString libraryPath = m_mainWindow->getLibraryPath(libraryName);
        if (!libraryPath.isEmpty()) {
            return libraryPath;
        }
    }

    const QString projectFile = m_mainWindow->getCurrentProjectFile();
    if (!projectFile.isEmpty()) {
        return QFileInfo(projectFile).absoluteDir().absolutePath();
    }

    return {};
}

QStringList ExportDialog::collectSourceFiles() const
{
    const QString path = m_ui->editSourcePath->text().trimmed();
    if (path.isEmpty()) {
        return {};
    }

    const CoreExportService::Format format = currentFormat();
    const QStringList filters = CoreExportService::sourceNameFilters();
    const QFileInfo pathInfo(path);

    QStringList files;
    if (m_ui->radioSingleFile->isChecked()) {
        if (!pathInfo.isFile()) {
            return {};
        }
        files << pathInfo.absoluteFilePath();
    } else {
        if (!pathInfo.isDir()) {
            return {};
        }

        const QDir dir(pathInfo.absoluteFilePath());
        const QStringList names = dir.entryList(filters, QDir::Files, QDir::Name);
        for (const QString &name : names) {
            files << dir.absoluteFilePath(name);
        }
    }

    QStringList compatible;
    for (const QString &filePath : files) {
        const CoreViewIdentity identity = parseCoreViewIdentity(filePath);
        if (identity.valid && CoreExportService::coreViewMatchesFormat(format, identity.viewName)) {
            compatible << filePath;
        }
    }
    return compatible;
}

void ExportDialog::on_btnBrowseSource_clicked()
{
    const QStringList filters = CoreExportService::sourceNameFilters();
    const QString filterLine = filters.join(QStringLiteral(" "));
    const QString startDir = m_ui->editSourcePath->text().trimmed().isEmpty()
                                 ? defaultBrowseDirectory()
                                 : m_ui->editSourcePath->text();

    if (m_ui->radioSingleFile->isChecked()) {
        const QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("Select CORE view file"),
            startDir,
            tr("CORE files (%1);;All files (*)").arg(filterLine));
        if (!filePath.isEmpty()) {
            m_ui->editSourcePath->setText(QDir::toNativeSeparators(filePath));
        }
        return;
    }

    const QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select source folder"), startDir);
    if (!dirPath.isEmpty()) {
        m_ui->editSourcePath->setText(QDir::toNativeSeparators(dirPath));
    }
}

void ExportDialog::on_btnBrowseDestination_clicked()
{
    const QString startDir = m_ui->editDestinationPath->text().trimmed().isEmpty()
                                 ? defaultBrowseDirectory()
                                 : m_ui->editDestinationPath->text();
    const QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select destination folder"), startDir);
    if (!dirPath.isEmpty()) {
        m_ui->editDestinationPath->setText(QDir::toNativeSeparators(dirPath));
    }
}

void ExportDialog::on_btnExport_clicked()
{
    const QString destinationDir = m_ui->editDestinationPath->text().trimmed();
    if (destinationDir.isEmpty()) {
        QMessageBox::warning(this, tr("Export"), tr("Select a destination folder."));
        return;
    }

    const QStringList sourceFiles = collectSourceFiles();
    if (sourceFiles.isEmpty()) {
        QMessageBox::warning(this, tr("Export"), tr("Select valid CORE source file(s) for the chosen format."));
        return;
    }

    saveSettings();

    m_ui->btnExport->setEnabled(false);
    m_ui->plainLog->clear();
    appendLogLine(tr("Exporting %1 file(s) as %2...")
                      .arg(sourceFiles.size())
                      .arg(CoreExportService::formatDisplayName(currentFormat())));

    CoreExportService service(m_mainWindow);
    const QVector<CoreExportService::ExportItemResult> results =
        service.exportFiles(currentFormat(), destinationDir, sourceFiles);

    int successCount = 0;
    for (const CoreExportService::ExportItemResult &result : results) {
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
    m_ui->btnExport->setEnabled(true);
}

void ExportDialog::on_btnClose_clicked()
{
    saveSettings();
    reject();
}

void ExportDialog::on_radioSingleFile_toggled(bool checked)
{
    if (checked) {
        m_ui->labelSourcePath->setText(tr("File"));
    }
}

void ExportDialog::on_radioFolder_toggled(bool checked)
{
    if (checked) {
        m_ui->labelSourcePath->setText(tr("Folder"));
    }
}
