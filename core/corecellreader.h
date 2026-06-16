#ifndef CORECELLREADER_H
#define CORECELLREADER_H

#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

class CoreCellReader
{
public:
    struct CoreHierarchy {
        QStringList topCells;
        QMap<QString, QStringList> children;
        QSet<QString> allCells;
    };

    explicit CoreCellReader(const QString &fileName);

    void coreCreate(const QString &cellName);
    QStringList getErrors() const { return m_errorList; }

    bool readHierarchy(CoreHierarchy &out);

private:
    QString m_fileName;
    QStringList m_errorList;
};

#endif // CORECELLREADER_H
