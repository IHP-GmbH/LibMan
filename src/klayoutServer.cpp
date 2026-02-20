#include "src/mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>

/*!*******************************************************************************************************************
 * \brief Sends an "open GDS" request to a running KLayout server instance.
 *
 * The request is written as JSON into a command file polled by KLayout.
 * If \c cellName is empty, the whole layout is opened.
 *
 * \param gdsPath   Absolute or relative path to the GDS file.
 * \param cellName  Optional cell name to activate.
 * \return true on success, false otherwise.
 **********************************************************************************************************************/
bool MainWindow::sendKLayoutOpenRequest(const QString &gdsPath, const QString &cellName)
{
    if (m_klayoutCmdFile.isEmpty()) {
        return false;
    }

    const QString fileAbs = QFileInfo(gdsPath).absoluteFilePath();
    const QString tmp = m_klayoutCmdFile + ".tmp";

    QJsonObject obj;
    obj["action"] = "open";
    obj["file"]   = fileAbs;
    obj["cell"]   = cellName;

    const QByteArray payload =
        QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QFile f(tmp);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    f.write(payload);
    f.close();

    QFile::remove(m_klayoutCmdFile);
    if (!QFile::rename(tmp, m_klayoutCmdFile)) {
        QFile f2(m_klayoutCmdFile);
        if (!f2.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            return false;
        }
        f2.write(payload);
        f2.close();
    }

    return true;
}

/*!*******************************************************************************************************************
 * \brief Ensures that a KLayout instance with a polling server script is running.
 *
 * If a KLayout process is already running, this function does nothing.
 * Otherwise it starts KLayout with a generated server script that polls
 * a command JSON file.
 *
 * \param tool  Executable name or absolute path to KLayout.
 * \return true if KLayout is running or was successfully started.
 **********************************************************************************************************************/
bool MainWindow::ensureKLayoutServerRunning(const QString &tool)
{
    if (tool.isEmpty()) {
        return false;
    }

    if (m_klayoutProc && m_klayoutProc->state() != QProcess::NotRunning) {
        return true;
    }

    if (m_klayoutProc) {
        m_klayoutProc->deleteLater();
        m_klayoutProc = nullptr;
    }

    if (m_klayoutCmdFile.isEmpty()) {
        const QString base =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m_klayoutCmdFile = QDir::toNativeSeparators(
            base + "/libman_klayout_cmd_" +
            QString::number(QCoreApplication::applicationPid()) + ".json");
    }

    {
        QFile f(m_klayoutCmdFile);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write("");
            f.close();
        }
    }

    if (m_klayoutServerScript.isEmpty() ||
        !QFileInfo(m_klayoutServerScript).exists()) {

        m_klayoutServerScript =
            createKLayoutServerScript(m_klayoutCmdFile);

        if (m_klayoutServerScript.isEmpty() ||
            !QFileInfo(m_klayoutServerScript).exists()) {
            return false;
        }
    }

    m_klayoutProc = new QProcess(this);

    connect(m_klayoutProc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int, QProcess::ExitStatus)
            {
                if (m_klayoutProc) {
                    m_klayoutProc->deleteLater();
                    m_klayoutProc = nullptr;
                }
            });

    QStringList args;
    args << "-rr" << m_klayoutServerScript;

    m_klayoutProc->start(tool, args);
    if (!m_klayoutProc->waitForStarted(3000)) {
        m_klayoutProc->deleteLater();
        m_klayoutProc = nullptr;
        return false;
    }

    return true;
}

/*!*******************************************************************************************************************
 * \brief Sends a "select" request to the running KLayout server.
 *
 * This function writes a JSON command with action="select" into the command file
 * used by the persistent KLayout server instance.
 *
 * Unlike sendKLayoutOpenRequest(), this command:
 *  - does NOT load the layout if it is not already opened,
 *  - only selects the specified cell in an already opened view,
 *  - does nothing if the file or cell is not found.
 *
 * \param gdsPath   Absolute or relative path to the GDS file.
 * \param cellName  Name of the cell to select inside the already opened layout.
 * \return true if the command file was written successfully, otherwise false.
 **********************************************************************************************************************/
bool MainWindow::sendKLayoutSelectRequest(const QString &gdsPath, const QString &cellName)
{
    if (m_klayoutCmdFile.isEmpty()) {
        return false;
    }

    const QString tmp = m_klayoutCmdFile + ".tmp";
    const QString fileAbs = QFileInfo(gdsPath).absoluteFilePath();

    QJsonObject obj;
    obj["action"] = "select";
    obj["file"]   = fileAbs;
    obj["cell"]   = cellName;

    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QFile f(tmp);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    f.write(payload);
    f.close();

    QFile::remove(m_klayoutCmdFile);
    if (!QFile::rename(tmp, m_klayoutCmdFile)) {
        QFile f2(m_klayoutCmdFile);
        if (!f2.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            return false;
        }
        f2.write(payload);
        f2.close();
    }

    return true;
}

