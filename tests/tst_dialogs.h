#ifndef TST_DIALOGS_H
#define TST_DIALOGS_H

#include <QtTest/QtTest>

class DialogsTest : public QObject
{
    Q_OBJECT

private slots:
    void about_initializesUi();
    void about_licenseText_isNotEmpty();
    void about_okButton_closesDialog();

    void projectManager_initializesUi();
    void projectManager_stateFlagsChangeWindowTitle();
    void projectManager_cancel_closesDialog();
};

#endif // TST_DIALOGS_H
