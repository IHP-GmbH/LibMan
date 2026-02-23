// oasReader.cpp
#include "oasReader.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QByteArray>
#include <QElapsedTimer>

//#include <cstdint>
#include <cstring>
//#include <limits>
#include <cstdlib>

#include <zlib.h>

#ifndef OAS_TRACE
#define OAS_TRACE 1
#endif

#ifndef OAS_GUARD
#define OAS_GUARD 1
#endif

#ifndef OAS_GUARD_MAX_RECORDS
#define OAS_GUARD_MAX_RECORDS 50000000ULL
#endif

#ifndef OAS_GUARD_STALL_LIMIT
#define OAS_GUARD_STALL_LIMIT 200000ULL
#endif

#ifndef OAS_GUARD_TINY_PROGRESS_BYTES
#define OAS_GUARD_TINY_PROGRESS_BYTES 1
#endif

#ifndef OAS_GUARD_LOG_EVERY_N_RECORDS
#define OAS_GUARD_LOG_EVERY_N_RECORDS 200000ULL
#endif

#ifndef OAS_GUARD_HEX_AROUND
#define OAS_GUARD_HEX_AROUND 64
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
/*static inline QString hexDumpN(const uchar *p, const uchar *end, int maxBytes = 64)
{
    const int n = int(std::min<qint64>(maxBytes, end - p));
    QByteArray b(reinterpret_cast<const char*>(p), n);
    return b.toHex(' ');
}*/

/*!********************************************************************************************************************
 * \brief Produces a hex dump around a pointer (bytes before/after).
 *
 * Useful to verify offsets when string decoding looks wrong.
 *
 * \param base File base pointer.
 * \param end File end pointer.
 * \param p Current pointer.
 * \param radius Bytes before/after.
 * \return Hex-encoded string with marker position info.
 *********************************************************************************************************************/
/*static inline QString hexAround(const uchar *base, const uchar *end, const uchar *p, int radius = OAS_GUARD_HEX_AROUND)
{
    if (!base || !end || !p) return QString();

    const uchar *a = p - radius;
    const uchar *b = p + radius;

    if (a < base) a = base;
    if (b > end)  b = end;

    const qint64 offA = a - base;
    const qint64 offP = p - base;
    const qint64 offB = b - base;

    return QString("around[%1..%2] p=%3: %4")
        .arg(qulonglong(offA))
        .arg(qulonglong(offB))
        .arg(qulonglong(offP))
        .arg(hexDumpN(a, b, int(b - a)));
}*/

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

    case 1: { // x-dimension y-dimension x-space y-space
        quint64 nx = 0, ny = 0, xs = 0, ys = 0;
        return readUInt(c, nx) && readUInt(c, ny) && readUInt(c, xs) && readUInt(c, ys);
    }

    case 2: { // x-dimension x-space
        quint64 nx = 0, xs = 0;
        return readUInt(c, nx) && readUInt(c, xs);
    }

    case 3: { // y-dimension y-space
        quint64 ny = 0, ys = 0;
        return readUInt(c, ny) && readUInt(c, ys);
    }

    case 4: { // x-dimension x-space1 ... x-space(N-1), where N = xdim + 2 => count = xdim + 1
        quint64 xdim = 0;
        if (!readUInt(c, xdim)) return false;
        for (quint64 i = 0; i < xdim + 1; ++i) {
            quint64 xs = 0;
            if (!readUInt(c, xs)) return false;
        }
        return true;
    }

    case 5: { // x-dimension grid x-space1 ... x-space(N-1)
        quint64 xdim = 0, grid = 0;
        if (!readUInt(c, xdim)) return false;
        if (!readUInt(c, grid)) return false;
        for (quint64 i = 0; i < xdim + 1; ++i) {
            quint64 xs = 0;
            if (!readUInt(c, xs)) return false;
        }
        return true;
    }

    case 6: { // y-dimension y-space1 ... y-space(M-1), count = ydim + 1
        quint64 ydim = 0;
        if (!readUInt(c, ydim)) return false;
        for (quint64 i = 0; i < ydim + 1; ++i) {
            quint64 ys = 0;
            if (!readUInt(c, ys)) return false;
        }
        return true;
    }

    case 7: { // y-dimension grid y-space1 ... y-space(M-1)
        quint64 ydim = 0, grid = 0;
        if (!readUInt(c, ydim)) return false;
        if (!readUInt(c, grid)) return false;
        for (quint64 i = 0; i < ydim + 1; ++i) {
            quint64 ys = 0;
            if (!readUInt(c, ys)) return false;
        }
        return true;
    }

    case 8: { // n-dimension m-dimension n-displacement m-displacement (g-delta, g-delta)
        quint64 nd = 0, md = 0;
        return readUInt(c, nd) && readUInt(c, md) && skipGDelta(c) && skipGDelta(c);
    }

    case 9: { // dimension displacement (g-delta)
        quint64 dim = 0;
        return readUInt(c, dim) && skipGDelta(c);
    }

    case 10: { // dimension displacement1 ... displacement(P-1), count = dim + 1
        quint64 dim = 0;
        if (!readUInt(c, dim)) return false;
        for (quint64 i = 0; i < dim + 1; ++i) {
            if (!skipGDelta(c)) return false;
        }
        return true;
    }

    case 11: { // dimension grid displacement1 ... displacement(P-1), count = dim + 1
        quint64 dim = 0, grid = 0;
        if (!readUInt(c, dim)) return false;
        if (!readUInt(c, grid)) return false;
        for (quint64 i = 0; i < dim + 1; ++i) {
            if (!skipGDelta(c)) return false;
        }
        return true;
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
    const uchar *fileEnd  = nullptr;

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

#if OAS_GUARD
    quint64 recordCount = 0;
    quint64 stallCount  = 0;
    quint64 lastOff     = 0;
    quint64 lastGoodOff = 0;
    QElapsedTimer guardTimer;
#endif
};


