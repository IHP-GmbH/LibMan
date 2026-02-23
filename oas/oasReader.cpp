#include "oasreader.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QByteArray>
#include <QElapsedTimer>

#include <cstdint>
#include <cstring>
#include <limits>
#include <cstdlib>

#include <zlib.h>

#ifndef OAS_TRACE
#define OAS_TRACE 1
#endif

/*!********************************************************************************************************************
 * \brief Lightweight cursor over a memory-mapped OASIS byte buffer.
 *
 * Holds the current pointer \c p and the end pointer \c end and provides a bounds check helper.
 *********************************************************************************************************************/
struct OasCursor
{
    const uchar *p   = nullptr;
    const uchar *end = nullptr;

    /*!****************************************************************************************************************
     * \brief Checks whether at least \p n bytes are available from the current cursor position.
     * \param n Number of bytes required.
     * \return True if \p n bytes can be read safely, false otherwise.
     *****************************************************************************************************************/
    bool has(int n) const { return p && (p + n <= end); }
};

/*!********************************************************************************************************************
 * \brief Produces a hex dump of up to \p maxBytes from a buffer range.
 *
 * The function converts the first \p maxBytes bytes starting at \p p into a space-separated hex string.
 *
 * \param p Pointer to the start of the buffer range.
 * \param end Pointer one past the end of the buffer range.
 * \param maxBytes Maximum number of bytes to dump.
 * \return Hex-encoded string with spaces between bytes.
 *********************************************************************************************************************/
static inline QString hexDumpN(const uchar *p, const uchar *end, int maxBytes = 64)
{
    const int n = int(std::min<qint64>(maxBytes, end - p));
    QByteArray b(reinterpret_cast<const char*>(p), n);
    return b.toHex(' ');
}

#if OAS_TRACE
static inline void traceRecBegin(QStringList &errors,
                                 const OasParseState &st,
                                 const uchar *recStart,
                                 quint64 recId,
                                 const uchar *pAfterId,
                                 const uchar *end)
{
    errors << QString("OAS REC BEGIN off=%1 id=%2 next=%3")
    .arg(qulonglong(recStart - st.fileBase))
        .arg(qulonglong(recId))
        .arg(hexDumpN(pAfterId, end, 32));
}

static inline void traceRecEnd(QStringList &errors,
                               const OasParseState &st,
                               const uchar *recStart,
                               quint64 recId,
                               const uchar *recEnd,
                               const uchar *end,
                               const char *status)
{
    errors << QString("OAS REC %1 off=%2 id=%3 consumed=%4 tail=%5")
    .arg(status)
        .arg(qulonglong(recStart - st.fileBase))
        .arg(qulonglong(recId))
        .arg(qulonglong(recEnd - recStart))
        .arg(hexDumpN(recEnd, end, 32));
}

static inline void traceField(QStringList &errors,
                              const OasParseState &st,
                              const char *what,
                              const uchar *at,
                              const uchar *end,
                              const QString &extra = QString())
{
    if (extra.isEmpty()) {
        errors << QString("  FIELD %1 off=%2 head=%3")
        .arg(what)
            .arg(qulonglong(at - st.fileBase))
            .arg(hexDumpN(at, end, 24));
    } else {
        errors << QString("  FIELD %1 off=%2 %3 head=%4")
        .arg(what)
            .arg(qulonglong(at - st.fileBase))
            .arg(extra)
            .arg(hexDumpN(at, end, 24));
    }
}
#endif

/*!********************************************************************************************************************
 * \brief Reads a single byte and advances the cursor.
 *
 * \param c Cursor to read from.
 * \param out Output byte.
 * \return True on success, false if out of bounds.
 *********************************************************************************************************************/
static inline bool readByte(OasCursor &c, uchar &out)
{
    if (!c.has(1)) return false;
    out = *c.p++;
    return true;
}

/*!********************************************************************************************************************
 * \brief Reads an OASIS unsigned-integer (varint) and advances the cursor.
 *
 * OASIS uses a 7-bit payload per byte with continuation bit 0x80.
 *
 * \param c Cursor to read from.
 * \param out Decoded value.
 * \return True on success, false on out-of-bounds or overflow.
 *********************************************************************************************************************/
static inline bool readUInt(OasCursor &c, quint64 &out)
{
    out = 0;
    int shift = 0;

    for (;;) {
        if (!c.has(1)) return false;
        uchar b = *c.p++;
        out |= (quint64(b & 0x7F) << shift);
        if ((b & 0x80) == 0) break;
        shift += 7;
        if (shift > 63) return false;
    }
    return true;
}

/*!********************************************************************************************************************
 * \brief Heuristic validation for a decoded cell name.
 *
 * Rejects empty strings, excessively large strings, control characters, and U+FFFD replacement characters.
 *
 * \param s Candidate cell name.
 * \return True if the string looks reasonable, false otherwise.
 *********************************************************************************************************************/
static bool isLikelyValidCellName(const QString& s)
{
    if (s.isEmpty()) return false;
    if (s.size() > 4096) return false;

    for (QChar ch : s) {
        ushort u = ch.unicode();

        if (u < 0x20 || u == 0x7F) return false;
        if (u == 0xFFFD) return false;
    }

    return true;
}

