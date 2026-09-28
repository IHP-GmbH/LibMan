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

#endif
