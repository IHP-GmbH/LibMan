#include "core/core_file_lock.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <csignal>
#include <unistd.h>
#endif

namespace {

QString readJsonString(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (!value.isString()) {
        return QString();
    }
    return value.toString().trimmed();
}

bool isWindowsProcessAlive(qint64 pid)
{
    if (pid <= 0) {
        return false;
    }

#ifdef Q_OS_WIN
    const HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (handle == nullptr) {
        return false;
    }

    DWORD exitCode = 0;
    const bool alive = GetExitCodeProcess(handle, &exitCode) != 0 && exitCode == STILL_ACTIVE;
    CloseHandle(handle);
    return alive;
#else
    return ::kill(static_cast<pid_t>(pid), 0) == 0;
#endif
}

bool isWslProcessAlive(qint64 pid)
{
    if (pid <= 0) {
        return false;
    }

#ifdef Q_OS_WIN
    QProcess process;
    process.setProgram(QStringLiteral("wsl.exe"));
    process.setArguments(QStringList()
                         << QStringLiteral("-e")
                         << QStringLiteral("kill")
                         << QStringLiteral("-0")
                         << QString::number(pid));
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished(1000);
        return false;
    }
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
#else
    return ::kill(static_cast<pid_t>(pid), 0) == 0;
#endif
}

bool isLocalWindowsTool(const CoreFileLockInfo &info)
{
    if (info.tool.isEmpty()) {
        return true;
    }
    return info.tool.compare(QStringLiteral("qucs-s"), Qt::CaseInsensitive) == 0
        || info.tool.compare(QStringLiteral("klayout"), Qt::CaseInsensitive) == 0;
}

} // namespace

QString lockFilePathForCore(const QString &corePath)
{
    return QFileInfo(corePath).absoluteFilePath() + QStringLiteral(".lck");
}

CoreFileLockInfo readCoreLockFile(const QString &corePath)
{
    CoreFileLockInfo info;
    info.corePath = QFileInfo(corePath).absoluteFilePath();
    info.lockPath = lockFilePathForCore(info.corePath);
    info.present = QFileInfo::exists(info.lockPath);
    if (!info.present) {
        return info;
    }

    QFile file(info.lockPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        info.parseOk = false;
        info.parseError = QStringLiteral("Could not read lock file.");
        return info;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        info.parseOk = false;
        info.parseError = QStringLiteral("Lock file is not valid JSON.");
        return info;
    }

    const QJsonObject root = document.object();
    info.parseOk = true;
    info.createdAt = readJsonString(root, QStringLiteral("createdAt"));
    info.corePath = readJsonString(root, QStringLiteral("corePath"));
    if (info.corePath.isEmpty()) {
        info.corePath = QFileInfo(corePath).absoluteFilePath();
    }

    const QJsonObject holder = root.value(QStringLiteral("holder")).toObject();
    info.user = readJsonString(holder, QStringLiteral("user"));
    info.host = readJsonString(holder, QStringLiteral("host"));
    info.tool = readJsonString(holder, QStringLiteral("tool"));
    info.toolVersion = readJsonString(holder, QStringLiteral("toolVersion"));
    if (holder.value(QStringLiteral("pid")).isDouble()) {
        info.pid = holder.value(QStringLiteral("pid")).toInt();
    }

    return info;
}

bool isStaleCoreLock(const CoreFileLockInfo &info)
{
    if (!info.present) {
        return false;
    }

    if (!info.parseOk || info.pid <= 0) {
        return true;
    }

    if (isLocalWindowsTool(info)) {
        return !isWindowsProcessAlive(info.pid);
    }

    // xschem runs in WSL — Linux PID, probe via wsl.exe.
    return !isWslProcessAlive(info.pid);
}

CoreFileLockInfo readActiveCoreLockFile(const QString &corePath)
{
    CoreFileLockInfo info = readCoreLockFile(corePath);
    if (!info.present) {
        return info;
    }

    if (!isStaleCoreLock(info)) {
        return info;
    }

    QFile::remove(info.lockPath);
    CoreFileLockInfo cleared;
    cleared.corePath = QFileInfo(corePath).absoluteFilePath();
    cleared.lockPath = lockFilePathForCore(cleared.corePath);
    return cleared;
}

QString formatCoreLockInfoBlock(const CoreFileLockInfo &info)
{
    QString block = QStringLiteral("\tLock: ");
    if (!info.present) {
        block += QStringLiteral("none\n");
        return block;
    }

    block += QStringLiteral("active\n");
    block += QStringLiteral("\tLock file: ") + info.lockPath + QStringLiteral("\n");

    if (!info.parseOk) {
        block += QStringLiteral("\tLock details: ") + info.parseError + QStringLiteral("\n");
        return block;
    }

    if (!info.user.isEmpty() || !info.host.isEmpty()) {
        block += QStringLiteral("\tLocked by: ") + info.user;
        if (!info.host.isEmpty()) {
            block += QStringLiteral(" @ ") + info.host;
        }
        block += QStringLiteral("\n");
    }

    if (!info.tool.isEmpty()) {
        block += QStringLiteral("\tTool: ") + info.tool;
        if (!info.toolVersion.isEmpty()) {
            block += QStringLiteral(" ") + info.toolVersion;
        }
        block += QStringLiteral("\n");
    }

    if (info.pid > 0) {
        block += QStringLiteral("\tPID: ") + QString::number(info.pid) + QStringLiteral("\n");
    }

    if (!info.createdAt.isEmpty()) {
        block += QStringLiteral("\tSince: ") + info.createdAt + QStringLiteral("\n");
    }

    return block;
}