/*!********************************************************************************************************************
 * \brief Reads an OASIS signed-integer and advances the cursor.
 *
 * OASIS signed-integer is encoded via an unsigned varint with zigzag-like mapping:
 *   out = (u & 1) ? -((u + 1) >> 1) : (u >> 1)
 *
 * \param c Cursor to read from.
 * \param out Decoded signed value.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool readSInt(OasCursor &c, qint64 &out)
{
    quint64 u = 0;
    if (!readUInt(c, u)) return false;
    out = (u & 1ULL) ? -qint64((u + 1ULL) >> 1) : qint64(u >> 1);
    return true;
}

/*!********************************************************************************************************************
 * \brief Reads a length-prefixed byte string and advances the cursor.
 *
 * \param c Cursor to read from.
 * \param out Output byte array.
 * \return True on success, false if out-of-bounds.
 *********************************************************************************************************************/
static inline bool readString(OasCursor &c, QByteArray &out)
{
    quint64 n = 0;
    if (!readUInt(c, n)) return false;
    if (n > quint64(c.end - c.p)) return false;
    out = QByteArray(reinterpret_cast<const char*>(c.p), int(n));
    c.p += int(n);
    return true;
}

/*!********************************************************************************************************************
 * \brief Reads an A-string (Latin-1) and advances the cursor.
 *
 * \param c Cursor to read from.
 * \param out Output string.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool readAString(OasCursor &c, QString &out)
{
    QByteArray b;
    if (!readString(c, b)) return false;
    out = QString::fromLatin1(b);
    return true;
}

/*!********************************************************************************************************************
 * \brief Reads an N-string (name string) and advances the cursor.
 *
 * Primary decoding is UTF-8; if replacement characters are present, a Latin-1 fallback is used.
 *
 * \param c Cursor to read from.
 * \param out Output string.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool readNString(OasCursor &c, QString &out)
{
    QByteArray b;
    if (!readString(c, b)) return false;

    QString s = QString::fromUtf8(b.constData(), b.size());

    if (s.contains(QChar(0xFFFD))) {
        s = QString::fromLatin1(b.constData(), b.size());
    }

    out = s;
    return true;
}

/*!********************************************************************************************************************
 * \brief Skips an OASIS real value and advances the cursor.
 *
 * The encoding starts with a type code (0..7) followed by payload according to the OASIS spec.
 *
 * \param c Cursor to advance.
 * \return True on success, false on parse failure or out-of-bounds.
 *********************************************************************************************************************/
static inline bool skipReal(OasCursor &c)
{
    quint64 t = 0;
    if (!readUInt(c, t)) return false;
    switch (t) {
    case 0: { quint64 x; return readUInt(c, x); }
    case 1: { quint64 x; return readUInt(c, x); }
    case 2: { quint64 d; return readUInt(c, d); }
    case 3: { quint64 d; return readUInt(c, d); }
    case 4: { quint64 n,d; return readUInt(c,n) && readUInt(c,d); }
    case 5: { quint64 n,d; return readUInt(c,n) && readUInt(c,d); }
    case 6: { if (!c.has(4)) return false; c.p += 4; return true; }
    case 7: { if (!c.has(8)) return false; c.p += 8; return true; }
    default:
        return false;
    }
}

