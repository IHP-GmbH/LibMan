#include "about.h"
#include "ui_about.h"

/*!****************************************************************************************
 * \brief Constracts About dialog window to show information about LibMan version and
 * its license.
 * \param parent       Parent widget, by default is NULL.
 *****************************************************************************************/
About::About(QWidget *parent) :
    QDialog(parent),    
    m_ui(new Ui::About),
    m_version("1.0")
{
    m_ui->setupUi(this);

    QPixmap pix(":logo");
    m_ui->lblLogo->resize(200, 200);
    pix = pix.scaled(m_ui->lblLogo->size(), Qt::KeepAspectRatio);
    m_ui->lblLogo->setPixmap(pix);

    QPalette palette;
    palette.setColor(QPalette::Base, Qt::gray);
    m_ui->textAbout->setPalette(palette);

    QFont font = m_ui->textAbout->font();

    font.setBold(true);
    m_ui->textAbout->setFont(font);
    m_ui->textAbout->setAlignment(Qt::AlignCenter);
    m_ui->textAbout->append("LaibMan (" + m_version + ")\n");

    m_ui->textAbout->setAlignment(Qt::AlignLeft);
    font.setBold(false);
    m_ui->textAbout->setFont(font);
    m_ui->textAbout->append("By Anton Datsuk from IHP (datsuk@ihp-microlectronics.com)\n");
    m_ui->textAbout->append(getLicenseText().join("\n"));

    QTextCursor cursor = m_ui->textAbout->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_ui->textAbout->setTextCursor(cursor);

    m_ui->textAbout->setReadOnly(true);

    setWindowTitle("About LibMan");
}

/*!****************************************************************************************
 * \brief Destructs About obejct and clears up UI members.
 *****************************************************************************************/
About::~About()
{
    delete m_ui;
}

/*!****************************************************************************************
 * \brief returns Apache License text for LibMan.
 *****************************************************************************************/
QStringList About::getLicenseText() const
{
    QStringList text;

    text<<"Copyright 2023 IHP PDK Authors";
    text<<"";
    text<<"Licensed under the Apache License, Version 2.0 (the \"License\");";
    text<<"you may not use this file except in compliance with the License.";
    text<<"You may obtain a copy of the License at";
    text<<"";
    text<<"   https://www.apache.org/licenses/LICENSE-2.0";
    text<<"";
    text<<"Unless required by applicable law or agreed to in writing, software";
    text<<"distributed under the License is distributed on an \"AS IS\" BASIS,";
    text<<"WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.";
    text<<"See the License for the specific language governing permissions and";
    text<<"limitations under the License.";

    return text;
}

/*!****************************************************************************************
 * \brief closes About window.
 *****************************************************************************************/
void About::on_btnOk_clicked()
{
    close();
}
