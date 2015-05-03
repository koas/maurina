#include "aboutWindow.h"
#include "ui_aboutWindow.h"

aboutWindow::aboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutWindow)
{
    ui->setupUi(this);

    connect(ui->uiCloseButton,  SIGNAL(clicked()),
            this,               SLOT(close()));
}

aboutWindow::~aboutWindow()
{
    delete ui;
}
