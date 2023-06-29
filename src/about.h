#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

/*!****************************************************************************************
 * \brief The About class creates and shows a dialog window with an information about
 * LibMan.
 *****************************************************************************************/
class About : public QDialog
{
    Q_OBJECT
    
public:
    explicit About(QWidget *parent = 0);
    ~About();

private slots:
    void on_btnOk_clicked();

private:
    QStringList                 getLicenseText() const;
    
private:
    Ui::About                   *m_ui;      /*!< A pointer to acess About graphic items.*/
    QString                     m_version;  /*!< Keeps information about LibMan version.*/
};

#endif // ABOUT_H
