#ifndef LSTREAMCELLREADER_H
#define LSTREAMCELLREADER_H

#include <QString>
#include <QStringList>

class LStreamCellReader
{
public:
    struct Result
    {
        QStringList         cellNames;
        QStringList         errors;
        bool                loaded = false;
    };

public:
    static Result          read(const QString &fileName);
};

#endif // LSTREAMCELLREADER_H
