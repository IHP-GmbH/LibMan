#include "python_console.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace {

constexpr const char kDoneSentinel[] = "@@LIBMAN_DONE@@";

const char kDriverScript[] = R"PY(
import sys
import traceback

g = {"__name__": "__main__"}
try:
    import room
    g["room"] = room
except Exception as e:
    print("WARNING: import room failed: %s" % e, file=sys.stderr)

sys.stdout.flush()
sys.stderr.flush()

while True:
    header = sys.stdin.readline()
    if not header:
        break
    header = header.strip()
    if not header:
        continue
    try:
        nbytes = int(header)
    except ValueError:
        print("ERROR: bad command header: %r" % (header,), file=sys.stderr)
        print("@@LIBMAN_DONE@@")
        sys.stdout.flush()
        sys.stderr.flush()
        continue
    buf = sys.stdin.read(nbytes)
    if len(buf) < nbytes:
        break
    src = buf if isinstance(buf, str) else buf.decode("utf-8", "replace")
    try:
        # Prefer expression (shows repr); assignment/statements need exec.
        # Do not nest try/except — otherwise a later RuntimeError chains a
        # confusing "During handling of SyntaxError" for simple `db = ...`.
        try:
            code = compile(src, "<libman>", "eval")
        except SyntaxError:
            code = None
        if code is not None:
            result = eval(code, g, g)
            if result is not None:
                print(repr(result))
        else:
            exec(compile(src, "<libman>", "exec"), g, g)
    except SystemExit:
        raise
    except BaseException:
        traceback.print_exc()
    print("@@LIBMAN_DONE@@")
    sys.stdout.flush()
    sys.stderr.flush()
)PY";

QStringList uniqueExistingDirs(const QStringList &candidates)
{
    QStringList out;
    for (const QString &path : candidates) {
        if (path.isEmpty()) {
            continue;
        }
        const QFileInfo fi(path);
        if (!fi.exists() || !fi.isDir()) {
            continue;
        }
        const QString abs = fi.absoluteFilePath();
        if (!out.contains(abs)) {
            out.append(abs);
        }
    }
    return out;
}

} // namespace

PythonConsole::PythonConsole(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyRead, this, &PythonConsole::onReadyRead);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &PythonConsole::onProcessFinished);
}

PythonConsole::~PythonConsole()
{
    stop();
}

bool PythonConsole::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

bool PythonConsole::isBusy() const
{
    return m_busy;
}