/*!********************************************************************************************************************
 * \brief Skips a 1-delta encoded value and advances the cursor.
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skip1Delta(OasCursor &c)
{
    qint64 v;
    return readSInt(c, v);
}

/*!********************************************************************************************************************
 * \brief Skips a 2-delta encoded value and advances the cursor.
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skip2Delta(OasCursor &c)
{
    quint64 v;
    return readUInt(c, v);
}

/*!********************************************************************************************************************
 * \brief Skips a 3-delta encoded value and advances the cursor.
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skip3Delta(OasCursor &c)
{
    qint64 v;
    return readSInt(c, v);
}

/*!********************************************************************************************************************
 * \brief Skips a g-delta encoded value and advances the cursor.
 *
 * g-delta is stored as an unsigned-integer. If the LSB is 1, a second unsigned-integer follows.
 *
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skipGDelta(OasCursor &c)
{
    quint64 a = 0;
    if (!readUInt(c, a)) return false;
    if (a & 1ULL) {
        quint64 b = 0;
        return readUInt(c, b);
    }
    return true;
}

/*!********************************************************************************************************************
 * \brief Skips an OASIS point-list and advances the cursor.
 *
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skipPointList(OasCursor &c)
{
    quint64 ptType = 0;
    if (!readUInt(c, ptType)) return false;

    quint64 count = 0;
    if (!readUInt(c, count)) return false;

    switch (ptType) {
    case 0:
    case 1:
        for (quint64 i = 0; i < count; i++) if (!skip1Delta(c)) return false;
        return true;

    case 2:
        for (quint64 i = 0; i < count; i++) if (!skip2Delta(c)) return false;
        return true;

    case 3:
        for (quint64 i = 0; i < count; i++) {
            if (!skip3Delta(c)) return false;
            if (!skip3Delta(c)) return false;
        }
        return true;

    case 4:
        for (quint64 i = 0; i < count; i++) if (!skipGDelta(c)) return false;
        return true;

    default:
        return false;
    }
}

/*!********************************************************************************************************************
 * \brief Skips an OASIS repetition structure and advances the cursor.
 *
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skipRepetition(OasCursor &c)
{
    quint64 repType = 0;
    if (!readUInt(c, repType)) return false;

    switch (repType) {
    case 0:
        return true;

    case 1: {
        quint64 nx = 0, ny = 0;
        qint64 dx = 0, dy = 0;
        return readUInt(c, nx) && readUInt(c, ny) && readSInt(c, dx) && readSInt(c, dy);
    }

    case 2: {
        quint64 n = 0;
        qint64 dx = 0, dy = 0;
        return readUInt(c, n) && readSInt(c, dx) && readSInt(c, dy);
    }

    case 3: {
        quint64 n = 0;
        return readUInt(c, n) && skipPointList(c);
    }

    default:
        return false;
    }
}

/*!********************************************************************************************************************
 * \brief Skips an OASIS interval and advances the cursor.
 *
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skipInterval(OasCursor &c)
{
    quint64 type = 0;
    if (!readUInt(c, type)) return false;
    quint64 a = 0, b = 0;

    switch (type) {
    case 0: return true;
    case 1: return readUInt(c, a);
    case 2: return readUInt(c, a);
    case 3: return readUInt(c, a);
    case 4: return readUInt(c, a) && readUInt(c, b);
    default:
        return false;
    }
}

/*!********************************************************************************************************************
 * \brief Heuristically skips an XGEOMETRY-like vendor record payload.
 *
 * Attempts to parse a common pattern and advances the cursor only if the record looks plausible.
 *
 * \param c Cursor to attempt advancing.
 * \return True if the cursor was advanced, false otherwise.
 *********************************************************************************************************************/
static inline bool trySkipXGeometryLike(OasCursor &c)
{
    OasCursor t = c;

    uchar info = 0;
    if (!readByte(t, info)) return false;

    quint64 a = 0, layer = 0, dtype = 0;
    if (!readUInt(t, a))     return false;
    if (!readUInt(t, layer)) return false;
    if (!readUInt(t, dtype)) return false;

    if (layer > 1000000ULL || dtype > 1000000ULL) return false;

    QByteArray blob;
    if (!readString(t, blob)) return false;

    if (blob.size() > 10 * 1024 * 1024) return false;

    qint64 x = 0, y = 0;
    if (!readSInt(t, x)) return false;
    if (!readSInt(t, y)) return false;

    if (std::llabs(x) > (1LL << 50) || std::llabs(y) > (1LL << 50)) return false;

    c = t;
    return true;
}

/*!********************************************************************************************************************
 * \brief Heuristically skips an XNAME/XELEMENT-like vendor record payload.
 *
 * Tries to parse a pattern: unsigned-int followed by a length-prefixed byte string.
 *
 * \param c Cursor to attempt advancing.
 * \return True if the cursor was advanced, false otherwise.
 *********************************************************************************************************************/
static inline bool trySkipXNameLike(OasCursor &c)
{
    OasCursor t = c;

    quint64 n = 0;
    if (!readUInt(t, n)) return false;

    QByteArray s;
    if (!readString(t, s)) return false;

    if (n > (1ULL << 40)) return false;
    if (s.size() > 32 * 1024 * 1024) return false;

    c = t;
    return true;
}

/*!********************************************************************************************************************
 * \brief Skips a property value and advances the cursor.
 *
 * Handles both real types (0..7) and additional property encodings (8..15 subset).
 *
 * \param c Cursor to advance.
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static inline bool skipPropertyValue(OasCursor &c)
{
    quint64 t = 0;
    if (!readUInt(c, t)) return false;

    if (t <= 7) {
        return skipReal(c);
    }

    switch (t) {
    case 8:  { quint64 u; return readUInt(c, u); }
    case 9:  { qint64 s;  return readSInt(c, s); }
    case 10: { QString x; return readAString(c, x); }
    case 11: { QByteArray b; return readString(c, b); }
    case 12: { QString n; return readNString(c, n); }
    case 13: { quint64 rn; return readUInt(c, rn); }
    case 14: { quint64 rn; return readUInt(c, rn); }
    case 15: { quint64 rn; return readUInt(c, rn); }
    default:
        return false;
    }
}

/*!********************************************************************************************************************
 * \brief Constructs an OASIS reader for a file.
 * \param fileName Path to the OASIS file.
 *********************************************************************************************************************/
oasReader::oasReader(const QString &fileName)
    : m_fileName(fileName)
{
    m_errorList.clear();
}

/*!********************************************************************************************************************
 * \brief Returns a copy of the collected error messages.
 * \return Error list.
 *********************************************************************************************************************/
QStringList oasReader::getErrors() const
{
    return m_errorList;
}

