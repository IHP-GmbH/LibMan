/************************************************************************
 *  LibMan – library and view management tool for IC design projects.
 *
 *  Copyright (C) 2023–2025 IHP Authors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ************************************************************************/

/*!********************************************************************************************************************
 * \file gdsreader.cpp
 * \brief GDSII reader and writer implementation.
 *
 * This file implements the GdsReader class used by LibMan to:
 *  - create minimal valid GDSII files (writer part),
 *  - read structural hierarchy information from existing GDSII files (reader part).
 *
 * The reader part is intentionally lightweight and parses only:
 *  - structure (cell) names,
 *  - SREF / AREF references,
 *  - parent–child relationships between cells.
 *
 * Geometry, layers and shapes are ignored to allow fast hierarchy inspection
 * even for large hierarchical GDS files.
 *********************************************************************************************************************/

#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>

#include <QByteArray>
#include <QSet>

#include "gdsreader.h"

// =====================================================================================================================
// Constructor
// =====================================================================================================================

/*!********************************************************************************************************************
 * \brief Constructs GdsReader object for a given GDS file.
 * \param fileName Absolute or relative path to the GDS file.
 *********************************************************************************************************************/
GdsReader::GdsReader(const QString &fileName)
    : m_fileName(fileName),
    m_gdsFile(nullptr)
{
    m_errorList.clear();
}

// =====================================================================================================================
// Writer part
// =====================================================================================================================

/*!********************************************************************************************************************
 * \brief Creates a minimal valid GDSII file containing a single top-level cell.
 *
 * The function creates a new GDS file and writes:
 *  - HEADER
 *  - BGNLIB
 *  - LIBNAME
 *  - UNITS
 *  - ENDLIB
 *
 * \param cellName Name of the top-level cell.
 *********************************************************************************************************************/
void GdsReader::gdsCreate(const QString &cellName)
{
    if (m_fileName.isEmpty()) {
        return;
    }

    m_gdsFile = fopen(m_fileName.toStdString().c_str(), "wb");
    if (!m_gdsFile) {
        m_errorList << QString("Failed to open GDS for write: '%1'").arg(m_fileName);
        return;
    }

    gdsBegin(cellName);
    gdsEnd();

    fclose(m_gdsFile);
    m_gdsFile = nullptr;
}

/*!********************************************************************************************************************
 * \brief Writes GDS library header records.
 * \param libName Name of the GDS library.
 *********************************************************************************************************************/
void GdsReader::gdsBegin(const QString &libName) const
{
    int header[1];
    header[0] = 600;

    writeInt(GDS_HEADER, header, 1);
    writeInt(GDS_BGNLIB, gdsTime(), 12);
    writeStr(GDS_LIBNAME, libName.toStdString());
    writeUnits();
}

/*!********************************************************************************************************************
 * \brief Writes ENDLIB record to close the GDS file.
 *********************************************************************************************************************/
void GdsReader::gdsEnd() const
{
    writeRec(GDS_ENDLIB);
}

/*!********************************************************************************************************************
 * \brief Returns current timestamp formatted for GDS BGNLIB record.
 * \return Pointer to static array with 12 integers representing time.
 *********************************************************************************************************************/
int* GdsReader::gdsTime() const
{
    static int timeIO[12];
    static bool timeSet = false;

    if (!timeSet) {
        time_t now = time(nullptr);
        tm lctn = *localtime(&now);

        timeIO[0]  = lctn.tm_year + 1900;
        timeIO[1]  = lctn.tm_mon + 1;
        timeIO[2]  = lctn.tm_mday;
        timeIO[3]  = lctn.tm_hour;
        timeIO[4]  = lctn.tm_min;
        timeIO[5]  = lctn.tm_sec;
        timeIO[6]  = timeIO[0];
        timeIO[7]  = timeIO[1];
        timeIO[8]  = timeIO[2];
        timeIO[9]  = timeIO[3];
        timeIO[10] = timeIO[4];
        timeIO[11] = timeIO[5];

        timeSet = true;
    }

    return timeIO;
}

/*!********************************************************************************************************************
 * \brief Writes integer-based GDS record.
 * \param record GDS record type.
 * \param arrInt Integer data array.
 * \param cnt Number of integers.
 * \return 0 on success, non-zero on error.
 *********************************************************************************************************************/
