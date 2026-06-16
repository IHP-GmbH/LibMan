#include "core/corecellreader.h"

#include "database.h"

#include <QString>

CoreCellReader::CoreCellReader(const QString &fileName)
    : m_fileName(fileName)
{
}

void CoreCellReader::coreCreate(const QString &cellName)
{
    try {
        core::Database db;
        db.setGenerator("LibMan");
        db.lib().getOrCreateCell(cellName.toStdString());
        db.lib().refreshIndex(core::ViewType::Layout);
        db.saveToFile(m_fileName.toStdString());
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

        if (!lib.hasIndex()) {
            lib.refreshIndex(core::ViewType::Layout);
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
