#include "core/corecellreader.h"

#include "core_paths.h"
#include "database.h"

#include <QString>

namespace {

core::ViewType viewTypeForName(const QString &viewName)
{
    const std::optional<core::ViewType> parsed = core::parseViewTypeName(viewName.toStdString());
    if (parsed.has_value()) {
        return *parsed;
    }
    return core::ViewType::Layout;
}

} // namespace

CoreCellReader::CoreCellReader(const QString &fileName)
    : m_fileName(fileName)
{
}

void CoreCellReader::coreCreate(const QString &cellName, const QString &viewName)
{
    try {
        const core::ViewType fileView = viewTypeForName(viewName);
        core::Database db;
        db.setGenerator("LibMan");
        core::Cell &cell = db.lib().getOrCreateCell(cellName.toStdString());
        cell.getOrCreateContent(fileView);
        if (fileView == core::ViewType::Layout) {
            db.lib().refreshIndex(fileView);
        }
        db.saveToFile(m_fileName.toStdString(), fileView);
    }
    catch (const std::exception &e) {
        m_errorList << QString::fromUtf8(e.what());
    }
}

bool CoreCellReader::readHierarchy(CoreHierarchy &out)
{
    try {
        core::Database db = core::Database::loadFromFile(m_fileName.toStdString());
        core::Lib &lib = db.lib();

        const core::ViewType fileView = db.fileView();
        if (fileView == core::ViewType::Layout) {
            lib.refreshIndex(fileView);
        } else if (!lib.hasIndex()) {
            lib.refreshIndex(fileView);
        }

        const core::LibIndex &idx = lib.index();

        for (const std::string &top : idx.topCells) {
            const QString name = QString::fromStdString(top);
            out.topCells << name;
            out.allCells.insert(name);
        }

        for (const core::Cell &cell : lib.cells()) {
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
            for (const core::Cell &cell : lib.cells()) {
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
