#ifndef LIBDEFINE_UTILS_H
#define LIBDEFINE_UTILS_H

#include <QString>
#include <QStringList>

namespace libdefine {

bool isWildcardDefinePath(const QString &path);

QString wildcardPatternToRelative(const QString &pattern);

QString wildcardScanRoot(const QString &projectDir, const QString &pattern);

QString wildcardLibraryRoot(const QString &projectDir, const QString &pattern);

QStringList expandWildcardDefinePath(const QString &projectDir, const QString &pattern);

bool isPathCoveredByWildcardDefine(const QString &projectDir,
                                   const QString &pattern,
                                   const QString &absoluteFilePath);

} // namespace libdefine

#endif // LIBDEFINE_UTILS_H
