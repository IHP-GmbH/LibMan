#include "tst_dialogs.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "about.h"
#include "mainwindow.h"
#include "projectmanager.h"
#include "property.h"

namespace
{

/*!********************************************************************************************************
 * \brief Returns first child widget of given type.
 *********************************************************************************************************/
template <typename T>
static T *findChildOfType(QObject *obj)
{
    QList<T*> items = obj->findChildren<T*>();
    if(items.isEmpty()) {
        return nullptr;
    }

    return items.first();
}

} // namespace

/*!********************************************************************************************************
 * \brief Verifies About dialog initializes title, text widget and logo.
 *********************************************************************************************************/
void DialogsTest::about_initializesUi()
{
    About dlg(nullptr);
    dlg.show();

    QVERIFY(QTest::qWaitForWindowExposed(&dlg));

    QCOMPARE(dlg.windowTitle(), QString("About LibMan"));

    QTextEdit *textAbout = dlg.findChild<QTextEdit*>("textAbout");
    QVERIFY2(textAbout, "textAbout widget not found.");

    QVERIFY(textAbout->isReadOnly());

    const QString text = textAbout->toPlainText();
    QVERIFY2(text.contains("LaibMan"), "About text does not contain application name.");
    QVERIFY2(text.contains("Anton Datsuk"), "About text does not contain author name.");
    QVERIFY2(text.contains("Apache License"), "About text does not contain license heading.");

    QLabel *lblLogo = dlg.findChild<QLabel*>("lblLogo");
    QVERIFY2(lblLogo, "lblLogo widget not found.");
    QVERIFY2(!lblLogo->pixmap(Qt::ReturnByValue).isNull(), "Logo pixmap is not set.");
}

/*!********************************************************************************************************
 * \brief Verifies About license text provider returns expected content.
 *********************************************************************************************************/
void DialogsTest::about_licenseText_isNotEmpty()
{
    About dlg(nullptr);

    const QStringList license = dlg.getLicenseText();

    QVERIFY(!license.isEmpty());
    QVERIFY(license.join("\n").contains("Apache License"));
    QVERIFY(license.join("\n").contains("Copyright"));
}

/*!********************************************************************************************************
 * \brief Verifies About dialog closes on OK action.
 *********************************************************************************************************/
void DialogsTest::about_okButton_closesDialog()
{
    About dlg(nullptr);
    dlg.show();

    QVERIFY(QTest::qWaitForWindowExposed(&dlg));
    QVERIFY(dlg.isVisible());

    QVERIFY(QMetaObject::invokeMethod(&dlg, "on_btnOk_clicked", Qt::DirectConnection));

    QCoreApplication::processEvents();
    QVERIFY(!dlg.isVisible());
}

/*!********************************************************************************************************
 * \brief Verifies ProjectManager initializes property browser and dialog state.
 *********************************************************************************************************/
void DialogsTest::projectManager_initializesUi()
{
    Properties props;
    MainWindow mw(QString(), QString(), nullptr);
    ProjectManager dlg(&mw, &props);

    dlg.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dlg));

    QCOMPARE(dlg.windowTitle(), QString("Project Manager"));

    QPushButton *btnOk = dlg.findChild<QPushButton*>("btnOk");
    QVERIFY2(btnOk, "btnOk not found.");
    QVERIFY2(!btnOk->isEnabled(), "btnOk should be disabled initially.");

    QVERIFY2(dlg.findChild<QWidget*>() != nullptr, "Dialog should contain child widgets.");

    QList<QWidget*> widgets = dlg.findChildren<QWidget*>();
    QVERIFY(!widgets.isEmpty());
}

/*!********************************************************************************************************
 * \brief Verifies ProjectManager state change helpers update title and OK button.
 *********************************************************************************************************/
void DialogsTest::projectManager_stateFlagsChangeWindowTitle()
{
    Properties props;
    MainWindow mw(QString(), QString(), nullptr);
    ProjectManager dlg(&mw, &props);

    dlg.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dlg));

    QPushButton *btnOk = dlg.findChild<QPushButton*>("btnOk");
    QVERIFY2(btnOk, "btnOk not found.");

    QCOMPARE(dlg.windowTitle(), QString("Project Manager"));
    QVERIFY(!btnOk->isEnabled());

    dlg.setStateChanged();

    QVERIFY(dlg.windowTitle().contains("*"));
    QVERIFY(btnOk->isEnabled());

    dlg.setStateSaved();

    QVERIFY(!dlg.windowTitle().contains("*"));
    QVERIFY(!btnOk->isEnabled());
}

/*!********************************************************************************************************
 * \brief Verifies ProjectManager closes on Cancel action.
 *********************************************************************************************************/
void DialogsTest::projectManager_cancel_closesDialog()
{
    Properties props;
    MainWindow mw(QString(), QString(), nullptr);
    ProjectManager dlg(&mw, &props);

    dlg.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dlg));
    QVERIFY(dlg.isVisible());

    QVERIFY(QMetaObject::invokeMethod(&dlg, "on_btnCancel_clicked", Qt::DirectConnection));

    QCoreApplication::processEvents();
    QVERIFY(!dlg.isVisible());
}
