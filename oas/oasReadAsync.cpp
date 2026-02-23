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


#include "src/mainwindow.h"
#include "ui_mainwindow.h"

static QString dumpU16(const QString& s)
{
    QStringList parts;
    parts.reserve(s.size());
    for (QChar ch : s)
        parts << QString("U+%1").arg(ch.unicode(), 4, 16, QLatin1Char('0'));
    return parts.join(' ');
}

static void dbgStr(const char* tag, const QString& s)
{
    qDebug().noquote()
    << tag
    << "text='" << s << "'"
    << "u16=[" << dumpU16(s) << "]"
    << "utf8hex=" << QString(s.toUtf8().toHex(' '));
}

std::shared_ptr<MainWindow::OasCacheEntry> MainWindow::ensureOasLoaded(const QString &oasPath)
{
    const QString key = QFileInfo(oasPath).absoluteFilePath();

    auto it = m_oasCache.find(key);
    if (it != m_oasCache.end()) {
        return it.value();
    }

    auto entry = std::make_shared<OasCacheEntry>();
    entry->path = key;

    m_oasCache.insert(key, entry);
    return entry;
}

void MainWindow::populateOasTopLevel(QTreeWidgetItem *oasItem,
                                     const std::shared_ptr<OasCacheEntry> &entry)
{
    if (!oasItem || !entry) {
        return;
    }

    if (oasItem->childCount() > 0) {
        return;
    }

    for (const QString &topCell : entry->hierarchy.topCells) {
        auto *cellItem = new QTreeWidgetItem(oasItem);

        //dbgStr("OAS TOPCELL", topCell);

        cellItem->setText(0, topCell);
        cellItem->setData(0, RoleType, ItemCell);
        cellItem->setData(0, RoleCellName, topCell);
        cellItem->setData(0, RoleOasPath, entry->path);

        const auto it = entry->hierarchy.children.find(topCell);
        if (it != entry->hierarchy.children.end() && !it.value().isEmpty()) {
            cellItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }
}

void MainWindow::populateOasCellChildren(QTreeWidgetItem *cellItem,
                                         const std::shared_ptr<OasCacheEntry> &entry,
                                         const QString &cellName)
{
    if (!cellItem || !entry) {
        return;
    }

    if (cellItem->childCount() > 0) {
        return;
    }

    const auto it = entry->hierarchy.children.find(cellName);
    if (it == entry->hierarchy.children.end()) {
        return;
    }

    const QStringList childs = it.value();
    for (const QString &ch : childs) {

        //dbgStr("OAS CHILD", ch);

        auto *chItem = new QTreeWidgetItem(cellItem);
        chItem->setText(0, ch);
        chItem->setData(0, RoleType, ItemCell);
        chItem->setData(0, RoleCellName, ch);
        chItem->setData(0, RoleOasPath, entry->path);

        const auto it2 = entry->hierarchy.children.find(ch);
        if (it2 != entry->hierarchy.children.end() && !it2.value().isEmpty()) {
            chItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }
}

void MainWindow::loadOasHierarchyAsync(const QString &oasPath,
                                       const std::shared_ptr<OasCacheEntry> &entry,
                                       QTreeWidgetItem *targetItem,
                                       const QString &requestedCellName /* = QString() */)
{
    if (!entry || oasPath.isEmpty()) {
        return;
    }

    if (entry->loading || entry->loaded) {
        return;
    }

    entry->loading = true;
    statusBar()->showMessage("Scanning OASIS hierarchy…", 0);

    if (targetItem) {
        setLoadingSpinner(targetItem, true);
    }

    auto *watcher = new QFutureWatcher<OasCacheEntry>(this);

    connect(watcher, &QFutureWatcher<OasCacheEntry>::finished, this,
            [this, watcher, entry, targetItem, requestedCellName]()
            {
                const OasCacheEntry r = watcher->result();
                watcher->deleteLater();

                if (targetItem) {
                    setLoadingSpinner(targetItem, false);
                }

                entry->errors    = r.errors;
                entry->hierarchy = r.hierarchy;
                entry->loaded    = r.loaded;
                entry->loading   = false;

                if (!entry->loaded) {
                    for (const QString &e : entry->errors) {
                        error(e, false);
                    }
                    statusBar()->showMessage("OASIS load failed.", 10000);
                    return;
                }

                statusBar()->showMessage(
                    QString("OASIS loaded: %1 cells").arg(entry->hierarchy.allCells.size()),
                    10000
                    );

                if (!targetItem) {
                    return;
                }

                if (!requestedCellName.isEmpty()) {
                    populateOasCellChildren(targetItem, entry, requestedCellName);
                    targetItem->setExpanded(true);
                    return;
                }

                populateOasTopLevel(targetItem, entry);
                targetItem->setExpanded(true);
            });

    auto future = QtConcurrent::run([oasPath]() -> OasCacheEntry {
        OasCacheEntry out;
        out.path = QFileInfo(oasPath).absoluteFilePath();

        oasReader reader(out.path);
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
