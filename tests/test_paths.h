#ifndef TEST_PATHS_H
#define TEST_PATHS_H

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include <QtTest/QtTest>

/*!********************************************************************************************************
 * \brief Returns absolute path to tests/data (or directory containing sg13g2.projects).
 *
 * Prefers LIBMAN_TEST_DATA_DIR when set (CI/local run_tests scripts). Otherwise falls back to
 * QFINDTESTDATA, which resolves next to the test binary (build-tests/data after qmake TESTDATA).
 *********************************************************************************************************/
inline QString libmanTestDataDir()
{
    const QByteArray env = qgetenv("LIBMAN_TEST_DATA_DIR");
    if(!env.isEmpty()) {
        return QDir(QString::fromUtf8(env)).absolutePath();
    }

    const QString projectsFile = QFINDTESTDATA("data/sg13g2.projects");
    if(!projectsFile.isEmpty()) {
        return QFileInfo(projectsFile).absolutePath();
    }

    return QString();
}

/*!********************************************************************************************************
 * \brief Returns absolute path to the sg13g2.projects fixture.
 *********************************************************************************************************/
inline QString libmanTestProjectFile()
{
    const QString dataDir = libmanTestDataDir();
    if(dataDir.isEmpty()) {
        return QString();
    }

    const QString projectsFile = QDir(dataDir).filePath(QStringLiteral("sg13g2.projects"));
    if(!QFileInfo::exists(projectsFile)) {
        return QString();
    }

    return QFileInfo(projectsFile).absoluteFilePath();
}

/*!********************************************************************************************************
 * \brief Resolves a path relative to the test data directory (forward slashes).
 *********************************************************************************************************/
inline QString libmanTestDataFile(const QString &relativeUnderData)
{
    const QString dataDir = libmanTestDataDir();
    if(dataDir.isEmpty()) {
        return QString();
    }

    return QDir(dataDir).filePath(relativeUnderData);
}

#endif // TEST_PATHS_H
