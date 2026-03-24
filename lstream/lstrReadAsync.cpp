#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QMovie>
#include <QPointer>
#include <QTreeWidget>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QtMath>
#include <QDebug>

#include <capnp/serialize-packed.h>
#include <kj/io.h>
#include <kj/array.h>

#include "header.capnp.h"
#include "library.capnp.h"

#include "src/mainwindow.h"
#include "ui_mainwindow.h"

#include "src/lstreamcellreader.h"

/*!*******************************************************************************************************************
 * \brief Ensures that an LStream cache entry exists for the given file path.
 *
 * If the LStream file is already cached, the existing entry is returned.
 * Otherwise, a new cache entry is created, stored and returned.
 *
 * \param path Path to the LStream file.
 * \return Shared pointer to the corresponding cache entry.
 *********************************************************************************************************************/
std::shared_ptr<MainWindow::LStreamCacheEntry> MainWindow::ensureLStreamLoaded(const QString &path)
{
    const QString key = QFileInfo(path).absoluteFilePath();

    auto it = m_lstreamCache.find(key);
    if (it != m_lstreamCache.end()) {
        return it.value();
    }

    auto entry = std::make_shared<LStreamCacheEntry>();
    entry->path = key;

    m_lstreamCache.insert(key, entry);
    return entry;
}

/*!*******************************************************************************************************************
 * \brief Asynchronously loads and parses LStream (Cap'n Proto) cell names.
 *
 * Runs the LStream parser in a background thread and updates the UI
 * once loading is complete. Extracts cell names from the library section
 * of the LStream file.
 *
 * \param path        Path to the LStream file.
 * \param entry       Shared cache entry to populate.
 * \param targetItem  Tree widget item representing the LStream node.
 *********************************************************************************************************************/
void MainWindow::loadLStreamAsync(const QString &path,
                                  const std::shared_ptr<LStreamCacheEntry> &entry,
                                  QTreeWidgetItem *targetItem)
{
    if (!entry || path.isEmpty()) {
        return;
    }

    if (entry->loading || entry->loaded) {
        return;
    }

    entry->loading = true;
    statusBar()->showMessage("Scanning LStream…", 0);

    auto *watcher = new QFutureWatcher<LStreamCacheEntry>(this);

    connect(watcher, &QFutureWatcher<LStreamCacheEntry>::finished, this,
            [this, watcher, entry, targetItem]()
            {
                const LStreamCacheEntry r = watcher->result();
                watcher->deleteLater();

                statusBar()->clearMessage();

                entry->cellNames = r.cellNames;
                entry->loaded    = r.loaded;
                entry->loading   = false;
                entry->errors    = r.errors;

                if (!entry->loaded) {
                    for (const QString &e : entry->errors) {
                        error(e, false);
                    }
                    return;
                }

                populateLStreamTopLevel(targetItem, entry);
            });

    auto future = QtConcurrent::run([path]() -> LStreamCacheEntry
                                    {
                                        LStreamCacheEntry out;
                                        out.path = path;

                                        const LStreamCellReader::Result result = LStreamCellReader::read(path);

                                        out.cellNames = result.cellNames;
                                        out.errors    = result.errors;
                                        out.loaded    = result.loaded;

                                        return out;
                                    });

    watcher->setFuture(future);
}

/*!*******************************************************************************************************************
 * \brief Populates the top-level LStream cells in the tree widget.
 *
 * Creates one tree item per cell found in the LStream library.
 * No hierarchy is processed at this stage (flat list of cell names).
 *
 * \param item   Tree widget item representing the LStream file.
 * \param entry  Shared cache entry containing parsed cell names.
 *********************************************************************************************************************/
void MainWindow::populateLStreamTopLevel(QTreeWidgetItem *lstreamItem,
                                         const std::shared_ptr<LStreamCacheEntry> &entry)
{
    if (!lstreamItem || !entry) {
        return;
    }

    if (lstreamItem->childCount() > 0) {
        return;
    }

    for (const QString &cellName : entry->cellNames) {
        auto *cellItem = new QTreeWidgetItem(lstreamItem);
        cellItem->setText(0, cellName);
        cellItem->setData(0, RoleType, ItemCell);
        cellItem->setData(0, RoleCellName, cellName);
        cellItem->setData(0, RoleLStreamPath, entry->path);
    }
}

