#ifndef CORE_EXPORT_SERVICE_H
#define CORE_EXPORT_SERVICE_H

#include <QString>
#include <QStringList>
#include <QVector>

class MainWindow;

class CoreExportService
{
public:
    enum class Format {
        Gds,
        Xschem,
        Qucs,
    };

    struct ExportItemResult {
        QString sourcePath;
        QString destinationPath;
        bool    success = false;
        QString message;
    };

    explicit CoreExportService(MainWindow *mainWindow);

    static QString formatDisplayName(Format format);
    static QStringList sourceNameFilters();
    static QString converterBaseName(Format format);
    static bool coreViewMatchesFormat(Format format, const QString &viewName);

    QVector<ExportItemResult> exportFiles(Format format,
                                          const QString &destinationDir,
                                          const QStringList &sourceFiles) const;

private:
    ExportItemResult exportOne(Format format,
                               const QString &destinationDir,
                               const QString &sourcePath) const;

    bool runConverter(const QString &program,
                      const QStringList &arguments,
                      QString *errorMessage) const;

    QString destinationFilePath(const QString &destinationDir,
                                const QString &cellName,
                                const QString &viewName) const;

    MainWindow *m_mainWindow = nullptr;
};

#endif // CORE_EXPORT_SERVICE_H
