// oasreader.h
#ifndef OASREADER_H
#define OASREADER_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>

struct LayoutHierarchy
{
    QStringList                 topCells;
    QHash<QString, QStringList> children;   // parent -> [child...]
    QSet<QString>               allCells;
};

class oasReader
{
public:
    explicit oasReader(const QString &fileName);

    // Читает имена ячеек + связи parent->child (PLACEMENT) и строит дерево.
    bool        readHierarchy(LayoutHierarchy &out);

    QStringList getErrors() const;

private:
    QString     m_fileName;
    QStringList m_errorList;
};

#endif // OASREADER_H