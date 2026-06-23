#include "core/coreKlayoutBridge.h"
#include "core/core_path_utils.h"

#include <QDir>
#include <QFileInfo>

QString coreLayoutPathForKLayout(const QString &viewPath, QStringList *errors)
{
    const QFileInfo fi(viewPath);
    if (!fi.exists() || !fi.isFile()) {
        if (errors) {
            *errors << QString("CORE file not found: %1").arg(viewPath);
        }
        return QString();
    }

    const CoreViewIdentity identity = parseCoreViewIdentity(viewPath);
    if (identity.valid && !isLayoutCoreViewName(identity.viewName)) {
        if (errors) {
            *errors << QString("CORE file is not a layout view: %1").arg(viewPath);
        }
        return QString();
    }

    const bool isCoreLayout = identity.valid
        || fi.suffix().compare(QStringLiteral("core"), Qt::CaseInsensitive) == 0;

    if (isCoreLayout) {
        // KLayout opens *.layout.core natively via the mcore streamer plugin.
        return QDir::toNativeSeparators(fi.absoluteFilePath());
    }

    if (errors) {
        *errors << QString("Not a CORE layout file: %1").arg(viewPath);
    }
    return QString();
}