/*!********************************************************************************************************************
 * \brief Inflates raw DEFLATE data into a pre-sized output buffer.
 *
 * Uses zlib raw mode (windowBits = -MAX_WBITS) and validates that the produced output size matches \p expectedOutLen.
 *
 * \param src Pointer to compressed data.
 * \param srcLen Size of compressed data in bytes.
 * \param expectedOutLen Expected uncompressed size in bytes.
 * \param out Output buffer (resized to \p expectedOutLen).
 * \return True on success, false otherwise.
 *********************************************************************************************************************/
static bool inflateRawDeflate(const uchar *src, int srcLen, int expectedOutLen, QByteArray &out)
{
    out.clear();
    out.resize(expectedOutLen);

    z_stream zs;
    std::memset(&zs, 0, sizeof(zs));
    zs.next_in   = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(src));
    zs.avail_in  = uInt(srcLen);
    zs.next_out  = reinterpret_cast<Bytef*>(out.data());
    zs.avail_out = uInt(expectedOutLen);

    if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
        return false;
    }

    const int rc = inflate(&zs, Z_FINISH);
    inflateEnd(&zs);

    if (rc != Z_STREAM_END) {
        return false;
    }

    if (int(zs.total_out) != expectedOutLen) {
        return false;
    }

    return true;
}

/*!********************************************************************************************************************
 * \brief Parser state carried across record parsing.
 *********************************************************************************************************************/
struct OasParseState
{
    QHash<quint64, QString> cellNameByRef;

    const uchar *fileBase = nullptr;

    quint64 nextCellNameRef = 0;
    bool    explicitCellNameRefsSeen = false;
    bool    implicitCellNameRefsSeen = false;

    QString currentCell;
    QString modalPlacementCell;

    bool    seenEnd = false;

#if OAS_TRACE
    int     traceLimit = 2000;
    int     traceCount = 0;
#endif
};

/*!********************************************************************************************************************
 * \brief Dumps a string as a sequence of U+XXXX code points.
 * \param s Input string.
 * \return Space-separated list of code points.
 *********************************************************************************************************************/
static inline QString dumpU16(const QString& s)
{
    QStringList parts;
    parts.reserve(s.size());
    for (QChar ch : s)
        parts << QString("U+%1").arg(ch.unicode(), 4, 16, QLatin1Char('0'));
    return parts.join(' ');
}

/*!********************************************************************************************************************
 * \brief Parses a buffer of OASIS records into a hierarchy model.
 * \param c Cursor over the buffer.
 * \param out Output hierarchy.
 * \param errors Error list (append-only).
 * \param st Parser state.
 * \return True on success, false on parse failure.
 *********************************************************************************************************************/
static bool parseBuffer(OasCursor c,
                        LayoutHierarchy &out,
                        QStringList &errors,
                        OasParseState &st);

#if OAS_TRACE
static inline bool readUIntT(OasCursor &c, quint64 &out,
                             QStringList &errors, const OasParseState &st, const char *tag)
{
    const uchar *at = c.p;
    traceField(errors, st, tag, at, c.end);
    if (!readUInt(c, out)) return false;
    errors << QString("  FIELD %1 = %2").arg(tag).arg(qulonglong(out));
    return true;
}

static inline bool readSIntT(OasCursor &c, qint64 &out,
                             QStringList &errors, const OasParseState &st, const char *tag)
{
    const uchar *at = c.p;
    traceField(errors, st, tag, at, c.end);
    if (!readSInt(c, out)) return false;
    errors << QString("  FIELD %1 = %2").arg(tag).arg(qlonglong(out));
    return true;
}

static inline bool readByteT(OasCursor &c, uchar &out,
                             QStringList &errors, const OasParseState &st, const char *tag)
{
    const uchar *at = c.p;
    traceField(errors, st, tag, at, c.end);
    if (!readByte(c, out)) return false;
    errors << QString("  FIELD %1 = 0x%2").arg(tag).arg(int(out), 2, 16, QLatin1Char('0'));
    return true;
}

static inline bool readStringT(OasCursor &c, QByteArray &out,
                               QStringList &errors, const OasParseState &st, const char *tag, int hexLimit = 128)
{
    const uchar *at = c.p;
    traceField(errors, st, tag, at, c.end);

    quint64 n = 0;
    if (!readUIntT(c, n, errors, st, "len(varint)")) return false;
    if (n > quint64(c.end - c.p)) return false;

    out = QByteArray(reinterpret_cast<const char*>(c.p), int(n));
    c.p += int(n);

    const QByteArray shown = (out.size() > hexLimit) ? out.left(hexLimit) : out;
    errors << QString("  FIELD %1 bytes=%2%3")
                  .arg(tag)
                  .arg(QString(shown.toHex(' ')))
                  .arg(out.size() > hexLimit ? " ..." : "");
    return true;
}
#endif

/*!********************************************************************************************************************
 * \brief Parses a single OASIS record and advances the cursor.
 *
 * \param c Cursor positioned at the record start; advanced past the record on success.
 * \param out Output hierarchy.
 * \param errors Error list (append-only).
 * \param st Parser state.
 * \return True on successful parsing/skipping, false on failure.
 *********************************************************************************************************************/
