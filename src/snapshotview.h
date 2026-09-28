#ifndef SNAPSHOTVIEW_H
#define SNAPSHOTVIEW_H

#include "snapshotscene.h"

#include <QWidget>

class SnapshotView : public QWidget {
    Q_OBJECT

public:
    explicit SnapshotView(QWidget *parent = nullptr);

    void setScene(const SnapshotScene &scene);
    void clearScene(const QString &message);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    SnapshotScene m_scene;
};

#endif
