#include "ClientDialog.h"
#include "ui_ClientDialog.h"

ClientDialog::ClientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientDialog)
{
    ui->setupUi(this);
}

ClientDialog::~ClientDialog()
{
    delete ui;
}