/*!********************************************************************************************************************
 * \brief Guarded debug log helper (throttled).
 *
 * Adds a line to errors and optionally qDebug().
 *
 * \param errors Error list.
 * \param st Parser state.
 * \param s Message.
 *********************************************************************************************************************/
/*static inline void dbgLog(QStringList &errors, OasParseState &st, const QString &s)
{
    errors << s;
#if OAS_TRACE
    Q_UNUSED(st);
#else
    Q_UNUSED(st);
#endif
}*/

/*!********************************************************************************************************************
 * \brief Parses a buffer of OASIS records into a hierarchy model.
 * \param c Cursor over the buffer.
 * \param out Output hierarchy.
 * \param errors Error list (append-only).
 * \param st Parser state.
 * \return True on success, false on parse failure.
 *********************************************************************************************************************/
static bool parseBuffer(OasCursor &c,
                        LayoutHierarchy &out,
                        QStringList &errors,
                        OasParseState &st);

static inline bool peekUInt(const OasCursor &c, quint64 &out)
{
    OasCursor t = c;
    return readUInt(t, out);
}

static inline bool skipRepetitionEx(OasCursor &c)
{
    quint64 repType = 0;
    if (!readUInt(c, repType)) return false;

    if (repType > 64) return false;

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

    case 10: {
        quint64 dim = 0;
        if (!readUInt(c, dim)) return false;
        if (dim > 1000000ULL) return false;

        for (quint64 i = 0; i < dim; ++i) {
            if (!skipGDelta(c)) return false; // x
            if (!skipGDelta(c)) return false; // y
        }
        return true;
    }
    case 11: {
        quint64 dim = 0;
        quint64 grid = 0;
        if (!readUInt(c, dim)) return false;
        if (!readUInt(c, grid)) return false;
        if (dim > 1000000ULL || grid > 100000000ULL) return false;

        // Often dim-1 displacement pairs
        const quint64 k = (dim > 0) ? (dim - 1) : 0;
        for (quint64 i = 0; i < k; ++i) {
            if (!skipGDelta(c)) return false; // x
            if (!skipGDelta(c)) return false; // y
        }
        return true;
    }

    default:
        return false;
    }
}

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
    //const uchar *recStart = c.p;

    quint64 recId = 0;
    if (!readUInt(c, recId)) return false;

