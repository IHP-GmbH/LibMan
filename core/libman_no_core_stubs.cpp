#include "core/corecellreader.h"
#include "core/coreKlayoutBridge.h"
#include "src/mainwindow.h"

#include <QFileInfo>

CoreCellReader::CoreCellReader(const QString &fileName)
    : m_fileName(fileName)
{
}

void CoreCellReader::coreCreate(const QString &cellName)
{
    Q_UNUSED(cellName);
    m_errorList << QStringLiteral("CORE support is not built (CONFIG+=no_core).");
}

bool CoreCellReader::readHierarchy(CoreHierarchy &out)
{
    Q_UNUSED(out);
    m_errorList << QStringLiteral("CORE support is not built (CONFIG+=no_core).");
    return false;
}

QString coreLayoutPathForKLayout(const QString &viewPath, QStringList *errors)
{
    const QFileInfo fi(viewPath);
    if(!fi.exists() || !fi.isFile()) {
        if(errors) {
            *errors << QStringLiteral("CORE file not found: %1").arg(viewPath);
        }
        return QString();
    }

    return fi.absoluteFilePath();
}

void MainWindow::loadCoreHierarchyAsync(const QString &corePath,
                                        const std::shared_ptr<CoreCacheEntry> &entry,
                                        QTreeWidgetItem *targetItem,
                                        const QString &requestedCellName)
{
    Q_UNUSED(corePath);
    Q_UNUSED(targetItem);
    Q_UNUSED(requestedCellName);

    if(!entry) {
        return;
    }

    entry->loading = false;
    entry->loaded = false;
    entry->errors << QStringLiteral("CORE support is not built (CONFIG+=no_core).");
}
