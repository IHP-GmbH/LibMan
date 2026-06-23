#include "klayoutCellResolver.h"

#include "core/corecellreader.h"
#include "core/core_path_utils.h"
#include "gds/gdsreader.h"
#include "oas/oasReader.h"
#include "src/lstreamcellreader.h"

#include <QFileInfo>

namespace {

LayoutHierarchySnapshot snapshotFromGds(const GdsReader::GdsHierarchy &h)
{
    LayoutHierarchySnapshot out;
    out.topCells = h.topCells;
    out.allCells = h.allCells;
    return out;
}

LayoutHierarchySnapshot snapshotFromOas(const LayoutHierarchy &h)
{
    LayoutHierarchySnapshot out;
    out.topCells = h.topCells;
    out.allCells = h.allCells;
    return out;
}

LayoutHierarchySnapshot snapshotFromCore(const CoreCellReader::CoreHierarchy &h)
{
    LayoutHierarchySnapshot out;
    out.topCells = h.topCells;
    out.allCells = h.allCells;
    return out;
}

LayoutHierarchySnapshot snapshotFromLStream(const QStringList &cellNames)
{
    LayoutHierarchySnapshot out;
    out.topCells = cellNames;
    out.topCells.sort();
    for (const QString &name : cellNames) {
        out.allCells.insert(name);
    }
    return out;
}

bool isLayoutCorePath(const QString &path)
{
    const CoreViewIdentity identity = parseCoreViewIdentity(path);
    if (identity.valid) {
        return isLayoutCoreViewName(identity.viewName);
    }

    return QFileInfo(path).suffix().compare(QStringLiteral("core"), Qt::CaseInsensitive) == 0;
}

} // namespace

QString resolveKLayoutRootCell(const LayoutHierarchySnapshot &hierarchy,
                               const QString &groupName)
{
    if (!groupName.isEmpty() && hierarchy.allCells.contains(groupName)) {
        return groupName;
    }

    if (hierarchy.topCells.size() == 1) {
        return hierarchy.topCells.first();
    }

    if (!hierarchy.topCells.isEmpty()) {
        return hierarchy.topCells.first();
    }

    return QString();
}

bool loadLayoutHierarchySnapshot(const QString &layoutPath,
                                 LayoutHierarchySnapshot &out,
                                 QStringList *errors)
{
    const QFileInfo fi(layoutPath);
    if (!fi.exists() || !fi.isFile()) {
        if (errors) {
            *errors << QStringLiteral("Layout file not found: %1").arg(layoutPath);
        }
        return false;
    }

    const QString suffix = fi.suffix().toLower();

    if (isLayoutCorePath(layoutPath)) {
        CoreCellReader reader(layoutPath);
        CoreCellReader::CoreHierarchy hierarchy;
        if (!reader.readHierarchy(hierarchy)) {
            if (errors) {
                *errors = reader.getErrors();
            }
            return false;
        }
        out = snapshotFromCore(hierarchy);
        return !out.allCells.isEmpty() || !out.topCells.isEmpty();
    }

    if (suffix == QLatin1String("gds")) {
        GdsReader reader(layoutPath);
        GdsReader::GdsHierarchy hierarchy;
        if (!reader.readHierarchy(hierarchy)) {
            if (errors) {
                *errors = reader.getErrors();
            }
            return false;
        }
        out = snapshotFromGds(hierarchy);
        return !out.allCells.isEmpty() || !out.topCells.isEmpty();
    }

    if (suffix == QLatin1String("oas")) {
        oasReader reader(layoutPath);
        LayoutHierarchy hierarchy;
        if (!reader.readHierarchy(hierarchy)) {
            if (errors) {
                *errors = reader.getErrors();
            }
            return false;
        }
        out = snapshotFromOas(hierarchy);
        return !out.allCells.isEmpty() || !out.topCells.isEmpty();
    }

    if (suffix == QLatin1String("lstr")) {
        const LStreamCellReader::Result result = LStreamCellReader::read(layoutPath);
        if (!result.loaded) {
            if (errors) {
                *errors = result.errors;
            }
            return false;
        }
        out = snapshotFromLStream(result.cellNames);
        return !out.allCells.isEmpty();
    }

    if (errors) {
        *errors << QStringLiteral("Unsupported layout file type: %1").arg(layoutPath);
    }
    return false;
}
