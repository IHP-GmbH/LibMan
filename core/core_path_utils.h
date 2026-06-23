#ifndef CORE_PATH_UTILS_H
#define CORE_PATH_UTILS_H

#include <QString>

struct CoreViewIdentity {
    QString cellName;
    QString viewName;
    bool    valid = false;
};

CoreViewIdentity parseCoreViewIdentity(const QString &filePath);
bool isCoreViewName(const QString &viewName);
bool isLayoutCoreViewName(const QString &viewName);
QString coreViewFileName(const QString &cellName, const QString &viewName);
QString coreViewFilePath(const QString &directory, const QString &cellName, const QString &viewName);

#endif // CORE_PATH_UTILS_H
