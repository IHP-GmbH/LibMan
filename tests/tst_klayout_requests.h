#ifndef TST_KLAYOUT_REQUESTS_H
#define TST_KLAYOUT_REQUESTS_H

#include <QObject>

class KLayoutRequestsTest : public QObject
{
    Q_OBJECT

private slots:
    void sendKLayoutOpenRequest_emptyCommandFile_returnsFalse();
    void sendKLayoutOpenRequest_writesJsonCommand();
    void sendKLayoutSelectRequest_emptyCommandFile_returnsFalse();
    void sendKLayoutSelectRequest_writesJsonCommand();
    void createKLayoutServerScript_createsPythonFile();
};

#endif // TST_KLAYOUT_REQUESTS_H
