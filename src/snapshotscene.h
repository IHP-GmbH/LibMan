#ifndef SNAPSHOTSCENE_H
#define SNAPSHOTSCENE_H

#include <QColor>
#include <QPolygonF>
#include <QRectF>
#include <QString>
#include <QVector>

#include <functional>

struct SnapshotStroke {
    QPolygonF points;
    QColor color;
    bool filled = false;
};

struct SnapshotLabel {
    QPointF pos;
    QString text;
    QColor color;
};

struct SnapshotScene {
    QVector<SnapshotStroke> strokes;
    QVector<SnapshotLabel> labels;
    QRectF bounds;
    bool yUp = true;
    QString message;

    bool empty() const { return strokes.isEmpty() && labels.isEmpty(); }
};

using SnapshotSymbolResolver = std::function<QString(const QString &cellName)>;

SnapshotScene loadCoreSnapshot(const QString &corePath,
                               const QString &viewName,
                               const SnapshotSymbolResolver &symbolPathForCell,
                               const QString &cellName = QString());

/*!
 * From an EmModel ROOM file, resolve topology.layoutPath when it points at a
 * ROOM layout (*.layout.room / *.room). Relative paths are resolved beside the
 * emmodel file. Empty if missing, GDS-only, or unreadable.
 */
QString layoutRoomPathFromEmModel(const QString &emmodelPath, QString *topCellOut = nullptr);

#endif
