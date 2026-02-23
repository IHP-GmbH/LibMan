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

/*!********************************************************************************************************************
 * \brief Creates a spinner icon pixmap for loading indication.
 *
 * Renders a small animated spinner icon using QPainter. The spinner color is derived
 * from the widget palette to match light/dark UI themes. The animation frame is defined
 * by the rotation angle.
 *
 * The icon is intended for use in item views (e.g. QTreeWidget) to indicate background
 * loading activity.
 *
 * \param w Widget used to obtain palette colors (may be nullptr).
 * \param angleDeg Rotation angle in degrees for the current animation frame.
 * \param sizePx Icon size in pixels (width = height).
 * \return Generated spinner icon.
 *********************************************************************************************************************/
static QIcon makeSpinnerIcon(const QWidget *w, int angleDeg, int sizePx = 16)
{
    QPixmap pm(sizePx, sizePx);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QColor base = w ? w->palette().color(QPalette::Text) : QColor(80, 80, 80);

    const QPointF c(sizePx / 2.0, sizePx / 2.0);
    const double outerR = sizePx * 0.45;
    const double innerR = sizePx * 0.22;

    const int N = 12;

    const double step = 360.0 / N;

    p.translate(c);
    p.rotate(angleDeg);

    for (int i = 0; i < N; ++i) {
        double t = double(i) / (N - 1);          // 0..1
        int alpha = int(255 * (1.0 - t) * 0.90); // 0..~230

        QColor col = base;
        col.setAlpha(qBound(20, alpha, 255));

        QPen pen(col, qMax(1.0, sizePx * 0.10), Qt::SolidLine, Qt::RoundCap);
        p.setPen(pen);

        p.drawLine(QPointF(0, -innerR), QPointF(0, -outerR));

        p.rotate(step);
    }

    return QIcon(pm);
}

/*!********************************************************************************************************************
 * \brief Enables or disables a loading spinner icon for a tree widget item.
 *
 * When enabled, a timer-driven animated spinner icon is attached to the given
 * QTreeWidgetItem to indicate background processing (e.g. GDS hierarchy loading).
 * When disabled, the spinner is stopped, removed and cleaned up.
 *
 * The animation state is tracked per item to allow multiple independent spinners.
 *
 * \param item Tree widget item to decorate with a spinner.
 * \param on True to start spinner animation, false to stop and remove it.
 *********************************************************************************************************************/
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

    if (m_spinnerStates.contains(item) && m_spinnerStates[item].timer) {
        return;
    }

    SpinnerState st;
    st.angleDeg = 0;

    auto *timer = new QTimer(this);
    timer->setInterval(60); // скорость (мс). 50-90 обычно норм.

    st.timer = timer;
    m_spinnerStates.insert(item, st);

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

/*!********************************************************************************************************************
 * \brief Loads GDS hierarchy asynchronously and updates the UI on completion.
 *
 * Starts a background task that scans the GDS file hierarchy using GdsReader and
 * stores the result in the provided cache entry. While loading, a spinner is shown
 * on the target tree item and a status bar message is displayed.
 *
 * On successful completion:
 *  - hierarchy data is cached,
 *  - top-level or requested cell contents are populated in the tree view,
 *  - the target item is expanded automatically.
 *
 * On failure:
 *  - error messages are collected and reported,
 *  - loading state is cleared.
 *
 * The function ensures that the same cache entry is not loaded multiple times
 * concurrently.
 *
 * \param gdsPath Absolute or relative path to the GDS file.
 * \param entry Shared cache entry associated with this GDS file.
 * \param targetItem Tree widget item that represents the GDS node in the UI.
 * \param requestedCellName Optional cell name to expand directly after loading.
 *********************************************************************************************************************/
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
