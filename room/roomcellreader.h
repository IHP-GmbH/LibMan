#ifndef ROOMCELLREADER_H
#define ROOMCELLREADER_H

#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

class RoomCellReader
{
public:
    struct CoreHierarchy {
        QStringList topCells;
        QMap<QString, QStringList> children;
        QSet<QString> allCells;
    };

    explicit RoomCellReader(const QString &fileName);

    void coreCreate(const QString &cellName, const QString &viewName = QStringLiteral("layout"));
    QStringList getErrors() const { return m_errorList; }

    bool readHierarchy(CoreHierarchy &out);

private:
    QString m_fileName;
    QStringList m_errorList;
};

#endif // ROOMCELLREADER_H