int GdsReader::writeInt(int record, int arrInt[], int cnt) const
{
    unsigned int dataSize = record & 0xff;

    if (dataSize == 0x02 && cnt > 0) {
        dataSize = 2;
    } else if (dataSize == 0x03 && cnt > 0) {
        dataSize = 4;
    } else if (dataSize == 0x00 && cnt == 0) {
        dataSize = 0;
    } else {
        m_errorList << QString("Incorrect parameters for record: 0x%1").arg(record, 0, 16);
        return 1;
    }

    unsigned int sizeByte = cnt * dataSize + 4;
    unsigned char hdr[4];

    hdr[0] = (sizeByte >> 8) & 0xff;
    hdr[1] = sizeByte & 0xff;
    hdr[2] = (record >> 8) & 0xff;
    hdr[3] = record & 0xff;

    fwrite(hdr, 1, 4, m_gdsFile);

    unsigned char dataOut[4];
    for (int i = 0; i < cnt; ++i) {
        for (unsigned int j = 0; j < dataSize; ++j) {
            dataOut[j] = (arrInt[i] >> (((dataSize - 1) * 8) - (j * 8))) & 0xff;
        }
        fwrite(dataOut, 1, dataSize, m_gdsFile);
    }

    return 0;
}

/*!********************************************************************************************************************
 * \brief Writes string-based GDS record.
 * \param record GDS record type.
 * \param inStr String payload.
 * \return 0 on success, non-zero on error.
 *********************************************************************************************************************/
int GdsReader::writeStr(int record, std::string inStr) const
{
    if ((record & 0xff) != 0x06) {
        m_errorList << QString("Incorrect record: 0x%1").arg(record, 0, 16);
        return 1;
    }

    if (inStr.length() % 2 == 1) {
        inStr.push_back('\0');
    }

    const int len = static_cast<int>(inStr.length());
    unsigned char hdr[4];

    hdr[0] = ((len + 4) >> 8) & 0xff;
    hdr[1] = (len + 4) & 0xff;
    hdr[2] = (record >> 8) & 0xff;
    hdr[3] = record & 0xff;

    fwrite(hdr, 1, 4, m_gdsFile);
    fwrite(inStr.c_str(), 1, static_cast<size_t>(len), m_gdsFile);

    return 0;
}

/*!********************************************************************************************************************
 * \brief Writes UNITS record to GDS file.
 *********************************************************************************************************************/
void GdsReader::writeUnits() const
{
    static const unsigned char data[20] = {
        0x00, 0x14, 0x03, 0x05,
        0x3e, 0x41, 0x89, 0x37,
        0x4b, 0xc6, 0xa7, 0xf0,
        0x39, 0x44, 0xb8, 0x2f,
        0xa0, 0x9b, 0x5a, 0x50
    };

    fwrite(data, 1, 20, m_gdsFile);
}

/*!********************************************************************************************************************
 * \brief Writes a zero-payload GDS record.
 * \param record GDS record type.
 * \return 0 on success, non-zero on error.
 *********************************************************************************************************************/
int GdsReader::writeRec(int record) const
{
    unsigned char hdr[4] = {
        0x00, 0x04,
        static_cast<unsigned char>((record >> 8) & 0xff),
        static_cast<unsigned char>(record & 0xff)
    };

    if (hdr[3] != 0) {
        m_errorList << "Record contains no data";
        return 1;
    }

    fwrite(hdr, 1, 4, m_gdsFile);
    return 0;
}

// =====================================================================================================================
// Reader part
// =====================================================================================================================

/*!********************************************************************************************************************
 * \brief Reads one GDS record from file.
 * \param f Opened GDS file pointer.
 * \param recType Output record type.
 * \param payload Output record payload.
 * \return true if record was read successfully.
 *********************************************************************************************************************/
bool GdsReader::readRecord(FILE *f, quint16 &recType, QByteArray &payload)
{
    unsigned char hdr[4];
    if (fread(hdr, 1, 4, f) != 4) {
        return false;
    }

    quint16 len = (hdr[0] << 8) | hdr[1];
    recType = (hdr[2] << 8) | hdr[3];

    if (len < 4) {
        return false;
    }

    payload.resize(len - 4);
    if (!payload.isEmpty()) {
        if (fread(payload.data(), 1, payload.size(), f) != static_cast<size_t>(payload.size())) {
            return false;
        }
    }

    return true;
}

/*!********************************************************************************************************************
 * \brief Decodes GDS string payload to QString.
 * \param payload Raw GDS string payload.
 * \return Decoded string without padding zeros.
 *********************************************************************************************************************/
QString GdsReader::decodeGdsString(const QByteArray &payload) const
{
    int n = payload.size();
    while (n > 0 && payload[n - 1] == '\0') {
        --n;
    }
    return QString::fromLatin1(payload.constData(), n);
}