QString PythonConsole::findPythonExecutable() const
{
    const QString fromEnv = qEnvironmentVariable("LIBMAN_PYTHON").trimmed();
    if (!fromEnv.isEmpty() && QFileInfo::exists(fromEnv)) {
        return QFileInfo(fromEnv).absoluteFilePath();
    }

    const QStringList names = {
        QStringLiteral("python"),
        QStringLiteral("python3"),
#ifdef Q_OS_WIN
        QStringLiteral("python.exe"),
        QStringLiteral("python3.exe"),
#endif
    };
    for (const QString &name : names) {
        const QString found = QStandardPaths::findExecutable(name);
        if (!found.isEmpty()) {
            return found;
        }
    }

#ifdef Q_OS_WIN
    const QStringList winCandidates = {
        QDir::home().filePath(QStringLiteral("AppData/Local/Programs/Python/Python311/python.exe")),
        QDir::home().filePath(QStringLiteral("AppData/Local/Programs/Python/Python312/python.exe")),
        QDir::home().filePath(QStringLiteral("AppData/Local/Programs/Python/Python313/python.exe")),
    };
    for (const QString &path : winCandidates) {
        if (QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
#endif
    return {};
}

QString PythonConsole::findRoomPackageDir() const
{
    const QString env = qEnvironmentVariable("LIBMAN_ROOM_PYTHON").trimmed();
    QStringList candidates;
    if (!env.isEmpty()) {
        candidates << env;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    candidates << QDir(appDir).filePath(QStringLiteral("python"));
    // Dev / sibling layouts: LibMan/build -> ../CommonDB/python, or repo root.
    candidates << QDir(appDir).filePath(QStringLiteral("../CommonDB/python"));
    candidates << QDir(appDir).filePath(QStringLiteral("../../CommonDB/python"));
    candidates << QDir(appDir).filePath(QStringLiteral("../../../CommonDB/python"));
    candidates << QDir(appDir).filePath(QStringLiteral(".deps/Room/python"));
    candidates << QDir(appDir).filePath(QStringLiteral("../.deps/Room/python"));

    for (const QString &dir : uniqueExistingDirs(candidates)) {
        if (QFileInfo::exists(QDir(dir).filePath(QStringLiteral("room/__init__.py")))) {
            return dir;
        }
    }
    return {};
}

QString PythonConsole::findRoomCLibrary() const
{
    const QString env = qEnvironmentVariable("ROOM_C_DLL").trimmed();
    if (!env.isEmpty() && QFileInfo::exists(env)) {
        return QFileInfo(env).absoluteFilePath();
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList names;
#ifdef Q_OS_WIN
    names << QStringLiteral("room_c.dll") << QStringLiteral("libroom_c.dll");
#elif defined(Q_OS_MACOS)
    names << QStringLiteral("libroom_c.dylib") << QStringLiteral("room_c.dylib");
#else
    names << QStringLiteral("libroom_c.so") << QStringLiteral("room_c.so");
#endif

    QStringList dirs;
    dirs << appDir
         << QDir(appDir).filePath(QStringLiteral("python"))
         << QDir(appDir).filePath(QStringLiteral("_deps/commondb-build/python"))
         << QDir(appDir).filePath(QStringLiteral("../_deps/commondb-build/python"))
         << QDir(appDir).filePath(QStringLiteral("../../CommonDB/build-python/python"))
         << QDir(appDir).filePath(QStringLiteral("../../CommonDB/build/python"));

    for (const QString &dir : uniqueExistingDirs(dirs)) {
        for (const QString &name : names) {
            const QString candidate = QDir(dir).filePath(name);
            if (QFileInfo::exists(candidate)) {
                return QFileInfo(candidate).absoluteFilePath();
            }
        }
    }
    return {};
}

QString PythonConsole::writeDriverScript() const
{
    QTemporaryFile file(QDir::temp().filePath(QStringLiteral("libman_py_XXXXXX.py")));
    file.setAutoRemove(false);
    if (!file.open()) {
        return {};
    }
    if (file.write(kDriverScript) < 0) {
        return {};
    }
    file.flush();
    const QString path = file.fileName();
    file.close();
    return path;
}

void PythonConsole::start()
{
    if (isRunning()) {
        return;
    }

    const QString python = findPythonExecutable();
    if (python.isEmpty()) {
        emit failed(tr("Python interpreter not found. Set LIBMAN_PYTHON or install Python on PATH."));
        return;
    }

    const QString driver = writeDriverScript();
    if (driver.isEmpty()) {
        emit failed(tr("Failed to create temporary Python console driver."));
        return;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString roomPkg = findRoomPackageDir();
    if (!roomPkg.isEmpty()) {
        const QString existing = env.value(QStringLiteral("PYTHONPATH"));
        env.insert(QStringLiteral("PYTHONPATH"),
                   existing.isEmpty() ? roomPkg : (roomPkg + QDir::listSeparator() + existing));
    }
    const QString roomC = findRoomCLibrary();
    if (!roomC.isEmpty()) {
        env.insert(QStringLiteral("ROOM_C_DLL"), roomC);
    }

    m_process->setProcessEnvironment(env);
    m_process->setProgram(python);
    m_process->setArguments({QStringLiteral("-u"), driver});
    m_stdoutBuffer.clear();
    m_busy = false;
    m_process->start();

    if (!m_process->waitForStarted(10000)) {
        emit failed(tr("Failed to start Python: %1").arg(m_process->errorString()));
        QFile::remove(driver);
        return;
    }

    QString detail = tr("Python: %1").arg(QDir::toNativeSeparators(python));
    if (!roomPkg.isEmpty()) {
        detail += tr("\nPYTHONPATH: %1").arg(QDir::toNativeSeparators(roomPkg));
    }
    if (!roomC.isEmpty()) {
        detail += tr("\nROOM_C_DLL: %1").arg(QDir::toNativeSeparators(roomC));
    }
    emit started(detail);
}

void PythonConsole::stop()
{
    if (!m_process) {
        return;
    }
    if (m_process->state() != QProcess::NotRunning) {
        m_process->write("0\n");
        m_process->closeWriteChannel();
        if (!m_process->waitForFinished(1500)) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }
    m_busy = false;
}

void PythonConsole::execute(const QString &code)
{
    if (!isRunning()) {
        start();
    }
    if (!isRunning()) {
        emit commandFinished(false);
        return;
    }
    if (m_busy) {
        emit output(tr("Python console is busy; wait for the previous command to finish."));
        emit commandFinished(false);
        return;
    }

    QString trimmed = code;
    while (trimmed.endsWith(QLatin1Char('\n')) || trimmed.endsWith(QLatin1Char('\r'))) {
        trimmed.chop(1);
    }
    if (trimmed.trimmed().isEmpty()) {
        emit commandFinished(true);
        return;
    }

    const QByteArray payload = trimmed.toUtf8();
    m_busy = true;
    const QByteArray header = QByteArray::number(payload.size()) + '\n';
    if (m_process->write(header) < 0 || m_process->write(payload) < 0) {
        m_busy = false;
        emit output(tr("Failed to write command to Python process."));
        emit commandFinished(false);
        return;
    }
}

void PythonConsole::onReadyRead()
{
    m_stdoutBuffer += m_process->readAll();
    emitPendingOutput();
}

void PythonConsole::emitPendingOutput()
{
    while (true) {
        const int idx = m_stdoutBuffer.indexOf(kDoneSentinel);
        if (idx < 0) {
            // Stream incomplete lines only up to last newline to avoid cutting UTF-8 mid-command.
            const int lastNl = m_stdoutBuffer.lastIndexOf('\n');
            if (lastNl < 0) {
                return;
            }
            const QByteArray chunk = m_stdoutBuffer.left(lastNl + 1);
            m_stdoutBuffer.remove(0, lastNl + 1);
            const QString text = QString::fromUtf8(chunk);
            if (!text.isEmpty()) {
                emit output(text);
            }
            return;
        }

        const QByteArray before = m_stdoutBuffer.left(idx);
        m_stdoutBuffer.remove(0, idx + int(sizeof(kDoneSentinel) - 1));
        // Swallow trailing newline after sentinel.
        if (m_stdoutBuffer.startsWith('\r')) {
            m_stdoutBuffer.remove(0, 1);
        }
        if (m_stdoutBuffer.startsWith('\n')) {
            m_stdoutBuffer.remove(0, 1);
        }

        const QString text = QString::fromUtf8(before);
        if (!text.isEmpty()) {
            emit output(text);
        }
        m_busy = false;
        emit commandFinished(true);
    }
}

void PythonConsole::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    emitPendingOutput();
    if (!m_stdoutBuffer.isEmpty()) {
        emit output(QString::fromUtf8(m_stdoutBuffer));
        m_stdoutBuffer.clear();
    }
    m_busy = false;
    if (status != QProcess::NormalExit || exitCode != 0) {
        emit failed(tr("Python console exited (code %1).").arg(exitCode));
    }
    emit commandFinished(false);
}
