#ifndef NEWVIEW_H
#define NEWVIEW_H

#include <QDialog>

namespace Ui {
class NewView;
}

class MainWindow;

/*!*********************************************************************************************************************
 * \brief The NewView class creates a dialog box where user can select which view to be created.
 **********************************************************************************************************************/
class NewView : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewView(QWidget *parent = 0, const QString &libName = "", const QString &cellName = "");
    ~NewView();

private slots:
    void                        on_btnCancel_clicked();
    void                        on_btnCreate_clicked();

private:
    void                        setCurrentViewType(const QString &);
    
private:
    Ui::NewView                 *m_ui;  /*!< A pointer to access NewView graphic items.*/
    MainWindow                  *m_mw;  /*!< A pointer to MainWindow object.*/
};

#endif // NEWVIEW_H