/*!********************************************************************************************************************
 * \brief Reads only the 4-byte GDS record header (length + record type).
 *
 * This function is used for fast scanning of large GDS files. It allows the caller to decide whether the payload
 * must be read (for interesting records like STRNAME/SNAME) or skipped via fseek() for non-interesting records.
 *
 * \param f Opened GDS file pointer.
 * \param recType Output record type.
 * \param len Output record total length in bytes (including 4-byte header).
 * \return true if header was read successfully and len is valid (>= 4).
 *********************************************************************************************************************/
bool GdsReader::readRecordHeader(FILE *f, quint16 &recType, quint16 &len)
{
    unsigned char hdr[4];
    if (fread(hdr, 1, 4, f) != 4) {
        return false;
    }

    len = (hdr[0] << 8) | hdr[1];
    recType = (hdr[2] << 8) | hdr[3];

    return (len >= 4);
}

/*!********************************************************************************************************************
 * \brief Reads payload bytes of a given length into QByteArray.
 *
 * The function resizes the output buffer to payloadLen and reads exactly payloadLen bytes from the file.
 * For payloadLen <= 0, it returns true without reading.
 *
 * \param f Opened GDS file pointer.
 * \param payloadLen Number of payload bytes to read (record length minus 4-byte header).
 * \param payload Output buffer that will contain the payload.
 * \return true if payload was read successfully or payloadLen <= 0, otherwise false.
 *********************************************************************************************************************/
bool GdsReader::readPayload(FILE *f, int payloadLen, QByteArray &payload)
{
    payload.resize(payloadLen);
    if (payloadLen <= 0) {
        return true;
    }
    return fread(payload.data(), 1, static_cast<size_t>(payloadLen), f) == static_cast<size_t>(payloadLen);
}


/*!********************************************************************************************************************
 * \brief Reads hierarchy information from a GDS file.
 * \param out Output hierarchy structure.
 * \return true if hierarchy was successfully read.
 *********************************************************************************************************************/
bool GdsReader::readHierarchy(GdsHierarchy &out)
{
    out.topCells.clear();
    out.children.clear();
    out.allCells.clear();
    m_errorList.clear();

    FILE *f = fopen(m_fileName.toStdString().c_str(), "rb");
    if (!f) {
        m_errorList << QString("Failed to open GDS for read: '%1'").arg(m_fileName);
        return false;
    }

    QString currentCell;
    bool inRef = false;
    QSet<QString> referenced;

    quint16 recType = 0;
    quint16 len = 0;
    QByteArray payload;

    while (readRecordHeader(f, recType, len)) {

        const int payloadLen = static_cast<int>(len) - 4;
        if (payloadLen < 0) {
            break;
        }

        const bool needPayload =
            (recType == GDS_STRNAME) ||
            (recType == GDS_SREF)   ||
            (recType == GDS_AREF)   ||
            (recType == GDS_SNAME)  ||
            (recType == GDS_ENDEL)  ||
            (recType == GDS_ENDLIB);

        if (needPayload) {
            if (!readPayload(f, payloadLen, payload)) {
                break;
            }

            if (recType == GDS_STRNAME) {
                currentCell = decodeGdsString(payload);
                out.allCells.insert(currentCell);
                out.children[currentCell]; // create empty entry
                inRef = false;
            }
            else if (recType == GDS_SREF || recType == GDS_AREF) {
                inRef = true;
            }
            else if (recType == GDS_SNAME && inRef && !currentCell.isEmpty()) {
                const QString ref = decodeGdsString(payload);
                out.children[currentCell] << ref;
                out.allCells.insert(ref);
                referenced.insert(ref);
            }
            else if (recType == GDS_ENDEL) {
                inRef = false;
            }
            else if (recType == GDS_ENDLIB) {
                break;
            }
        }
        else {
            // Skip geometry/xy/etc without allocating or reading into RAM
            if (payloadLen > 0) {
                if (fseek(f, payloadLen, SEEK_CUR) != 0) {
                    break;
                }
            }
        }
    }

    fclose(f);

    QSet<QString> top = out.allCells;
    for (const QString &r : referenced) {
        top.remove(r);
    }

    out.topCells = QStringList(top.begin(), top.end());
    out.topCells.sort();

    for (auto it = out.children.begin(); it != out.children.end(); ++it) {
        it.value().removeDuplicates();
        it.value().sort();
    }

    return true;
}
