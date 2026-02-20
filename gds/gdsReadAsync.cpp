#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include "gds/gdsreader.h"
#include "src/mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::loadGdsHierarchyAsync(const QString &gdsPath,
                                       const std::shared_ptr<GdsCacheEntry> &entry,
                                       QTreeWidgetItem *targetItem,
                                       const QString &requestedCellName /* = QString() */)
{
    if (!entry || gdsPath.isEmpty()) {
        return;
    }

    if (entry->loading || entry->loaded) {
        return;
    }

    entry->loading = true;
    statusBar()->showMessage("Scanning GDS hierarchy…", 0);

    auto *watcher = new QFutureWatcher<GdsCacheEntry>(this);

    connect(watcher, &QFutureWatcher<GdsCacheEntry>::finished, this,
            [this, watcher, entry, targetItem, requestedCellName]()
            {
                const GdsCacheEntry r = watcher->result();
                watcher->deleteLater();

                entry->errors    = r.errors;
                entry->hierarchy = r.hierarchy;
                entry->loaded    = r.loaded;
                entry->loading   = false;

                if (!entry->loaded) {
                    for (const QString &e : entry->errors) {
                        error(e, false);
                    }
                    statusBar()->showMessage("GDS load failed.", 10000);
                    return;
                }

                statusBar()->showMessage(
                    QString("GDS loaded: %1 cells").arg(entry->hierarchy.allCells.size()),
                    10000
                    );

                if (!targetItem) {
                    return;
                }

                if (!requestedCellName.isEmpty()) {
                    populateCellChildren(targetItem, entry, requestedCellName);
                    targetItem->setExpanded(true);
                    return;
                }

                populateGdsTopLevel(targetItem, entry);
                targetItem->setExpanded(true);
            });

    auto future = QtConcurrent::run([gdsPath]() -> GdsCacheEntry {
        GdsCacheEntry out;
        out.path = QFileInfo(gdsPath).absoluteFilePath();

        GdsReader reader(out.path);
        if (!reader.readHierarchy(out.hierarchy)) {
            out.errors = reader.getErrors();
            out.loaded = false;
            return out;
        }

        out.loaded = true;
        return out;
    });

    watcher->setFuture(future);
}
