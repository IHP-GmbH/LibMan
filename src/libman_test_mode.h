#ifndef LIBMAN_TEST_MODE_H
#define LIBMAN_TEST_MODE_H

#include <QByteArray>

inline bool libmanAutomatedTestRun()
{
    return !qgetenv("LIBMAN_TEST_DATA_DIR").isEmpty();
}

#endif // LIBMAN_TEST_MODE_H
