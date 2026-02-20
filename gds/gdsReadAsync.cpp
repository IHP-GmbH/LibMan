#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QMovie>
#include <QPointer>
#include <QTreeWidget>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QtMath>


#include "gds/gdsreader.h"
#include "src/mainwindow.h"
#include "ui_mainwindow.h"

static QIcon makeSpinnerIcon(const QWidget *w, int angleDeg, int sizePx = 16)
{
    QPixmap pm(sizePx, sizePx);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Берём цвет из палитры, чтобы выглядело нативно (тёмная/светлая тема)
    const QColor base = w ? w->palette().color(QPalette::Text) : QColor(80, 80, 80);

    const QPointF c(sizePx / 2.0, sizePx / 2.0);
    const double outerR = sizePx * 0.45;
    const double innerR = sizePx * 0.22;

    // 12 “лепестков”
    const int N = 12;

    // Крутим: самый яркий сегмент “впереди”
    const double step = 360.0 / N;

    p.translate(c);
    p.rotate(angleDeg);

    for (int i = 0; i < N; ++i) {
        // Чем дальше от “головы”, тем прозрачнее
        double t = double(i) / (N - 1);          // 0..1
        int alpha = int(255 * (1.0 - t) * 0.90); // 0..~230

        QColor col = base;
        col.setAlpha(qBound(20, alpha, 255));

        QPen pen(col, qMax(1.0, sizePx * 0.10), Qt::SolidLine, Qt::RoundCap);
        p.setPen(pen);

        // Рисуем сегмент вверх (по текущему вращению)
        p.drawLine(QPointF(0, -innerR), QPointF(0, -outerR));

        p.rotate(step);
    }

    return QIcon(pm);
}

void MainWindow::setLoadingSpinner(QTreeWidgetItem *item, bool on)
{
    if (!item) {
        return;
    }

    QTreeWidget *tw = item->treeWidget();

    if (!on) {
        auto it = m_spinnerStates.find(item);
        if (it != m_spinnerStates.end()) {
            if (it.value().timer) {
                it.value().timer->stop();
                it.value().timer->deleteLater();
                it.value().timer = nullptr;
            }
            m_spinnerStates.erase(it);
        }

        item->setIcon(0, QIcon());
        if (tw) {
            tw->viewport()->update(tw->visualItemRect(item));
        }
        return;
    }

    // Уже крутится
    if (m_spinnerStates.contains(item) && m_spinnerStates[item].timer) {
        return;
    }

    SpinnerState st;
    st.angleDeg = 0;

    auto *timer = new QTimer(this);
    timer->setInterval(60); // скорость (мс). 50-90 обычно норм.

    st.timer = timer;
    m_spinnerStates.insert(item, st);

    // Первый кадр сразу, чтобы не казалось что “не появилось”
    item->setIcon(0, makeSpinnerIcon(tw ? (QWidget*)tw : (QWidget*)this, 0, 16));
    if (tw) {
        tw->viewport()->update(tw->visualItemRect(item));
    }

    connect(timer, &QTimer::timeout, this, [this, item]() {
        if (!item) {
            return;
        }

        auto it = m_spinnerStates.find(item);
        if (it == m_spinnerStates.end()) {
            return;
        }

        SpinnerState &st = it.value();
        st.angleDeg = (st.angleDeg + 30) % 360;

        QTreeWidget *tw2 = item->treeWidget();
        item->setIcon(0, makeSpinnerIcon(tw2 ? (QWidget*)tw2 : (QWidget*)this, st.angleDeg, 16));

        if (tw2) {
            tw2->viewport()->update(tw2->visualItemRect(item));
        }
    });

    timer->start();
}

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

    if (targetItem) {
        setLoadingSpinner(targetItem, true);
    }

    auto *watcher = new QFutureWatcher<GdsCacheEntry>(this);

    connect(watcher, &QFutureWatcher<GdsCacheEntry>::finished, this,
            [this, watcher, entry, targetItem, requestedCellName]()
            {
                const GdsCacheEntry r = watcher->result();
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