#if OAS_TRACE
    if (st.traceCount < st.traceLimit) {
        /*qDebug() << QString("TRACE #%1 off=%2 recId=%3 head=%4")
                      .arg(st.traceCount)
                      .arg(qulonglong(recStart - st.fileBase))
                      .arg(qulonglong(recId))
                      .arg(hexDumpN(c.p, c.end, 32));*/
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
        // implicit CELLNAME
        st.implicitCellNameRefsSeen = true;

        if (st.explicitCellNameRefsSeen) {
            static bool warned = false;
            if (!warned) {
                warned = true;
            }
        }

        QByteArray raw;
        if (!readString(c, raw)) goto done_fail;

        QString nameUtf8 = QString::fromUtf8(raw.constData(), raw.size());
        QString nameLat1 = QString::fromLatin1(raw.constData(), raw.size());
        QString name = nameUtf8.contains(QChar(0xFFFD)) ? nameLat1 : nameUtf8;

        if (!isLikelyValidCellName(name)) goto done_ok;

        const quint64 rn = st.nextCellNameRef++;

        st.cellNameByRef.insert(rn, name);
        out.allCells.insert(name);
        out.children[name];
        goto done_ok;
    }

    case 4: {
        // explicit CELLNAME
        st.explicitCellNameRefsSeen = true;

        if (st.implicitCellNameRefsSeen) {
            static bool warned = false;
            if (!warned) {
                warned = true;
                //qDebug() << "CELLNAME: mixed implicit/explicit reference-number mode (3/4). Allowing.";
            }
        }

        QByteArray raw;
        if (!readString(c, raw)) goto done_fail;

        quint64 rn = 0;
        if (!readUInt(c, rn)) goto done_fail;

        QString nameUtf8 = QString::fromUtf8(raw.constData(), raw.size());
        QString nameLat1 = QString::fromLatin1(raw.constData(), raw.size());
        QString name = nameUtf8.contains(QChar(0xFFFD)) ? nameLat1 : nameUtf8;

        if (st.nextCellNameRef <= rn) st.nextCellNameRef = rn + 1;

        if (!isLikelyValidCellName(name)) {
            st.cellNameByRef.insert(rn, name);
            goto done_ok;
        }

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
            /*qDebug() << QString("CELL references unknown CELLNAME ref=%1 off=%2")
                          .arg(rn)
                          .arg(qulonglong(recStart - st.fileBase));
            qDebug() << QString("CELL unknown-ref context: %1").arg(hexAround(st.fileBase, st.fileEnd, recStart));*/
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
        //const uchar *strAt = c.p;
        if (!readString(c, raw)) goto done_fail;

        QString utf8 = QString::fromUtf8(raw.constData(), raw.size());
        QString lat1 = QString::fromLatin1(raw.constData(), raw.size());
        QString name = utf8.contains(QChar(0xFFFD)) ? lat1 : utf8;

        /*qDebug() << QString("CELL(name) off=%1 len=%2 bytes=%3 utf8='%4' lat1='%5' chosen='%6' u16=[%7] around=%8")
                      .arg(qulonglong(strAt - st.fileBase))
                      .arg(raw.size())
                      .arg(QString(raw.toHex(' ')))
                      .arg(utf8).arg(lat1).arg(name)
                      .arg(dumpU16(name))
                      .arg(hexAround(st.fileBase, st.fileEnd, strAt));*/

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
            //qDebug() << "PLACEMENT outside of CELL.";
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
                //const uchar *nmAt = c.p;
                if (!readNString(c, placedCell)) goto done_fail;

                if (!isLikelyValidCellName(placedCell)) {
                    /*qDebug() << QString("PLACEMENT inline-name suspicious parent='%1' chosen='%2' off=%3 nameAt=%4 around=%5")
                                    .arg(st.currentCell)
                                    .arg(placedCell)
                                    .arg(qulonglong(recStart - st.fileBase))
                                    .arg(qulonglong(nmAt - st.fileBase))
                                    .arg(hexAround(st.fileBase, st.fileEnd, nmAt));*/
                }
            } else {
                // N=0 -> reference-number
                quint64 rn = 0;
                if (!readUInt(c, rn)) goto done_fail;

                placedCell = st.cellNameByRef.value(rn);
                if (placedCell.isEmpty()) {
                    /*qDebug() << QString("PLACEMENT references unknown CELLNAME ref=%1 parent='%2' off=%3")
                                    .arg(rn)
                                    .arg(st.currentCell)
                                    .arg(qulonglong(recStart - st.fileBase));
                    qDebug() << QString("PLACEMENT unknown-ref context: %1")
                                    .arg(hexAround(st.fileBase, st.fileEnd, recStart));*/
                    goto done_fail;
                }
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

    case 19: { // TEXT
        uchar info = 0;
        if (!readByte(c, info)) goto done_fail;

        // bit pattern: 0 C N X Y R T L  (from spec)
        const bool L    = (info & 0x01) != 0;
        const bool T    = (info & 0x02) != 0;
        const bool R    = (info & 0x04) != 0;
        const bool Y    = (info & 0x08) != 0;
        const bool X    = (info & 0x10) != 0;
        const bool N    = (info & 0x20) != 0;
        const bool Cbit = (info & 0x40) != 0;

        // text-string / reference-number / modal textstring
        if (Cbit) {
            if (N) {
                quint64 rn = 0;
                if (!readUInt(c, rn)) goto done_fail;
            } else {
                QString s;
                if (!readAString(c, s)) goto done_fail;
            }
        } else {
            // C=0 => no modal textstring, bytes in record
        }

        if (L) { quint64 tlayer = 0; if (!readUInt(c, tlayer)) goto done_fail; }
        if (T) { quint64 ttype  = 0; if (!readUInt(c, ttype))  goto done_fail; }

        if (X) { qint64 vx = 0; if (!readSInt(c, vx)) goto done_fail; }
        if (Y) { qint64 vy = 0; if (!readSInt(c, vy)) goto done_fail; }

        if (R) { if (!skipRepetition(c)) goto done_fail; }

        goto done_ok;
    }

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
        //const quint64 recOff = quint64(recStart - st.fileBase);

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

        /*qDebug() << QString("REC22 off=%1 info=0x%2 flags[L=%3 D=%4 R=%5 Y=%6 X=%7 P=%8 W=%9 E=%10] head=%11")
                        .arg(qulonglong(recOff))
                        .arg(int(info), 2, 16, QLatin1Char('0'))
                        .arg(L).arg(D).arg(R).arg(Y).arg(X).arg(P).arg(W).arg(E)
                        .arg(hexDumpN(c.p, c.end, 64));*/

        if (L) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (D) { quint64 v; if (!readUInt(c, v)) goto done_fail; }
        if (W) { quint64 v; if (!readUInt(c, v)) goto done_fail; }

        auto parse_PXY = [&](OasCursor &cc, bool orderPXY) -> bool {
            if (orderPXY) {
                if (P) {
                    OasCursor t = cc;
                    quint64 ptType = 0, count = 0;
                    if (readUInt(t, ptType) && readUInt(t, count)) {
                        /*qDebug() << QString("REC22 P(list) off=%1 ptType=%2 count=%3")
                                        .arg(qulonglong(cc.p - st.fileBase))
                                        .arg(qulonglong(ptType))
                                        .arg(qulonglong(count));*/
                    }
                    if (!skipPointList(cc)) return false;
                }
                if (X) { qint64 v; if (!readSInt(cc, v)) return false; }
                if (Y) { qint64 v; if (!readSInt(cc, v)) return false; }
            } else {
                if (X) { qint64 v; if (!readSInt(cc, v)) return false; }
                if (Y) { qint64 v; if (!readSInt(cc, v)) return false; }
                if (P) {
                    OasCursor t = cc;
                    quint64 ptType = 0, count = 0;
                    if (readUInt(t, ptType) && readUInt(t, count)) {
                        /*qDebug() << QString("REC22 P(list) off=%1 ptType=%2 count=%3")
                                        .arg(qulonglong(cc.p - st.fileBase))
                                        .arg(qulonglong(ptType))
                                        .arg(qulonglong(count));*/
                    }
                    if (!skipPointList(cc)) return false;
                }
            }
            return true;
        };

        auto parse_E = [&](OasCursor &cc) -> bool {
            if (!E) return true;
            quint64 a = 0, b = 0;
            return readUInt(cc, a) && readUInt(cc, b);
        };

        auto parse_R = [&](OasCursor &cc) -> bool {
            if (!R) return true;

            quint64 repPeek = 0;
            if (peekUInt(cc, repPeek)) {
                /*qDebug() << QString("REC22 repetition peek repType=%1 at off=%2")
                                .arg(qulonglong(repPeek))
                                .arg(qulonglong(cc.p - st.fileBase));*/
            } else {
                /*qDebug() << QString("REC22 repetition peek FAILED at off=%1")
                                .arg(qulonglong(cc.p - st.fileBase));*/
            }

            //const quint64 repOff = quint64(cc.p - st.fileBase);
            /*qDebug() << QString("REC22 repetition startOff=%1 ctx=%2")
                            .arg(qulonglong(repOff))
                            .arg(hexAround(st.fileBase, st.fileEnd, cc.p));*/

            if (!skipRepetitionEx(cc)) {
                /*qDebug() << QString("REC22 FAIL off=%1 what=skipRepetition repOff=%2 ctx=%3")
                                .arg(qulonglong(recOff))
                                .arg(qulonglong(repOff))
                                .arg(hexAround(st.fileBase, st.fileEnd, cc.p));*/
                return false;
            }
            return true;
        };

        {
            OasCursor t = c;
            bool ok = true;
            ok = ok && parse_PXY(t, /*orderPXY=*/true);
            ok = ok && parse_E(t);
            ok = ok && parse_R(t);

            if (ok) {
                c = t;
                goto done_ok;
            }
        }

        {
            OasCursor t = c;
            bool ok = true;
            ok = ok && parse_PXY(t, /*orderPXY=*/false);
            ok = ok && parse_E(t);
            ok = ok && parse_R(t);

            if (ok) {
                c = t;
                goto done_ok;
            }
        }

        /*qDebug() << QString("REC22 parse failed in both orders at off=%1 ctx=%2")
                        .arg(qulonglong(recOff))
                        .arg(hexAround(st.fileBase, st.fileEnd, c.p));*/
        goto done_fail;
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
            //qDebug() << QString("CBLOCK comp-type=%1 not supported (only 0=DEFLATE).").arg(compType);
            goto done_fail;
        }

        QByteArray dec;
        if (!inflateRawDeflate(compData, int(comp), int(uncomp), dec)) {
            //qDebug() << "CBLOCK inflate failed or size mismatch.";
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

        /*qDebug() << QString("Unsupported/unknown OASIS record id=%1 at offset=%2 tail=%3")
                      .arg(qulonglong(recId))
                      .arg(qulonglong(recStart - st.fileBase))
                      .arg(hexDumpN(c.p, c.end, 96));
        qDebug() << QString("Unknown rec context: %1").arg(hexAround(st.fileBase, st.fileEnd, recStart));*/
        goto done_fail;
    }