static bool parseOneRecord(OasCursor &c,
                           LayoutHierarchy &out,
                           QStringList &errors,
                           OasParseState &st)
{
    const uchar *recStart = c.p;

    quint64 recId = 0;
#if OAS_TRACE
    if (!readUIntT(c, recId, errors, st, "recId(varint)")) return false;
#else
    if (!readUInt(c, recId)) return false;
#endif

#if OAS_TRACE
    if (st.traceCount < st.traceLimit) {
        traceRecBegin(errors, st, recStart, recId, c.p, c.end);
    }
#endif

#if OAS_TRACE
    if (st.traceCount < st.traceLimit) {
        errors << QString("TRACE #%1 off=%2 recId=%3 head=%4")
        .arg(st.traceCount)
            .arg(qulonglong(recStart - st.fileBase))
            .arg(qulonglong(recId))
            .arg(hexDumpN(c.p, c.end, 32));
    }
#endif

    switch (recId) {
    case 0:
        goto done_ok;

    case 1: {
        QString version;
        if (!readAString(c, version)) goto done_fail;
        if (!skipReal(c)) goto done_fail;

        quint64 offsetFlag = 0;
        if (!readUInt(c, offsetFlag)) goto done_fail;

        if (offsetFlag == 0) {
            for (int i = 0; i < 12; ++i) {
                quint64 tmp = 0;
                if (!readUInt(c, tmp)) goto done_fail;
            }
        } else {
            for (int i = 0; i < 12; ++i) {
                qint64 tmp = 0;
                if (!readSInt(c, tmp)) goto done_fail;
            }
        }

        goto done_ok;
    }

    case 2: {
        st.seenEnd = true;
        goto done_ok;
    }

    case 3: {
        st.implicitCellNameRefsSeen = true;
        if (st.explicitCellNameRefsSeen) {
            errors << "CELLNAME mixed implicit/explicit reference-number mode (3/4).";
            goto done_fail;
        }

        QByteArray raw;
        if (!readString(c, raw)) goto done_fail;

        QString nameUtf8 = QString::fromUtf8(raw.constData(), raw.size());
        QString nameLat1 = QString::fromLatin1(raw.constData(), raw.size());
        errors << QString("CELLNAME(implicit) bytes=%1 utf8='%2' latin1='%3'")
                      .arg(QString(raw.toHex(' ')))
                      .arg(nameUtf8)
                      .arg(nameLat1);

        QString name = nameUtf8.contains(QChar(0xFFFD)) ? nameLat1 : nameUtf8;

        if (!isLikelyValidCellName(name)) {
            goto done_ok;
        }

        st.cellNameByRef.insert(st.nextCellNameRef++, name);
        out.allCells.insert(name);
        out.children[name];
        goto done_ok;
    }

    case 4: {
        st.explicitCellNameRefsSeen = true;
        if (st.implicitCellNameRefsSeen) {
            errors << "CELLNAME mixed implicit/explicit reference-number mode (3/4).";
            goto done_fail;
        }

        QByteArray raw;
        if (!readString(c, raw)) goto done_fail;

        quint64 rn = 0;
        if (!readUInt(c, rn)) goto done_fail;

        QString nameUtf8 = QString::fromUtf8(raw.constData(), raw.size());
        QString nameLat1 = QString::fromLatin1(raw.constData(), raw.size());
        errors << QString("CELLNAME(explicit) rn=%1 bytes=%2 utf8='%3' latin1='%4'")
                      .arg(rn)
                      .arg(QString(raw.toHex(' ')))
                      .arg(nameUtf8)
                      .arg(nameLat1);

        QString name = nameUtf8.contains(QChar(0xFFFD)) ? nameLat1 : nameUtf8;

        st.cellNameByRef.insert(rn, name);
        out.allCells.insert(name);
        out.children[name];
        goto done_ok;
    }

    case 5:  { QString s; if (!readAString(c, s)) goto done_fail; goto done_ok; }
    case 6:  { QString s; quint64 rn = 0; if (!readAString(c, s)) goto done_fail; if (!readUInt(c, rn)) goto done_fail; goto done_ok; }
    case 7:  { QString s; if (!readNString(c, s)) goto done_fail; goto done_ok; }
    case 8:  { QString s; quint64 rn = 0; if (!readNString(c, s)) goto done_fail; if (!readUInt(c, rn)) goto done_fail; goto done_ok; }
    case 9:  { QString s; if (!readAString(c, s)) goto done_fail; goto done_ok; }
    case 10: { QString s; quint64 rn = 0; if (!readAString(c, s)) goto done_fail; if (!readUInt(c, rn)) goto done_fail; goto done_ok; }

    case 11: {
        QString s;
        if (!readNString(c, s)) goto done_fail;
        if (!skipInterval(c)) goto done_fail;
        if (!skipInterval(c)) goto done_fail;
        goto done_ok;
    }

    case 12: {
        QString s;
        if (!readNString(c, s)) goto done_fail;
        if (!skipInterval(c)) goto done_fail;
        if (!skipInterval(c)) goto done_fail;
        if (!skipInterval(c)) goto done_fail;
        if (!skipInterval(c)) goto done_fail;
        goto done_ok;
    }

    case 13: {
        quint64 rn = 0;
        if (!readUInt(c, rn)) goto done_fail;

        const QString name = st.cellNameByRef.value(rn);
        if (name.isEmpty()) {
            errors << QString("CELL references unknown CELLNAME ref=%1").arg(rn);
            goto done_fail;
        }

        st.currentCell = name;
        out.allCells.insert(name);
        out.children[name];
        st.modalPlacementCell.clear();
        goto done_ok;
    }

    case 14: {
        QByteArray raw;
        if (!readString(c, raw)) goto done_fail;

        QString utf8 = QString::fromUtf8(raw.constData(), raw.size());
        QString lat1 = QString::fromLatin1(raw.constData(), raw.size());
        QString name = utf8.contains(QChar(0xFFFD)) ? lat1 : utf8;

        errors << QString("CELL(name) bytes=%1 utf8='%2' lat1='%3' chosen='%4' u16=[%5]")
                      .arg(QString(raw.toHex(' ')))
                      .arg(utf8).arg(lat1).arg(name)
                      .arg(dumpU16(name));

        st.currentCell = name;
        out.allCells.insert(name);
        out.children[name];
        st.modalPlacementCell.clear();
        goto done_ok;
    }

    case 15:
    case 16:
        goto done_ok;

    case 17: {
        if (st.currentCell.isEmpty()) {
            errors << "PLACEMENT outside of CELL.";
            goto done_fail;
        }

        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool R    = (info & 0x01) != 0;
        const bool Y    = (info & 0x02) != 0;
        const bool X    = (info & 0x04) != 0;
        const bool N    = (info & 0x08) != 0;
        const bool Cbit = (info & 0x10) != 0;

        QString placedCell;

        if (Cbit) {
            if (N) {
                quint64 rn = 0;
                if (!readUInt(c, rn)) goto done_fail;
                placedCell = st.cellNameByRef.value(rn);
                if (placedCell.isEmpty()) {
                    errors << QString("PLACEMENT references unknown CELLNAME ref=%1").arg(rn);
                    goto done_fail;
                }
            } else {
                if (!readNString(c, placedCell)) goto done_fail;
            }
            st.modalPlacementCell = placedCell;
        } else {
            placedCell = st.modalPlacementCell;
        }

        if (X) { qint64 x; if (!readSInt(c, x)) goto done_fail; }
        if (Y) { qint64 y; if (!readSInt(c, y)) goto done_fail; }

        if (R) {
            if (!skipRepetition(c)) goto done_fail;
        }

        if (!placedCell.isEmpty()) {
            out.children[st.currentCell] << placedCell;
            out.allCells.insert(placedCell);
        }

        goto done_ok;
    }

    case 18: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool R    = (info & 0x01) != 0;
        const bool T    = (info & 0x02) != 0;
        const bool Y    = (info & 0x04) != 0;
        const bool X    = (info & 0x08) != 0;
        const bool N    = (info & 0x10) != 0;
        const bool Cbit = (info & 0x20) != 0;

        if (Cbit) {
            if (N) { quint64 rn = 0; if (!readUInt(c, rn)) goto done_fail; }
            else   { QString s; if (!readAString(c, s)) goto done_fail; }
        }

        if (X) { qint64 x; if (!readSInt(c, x)) goto done_fail; }
        if (Y) { qint64 y; if (!readSInt(c, y)) goto done_fail; }

        if (T) { quint64 tt; if (!readUInt(c, tt)) goto done_fail; }
        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 19:
        errors << "Unexpected record id 19 (unsupported mapping).";
        goto done_fail;

    case 20: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool L = (info & 0x01) != 0;
        const bool D = (info & 0x02) != 0;
        const bool R = (info & 0x04) != 0;
        const bool Y = (info & 0x08) != 0;
        const bool X = (info & 0x10) != 0;
        const bool H = (info & 0x20) != 0;
        const bool W = (info & 0x40) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (W) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (H) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 21: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool L = (info & 0x01) != 0;
        const bool D = (info & 0x02) != 0;
        const bool R = (info & 0x04) != 0;
        const bool Y = (info & 0x08) != 0;
        const bool X = (info & 0x10) != 0;
        const bool P = (info & 0x20) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (P) { if (!skipPointList(c)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 22: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool L = (info & 0x01) != 0;
        const bool D = (info & 0x02) != 0;
        const bool R = (info & 0x04) != 0;
        const bool Y = (info & 0x08) != 0;
        const bool X = (info & 0x10) != 0;
        const bool P = (info & 0x20) != 0;
        const bool W = (info & 0x40) != 0;
        const bool E = (info & 0x80) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (W) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (P) { if (!skipPointList(c)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }

        if (E) {
            quint64 a = 0, b = 0;
            if (!readUInt(c, a)) goto done_fail;
            if (!readUInt(c, b)) goto done_fail;
        }

        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 23: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool L   = (info & 0x01) != 0;
        const bool D   = (info & 0x02) != 0;
        const bool R   = (info & 0x04) != 0;
        const bool Y   = (info & 0x08) != 0;
        const bool X   = (info & 0x10) != 0;
        const bool H   = (info & 0x20) != 0;
        const bool W   = (info & 0x40) != 0;
        const bool Dlt = (info & 0x80) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (W) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (H) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }

        if (Dlt) {
            if (!skip1Delta(c)) goto done_fail;
            if (!skip1Delta(c)) goto done_fail;
        }

        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 24: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool L = (info & 0x01) != 0;
        const bool D = (info & 0x02) != 0;
        const bool R = (info & 0x04) != 0;
        const bool Y = (info & 0x08) != 0;
        const bool X = (info & 0x10) != 0;
        const bool H = (info & 0x20) != 0;
        const bool W = (info & 0x40) != 0;
        const bool T = (info & 0x80) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (W) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (H) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (T) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 25: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool L = (info & 0x01) != 0;
        const bool D = (info & 0x02) != 0;
        const bool R = (info & 0x04) != 0;
        const bool Y = (info & 0x08) != 0;
        const bool X = (info & 0x10) != 0;
        const bool W = (info & 0x20) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (W) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

    case 28: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        const bool S    = (info & 0x01) != 0;
        const bool N    = (info & 0x02) != 0;
        const bool Cbit = (info & 0x04) != 0;
        const bool V    = (info & 0x08) != 0;

        const quint64 UUUU = (quint64(info) >> 4) & 0x0F;

        if (Cbit) {
            if (N) { quint64 rn = 0; if (!readUInt(c, rn)) goto done_fail; }
            else   { QString s; if (!readNString(c, s)) goto done_fail; }
        }

        if (V) {
            Q_UNUSED(S);
            goto done_ok;
        }

        quint64 cnt = 0;
        if (UUUU < 15) cnt = UUUU;
        else { if (!readUInt(c, cnt)) goto done_fail; }

        for (quint64 i = 0; i < cnt; ++i) {
            if (!skipPropertyValue(c)) goto done_fail;
        }

        Q_UNUSED(S);
        goto done_ok;
    }

    case 29:
        goto done_ok;

    case 30: {
        quint64 attr = 0;
        if (!readUInt(c, attr)) goto done_fail;
        QByteArray b;
        if (!readString(c, b)) goto done_fail;
        Q_UNUSED(attr);
        goto done_ok;
    }

    case 31: {
        quint64 attr = 0, rn = 0;
        if (!readUInt(c, attr)) goto done_fail;
        QByteArray b;
        if (!readString(c, b)) goto done_fail;
        if (!readUInt(c, rn)) goto done_fail;
        Q_UNUSED(attr);
        goto done_ok;
    }

    case 32: {
        quint64 attr = 0;
        if (!readUInt(c, attr)) goto done_fail;
        QByteArray b;
        if (!readString(c, b)) goto done_fail;
        Q_UNUSED(attr);
        goto done_ok;
    }

    case 33: {
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        quint64 attr = 0;
        if (!readUInt(c, attr)) goto done_fail;

        const bool L = (info & 0x01) != 0;
        const bool D = (info & 0x02) != 0;
        const bool R = (info & 0x04) != 0;
        const bool Y = (info & 0x08) != 0;
        const bool X = (info & 0x10) != 0;

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (X) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (Y) { qint64 v;  if (!readSInt(c, v)) goto done_fail; }
        if (R) { if (!skipRepetition(c)) goto done_fail; }

        QByteArray geom;
        if (!readString(c, geom)) goto done_fail;

        Q_UNUSED(attr);
        goto done_ok;
    }

    case 34: {
        quint64 compType = 0, uncomp = 0, comp = 0;
        if (!readUInt(c, compType)) goto done_fail;
        if (!readUInt(c, uncomp)) goto done_fail;
        if (!readUInt(c, comp)) goto done_fail;

        if (comp > quint64(c.end - c.p)) goto done_fail;

        const uchar *compData = c.p;
        c.p += int(comp);

        if (compType != 0) {
            errors << QString("CBLOCK comp-type=%1 not supported (only 0=DEFLATE).").arg(compType);
            goto done_fail;
        }

        QByteArray dec;
        if (!inflateRawDeflate(compData, int(comp), int(uncomp), dec)) {
            errors << "CBLOCK inflate failed or size mismatch.";
            goto done_fail;
        }

        OasCursor sub;
        sub.p   = reinterpret_cast<const uchar*>(dec.constData());
        sub.end = sub.p + dec.size();

        if (!parseBuffer(sub, out, errors, st)) goto done_fail;
        goto done_ok;
    }

    default:
        if (trySkipXGeometryLike(c) || trySkipXNameLike(c)) {
            goto done_ok;
        }

        errors << QString("Unsupported/unknown OASIS record id=%1 at offset=%2 tail=%3")
                      .arg(qulonglong(recId))
                      .arg(qulonglong(recStart - st.fileBase))
                      .arg(hexDumpN(c.p, c.end, 96));
        goto done_fail;
    }

done_ok:
#if OAS_TRACE
    if (st.traceCount < st.traceLimit) {
        errors << QString("TRACE OK  off=%1 recId=%2 consumed=%3 tail=%4")
        .arg(qulonglong(recStart - st.fileBase))
            .arg(qulonglong(recId))
            .arg(qulonglong(c.p - recStart))
            .arg(hexDumpN(c.p, c.end, 32));
    }
    st.traceCount++;
#endif
    return true;

done_fail:
#if OAS_TRACE
    errors << QString("TRACE FAIL off=%1 recId=%2 consumed=%3 tail=%4")
                  .arg(qulonglong(recStart - st.fileBase))
                  .arg(qulonglong(recId))
                  .arg(qulonglong(c.p - recStart))
                  .arg(hexDumpN(c.p, c.end, 64));
#endif
    return false;
}

/*!********************************************************************************************************************
 * \brief Checks whether the remaining bytes are only padding.
 *
 * Treats NUL, space, tab, LF and CR as padding.
 *
 * \param p Pointer to start of remaining bytes.
 * \param end Pointer one past the end.
 * \return True if the remaining region is padding-only, false otherwise.
 *********************************************************************************************************************/
static inline bool isPaddingTail(const uchar *p, const uchar *end)
{
    while (p < end) {
        const uchar b = *p++;
        if (b == 0x00) continue;
        if (b == 0x20) continue;
        if (b == 0x09) continue;
        if (b == 0x0A) continue;
        if (b == 0x0D) continue;
        return false;
    }
    return true;
}

/*!********************************************************************************************************************
 * \brief Parses records from the cursor until END or padding tail.
 *
 * \param c Cursor over the buffer.
 * \param out Output hierarchy.
 * \param errors Error list (append-only).
 * \param st Parser state.
 * \return True on success, false on parse failure.
 *********************************************************************************************************************/
static bool parseBuffer(OasCursor c,
                        LayoutHierarchy &out,
                        QStringList &errors,
                        OasParseState &st)
{
    while (c.p < c.end) {

        if (isPaddingTail(c.p, c.end)) {
#if OAS_TRACE
            errors << QString("TRACE STOP: padding tail at off=%1")
                          .arg(qulonglong(c.p - st.fileBase));
#endif
            break;
        }

        const uchar *before = c.p;

        if (!parseOneRecord(c, out, errors, st)) {

            if (isPaddingTail(before, c.end)) {
#if OAS_TRACE
                errors << QString("TRACE STOP: failed-in-padding at off=%1")
                              .arg(qulonglong(before - st.fileBase));
#endif
                break;
            }

            return false;
        }

        if (st.seenEnd) {
            break;
        }
    }
    return true;
}

/*!********************************************************************************************************************
 * \brief Reads OASIS hierarchy (CELL / PLACEMENT relationships) from the file.
 *
 * Memory-maps the file, validates the %SEMI-OASIS magic, then parses records until END.
 * On success, fills \p out with:
 *   - allCells: set of all encountered cells
 *   - children: adjacency list of placements per parent cell
 *   - topCells: cells that are not referenced as children
 *
 * \param out Output hierarchy structure.
 * \return True on success, false on failure (see \c getErrors()).
 *********************************************************************************************************************/
bool oasReader::readHierarchy(LayoutHierarchy &out)
{
    out.topCells.clear();
    out.children.clear();
    out.allCells.clear();
    m_errorList.clear();

    if (m_fileName.isEmpty()) {
        m_errorList << "Empty OASIS filename.";
        return false;
    }

    QFile f(m_fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        m_errorList << QString("Failed to open OASIS for read: '%1'").arg(m_fileName);
        return false;
    }

    const qint64 sz = f.size();
    if (sz < 16) {
        m_errorList << QString("OASIS file too small: '%1'").arg(m_fileName);
        return false;
    }

    uchar *base = f.map(0, sz);
    if (!base) {
        m_errorList << QString("Failed to memory-map OASIS: '%1'").arg(m_fileName);
        return false;
    }

    static const char magic[] = "%SEMI-OASIS";
    if (std::memcmp(base, magic, sizeof(magic) - 1) != 0) {
        f.unmap(base);
        m_errorList << "Not an OASIS file (missing %SEMI-OASIS magic).";
        return false;
    }

    const uchar *p = base + (sizeof(magic) - 1);
    while (p < base + sz && (*p == '\r' || *p == '\n')) ++p;

    OasCursor c;
    c.p   = p;
    c.end = base + sz;

    OasParseState st;
    st.fileBase = base;

    QElapsedTimer timer;
    timer.start();

    if (!parseBuffer(c, out, m_errorList, st)) {
        f.unmap(base);
        return false;
    }

    f.unmap(base);

    QSet<QString> referenced;
    for (auto it = out.children.begin(); it != out.children.end(); ++it) {
        const QStringList &chs = it.value();
        for (const QString &ch : chs) referenced.insert(ch);
    }

    out.topCells.clear();
    out.topCells.reserve(out.allCells.size());
    for (const QString &cname : out.allCells) {
        if (!referenced.contains(cname)) out.topCells << cname;
    }
    out.topCells.sort();

    return true;
}
