#ifndef LSTREAMCELLWRITER_H
#define LSTREAMCELLWRITER_H

#include <QString>
#include <QStringList>

/*!********************************************************************************************************************
 * \brief Writes a minimal LStream file containing a library and a list of cells.
 *********************************************************************************************************************/
class LStreamCellWriter
{
public:
    /*!****************************************************************************************************************
     * \brief Result of LStream write operation.
     *****************************************************************************************************************/
    struct Result
    {
        bool        written = false;
        QStringList errors;
    };

public:
    static Result  write(const QString &path,
                        const QString &libraryName,
                        const QStringList &cellNames,
                        const QString &technology = QStringLiteral("SG13G2"),
                        const QString &generator = QStringLiteral("LibMan"));
};

#endif // LSTREAMCELLWRITER_H
