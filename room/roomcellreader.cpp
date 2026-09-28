#include "room/roomcellreader.h"

#include "core_paths.h"
#include "database.h"

#include <QString>

namespace {

room::ViewType viewTypeForName(const QString &viewName)
{
    const std::optional<room::ViewType> parsed = room::parseViewTypeName(viewName.toStdString());
    if (parsed.has_value()) {
        return *parsed;
    }
    return room::ViewType::Layout;
}

} // namespace

RoomCellReader::RoomCellReader(const QString &fileName)
    : m_fileName(fileName)
{
}

void RoomCellReader::coreCreate(const QString &cellName, const QString &viewName)
{
    try {
        const room::ViewType fileView = viewTypeForName(viewName);
        room::Database db;
        db.setGenerator("LibMan");
        room::Cell &cell = db.lib().getOrCreateCell(cellName.toStdString());
        cell.getOrCreateContent(fileView);
        if (fileView == room::ViewType::Layout) {
            db.lib().refreshIndex(fileView);
        }
        db.saveToFile(m_fileName.toStdString(), fileView);
    }
    catch (const std::exception &e) {
        m_errorList << QString::fromUtf8(e.what());
    }
}

bool RoomCellReader::readHierarchy(CoreHierarchy &out)
{
    try {
        room::Database db = room::Database::loadFromFile(m_fileName.toStdString());
        room::Lib &lib = db.lib();

        const room::ViewType fileView = db.fileView();
        if (fileView == room::ViewType::Layout) {
            lib.refreshIndex(fileView);
        } else if (!lib.hasIndex()) {
            lib.refreshIndex(fileView);
        }

        const room::LibIndex &idx = lib.index();

        for (const std::string &top : idx.topCells) {
            const QString name = QString::fromStdString(top);
            out.topCells << name;
            out.allCells.insert(name);
        }

        for (const room::Cell &cell : lib.cells()) {
            out.allCells.insert(QString::fromStdString(cell.name()));
        }

        for (const auto &kv : idx.childRefs) {
            QStringList children;
            for (const std::string &ch : kv.second) {
                children << QString::fromStdString(ch);
            }
            out.children.insert(QString::fromStdString(kv.first), children);
        }

        if (out.topCells.isEmpty()) {
            for (const room::Cell &cell : lib.cells()) {
                out.topCells << QString::fromStdString(cell.name());
            }
        }

        return true;
    }
    catch (const std::exception &e) {
        m_errorList << QString::fromUtf8(e.what());
        return false;
    }
}
