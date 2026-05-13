#include "lstreamcellwriter.h"

#include <QFile>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/io.h>
#include <kj/vector.h>
#include <kj/exception.h>

#include "layoutView.capnp.h"
#include "geometry.capnp.h"
#include "header.capnp.h"
#include "library.capnp.h"
#include "cell.capnp.h"
#include "metaData.capnp.h"

/*!********************************************************************************************************************
 * \brief Serializes one Cap'n Proto message into a QByteArray using packed encoding.
 *
 * \param message Cap'n Proto message builder.
 * \return Packed serialized message bytes.
 *********************************************************************************************************************/
static QByteArray serializePackedMessage(::capnp::MessageBuilder &message)
{
    ::kj::VectorOutputStream output;
    ::capnp::writePackedMessage(output, message);

    const ::kj::ArrayPtr<const ::kj::byte> arr = output.getArray();

    return QByteArray(reinterpret_cast<const char *>(arr.begin()),
                      static_cast<int>(arr.size()));
}

/*!********************************************************************************************************************
 * \brief Appends one packed Cap'n Proto message to the target file buffer.
 *
 * \param fileData Target file buffer.
 * \param message  Cap'n Proto message builder.
 *********************************************************************************************************************/
static void appendPackedMessage(QByteArray &fileData, ::capnp::MessageBuilder &message)
{
    fileData.append(serializePackedMessage(message));
}

/*!********************************************************************************************************************
 * \brief Writes a minimal LStream file using Cap'n Proto packed serialization.
 *
 * File layout:
 *   - ASCII header "LStream_1.0\0"
 *   - packed stream::header::Header
 *   - packed stream::library::Library
 *   - packed stream::cell::Cell for each cell
 *
 * The generated file contains:
 *   - one logical library with type "layout"
 *   - one view spec named "layout"
 *   - empty property/text tables
 *   - one cell spec per input cell name
 *   - one hierarchy node per input cell, all top-level
 *   - one cell payload per input cell with viewIds = [0]
 *
 * \param path         Output file path.
 * \param libraryName  Library name written into header/library section.
 * \param cellNames    Cell names to write.
 * \param technology   Technology string stored in header.
 * \param generator    Generator string stored in header.
 * \return Result of the write operation.
 *********************************************************************************************************************/
LStreamCellWriter::Result LStreamCellWriter::write(const QString &path,
                                                   const QString &libraryName,
                                                   const QStringList &cellNames,
                                                   const QString &technology,
                                                   const QString &generator)
{
    Result out;

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        out.errors << QString("Failed to open LStream file for writing: '%1'").arg(path);
        return out;
    }

    try {
        QByteArray fileData;

        fileData.append("LStream_1.0\0", 12);

        {
            ::capnp::MallocMessageBuilder message;
            auto header = message.initRoot< ::stream::header::Header>();

            header.setGenerator(generator.toUtf8().constData());
            header.setTechnology(technology.toUtf8().constData());

            {
                auto metaData = header.initMetaData();
                metaData.initEntries(0);
            }

            {
                auto libraries = header.initLibraries(1);
                libraries[0].setName(libraryName.toUtf8().constData());
                libraries[0].setType("layout");
            }

            appendPackedMessage(fileData, message);
        }

        {
            ::capnp::MallocMessageBuilder message;
            auto library = message.initRoot< ::stream::library::Library>();

            {
                auto propertyNamesTable = library.initPropertyNamesTable();
                propertyNamesTable.initNamespaces(0);
                propertyNamesTable.initNames(0);
            }

            {
                auto propertiesTable = library.initPropertiesTable();
                propertiesTable.initPropertySets(0);
            }

            {
                auto textStringsTable = library.initTextStringsTable();
                textStringsTable.initTextStrings(0);
            }

            {
                auto libraryRefs = library.initLibraryRefs();
                libraryRefs.initRefs(0);
            }

            {
                auto viewSpecsTable = library.initViewSpecsTable();
                auto viewSpecs = viewSpecsTable.initViewSpecs(static_cast<unsigned int>(1));

                auto view = viewSpecs[0];
                view.setName("layout");
                view.setClass("LayoutView");
                view.setResolution(0.001);
                view.setPropertySetId(0);

                auto layerTable = view.initLayerTable();
                layerTable.initLayerEntries(0);
            }

            {
                auto cellSpecsTable = library.initCellSpecsTable();
                auto cellSpecs = cellSpecsTable.initCellSpecs(static_cast<unsigned int>(cellNames.size()));

                for(int i = 0; i < cellNames.size(); ++i) {
                    auto cellSpec = cellSpecs[static_cast<unsigned int>(i)];
                    cellSpec.setName(cellNames.at(i).toUtf8().constData());
                    cellSpec.setLibraryRefId(0);
                    cellSpec.setPropertySetId(0);
                }
            }

            {
                auto hierarchy = library.initCellHierarchyTree();
                hierarchy.setNumberOfTopCells(static_cast<quint64>(cellNames.size()));

                auto nodes = hierarchy.initNodes(static_cast<unsigned int>(cellNames.size()));
                for(int i = 0; i < cellNames.size(); ++i) {
                    auto node = nodes[static_cast<unsigned int>(i)];
                    node.setCellId(static_cast<quint64>(i));
                    node.initChildCellIds(0);
                }
            }

            appendPackedMessage(fileData, message);
        }

        for(int i = 0; i < cellNames.size(); ++i) {
            Q_UNUSED(i);

            {
                ::capnp::MallocMessageBuilder message;
                auto cell = message.initRoot< ::stream::cell::Cell>();

                auto viewIds = cell.initViewIds(1);
                viewIds.set(0, 0);

                appendPackedMessage(fileData, message);
            }

            {
                ::capnp::MallocMessageBuilder message;
                auto layoutView = message.initRoot< ::stream::layoutView::LayoutView>();

                {
                    auto bbox = layoutView.initBoundingBox();

                    auto p1 = bbox.initP1();
                    p1.setX(0);
                    p1.setY(0);

                    auto delta = bbox.initDelta();
                    delta.setDx(0);
                    delta.setDy(0);
                }

                layoutView.initLayers(0);
                layoutView.initInstanceRepetitions(0);

                {
                    auto instances = layoutView.initInstances();
                    instances.initBasic(0);
                    instances.initWithProperties(0);
                }

                appendPackedMessage(fileData, message);
            }
        }

        if(file.write(fileData) != fileData.size()) {
            out.errors << QString("Failed to write complete LStream file: '%1'").arg(path);
            file.close();
            return out;
        }

        file.close();
        out.written = true;
    }
    catch(const ::kj::Exception &e) {
        out.errors << QString("Cap'n Proto write failed: %1")
        .arg(QString::fromUtf8(e.getDescription().cStr()));
        file.close();
        return out;
    }
    catch(const std::exception &e) {
        out.errors << QString("LStream write failed: %1")
        .arg(QString::fromUtf8(e.what()));
        file.close();
        return out;
    }
    catch(...) {
        out.errors << "Unknown error while writing LStream file.";
        file.close();
        return out;
    }

    return out;
}