done_ok:
#if OAS_TRACE
    if (st.traceCount < st.traceLimit) {
        /*qDebug() << QString("TRACE OK  off=%1 recId=%2 consumed=%3 tail=%4")
                      .arg(qulonglong(recStart - st.fileBase))
                      .arg(qulonglong(recId))
                      .arg(qulonglong(c.p - recStart))
                      .arg(hexDumpN(c.p, c.end, 32));*/
    }
    st.traceCount++;
#endif
    return true;

done_fail:
#if OAS_TRACE
    /*qDebug() << QString("TRACE FAIL off=%1 recId=%2 consumed=%3 tail=%4")
                  .arg(qulonglong(recStart - st.fileBase))
                  .arg(qulonglong(recId))
                  .arg(qulonglong(c.p - recStart))
                  .arg(hexDumpN(c.p, c.end, 64));*/
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
static bool parseBuffer(OasCursor &c,
                        LayoutHierarchy &out,
                        QStringList &errors,
                        OasParseState &st)
{
#if OAS_GUARD
    if (!st.guardTimer.isValid()) st.guardTimer.start();
#endif

    while (c.p < c.end) {

        if (isPaddingTail(c.p, c.end)) {
#if OAS_TRACE
            /*qDebug() << QString("TRACE STOP: padding tail at off=%1")
                          .arg(qulonglong(c.p - st.fileBase));*/
#endif
            break;
        }

        const uchar *before = c.p;

#if OAS_GUARD
        st.recordCount++;
        const quint64 offBefore = quint64(before - st.fileBase);

        if ((st.recordCount % OAS_GUARD_LOG_EVERY_N_RECORDS) == 0) {
            /*qDebug() << QString("GUARD progress: records=%1 off=%2 elapsed_ms=%3")
                          .arg(qulonglong(st.recordCount))
                          .arg(qulonglong(offBefore))
                          .arg(qulonglong(st.guardTimer.elapsed()));*/
        }

        if (st.recordCount > OAS_GUARD_MAX_RECORDS) {
            /*qDebug() << QString("GUARD abort: too many records=%1 off=%2 elapsed_ms=%3 ctx=%4")
                          .arg(qulonglong(st.recordCount))
                          .arg(qulonglong(offBefore))
                          .arg(qulonglong(st.guardTimer.elapsed()))
                          .arg(hexAround(st.fileBase, st.fileEnd, before));*/
            return false;
        }
#endif

        if (!parseOneRecord(c, out, errors, st)) {

            if (isPaddingTail(before, c.end)) {
#if OAS_TRACE
                /*qDebug() << QString("TRACE STOP: failed-in-padding at off=%1")
                              .arg(qulonglong(before - st.fileBase));*/
#endif
                break;
            }

#if OAS_GUARD
            /*qDebug() << QString("GUARD fail: lastGoodOff=%1 failOff=%2 ctx=%3")
                          .arg(qulonglong(st.lastGoodOff))
                          .arg(qulonglong(offBefore))
                          .arg(hexAround(st.fileBase, st.fileEnd, before));*/
#endif
            return false;
        }

#if OAS_GUARD
        const quint64 offAfter = quint64(c.p - st.fileBase);
        const quint64 delta = (offAfter >= offBefore) ? (offAfter - offBefore) : 0;

        if (delta <= OAS_GUARD_TINY_PROGRESS_BYTES) {
            st.stallCount++;
        } else {
            st.stallCount = 0;
            st.lastGoodOff = offAfter;
        }

        if (offAfter == offBefore) {
            /*qDebug() << QString("GUARD abort: zero advance at off=%1 records=%2 ctx=%3")
                          .arg(qulonglong(offBefore))
                          .arg(qulonglong(st.recordCount))
                          .arg(hexAround(st.fileBase, st.fileEnd, before));*/
            return false;
        }

        if (st.stallCount > OAS_GUARD_STALL_LIMIT) {
            /*qDebug() << QString("GUARD abort: stallCount=%1 lastOff=%2 nowOff=%3 records=%4 elapsed_ms=%5 ctx=%6")
                          .arg(qulonglong(st.stallCount))
                          .arg(qulonglong(st.lastOff))
                          .arg(qulonglong(offAfter))
                          .arg(qulonglong(st.recordCount))
                          .arg(qulonglong(st.guardTimer.elapsed()))
                          .arg(hexAround(st.fileBase, st.fileEnd, before));*/
            return false;
        }

        st.lastOff = offAfter;
#endif

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
    st.fileEnd  = base + sz;

#if OAS_GUARD
    st.recordCount = 0;
    st.stallCount  = 0;
    st.lastOff     = quint64(c.p - st.fileBase);
    st.lastGoodOff = st.lastOff;
    st.guardTimer.invalidate();
#endif

    QElapsedTimer timer;
    timer.start();

    /*qDebug() << QString("OAS start: file='%1' size=%2 startOff=%3")
                       .arg(m_fileName)
                       .arg(qulonglong(sz))
                       .arg(qulonglong(c.p - st.fileBase));*/

    if (!parseBuffer(c, out, m_errorList, st)) {
        /*qDebug() << QString("OAS parse FAILED: elapsed_ms=%1 stopOff=%2")
                           .arg(qulonglong(timer.elapsed()))
                           .arg(qulonglong(c.p - st.fileBase));*/
        f.unmap(base);
        return false;
    }

    /*qDebug() << QString("OAS parse OK: elapsed_ms=%1 stopOff=%2 records=%3 cells=%4")
                       .arg(qulonglong(timer.elapsed()))
                       .arg(qulonglong(c.p - st.fileBase))
                       .arg(qulonglong(
#if OAS_GUARD
                           st.recordCount
#else
                           0
#endif
                           ))
                       .arg(out.allCells.size());*/

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
