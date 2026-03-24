#include "lstreamcellreader.h"

#include <QFile>
#include <QDebug>

#include <capnp/serialize-packed.h>
#include <kj/io.h>
#include <kj/array.h>

#include "header.capnp.h"
#include "library.capnp.h"

LStreamCellReader::Result LStreamCellReader::read(const QString &fileName)
{
    Result out;

    try {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            out.errors << QString("Cannot open file: %1").arg(fileName);
            return out;
        }

        QByteArray data = file.readAll();
        if (data.isEmpty()) {
            out.errors << QString("File is empty: %1").arg(fileName);
            return out;
        }

        const QByteArray prefix("LStream_");

        if (!data.startsWith(prefix)) {
            out.errors << QString("Invalid LStream header in file: %1").arg(fileName);
            return out;
        }

        const int headerEnd = data.indexOf('\0');
        if (headerEnd < 0) {
            out.errors << QString("Unterminated LStream header in file: %1").arg(fileName);
            return out;
        }

        const QByteArray versionString = data.left(headerEnd);
        if (!versionString.startsWith("LStream_")) {
            out.errors << QString("Invalid LStream version header in file: %1").arg(fileName);
            return out;
        }

        data.remove(0, headerEnd + 1);

        kj::ArrayPtr<const kj::byte> bytes(
            reinterpret_cast<const kj::byte *>(data.constData()),
            static_cast<size_t>(data.size()));

        kj::ArrayInputStream input(bytes);

        capnp::PackedMessageReader headerMsg(input);
        auto header = headerMsg.getRoot<stream::header::Header>();
        Q_UNUSED(header);

        capnp::PackedMessageReader libMsg(input);
        auto library = libMsg.getRoot<stream::library::Library>();

        auto cellSpecs = library.getCellSpecsTable().getCellSpecs();

        for (auto c : cellSpecs) {
            out.cellNames << QString::fromStdString(c.getName());
        }

        out.loaded = true;
    }
    catch (const kj::Exception &e) {
        out.errors << QString("Cap'n Proto error: %1")
        .arg(QString::fromUtf8(e.getDescription().cStr()));
    }
    catch (const std::exception &e) {
        out.errors << QString("Exception: %1").arg(e.what());
    }
    catch (...) {
        out.errors << "Unknown error while reading LStream file";
    }

    return out;
}
