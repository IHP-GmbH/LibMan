#ifndef CORE_IMPORT_SERVICE_H
#define CORE_IMPORT_SERVICE_H

#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
class MainWindow;

class CoreImportService
{
public:
    enum class Format {
        Gds,
        Xschem,
        Qucs,
        Oas,
    };

    struct ImportItemResult {
        QString sourcePath;
        QString destinationPath;
        bool    success = false;
        QString message;
    };

    explicit CoreImportService(MainWindow *mainWindow);

    using LogCallback = std::function<void(const QString &)>;

    static QString formatDisplayName(Format format);
    static QStringList sourceNameFilters(Format format);
    static QString converterBaseName(Format format);

    QVector<ImportItemResult> importFiles(Format format,
                                          const QString &libraryName,
                                          const QStringList &sourceFiles,
                                          bool overwriteExisting = false,
                                          const LogCallback &logCallback = {}) const;

private:
    ImportItemResult importOne(Format format,
                                 const QString &libraryName,
                                 const QString &sourcePath,
                                 bool overwriteExisting,
                                 const LogCallback &logCallback = {}) const;

    bool runConverter(const QString &program,
                      const QStringList &arguments,
                      QString *errorMessage,
                      const LogCallback &logCallback = {}) const;

    QString libraryRootPath(const QString &libraryName) const;
    QString destinationCorePath(const QString &libraryRoot,
                                const QString &cellName,
                                const QString &viewName) const;

    MainWindow *m_mainWindow = nullptr;
};

#endif // CORE_IMPORT_SERVICE_H
