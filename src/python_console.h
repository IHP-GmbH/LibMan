#ifndef PYTHON_CONSOLE_H
#define PYTHON_CONSOLE_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
#include <QProcess>
#include <QString>

/*!*********************************************************************************************************************
 * \brief Persistent out-of-process Python REPL with ROOM (`import room`) preloaded.
 *
 * Driver script: ``python/libman_console.py`` (next to the binary or in the source tree).
 * Commands are sent as length-prefixed UTF-8 blocks; stdout/stderr are emitted as they arrive.
 * Completion of each command is marked with a sentinel so the UI can re-enable input.
 *
 * Protocol lines ``@@LIBMAN_JSON@@{...}`` are not shown in Messages; they are emitted as
 * ``libmanCommand`` so MainWindow can update the project without a full reload dialog.
 **********************************************************************************************************************/
class PythonConsole : public QObject
{
    Q_OBJECT

public:
    explicit PythonConsole(QObject *parent = nullptr);
    ~PythonConsole() override;

    bool                                isRunning() const;
    bool                                isBusy() const;

public slots:
    void                                start();
    void                                stop();
    void                                execute(const QString &code);

signals:
    void                                started(const QString &detail);
    void                                failed(const QString &reason);
    void                                output(const QString &text);
    void                                commandFinished(bool ok);
    void                                libmanCommand(const QJsonObject &cmd);

private slots:
    void                                onReadyRead();
    void                                onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QString                             findPythonExecutable() const;
    QString                             findRoomPackageDir() const;
    QString                             findRoomCLibrary() const;
    QString                             findDriverScript() const;
    QString                             writeEmbeddedDriverFallback() const;
    void                                emitPendingOutput();
    void                                handleOutputChunk(const QString &chunk);

    QProcess                           *m_process = nullptr;
    QByteArray                          m_stdoutBuffer;
    QString                             m_lineBuffer;
    bool                                m_busy = false;
};

#endif // PYTHON_CONSOLE_H
