#ifndef KLAYOUT_TOOLS_H
#define KLAYOUT_TOOLS_H

#include <QObject>
#include <QString>
#include <QStringList>

/*!*******************************************************************************************************************
 * \brief KLayout integration: persistent server, layout open scripts, and batch XOR.
 ********************************************************************************************************************/
class KLayoutTools : public QObject
{
    Q_OBJECT

public:
    explicit KLayoutTools(QObject *parent = nullptr);

    bool isServerRunning() const;
    bool ensureServerRunning(const QString &tool, const QString &projectFile);
    bool sendOpenRequest(const QString &gdsPath, const QString &cellName);
    bool sendSelectRequest(const QString &gdsPath, const QString &cellName);

    void openLayoutFile(const QString &tool,
                        const QString &layoutPath,
                        const QString &cellName,
                        const QString &projectFile);

    QString createOpenScript(const QString &gdsPath, const QString &cellName) const;
    QString createXorScript(const QString &pathA,
                            const QString &cellA,
                            const QString &pathB,
                            const QString &cellB,
                            const QString &outputPath) const;

    void startXorProcess(const QString &program,
                         const QStringList &args,
                         const QString &scriptPath,
                         const QString &outputPath);

signals:
    void info(const QString &msg);
    void error(const QString &msg);
    void fileLink(const QString &path, const QString &label);

private:
    QString createServerScript(const QString &cmdFile) const;
    void startToolWithTempScript(const QString &tool,
                                 const QStringList &args,
                                 const QString &scriptPath);
    void logXorProcessOutput(const QString &text);

    qint64  m_serverPid      = 0;
    QString m_cmdFile;
    QString m_serverScript;
};

#endif // KLAYOUT_TOOLS_H
