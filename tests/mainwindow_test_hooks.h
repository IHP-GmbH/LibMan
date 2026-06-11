#ifndef MAINWINDOW_TEST_HOOKS_H
#define MAINWINDOW_TEST_HOOKS_H

#include "mainwindow.h"

#include <QListWidget>
#include <QMap>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

/*!
 * \brief Test-only accessors for private MainWindow helpers (LIBMAN_TESTING builds).
 */
class MainWindowTestHooks
{
public:
    static bool removeDir(MainWindow *w, const QString &dirName)
    {
        return w->removeDir(dirName);
    }

    static void copyDir(MainWindow *w, const QString &sourceFolder, const QString &destFolder)
    {
        w->copyDir(sourceFolder, destFolder);
    }

    static QString expandShellVariables(MainWindow *w, const QString &path)
    {
        return w->expandShellVariables(path);
    }

    static QString detectViewFromPath(MainWindow *w, const QString &filePath)
    {
        return w->detectViewFromPath(filePath);
    }

    static QString generateCopyName(MainWindow *w,
                                    const QString &name,
                                    const QString &path,
                                    const QString &suffix)
    {
        return w->generateCopyName(name, path, suffix);
    }

    static QString getViewPath(MainWindow *w,
                               const QString &libName,
                               const QString &groupName,
                               const QString &viewName)
    {
        return w->getViewPath(libName, groupName, viewName);
    }

    static QString getCurrentViewFilePath(MainWindow *w, const QString &viewName)
    {
        return w->getCurrentViewFilePath(viewName);
    }

    static QString findRepresentativeLibraryFile(MainWindow *w, const QString &libName)
    {
        return w->findRepresentativeLibraryFile(libName);
    }

    static QMap<QString, QStringList> getSupportedViewsByTool(MainWindow *w)
    {
        return w->getSupportedViewsByTool();
    }

    static QString getLibraryPath(MainWindow *w, const QString &libName)
    {
        return w->getLibraryPath(libName);
    }

    static void setLibraryRootDirectory(MainWindow *w,
                                      const QString &libName,
                                      const QString &dirPath)
    {
        w->setLibraryRootDirectory(libName, dirPath);
    }

    static bool importCellViewFile(MainWindow *w,
                                   const QString &libName,
                                   const QString &srcFilePath)
    {
        return w->importCellViewFile(libName, srcFilePath);
    }

    static void loadDocuments(MainWindow *w, const QString &libPath)
    {
        w->loadDocuments(libPath);
    }

    static void loadCategories(MainWindow *w, const QString &libPath)
    {
        w->loadCategories(libPath);
    }

    static void loadCombinedLibs(MainWindow *w, const QMap<QString, QStringList> &combinedLibs)
    {
        w->loadCombinedLibs(combinedLibs);
    }

    static void hideTreeItem(MainWindow *w, QTreeWidget *tree, const QString &filter)
    {
        w->hideTreeItem(tree, filter);
    }

    static void hideListItem(MainWindow *w, QListWidget *list, const QString &filter)
    {
        w->hideListItem(list, filter);
    }

    static bool filterTreeItem(MainWindow *w, QTreeWidgetItem *item, const QString &filter)
    {
        return w->filterTreeItem(item, filter);
    }
};

#endif // MAINWINDOW_TEST_HOOKS_H
