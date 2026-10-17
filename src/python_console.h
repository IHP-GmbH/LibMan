#ifndef PYTHON_CONSOLE_H
#define PYTHON_CONSOLE_H

#include <QObject>
#include <QByteArray>
#include <QProcess>
#include <QString>

/*!*********************************************************************************************************************
 * \brief Persistent out-of-process Python REPL with ROOM (`import room`) preloaded.
 *
 * Commands are sent as length-prefixed UTF-8 blocks; stdout/stderr are emitted as they arrive.
 * Completion of each command is marked with a sentinel so the UI can re-enable input.
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

private slots:
    void                                onReadyRead();
    void                                onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QString                             findPythonExecutable() const;
    QString                             findRoomPackageDir() const;
    QString                             findRoomCLibrary() const;
    QString                             writeDriverScript() const;
    void                                emitPendingOutput();

    QProcess                           *m_process = nullptr;
    QByteArray                          m_stdoutBuffer;
    bool                                m_busy = false;
};

#endif // PYTHON_CONSOLE_H
