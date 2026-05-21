#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTextStream>

#define private public
#include "src/mainwindow.h"
#include "tst_klayout_requests.h"
#undef private

/*!*********************************************************************************************************************
 * \brief Verifies that sendKLayoutOpenRequest() fails when command file path is empty.
 **********************************************************************************************************************/
void KLayoutRequestsTest::sendKLayoutOpenRequest_emptyCommandFile_returnsFalse()
{
    MainWindow w(QString(), QDir::currentPath());
    w.m_klayoutCmdFile.clear();

    QVERIFY(!w.sendKLayoutOpenRequest("dummy.gds", "TOP"));
}

/*!*********************************************************************************************************************
 * \brief Verifies that sendKLayoutOpenRequest() writes correct JSON command file.
 **********************************************************************************************************************/
void KLayoutRequestsTest::sendKLayoutOpenRequest_writesJsonCommand()
{
    MainWindow w(QString(), QDir::currentPath());

    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temporary directory");

    const QString gdsPath = dir.filePath("sample.gds");
    {
        QFile f(gdsPath);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("dummy");
        f.close();
    }

    const QString cmdFile = dir.filePath("klayout_cmd.json");
    w.m_klayoutCmdFile = cmdFile;

    QVERIFY(w.sendKLayoutOpenRequest(gdsPath, "TOP"));

    QFile f(cmdFile);
    QVERIFY2(f.exists(), "Command file was not created");
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));

    const QByteArray data = f.readAll();
    f.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY2(doc.isObject(), "Command file does not contain valid JSON object");

    const QJsonObject obj = doc.object();

    QCOMPARE(obj.value("action").toString(), QString("open"));
    QCOMPARE(obj.value("cell").toString(), QString("TOP"));
    QCOMPARE(obj.value("file").toString(), QFileInfo(gdsPath).absoluteFilePath());
}

/*!*********************************************************************************************************************
 * \brief Verifies that sendKLayoutSelectRequest() fails when command file path is empty.
 **********************************************************************************************************************/
void KLayoutRequestsTest::sendKLayoutSelectRequest_emptyCommandFile_returnsFalse()
{
    MainWindow w(QString(), QDir::currentPath());
    w.m_klayoutCmdFile.clear();

    QVERIFY(!w.sendKLayoutSelectRequest("dummy.gds", "TOP"));
}

/*!*********************************************************************************************************************
 * \brief Verifies that sendKLayoutSelectRequest() writes correct JSON command file.
 **********************************************************************************************************************/
void KLayoutRequestsTest::sendKLayoutSelectRequest_writesJsonCommand()
{
    MainWindow w(QString(), QDir::currentPath());

    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temporary directory");

    const QString gdsPath = dir.filePath("sample.gds");
    {
        QFile f(gdsPath);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("dummy");
        f.close();
    }

    const QString cmdFile = dir.filePath("klayout_cmd.json");
    w.m_klayoutCmdFile = cmdFile;

    QVERIFY(w.sendKLayoutSelectRequest(gdsPath, "MY_CELL"));

    QFile f(cmdFile);
    QVERIFY2(f.exists(), "Command file was not created");
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));

    const QByteArray data = f.readAll();
    f.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY2(doc.isObject(), "Command file does not contain valid JSON object");

    const QJsonObject obj = doc.object();

    QCOMPARE(obj.value("action").toString(), QString("select"));
    QCOMPARE(obj.value("cell").toString(), QString("MY_CELL"));
    QCOMPARE(obj.value("file").toString(), QFileInfo(gdsPath).absoluteFilePath());
}

/*!*********************************************************************************************************************
 * \brief Verifies that createKLayoutServerScript() creates a Python file with expected content.
 **********************************************************************************************************************/
void KLayoutRequestsTest::createKLayoutServerScript_createsPythonFile()
{
    MainWindow w(QString(), QDir::currentPath());

    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temporary directory");

    const QString cmdFile = dir.filePath("server_cmd.json");
    const QString scriptPath = w.createKLayoutServerScript(cmdFile);

    QVERIFY2(!scriptPath.isEmpty(), "Script path is empty");
    QVERIFY2(QFileInfo(scriptPath).exists(), "Script file was not created");

    QFile f(scriptPath);
    QVERIFY2(f.open(QIODevice::ReadOnly | QIODevice::Text), "Failed to open generated script");

    const QString text = QString::fromUtf8(f.readAll());
    f.close();

    QVERIFY(text.contains("CMD_FILE = "));
    QVERIFY(text.contains("_handle"));
    QVERIFY(text.contains("_poll"));
    QVERIFY(text.contains("\"open\"") || text.contains("open"));
    QVERIFY(text.contains("\"select\"") || text.contains("select"));
    QVERIFY(text.contains("libman_klayout_server") ||
            text.contains("QTimer") ||
            text.contains("load_layout"));

    QFile::remove(scriptPath);
}
