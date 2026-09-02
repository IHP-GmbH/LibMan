#include "libdefine_utils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

namespace libdefine {

namespace {

QString normalizedPattern(const QString &pattern)
{
    return QDir::fromNativeSeparators(pattern.trimmed());
}

QString absolutePath(const QString &projectDir, const QString &relativePath)
{
    if (QDir::isAbsolutePath(relativePath)) {
        return QDir::cleanPath(relativePath);
    }
    return QDir::cleanPath(QDir(projectDir).absoluteFilePath(relativePath));
}

bool isCellLevelWildcard(const QString &pattern)
{
    const QString normalized = normalizedPattern(pattern);
    if (!normalized.endsWith(QStringLiteral("/*"))) {
        return false;
    }

    const QString base = normalized.left(normalized.size() - 2);
    const QString libName = base.section(QLatin1Char('/'), 0, 0);
    return base.contains(QLatin1Char('/')) && base.section(QLatin1Char('/'), -1) != libName;
}

} // namespace

bool isWildcardDefinePath(const QString &path)
{
    const QString normalized = normalizedPattern(path);
    return normalized.endsWith(QStringLiteral("/*"));
}

QString wildcardPatternToRelative(const QString &pattern)
{
    return QDir::fromNativeSeparators(pattern.trimmed());
}

QString wildcardScanRoot(const QString &projectDir, const QString &pattern)
{
    if (!isWildcardDefinePath(pattern)) {
        return QString();
    }

    const QString normalized = normalizedPattern(pattern);
    const QString base = normalized.left(normalized.size() - 2);
    const QString absBase = absolutePath(projectDir, base);
    const QFileInfo baseInfo(absBase);
    if (!baseInfo.exists()) {
        return QString();
    }
    return baseInfo.isDir() ? baseInfo.absoluteFilePath() : baseInfo.absolutePath();
}

QString wildcardLibraryRoot(const QString &projectDir, const QString &pattern)
{
    if (!isWildcardDefinePath(pattern)) {
        return QString();
    }

    const QString scanRoot = wildcardScanRoot(projectDir, pattern);
    if (scanRoot.isEmpty()) {
        return QString();
    }

    if (isCellLevelWildcard(pattern)) {
        return QFileInfo(scanRoot).absolutePath();
    }

    return scanRoot;
}

QStringList expandWildcardDefinePath(const QString &projectDir, const QString &pattern)
{
    QStringList paths;
    if (!isWildcardDefinePath(pattern)) {
        return paths;
    }

    const QString scanRoot = wildcardScanRoot(projectDir, pattern);
    if (scanRoot.isEmpty()) {
        return paths;
    }

    const QStringList nameFilters{QStringLiteral("*.core")};

    const QDirIterator::IteratorFlags flags =
        isCellLevelWildcard(pattern) ? QDirIterator::NoIteratorFlags : QDirIterator::Subdirectories;

    QDirIterator it(scanRoot, nameFilters, QDir::Files, flags);
    while (it.hasNext()) {
        paths.append(QDir::toNativeSeparators(QFileInfo(it.next()).absoluteFilePath()));
    }

    paths.sort();
    paths.removeDuplicates();
    return paths;
}

bool isPathCoveredByWildcardDefine(const QString &projectDir,
                                   const QString &pattern,
                                   const QString &absoluteFilePath)
{
    const QString target = QDir::toNativeSeparators(QFileInfo(absoluteFilePath).absoluteFilePath());
    const QStringList expanded = expandWildcardDefinePath(projectDir, pattern);
    for (const QString &candidate : expanded) {
        if (QDir::toNativeSeparators(QFileInfo(candidate).absoluteFilePath()) == target) {
            return true;
        }
    }
    return false;
}

} // namespace libdefine
