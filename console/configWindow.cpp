#include "configWindow.h"
#include "ui_configWindow.h"

configWindow::configWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::configWindow)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()),
            this,          SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()),
            this,          SLOT(reject()));
}

configWindow::~configWindow()
{
    delete ui;
}

void configWindow::setServerAdress(QHostAddress ip)
{
    ui->uiServerIp->setText(ip.toString());
}

void configWindow::setServerPort(quint16 port)
{
    ui->uiServerPort->setValue(port);
}

void configWindow::setStyles(QHash<QString, QString> styles)
{
    foreach(QString key, styles.keys())
    {
        QString value = styles[key];

        if (key == "time")
            ui->leTime->setText(value);
        else if (key == "pre")
            ui->lePre->setText(value);
        else if (key == "var")
            ui->leVar->setText(value);
        else if (key == "h1")
            ui->leH1->setText(value);
        else if (key == "h2")
            ui->leH2->setText(value);
        else if (key == "h3")
            ui->leH3->setText(value);
        else if (key == "h4")
            ui->leH4->setText(value);
        else if (key == "h5")
            ui->leH5->setText(value);
        else if (key == "h6")
            ui->leH6->setText(value);
    }
}

QHostAddress configWindow::getServerAddress()
{
    QHostAddress address(ui->uiServerIp->text());

    return address;
}

quint16 configWindow::getServerPort()
{
    return ui->uiServerPort->value();
}

QHash<QString, QString> configWindow::getStyles()
{
    QHash<QString, QString> styles;

    styles["time"] = ui->leTime->text();
    styles["pre"]  = ui->lePre->text();
    styles["var"]  = ui->leVar->text();
    styles["h1"]   = ui->leH1->text();
    styles["h2"]   = ui->leH2->text();
    styles["h3"]   = ui->leH3->text();
    styles["h4"]   = ui->leH4->text();
    styles["h5"]   = ui->leH5->text();
    styles["h6"]   = ui->leH6->text();

    return styles;
}
