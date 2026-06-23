#ifndef KLAYOUT_CELL_RESOLVER_H
#define KLAYOUT_CELL_RESOLVER_H

#include <QSet>
#include <QString>
#include <QStringList>

/*!
 * \brief Minimal hierarchy snapshot used to pick a KLayout root cell.
 */
struct LayoutHierarchySnapshot
{
    QStringList   topCells;
    QSet<QString> allCells;
};

QString resolveKLayoutRootCell(const LayoutHierarchySnapshot &hierarchy,
                               const QString &groupName);

bool loadLayoutHierarchySnapshot(const QString &layoutPath,
                                 LayoutHierarchySnapshot &out,
                                 QStringList *errors = nullptr);

#endif // KLAYOUT_CELL_RESOLVER_H
