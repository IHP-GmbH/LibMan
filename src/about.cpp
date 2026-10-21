#include "about.h"
#include "ui_about.h"

#include <QIcon>

/*!****************************************************************************************
 * \brief Constructs About dialog window to show information about DOOR version and
 * its license.
 * \param parent       Parent widget, by default is NULL.
 *****************************************************************************************/
About::About(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::About),
    m_version("1.0")
{
    m_ui->setupUi(this);
    setWindowTitle(tr("About DOOR"));
    setWindowIcon(QIcon(QStringLiteral(":appicon")));

    QPixmap pix(QStringLiteral(":logo"));
    // Fill dialog width (margins ≈ 16+16); keep aspect ratio so the lockup stays sharp.
    const int logoWidth = qMax(520, width() - 32);
    pix = pix.scaled(QSize(logoWidth, 280), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ui->lblLogo->setPixmap(pix);
    m_ui->lblLogo->setMinimumHeight(pix.height());
    m_ui->lblLogo->setMaximumHeight(pix.height() + 8);

    // Branding is already in the logo; keep text to version, credit, license.
    QString html;
    html += QStringLiteral("<p style='margin:0 0 8px 0; font-size:14px;'>"
                           "<b>DOOR</b> (%1)</p>")
                .arg(m_version);
    html += QStringLiteral(
        "<p style='margin:0 0 12px 0;'>"
        "By Anton Datsuk from IHP "
        "(<a href='mailto:datsuk@ihp-microlectronics.com'>datsuk@ihp-microlectronics.com</a>)"
        "</p>");
    html += QStringLiteral("<p style='margin:0; text-align:left;'>%1</p>")
                .arg(getLicenseText().join(QStringLiteral("<br/>")));

    m_ui->lblAbout->setTextFormat(Qt::RichText);
    m_ui->lblAbout->setText(html);
    m_ui->lblAbout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
}

/*!****************************************************************************************
 * \brief Destructs About object and clears up UI members.
 *****************************************************************************************/
About::~About()
{
    delete m_ui;
}

/*!****************************************************************************************
 * \brief returns Apache License text for DOOR.
 *****************************************************************************************/
QStringList About::getLicenseText() const
{
    QStringList text;

    text << QStringLiteral("Copyright 2023–2026 IHP PDK Authors");
    text << QString();
    text << QStringLiteral("Licensed under the Apache License, Version 2.0 (the \"License\");");
    text << QStringLiteral("you may not use this file except in compliance with the License.");
    text << QStringLiteral("You may obtain a copy of the License at");
    text << QString();
    text << QStringLiteral(
        "<a href='https://www.apache.org/licenses/LICENSE-2.0'>"
        "https://www.apache.org/licenses/LICENSE-2.0</a>");
    text << QString();
    text << QStringLiteral("Unless required by applicable law or agreed to in writing, software");
    text << QStringLiteral("distributed under the License is distributed on an \"AS IS\" BASIS,");
    text << QStringLiteral("WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.");
    text << QStringLiteral("See the License for the specific language governing permissions and");
    text << QStringLiteral("limitations under the License.");

    return text;
}

/*!****************************************************************************************
 * \brief closes About window.
 *****************************************************************************************/
void About::on_btnOk_clicked()
{
    close();
}
