#include "newview.h"
#include "ui_newview.h"

#include "mainwindow.h"

/*!*********************************************************************************************************************
 * \brief Constracts a NewView dialog window to let user select which type of views to create.
 * \param parent         Parent widget, by default is NULL.
 * \param libName        Name of the library. Be default, it is empty.
 * \param cellName       Name of the cell. Be default, it is empty.
 **********************************************************************************************************************/
NewView::NewView(QWidget *parent, const QString &libName, const QString &cellName) :
    QDialog(parent),
    m_ui(new Ui::NewView)
{
    m_ui->setupUi(this);

    m_mw = static_cast<MainWindow *> (parent);

    m_ui->txtCell->setEnabled(false);
    m_ui->txtLibrary->setEnabled(false);

    m_ui->txtCell->setText(cellName);
    m_ui->txtLibrary->setText(libName);

    setCurrentViewType("gds");
}

/*!*********************************************************************************************************************
 * \brief Destructs NewView obejct and clears up UI members.
 **********************************************************************************************************************/
NewView::~NewView()
{
    delete m_ui;
}

/*!*********************************************************************************************************************
 * \brief Sets the provided type name to be the current one in the view type ComboBox.
 * \param type       Name of view type (ex., cdl, gds, spice, etc.).
 **********************************************************************************************************************/
void NewView::setCurrentViewType(const QString &type)
{
    for(int i = 0; i < m_ui->cbxView->count(); ++i) {
        if(m_ui->cbxView->itemText(i) == type) {
            m_ui->cbxView->setCurrentIndex(i);
            break;
        }
    }
}

/*!*********************************************************************************************************************
 * \brief Closes NewView window
 **********************************************************************************************************************/
void NewView::on_btnCancel_clicked()
{
    close();
}

/*!*********************************************************************************************************************
 * \brief Creates view based on user input and closes the NewView window.
 **********************************************************************************************************************/
void NewView::on_btnCreate_clicked()
{
    if(m_mw) {
        QString type = m_ui->cbxView->currentText();
        if(type == "cdl") {
            m_mw->addNewSchematicView();
        }
        else if(type == "gds") {
            m_mw->addNewLayoutView();
        }
        else if(type == "spice") {
            m_mw->addNewSpiceView();
        }
    }

    close();
}