/*!*******************************************************************************************************************
 * \brief Creates a persistent KLayout server script.
 *
 * The script polls a JSON command file and reacts to commands:
 *  - action="open":   opens/loads a GDS file into an existing view and optionally selects a cell.
 *  - action="select": selects a cell in an already opened view (does not load a file).
 *
 * The script also tries to raise/activate the KLayout main window to make the action visible.
 *
 * \param cmdFile  Absolute path to the command JSON file.
 * \return Absolute path to the generated Python script.
 **********************************************************************************************************************/
QString MainWindow::createKLayoutServerScript(const QString &cmdFile) const
{
    auto pyRaw = [](const QString &s) -> QString {
        QString t = QDir::toNativeSeparators(s);
        t.replace("\\", "\\\\");
        t.replace("'", "\\'");
        return QString("r'%1'").arg(t);
    };

    QTemporaryFile tf(QDir::tempPath() +
                      QDir::separator() +
                      "libman_klayout_server_XXXXXX.py");
    tf.setAutoRemove(false);

    if (!tf.open()) {
        return QString();
    }

    QTextStream out(&tf);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif

    out <<
        R"(# -*- coding: utf-8 -*-
import pya
import os
import json

CMD_FILE = )" << pyRaw(cmdFile) << R"(

_app = pya.Application.instance()
_mw  = _app.main_window() if _app is not None else None

def _norm(p):
    try:
        return os.path.normcase(os.path.normpath(p))
    except Exception:
        return p

def _raise_main_window():
    # Make action visible: bring KLayout to front
    try:
        _mw.raise_()
        _mw.activateWindow()
    except Exception:
        pass

def _find_view_for_file(fn):
    if _mw is None:
        return (None, None, -1, -1)
    for lv_idx in range(_mw.views()):
        lv = _mw.view(lv_idx)
        for i in range(lv.cellviews()):
            cv = lv.cellview(i)
            try:
                if _norm(cv.filename()) == _norm(fn):
                    return (lv, cv, lv_idx, i)
            except Exception:
                pass
    return (None, None, -1, -1)

def _open_or_load(fn):
    # Ensure file is loaded into SOME view (same-view mode)
    (lv, cv, lv_idx, cv_idx) = _find_view_for_file(fn)
    if cv_idx == -1:
        _mw.load_layout(fn, 1)   # 1 = same view
        (lv, cv, lv_idx, cv_idx) = _find_view_for_file(fn)
    return (lv, cv, lv_idx, cv_idx)

def _select_cell(lv, cv, cv_idx, cell):
    if not cell:
        return False
    try:
        c = cv.layout().cell_by_name(cell)
        if c is not None:
            lv.select_cell(c, cv_idx)
            return True
    except Exception:
        pass
    return False

def _zoom_fit_delayed():
    # Do zoom_fit only when view is ready
    try:
        mw2 = pya.Application.instance().main_window()
        lv2 = mw2.current_view() if mw2 is not None else None
        ready = (lv2 is not None) and (lv2.cellviews() > 0) and (lv2.active_cellview() is not None)
        if ready:
            try:
                lv2.zoom_fit()
            except Exception:
                pass
            return True
    except Exception:
        pass
    return False

def _schedule_zoom_fit():
    global _fit_timer
    try:
        _fit_timer
    except NameError:
        _fit_timer = None

    if _fit_timer is None:
        _fit_timer = pya.QTimer(_mw)
        _fit_timer.setSingleShot(True)

        def _try_fit():
            if not _zoom_fit_delayed():
                _fit_timer.start(200)

        _fit_timer.timeout(_try_fit)

    if _fit_timer.isActive():
        _fit_timer.stop()
    _fit_timer.start(200)

def _handle(cmd):
    if _mw is None:
        return

    action = cmd.get("action", "")
    fn     = cmd.get("file", "")
    cell   = cmd.get("cell", "")

    if action not in ("open", "select"):
        return

    if not fn:
        return

    # For "select" do not load file - only operate if file is already open
    if action == "select":
        (lv, cv, lv_idx, cv_idx) = _find_view_for_file(fn)
        if lv is None:
            return
        _mw.select_view(lv_idx)
        _select_cell(lv, cv, cv_idx, cell)
        _raise_main_window()
        _schedule_zoom_fit()
        return

    # action == "open": load if needed
    if not os.path.exists(fn):
        return

    (lv, cv, lv_idx, cv_idx) = _open_or_load(fn)
    if lv is None:
        return

    _mw.select_view(lv_idx)
    _select_cell(lv, cv, cv_idx, cell)

    _raise_main_window()
    _schedule_zoom_fit()

def _poll():
    if not os.path.exists(CMD_FILE):
        return
    try:
        txt = open(CMD_FILE, "r", encoding="utf-8").read()
        if not txt.strip():
            return
        cmd = json.loads(txt)
        open(CMD_FILE, "w").close()
        _handle(cmd)
    except Exception:
        try:
            open(CMD_FILE, "w").close()
        except Exception:
            pass

if _mw is not None:
    _t = pya.QTimer(_mw)
    _t.timeout(_poll)
    _t.start(250)
)";

    out.flush();
    tf.close();
    return tf.fileName();
}
