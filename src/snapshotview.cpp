#include "snapshotview.h"

#include <QPainter>
#include <QPaintEvent>

SnapshotView::SnapshotView(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    setMinimumHeight(80);
    m_scene.message = tr("Select a schematic, symbol, or layout");
}

void SnapshotView::setScene(const SnapshotScene &scene)
{
    m_scene = scene;
    update();
}

void SnapshotView::clearScene(const QString &message)
{
    m_scene = SnapshotScene();
    m_scene.message = message;
    update();
}

void SnapshotView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(252, 252, 252));

    if (m_scene.empty()) {
        painter.setPen(QColor(120, 120, 120));
        const QString message = m_scene.message.isEmpty()
            ? tr("Select a schematic, symbol, or layout")
            : m_scene.message;
        painter.drawText(rect().adjusted(8, 8, -8, -8),
                         Qt::AlignCenter | Qt::TextWordWrap,
                         message);
        return;
    }

    QRectF bounds = m_scene.bounds;
    if (bounds.width() < 1.0) {
        bounds.setWidth(1.0);
    }
    if (bounds.height() < 1.0) {
        bounds.setHeight(1.0);
    }

    const qreal pad = 10.0;
    const QRectF target = QRectF(rect()).adjusted(pad, pad, -pad, -pad);
    const qreal scale = qMin(target.width() / bounds.width(), target.height() / bounds.height());
    const qreal drawnW = bounds.width() * scale;
    const qreal drawnH = bounds.height() * scale;
    const qreal originX = target.left() + (target.width() - drawnW) * 0.5;
    const qreal originY = target.top() + (target.height() - drawnH) * 0.5;

    const auto mapPoint = [&](const QPointF &point) {
        const qreal x = originX + (point.x() - bounds.left()) * scale;
        const qreal yNorm = (point.y() - bounds.top()) * scale;
        const qreal y = m_scene.yUp
            ? originY + drawnH - yNorm
            : originY + yNorm;
        return QPointF(x, y);
    };

    for (const SnapshotStroke &stroke : m_scene.strokes) {
        if (stroke.points.isEmpty()) {
            continue;
        }
        QPolygonF mapped;
        mapped.reserve(stroke.points.size());
        for (const QPointF &point : stroke.points) {
            mapped << mapPoint(point);
        }

        QPen pen(stroke.color);
        pen.setCosmetic(true);
        pen.setWidthF(stroke.filled ? 1.2 : 1.4);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        if (stroke.filled && mapped.size() >= 3) {
            painter.setBrush(stroke.color);
            painter.drawPolygon(mapped);
        } else if (mapped.size() >= 2) {
            painter.setBrush(Qt::NoBrush);
            painter.drawPolyline(mapped);
        } else {
            painter.setBrush(stroke.color);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(mapped.first(), 2.5, 2.5);
        }
    }

    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    for (const SnapshotLabel &label : m_scene.labels) {
        if (label.text.isEmpty()) {
            continue;
        }
        painter.setPen(label.color);
        const QPointF at = mapPoint(label.pos);
        painter.drawText(QRectF(at.x() + 3, at.y() - 12, 120, 16),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         label.text);
    }
}
