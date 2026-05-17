#include "oasReader.h"

#include <zlib.h>
#include <QFile>

/*!********************************************************************************************************************
 * \brief Writes raw bytes to the OASIS file.
 * \param data Bytes to append.
 *********************************************************************************************************************/
void oasReader::writeRaw(const QByteArray &data) const
{
    QFile file(m_fileName);
    if(!file.open(QIODevice::Append)) {
        m_errorList << QString("Failed to open OASIS for append: '%1'").arg(m_fileName);
        return;
    }

    if(!data.isEmpty()) {
        file.write(data);
    }

    file.close();
}

/*!********************************************************************************************************************
 * \brief Writes a single byte to the OASIS file.
 * \param value Byte value to write.
 *********************************************************************************************************************/
void oasReader::writeByte(uchar value) const
{
    writeRaw(QByteArray(1, char(value)));
}

/*!********************************************************************************************************************
 * \brief Writes an OASIS unsigned integer (varint).
 * \param value Value to encode.
 *********************************************************************************************************************/
void oasReader::writeUInt(quint64 value) const
{
    QByteArray data;

    do {
        uchar b = uchar(value & 0x7F);
        value >>= 7;
        if(value) {
            b |= 0x80;
        }
        data.append(char(b));
    }
    while(value);

    writeRaw(data);
}

/*!********************************************************************************************************************
 * \brief Writes an OASIS signed integer using the same mapping as the reader.
 * \param value Signed value to encode.
 *********************************************************************************************************************/
void oasReader::writeSInt(qint64 value) const
{
    quint64 u = 0;

    if(value < 0) {
        u = (quint64(-value) << 1) - 1;
    }
    else {
        u = quint64(value) << 1;
    }

    writeUInt(u);
}

/*!********************************************************************************************************************
 * \brief Writes a length-prefixed byte string.
 * \param value Byte string to write.
 *********************************************************************************************************************/
void oasReader::writeString(const QByteArray &value) const
{
    writeUInt(quint64(value.size()));
    writeRaw(value);
}

/*!********************************************************************************************************************
 * \brief Writes an OASIS A-string (Latin-1).
 * \param value String to write.
 *********************************************************************************************************************/
void oasReader::writeAString(const QString &value) const
{
    writeString(value.toLatin1());
}

/*!********************************************************************************************************************
 * \brief Writes an OASIS N-string (UTF-8).
 * \param value String to write.
 *********************************************************************************************************************/
void oasReader::writeNString(const QString &value) const
{
    writeString(value.toUtf8());
}

/*!********************************************************************************************************************
 * \brief Writes the OASIS file magic.
 *********************************************************************************************************************/
void oasReader::writeMagic() const
{
    writeRaw("%SEMI-OASIS\r\n");
}

/*!********************************************************************************************************************
 * \brief Writes a minimal START record.
 *
 * Layout:
 *   - recId = 1
 *   - version = "1.0"
 *   - unit = real type 0, value 1000
 *   - offsetFlag = 0
 *   - 12 table offsets = 0
 *********************************************************************************************************************/
void oasReader::writeStartRecord() const
{
    writeUInt(1);           // START
    writeAString("1.0");    // version

    writeUInt(0);           // real type 0
    writeUInt(1000);        // unit

    writeUInt(0);           // offsetFlag = 0

    for(int i = 0; i < 12; ++i) {
        writeUInt(0);
    }
}

/*!********************************************************************************************************************
 * \brief Writes a CELL record with inline name.
 * \param cellName Cell name.
 *********************************************************************************************************************/
void oasReader::writeCellRecord(const QString &cellName) const
{
    writeUInt(14);                  // CELL with inline name
    writeString(cellName.toUtf8()); // inline N-string payload style used by current parser
}

/*!********************************************************************************************************************
 * \brief Writes a minimal END record.
 *
 * Layout:
 *   - recId = 2
 *   - validation-scheme = 0
 *   - trailing 4 zero bytes
 *********************************************************************************************************************/
void oasReader::writeEndRecord() const
{
    writeUInt(2);   // END
    writeUInt(0);   // validation-scheme = 0

    writeByte(0x00);
    writeByte(0x00);
    writeByte(0x00);
    writeByte(0x00);
}

/*!********************************************************************************************************************
 * \brief Creates a minimal valid OASIS file with a single cell.
 *
 * This function generates a valid OASIS file structure compatible with KLayout.
 * The file contains:
 *   - Magic header ("%SEMI-OASIS\r\n")
 *   - START record with default units and no table offsets
 *   - CELLNAME record (implicit reference numbering)
 *   - CELL record referencing the created cell
 *   - END record with proper padding (total size = 256 bytes)
 *
 * The implementation follows the SEMI OASIS specification requirements:
 *   - Variable-length integer encoding (varint)
 *   - UTF-8 string encoding (N-string)
 *   - Proper END record layout including padding-string and validation scheme
 *
 * \param cellName Name of the cell to create in the OASIS file.
 *
 * \note The resulting file is minimal and contains no geometry.
 * \note The END record is padded to exactly 256 bytes as required by the specification.
 *********************************************************************************************************************/
void oasReader::oasCreate(const QString &cellName)
{
    m_errorList.clear();

    QFile file(m_fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_errorList << QString("Failed to open file '%1'").arg(m_fileName);
        return;
    }

    auto writeUIntTo = [&](QByteArray &buf, quint64 value) {
        do {
            uchar b = uchar(value & 0x7F);
            value >>= 7;
            if(value) {
                b |= 0x80;
            }
            buf.append(char(b));
        } while(value);
    };

    auto writeOasStringTo = [&](QByteArray &buf, const QString &s) {
        const QByteArray data = s.toUtf8();
        writeUIntTo(buf, quint64(data.size()));
        buf.append(data);
    };

    // Magic
    file.write("%SEMI-OASIS\r\n");

    // START
    QByteArray startRec;
    writeUIntTo(startRec, 1);          // START
    writeOasStringTo(startRec, "1.0"); // version
    writeUIntTo(startRec, 0);          // unit: real type 0
    writeUIntTo(startRec, 1000);       // unit value
    writeUIntTo(startRec, 0);          // offset-flag = 0
    for(int i = 0; i < 12; ++i) {
        writeUIntTo(startRec, 0);
    }
    file.write(startRec);

    // CELLNAME (record 3 = implicit ref assignment, first one gets ref 0)
    QByteArray cnRec;
    writeUIntTo(cnRec, 3);
    writeOasStringTo(cnRec, cellName);
    file.write(cnRec);

    // CELL (record 13 = reference to CELLNAME ref 0)
    QByteArray cellRec;
    writeUIntTo(cellRec, 13);
    writeUIntTo(cellRec, 0);
    file.write(cellRec);

    // END
    // Format: '2' padding-string validation-scheme
    // offset-flag = 0 => no table-offsets in END
    // total END record length must be exactly 256 bytes
    QByteArray endRec;
    writeUIntTo(endRec, 2); // END

    // Need:
    // 1 byte recId
    // + varint(padLen)
    // + padLen bytes
    // + 1 byte validation-scheme(0)
    // = 256
    //
    // padLen = 252 gives:
    // 1 + 2 + 252 + 1 = 256
    const quint64 padLen = 252;

    writeUIntTo(endRec, padLen);                  // b-string length
    endRec.append(QByteArray(int(padLen), '\0')); // b-string bytes
    writeUIntTo(endRec, 0);                      // validation-scheme = 0 (No Validation)

    file.write(endRec);
    file.close();
}
